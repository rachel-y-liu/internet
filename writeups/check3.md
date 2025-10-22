Checkpoint 3 Writeup
====================

My name: Rachel Liu

My SUNet ID: rliu25

I collaborated with: climcaoco, aribarb

I would like to thank/reward these classmates for their help: climcaoco, aribarb

This checkpoint took me about 9 hours to do. I did attend the lab session.

Program Structure and Design of the TCPSender [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]:

This TCPSender implementation keeps sending TCPSender messages as long as there is available window size and bytes to read. Within the push() function, there is an internal tracker (remaining_window_size) which serves 3 main functions: (1)assess whether or not the window is full, (2)if the window is not full, calculate how many bytes to read from the stream (accounting for SYN+ and FIN+), and (3)if the window size is 0, pretend the window size is 1 without storing it internally. This design intentionally separates the objective window_size_ from the available window size that can actually be pushed to. This decreases risk of bugs by preventing any inadvertent changes to the window_size_ received from the TCPReceiverMessage; in fact, upon reception, window_size_ is set once and not touched again.

I used a queue to store outstanding bytes. When deciding on a data structure, three aspects were considered -- how easy was it to (1) Add elements, kept in order, (2) Remove the first element added, and (3) Access each element of the queue to count sequence numbers in flight. The queue is efficient in pushing to the back (O(1)) and popping/peeking from the front (O(1)). The downside is that iterating through the queue to count sequence_numbers_in_flight is costly in memory and in performance -- it's not possible to iterate without altering the original queue, so I made a copy of it and popped each element, which takes O(n). I considered using Vector<TCPSenderMessage>, which also takes O(n) time to iterate through each element, but achieves better performance because data is stored contiguously. However, this small performance benefit was not worth the difficulties of adding elements and removing from the front. Without the option of popping from the front, I'd have to keep track of a head index. Thus I opted for a Queue due to its FIFO structure.

If I had more time, I would consider implementing a bytes_in_flight counter, which tracks the bytes sent out minus the bytes received. However, I chose not to due to complexity and time restraints. It would've taken definitely 30 additional minutes to code and introduced risk to bugs because additional and subtraction from the variable are handled manually.

I also tracked abs_seqno(the next seqno that has not yet been popped from the bytestream) and abs_ackno (the last seqno that has been acknowledged). I could also have tracked stream index, but it would've required an additional state variable to track SYN and FIN flags, which would've added unnecessary complexity.


Taking advantage of a queue's FIFO structure makes removing the oldest unacknowledged element simple.

Implementation Challenges:
Handling window size was the biggest implementation challenge for me. There were many moving parts -- accounting for seqnos for the SYN and FIN flags, the exception of sending 1 byte when window_size_ was 0, and accounting for bytes in flight to not overfill the window. The difference between the window size and the window availability/fullness was easy to understand in theory, but difficult to implement in practice. All of these factors were complicated even further by the cap to the payload size, which required the sending of multiple consecutive messages. This took ~3-4 hours of trial and error to get right. I debugged my code my printing out debug statements that reveal the behavior of my elements before each major addition to the TCPSendermsg -- for example I printed out remaining_window_size before creating the message, after addiing SYN, after adding payload, after adding FIN, etc. 

Remaining Bugs:
I do not think there are remaining bugs. However, if there were more time, I would more robustly test the validity of the checkpoints that I am using for 'unwrap'. Also I would minimize use of sequence_numbers_in_flight() to minimize these O(n) scans. There could be bugs if ackno or other field within the TCPReceiverMessage are unexpectedly optional/empty.

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [submit as GitHub PR
  and include URL here]
