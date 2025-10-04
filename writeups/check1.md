Checkpoint 1 Writeup
====================

My name: Rachel Liu

My SUNet ID: rliu25

I collaborated with: climcaoco, aribarb

I would like to thank/reward these classmates for their help: fxchang, climcaoco

This lab took me about [8] hours to do. I did attend the lab session.

I was surprised by or edified to learn that: the abstraction for UDP is such a simple add-on to the existing IP abstraction. Writing out the code to send IPs and UDPs myself truly solidified my understanding of the simplicity of these headers!

Report from the hands-on component of the lab checkpoint: [include
information from 2.1(4), and report on your experience in 2.2]

2.1(4)
a. Average round trip delay: 24.122 ms
b. Delivery rate: 99.94%
   Loss rate: 0.058%
c. No, I did not see DUP.
e. Yes! I see identification, protocol, TTL, checksum, etc. in all their proper homes.
f. Carmel and I compared the same datagram with the same identification number. All fields were the same except two. Firstly, my TTL value was 64, and Carmel's was 63, because it took one router hop for her to receive this datagram. Secondly, my checksum value was a3 and Carmel's was a4. The TTL change meant the header of the datasum changed, and the checksum value changes to reflect that. 


Describe Reassembler structure and design. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

ALTERNATIVE DESIGNS
My first idea was to use a Vector of strings, which I quickly realized would not work because it is not idempotent, and there is no intuitive way to resolve overlapping strings. My second idea was just to use a string, because it is idempotent and can handle overlapping segments; however, a string is a contiguous sequence of elements and cannot have 'empty slots' between certain elements. Therefore there was no easy way to represent empty, unassembled segments of the bytestream.

MY DATA STRUCTURE - AN UNORDERED MAP!
The data structure I chose to use was an unordered map with keys of integers (representing the index of the byte) and values of chars (representing the byte contents). This allows flexible storage of existing data while allowing gaps, and insertion of existing elements overlaps the previous values, making it idempotent. These inherent map properties decrease the risk of bugs and optimize implementation time and difficulty -- it took me ~3-4 hours to draft my initial implementation, then ~2 hours to debug. It is simple and easy to read (consisting of ~40 lines of code), as it intuitively stores each byte in accordance with its index. 

TIME/SPACE COMPLEXITY
I chose to use an unordered map instead of a map, because it stores keys using a hash table and therefore has faster performance for basic functions. For instance, for insert, find, and erase, the time complexity for map is O(log n), wherease for unordered map the time complexity is O(1). Therefore the time complexity of my reassembler is O(N). One potential downside is the amount of memory required to store a key + value + hash value for each byte. If a string or other form of contiguous storage could have been implemented, it would take up less memory. However, to me, the tradeoff of ease of implementation was worth it.


Implementation Challenges:
One implementation challenge was understanding how the reassembler fit into the bytestream that we had built in check0. Another implementation challenge was figuring out how to handle the last substring, but watching Keith's lecture revealed the elegant solution of assigning the end of the bytestream to an index.

The biggest challenges, as mentioned above, was avoiding duplicating or skipping bytes, which I handled by using an idempotent data structure. Another one was pushing valid bytes to the bytestream, and not pushing until data was contiguous -- this involved repeatedly pushing from the first needed index from the map until a gap was reached.

My last challenge was facing a time out error, specifically on Test #15. I finally overcame this by directly pushing each valid byte to the bytestream, instead of first inserting the byte into a cumulative string. 


Remaining Bugs:
None, hopefully.

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
