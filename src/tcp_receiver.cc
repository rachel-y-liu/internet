#include "tcp_receiver.hh"
#include "debug.hh"
#include "wrapping_integers.hh"
#include <algorithm>
#include <iostream>
#include <optional>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Set ISN if necessary
  if ( message.SYN ) {
    ISN = message.seqno;
  }
  if ( message.RST )
    reassembler_.reader().set_error();
  if ( !ISN )
    return;
  // Push any data to the reassembler
  uint64_t checkpoint = reassembler_.writer().bytes_pushed();
  uint64_t abs_seqno = message.seqno.unwrap( *ISN, checkpoint );
  uint64_t stream_index = message.SYN ? 0 : abs_seqno - 1;

  reassembler_.insert( stream_index, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage message;
  if ( ISN ) {
    uint64_t abs_ackno = reassembler_.writer().bytes_pushed() + 1;
    if ( reassembler_.writer().is_closed() )
      abs_ackno += 1;
    message.ackno = Wrap32::wrap( abs_ackno, *ISN ); // stream index -> abs seqno -> seqno
  }
  message.window_size = min( reassembler_.writer().available_capacity(), (uint64_t)UINT16_MAX );
  if ( reassembler_.reader().has_error() )
    message.RST = true;
  return message;
}
