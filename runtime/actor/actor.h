// The default actor definition. This actor implementation is unique, in that:

// 1. Since nfactor runs in a single thread, the message passing procedure of
//    local actor communication is replaced by a direct function call to the
//    message handler of the receiver actor.

// 2. For remote actor communication, the message passing is replaced with a
//    packet based message passing, where messages are encoded as raw network
//    packets and passed directly in the network.

// 3. There is no dedicated scheduler implementation. The actors are scheduled
//    in the following modules:
//    3.1 The module that handles collective scheduling of all data plane
//        actors. When the module polls the input packets from the port, this
//        module treats each received packet as a message and directly calls the
//        packet message handler of the corresponding data plane actor.
//    3.2 The module that handles reliable message passing. This module polls
//        control port for control plane message packets and reasamble the
//        message. When a message is successfully reassambled, this module calls
//        the corresponding message handler of the flow actors.
//    3.3 A timer module. This module mains all the timers for the system. When
//        a timer is expired, the time module calls the timer handler of the
//        corresponding actor.
//    3.4 A module that polls messages from the shared ring with the
//        RPC worker thread. When the module gets a message, it calls the
//        message handler of the coordinator to handle the messages. 


