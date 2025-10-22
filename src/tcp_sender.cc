#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

#include <algorithm>
#include <queue>

using namespace std;

// How many sequence numbers are outstanding?
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  uint64_t total_seqnos = 0;
  std::queue<TCPSenderMessage> temp = outstanding_;
  while (!temp.empty()) {
    total_seqnos += temp.front().sequence_length();
    temp.pop();
  }
  return total_seqnos;
}

// How many consecutive retransmissions have happened?
uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  bool is_last_msg = false;

  while(!is_last_msg){
    TCPSenderMessage msg;
    msg.seqno = Wrap32::wrap(abs_seqno_, isn_);

    //Calculate if window size is full. Also, treat '0' window size as equal to 1.
    uint64_t remaining_window_size = 0;
    if (window_size_==0 && sequence_numbers_in_flight()==0) remaining_window_size = 1;
    else if (sequence_numbers_in_flight() < window_size_) remaining_window_size = window_size_ - sequence_numbers_in_flight();

    //Edge case: don't send anything if window size is full
    if (remaining_window_size == 0) return;

    //Edge case: Only send FIN once.
    if (FINsent) return;

    if (abs_seqno_ == 0) msg.SYN = true;
    remaining_window_size -= msg.SYN; //SYN flag takes up 1 seqno if present

    if ( input_.reader().has_error() ) msg.RST = true;

    //See if we can send over bytes. Check for available bytes in straem, accounting for SYN and FIN flags
    uint64_t num_bytes_to_read = min({  input_.reader().bytes_buffered(),
                                        remaining_window_size
                                        //max<uint64_t>(1, remaining_window_size)
                                      });
    uint64_t len = num_bytes_to_read;
    if (num_bytes_to_read < TCPConfig::MAX_PAYLOAD_SIZE) {
      is_last_msg = true;
    } else {
      len = TCPConfig::MAX_PAYLOAD_SIZE;
    }

   read(input_.reader(), len, msg.payload);
   remaining_window_size = max(0UL, remaining_window_size - len);

    //if stream is finished AND theres more room in window size -- add FIN
    if (input_.reader().is_finished() && remaining_window_size > 0) {
      msg.FIN = true;
      FINsent = true;
    }

    //Sanity check: do not send message if there is nothing to send
    if (msg.sequence_length() == 0) return; 

    transmit(msg);
    outstanding_.push(msg); //all msgs are outstanding when just transmitted
    abs_seqno_ += msg.sequence_length();
    if (!timer_on_) {
      timer_on_ = true;
      timer_time_ = 0;
    }
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap(abs_seqno_, isn_);
  if ( input_.reader().has_error() ) msg.RST = true;
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if (msg.RST) input_.reader().set_error();
  window_size_ = msg.window_size;
  uint64_t msg_abs_ackno = msg.ackno->unwrap(isn_, input_.reader().bytes_popped());

  //Edge case: Ackno is in range of seqno's we've sent
  if (msg_abs_ackno > abs_ackno_ + sequence_numbers_in_flight()) return;

  if (msg_abs_ackno > abs_ackno_){ //if new data was acked (yay!)
    RTO_ = initial_RTO_ms_;
    timer_time_ = 0;
    abs_ackno_ = msg_abs_ackno; //update abs_ackno_, which is furthest ackno seen so far
    consecutive_retransmissions_ = 0;

    //remove acked segments from outstanding_
    while (true && sequence_numbers_in_flight() > 0) {
      TCPSenderMessage outstanding_msg = outstanding_.front();
      uint64_t outstanding_msg_start = outstanding_msg.seqno.unwrap(isn_, input_.reader().bytes_popped()); //abs seqno
      uint64_t outstanding_msg_end = outstanding_msg_start + outstanding_msg.sequence_length();
      if (msg_abs_ackno >= outstanding_msg_end) {
        outstanding_.pop();
      } else {
        break;
      }
    }
  }

  //When all outstanding data has been acknowledged, stop the retransmission timer 
  if (sequence_numbers_in_flight() == 0) {
    timer_on_ = false;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if (!timer_on_) return;

  timer_time_ += ms_since_last_tick;
  //if retransmission timer has expired, retransmit earliest unack'ed segment (lowest seqno)
  if (timer_time_ >= RTO_) {
    transmit(outstanding_.front());
    if (window_size_ > 0) {
      consecutive_retransmissions_ += 1;
      RTO_ *= 2;
    }
    timer_time_ = 0; //reset timer
  }
}
