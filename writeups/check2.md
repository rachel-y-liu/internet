Checkpoint 2 Writeup
====================

My name: Rachel Liu

My SUNet ID: rliu25

I collaborated with: climcaoco, aribarb

I would like to thank/reward these classmates for their help: climcaoco, aribarb

This lab took me about 8 hours to do. I did attend the lab session.

Describe Wrap32 and TCPReceiver structure and design. [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

WRAP32

The wrap function simply adds the low 32 bits of n (which is the absolute sequence number) to the ISN. The unwrap function computes the closest 64-bit candidate to the provided 32-bit seqno. I first computed the 32-bit remainder ((seqno - zeropoint) mod 2^32) on the same 2^32 'bucket' as the checkpoint. Due to the loss of information, however, I realized the best option could be in one 'bucket' above (+2^32) or below (-2^32) as well. Thus I compute all 3 options and choose the one numerically closest to the candidate. The asymptotic performance is quite fast, at O(1). 

TCP RECEIVER

TCPReceiver only keeps track of one state variable: the ISN. It's stored in an optional Wrap32 variable. This achieves two main functions. Firstly, it represents whether or not the ISN has been received yet, which is only possible because the optional value can represent a null value. Secondly, it sets the zero_point when ISN is received, which remains constant and is passed in to 'wrap' and 'unwrap' throughout the remainder of the TCP connection. I considered other ways to track the presence or absence of the SYN flag, such is in a separate boolean variable, but it would've been more complex and there's the risk of accessing a garbage SYN value.

All other receiver information (i.e. window size, FIN flag, error, ACK value, etc.) stem from the bytestream's reader/writer interfaces. This design saves both implementation time by avoiding duplicate coding work, improves readability, and decreases risk of bugs by relying on already-tested interfaces.

Another choice was what to use as the 'checkpoint' value for 'unwrap.' This required reframing 'checkpoint', and thinking about what this value represented in the big picture of the reassembler/bytestream. Checkpoint is a guiding number for the next needed index number -- represented in the Reassembler by bytes_pushed. I briefly considered using previously received seqnos as the checkpoint; however, this would've included adding another state variable and is unstable since it changes as segments are received (adding at least ~5 additional lines of code and 30 minutes of implementation time), whereas bytes_pushed is simple and stable.

On output, the 'send' function only forms an ACK when the SYN flag is known. Sequence numbers are calculated, with an extra seqno added for SYN and FIN flags, if present.


Implementation Challenges:

One implementation challenge was figuring out how the 'receive' and 'send' functions of the TCPReceiver could communicate with each other. If a FIN flag was received, how could the 'send' function know that? And same with the error flag? I originally added many state variables, but I realized that any changes from the 'receive' function are reflected in the internal state of the Reassembler and Bytestream. In this example, I realized I could check if the writer stream is closed in response to a FIN flag.

It was quite tempting to reimplement functionality already covered by the Reassembler and Bytestream. It's difficult to think big-picture while coding, and linking all the moving parts is often more difficult than simply recreating it. One fradmework that guided me is that before adding any state variable, I required myself to dig through the header files of previous code to ensure I wasn't redundant. 

Remaining Bugs:
My TCPReceiver performance is likely slowed by my poor implementation of Reassembler.

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
