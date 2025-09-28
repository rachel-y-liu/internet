Checkpoint 0 Writeup
====================

My name: Rachel Liu

My SUNet ID: rliu25

I collaborated with: climcaoco, aribarb

I would like to credit/thank these classmates for their help: climcaoco, aribarb

This lab took me about 4.5 hours to do. I did attend the lab session.

My secret code from section 2.1 was: 445294

I was surprised by or edified to learn that: scamming people via email is so easy! I can't believe that all email messages are done via Simple Mail Transfer Protocol, and that just by connecting to the email server, we can send emails to anyone.

Describe ByteStream implementation. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

I used a string as a buffer to store bytes that were pushed and needed to be popped. Using the string as a data structure allowed me to easily append or remove from either the beginning or the end of the string. In addition to the buffer, I kept track of just a few internal states (i.e. cumulative bytes pushed, total bytes popped, and a boolean of whether the stream was closed). 

I considered using a queue, because its FIFO structure supports the need to add bytes to the back and remove them from the front. A queue was an appealing data structure because it takes O(1) time to remove a byte from the front, whereas for a string, it takes O(n) time for all remaining characters to shift up by 1 position. However, there was no easy way to peek at more than just the first byte with a queue, and this is ultimately why I went with using a string.

The advantages of a string are that it is easy to use and very readable, thus reducing implementation time and the potential for bugs. The main weakness is the slow performance time (O(n)) of removing characters in the front, as mentioned earlier.


Implementation Challenges:
The main challenge was choosing an appropriate data structure, especially because I was rusty on the advantages, disadvantages, functionality, and performance of each of them. I took time to do research on each before going with the string.

Remaining Bugs:
None

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: [provide Pull Request URL]

