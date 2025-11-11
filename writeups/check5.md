Checkpoint 5 Writeup
====================

My name: Rachel Liu

My SUNet ID: rliu25

I collaborated with: aribarb

I would like to thank/reward these classmates for their help:

This checkpoint took me about [6-7] hours to do. I [did] attend the lab session.

Program Structure and Design of the NetworkInterface [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]:

HOW I HANDLED TIME:
My code keeps track of a current_time_ state variable (of type size_t), which is updated according to tick(). Additionally, each time-sensitive entry (i.e. the ARP requests, the queued datagrams, and the ip-ether mappings) stores its own creation time. The creation time of each time-sensitive entry is compared to current_time_ to check whether it has expired; if unexpired, it remains unchanged. I considered the alternative of storing how much time had passed since each time-sensitive entry had been created (i.e. each entry has its own counter starting at 0ms). However, this method required updating the time passed for every single entry in the map if tick() were called, regardless of whether it had expired or not. Thus I opted to keep track of time with current_time_, which only requires one update of current_time_, reducing risk of bugs and unnecessary code complexity.

IP-ETHER MAPPINGS:
I stored IP-Ether mappings in an unordered map, where the key was the IP address, and the value was a custom made struct (ARPEntry) consisting of the ethernet address and the time at which this mapping was discovered. The map allowed me to quickly and conveniently search for IP addresses needed for the next hop. The custom struct allowed me to store and look up the map entry's creation time. Though this struct requires a bit of added complexity, it ultimately saves memory as opposed to the alternative, which is a separate map of IP addresses to time created.

DATAGRAMS QUEUED AND ARP REQUESTS:
datagrams_waiting_ is an unordered map which stores an IP address, with its corresponding queued datagrams. arp_requests_waiting_ is the complimentary unordered map which stores an IP address, with the time at which that ARP Request for that IP address was sent. I considered creating a struct with the queue of datagrams AND the time sent, but opted for a separate map to avoid such heavy nesting (it would involve a queue within a struct within a map), which requires more convoluted logic, higher risk of bugs, and higher implementation time.

The key functionality lies in being given an IP address and being able to extract relevant information (i.e. corresponding ethernet address + time created, corresponding ARP requests + time created, datagrams waiting). This is why it's so important that datagrams_waiting_, arp_requests_waiting_, and ip_ether_map_ all have ip address as the key. Unordered maps support O(1) average/O(n) worst case lookup, and O(n) iteration. Maps offer similar asymptotic performance, but unordered maps offer slightly faster performance due to better memory locality. 

Implementation Challenges:

I struggled with the conceptual difference between a arp request in waiting and a datagram in waiting; I thought each datagram entry needed a time stamp. 

Another challenge was learning how to utilize so many different structures (i.e. arp message, ethernet header, ethernet frame) and their required parameters, which required digging through many header files.

Remaining Bugs:
There remain some gaps in logic, such as resending ARP requests, which are beyond the scope of this project.


- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  How to erase items from a map?

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
