#include "reassembler.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  
  uint64_t last_index_stored = first_index + data.length() - 1; //the last index to store from data

  //where the stream ends
  if (is_last_substring){
    finish_index = last_index_stored + 1;
  }

  //where capacity ends
  uint64_t first_unacceptable_index = output_.writer().bytes_pushed() + output_.writer().available_capacity();

  //STORE UNASSEMBLED DATA
  uint64_t i = first_index;
  for (char c : data) { //extract each individual byte
    if (i < first_unacceptable_index) { //OFF BY 1 ERROR?
      unassembled[i] = c;
      //cout << "Added " << c << " at index " << i << endl;
    } else {
      break;
    }
    i++;
  }

  //IF BLOCK BECOMES CONTIGUOUS, PUSH TO STREAM
  uint64_t index_needed = output_.writer().bytes_pushed();
  string data_to_push = "";
  while (unassembled.contains(index_needed)) {
    data_to_push += unassembled[index_needed];
    //cout << data_to_push << endl;
    //cout << index_needed << endl;
    index_needed++;
  }
  output_.writer().push(data_to_push);

  if (index_needed == finish_index) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  return unassembled.size() - output_.writer().bytes_pushed();
}
