#include "reassembler.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  
  uint64_t last_index_stored = first_index + data.length() - 1; //the last index to store from data
  uint64_t index_needed = output_.writer().bytes_pushed();

  //where the stream ends
  if (is_last_substring){
    end_of_stream = last_index_stored + 1;
  }

  //where capacity ends
  uint64_t first_unacceptable_index = index_needed + output_.writer().available_capacity();

  //SECTION 1 : STORE UNASSEMBLED DATA
  uint64_t i = first_index;
  for (char c : data) { //extract each individual byte 
    if (i >= first_unacceptable_index) {
        break;
      } else if (i < index_needed) {
        i++;
        continue;
      }
      unassembled[i] = c;
      i++;
  }

  //SECTION 2 : IF BLOCK BECOMES CONTIGUOUS, PUSH TO STREAM
  string data_to_push = "";
  while (unassembled.find(index_needed) != unassembled.end()) {
    output_.writer().push(string(1, unassembled[index_needed]));
    unassembled.erase(index_needed);
    index_needed++;
  }

  
  if (index_needed == end_of_stream) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  return unassembled.size();
}
