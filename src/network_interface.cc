#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( InternetDatagram dgram, const Address& next_hop )
{
  EthernetHeader header;
  header.src = ethernet_address_;
  EthernetFrame frame;

  // If the destination Ethernet address is already known, send it right away.
  if ( ip_ether_map_.contains( next_hop.ipv4_numeric() ) ) {
    header.dst = ip_ether_map_[next_hop.ipv4_numeric()].ether_addr;
    header.type = EthernetHeader::TYPE_IPv4;
    frame.payload = serialize( dgram );
    frame.header = header;
    transmit( frame );
  } else { // If destination ethernet address is unkown...
    // if datagram is already in arp_requests_waiting_, that means we've sent an ARP Request for that IP addr
    // but, we should still queue that datagram
    if ( !arp_requests_waiting_.contains( next_hop.ipv4_numeric() ) ) {
      // If ARP Request for that IP address has not been sent in the past 5 seconds, broadcast ARP
      debug( "destination ethernet address is unknown, arp request has not been sent recently, broadcast arp" );
      ARPMessage request;
      request.opcode = ARPMessage::OPCODE_REQUEST;
      request.sender_ethernet_address = ethernet_address_;
      request.sender_ip_address = ip_address_.ipv4_numeric();
      request.target_ip_address = next_hop.ipv4_numeric();
      frame.payload = serialize( request );
      header.dst = ETHERNET_BROADCAST;
      header.type = EthernetHeader::TYPE_ARP;
      frame.header = header;
      transmit( frame );

      // Initialize datagrams queue for this IP addr
      datagrams_waiting_[next_hop.ipv4_numeric()] = {};
      // Log this ARP Request & time at which it was sent
      arp_requests_waiting_[next_hop.ipv4_numeric()] = current_time_;
    }

    // Queue that datagram regardless
    datagrams_waiting_[next_hop.ipv4_numeric()].push( dgram );
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST )
    return; // how to do broadcast addr
  // If ipv4 received, parse the payload as an internet datagram and push to datagrams_received_ queue
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram internet_datagram;
    if ( parse( internet_datagram, frame.payload ) ) {
      datagrams_received_.push( internet_datagram );
    }
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) { // if arp received
    // parse the payload as arp message
    ARPMessage arp_message;
    if ( parse( arp_message, frame.payload ) ) {
      // Store the IP-Ether mapping
      ARPEntry entry = { arp_message.sender_ethernet_address, current_time_ };
      ip_ether_map_[arp_message.sender_ip_address] = entry;

      // If it's an ARP request asking for our IP address...
      if ( arp_message.opcode == ARPMessage::OPCODE_REQUEST
           && arp_message.target_ip_address == ip_address_.ipv4_numeric() ) {
        debug( "It's an arp request asking for our IP address!" );
        // Send an arp reply
        ARPMessage reply;
        reply.opcode = ARPMessage::OPCODE_REPLY;
        reply.sender_ethernet_address = ethernet_address_;
        reply.sender_ip_address = ip_address_.ipv4_numeric();
        reply.target_ethernet_address = arp_message.sender_ethernet_address;
        reply.target_ip_address = arp_message.sender_ip_address;

        // construct a reply, serialize it, put it in a frame, and transmit
        EthernetHeader header;
        header.src = ethernet_address_;
        header.dst = arp_message.sender_ethernet_address;
        header.type = EthernetHeader::TYPE_ARP;
        EthernetFrame reply_frame;
        reply_frame.header = header;
        reply_frame.payload = serialize( reply );
        transmit( reply_frame );
        debug( "Arp reply sent!" );
      }

      // Check datagrams_waiting queue to send newly matched datagrams
      uint32_t this_ip = arp_message.sender_ip_address;
      if ( datagrams_waiting_.contains( this_ip ) ) {
        while ( !datagrams_waiting_[this_ip].empty() ) { // while queue of datagrams is not empty
          EthernetHeader header;

          header.src = ethernet_address_;
          header.dst = arp_message.sender_ethernet_address;
          header.type = EthernetHeader::TYPE_IPv4;
          EthernetFrame f;
          f.payload = serialize( datagrams_waiting_[this_ip].front() );
          f.header = header;
          transmit( f );
          datagrams_waiting_[this_ip].pop();
        }
        datagrams_waiting_.erase( this_ip );
      }
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  current_time_ += ms_since_last_tick;
  for ( auto i = ip_ether_map_.begin(); i != ip_ether_map_.end(); ) {
    if ( current_time_ - i->second.time_created > 30000 ) { // expire after 30s
      i = ip_ether_map_.erase( i );
    } else {
      ++i;
    }
  }
  for ( auto i = arp_requests_waiting_.begin(); i != arp_requests_waiting_.end(); ) {
    if ( current_time_ - i->second > 5000 ) { // expire after 30s
      if ( datagrams_waiting_.contains( i->first ) )
        datagrams_waiting_.erase( i->first ); // erase corresponding datagram queue for this IP as well
      i = arp_requests_waiting_.erase( i );
    } else {
      ++i;
    }
  }
}
