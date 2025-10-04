#include "socket.hh"

using namespace std;

int main()
{
  string d;

  d += 0b0100'0101;    // version and IHL
  d += string( 7, 0 ); // rest of first two lines

  d += 64;             // TTL
  d += 17;             // proto
  d += string( 6, 0 ); // rest of next two lines

  d += 10; // destination address
  d += 144;
  d += string( 1, 0 );
  d += 92u;

  d += 4; // source port
  d += 1;

  d += 4; // destination port (user ID)
  d += string( 1, 0 );

  string message = "shaylashaylashaylashayla";

  d += string( 1, 0 );
  d += char( message.length() + 8 );
  d += string( 2, 0 );
  d += message;

  RawSocket {}.send( d, Address { "10.144.0.92" } ); // the "1" needs to be chosen a little carefully -- the
                                                     // datagram needs to leave our computer on the right interface

  return 0;
}
