#ifndef LOCAL_MESSAGE_H
#define LOCAL_MESSAGE_H

struct local_message_base{
};

template<int>
struct local_message_derived : local_message_base{
public:
  static local_message_derived value;
};

#define local_message(actor_msg_enum, msg) local_message_derived<static_cast<int>(actor_msg_enum::msg)>

#endif
