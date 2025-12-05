#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
// the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
// which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.

// adds a route to the routing table.
// add data structure as private member in the Router class to store this info.
// this method needs to save the route for later use.

// PARTS OF A ROUTE:
// "match action" rule - tells router, if a datagram is headed for a particular network
// and if route is chosen as most specific route, then router should forward datagram to
// particular next hop on a particular interface

// MATCH: is it headed for this network?
// route_prefix & prefix_length specify range of IP addys, a network. that might include dest.
// route_prefix - 32 bit numeric IP address
// prefix_length is # between 0 and 32, inclusive, tells router how many bits are significant
// ACTION: what to do if the route matches, and is chosen

// interface_num: index of the router’s NetworkInterface that should use to send the datagram to the
// next hop. You can access this interface with the interface(interface num)
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  Route new_entry = { route_prefix, prefix_length, next_hop, interface_num };
  routes_.push_back( new_entry );
}

void Router::route()
{
  for ( const auto &cur_interface: interfaces_) {
    while ( !cur_interface->datagrams_received().empty() ) {
      auto& queue = cur_interface->datagrams_received();
      InternetDatagram d = queue.front();
      queue.pop();

      uint32_t dest = d.header.dst;
      
      int best_index = -1;
      int max_length = -1;
      
      for ( size_t i = 0; i < routes_.size(); i++ ) {
        const Route& route = routes_[i];
        uint32_t mask = ( route.prefix_length == 0 ) ? 0 : ( ~uint32_t { 0 } << ( 32 - route.prefix_length ) );          // mask the cur dst
        if ( ((dest & mask) == (route.route_prefix & mask)) && route.prefix_length > max_length ) {
          max_length = route.prefix_length; // update max length
          best_index = i;
        }
      }
      if ( best_index == -1 ) {
        continue;
      }

      if ( d.header.ttl <= 1 ) {
        continue; // drop it!
      }
      d.header.ttl--;
      d.header.compute_checksum();

      const Route& best_route = routes_[best_index];
      Address next_hop_addr
        = best_route.next_hop.has_value() ? best_route.next_hop.value() : Address::from_ipv4_numeric( dest );
        
      interface( best_route.interface_num )->send_datagram( d, next_hop_addr );
    }
  }
}
