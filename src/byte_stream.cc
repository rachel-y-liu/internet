#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// Push data to stream, but only as much as available capacity allows.
void Writer::push( string data )
{
  while ( buffer.length() < capacity_ && !data.empty() ) {
    buffer += data[0];
    data.erase( 0, 1 );
    total_pushed++;
  }
}

// Signal that the stream has reached its ending. Nothing more will be written.
void Writer::close()
{
  stream_closed = true;
}

// Has the stream been closed?
bool Writer::is_closed() const
{
  return stream_closed;
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer.length();
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const
{
  return total_pushed;
}

// Peek at the next bytes in the buffer -- ideally as many as possible.
// It's not required to return a string_view of the *whole* buffer, but
// if the peeked string_view is only one byte at a time, it will probably force
// the caller to do a lot of extra work.
string_view Reader::peek() const
{
  return buffer;
}

// Remove `len` bytes from the buffer.
void Reader::pop( uint64_t len )
{
  buffer.erase( 0, len );
  total_popped += len;
}

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const
{
  return stream_closed && total_pushed == total_popped;
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const
{
  return total_pushed - total_popped;
}

// Total number of bytes cumulatively popped from stream
uint64_t Reader::bytes_popped() const
{
  return total_popped;
}
