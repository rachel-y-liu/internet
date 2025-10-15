#include "wrapping_integers.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32{zero_point + n % (1ULL << 32) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  //basically checkpoint is like the ballpark range. a flag if you will.
  //out of all possible options, choose the one numerically closest to checkpoint
  //sequence number = this?

  uint32_t remainder = this->raw_value_ - zero_point.raw_value_;
  uint64_t med = checkpoint / (1ULL << 32) * (1ULL << 32) + remainder;
  uint64_t low = med - (1ULL << 32); //low option
  uint64_t high = med + (1ULL << 32); //high option

  uint64_t diffmed = (med > checkpoint) ? (med - checkpoint) : (checkpoint - med);
  uint64_t difflow = (low > checkpoint) ? (low - checkpoint) : (checkpoint - low);
  uint64_t diffhigh = (high > checkpoint) ? (high - checkpoint) : (checkpoint - high);

  //pick the best option
  uint64_t unwrapped = med;
  if (diffhigh < diffmed) unwrapped = high; //if option2 is better, choose option2
  if (med >= (1ULL << 32) && difflow < diffmed && difflow < diffhigh) unwrapped = low; //if option1 exists and is better, choose option1

  return unwrapped;
}
