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
  uint64_t option0 = checkpoint / (1ULL << 32) * (1ULL << 32) + remainder;
  uint64_t option1 = option0 - (1ULL << 32); //low option
  uint64_t option2 = option0 + (1ULL << 32); //high option

  uint64_t diff0 = (option0 > checkpoint) ? (option0 - checkpoint) : (checkpoint - option0);
  uint64_t diff1 = (option1 > checkpoint) ? (option1 - checkpoint) : (checkpoint - option1);
  uint64_t diff2 = (option2 > checkpoint) ? (option2 - checkpoint) : (checkpoint - option2);

  //pick the best option
  uint64_t unwrapped = option0;
  if (diff2 < diff0) unwrapped = option2; //if option2 is better, choose option2
  if (option0 >= (1ULL << 32) && diff1 < diff0 && diff1 < diff2) unwrapped = option1; //if option1 exists and is better, choose option1

  return unwrapped;
}
