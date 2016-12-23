#ifndef NETWORK_FUNCTION_H
#define NETWORK_FUNCTION_H

#include "../../bessport/packet.h"
#include "../../bessport/utils/simple_ring_buffer.h"
#include "../../bessport/mem_alloc.h"

class network_function_base{
public:

  explicit network_function_base(size_t nf_state_num, size_t nf_state_size)
    : ring_buf_(nf_state_num), nf_state_size_(nf_state_size){
    array_ = reinterpret_cast<char*>(mem_alloc(nf_state_size*nf_state_num));
    for(size_t i=0; i<nf_state_num; i++){
      ring_buf_.push(array_+i*nf_state_size);
    }
  }

  virtual ~network_function_base(){}

  inline char* allocate(){
    return ring_buf_.pop();
  }

  inline bool deallocate(char* state_ptr){
    return ring_buf_.push(state_ptr);
  }

  virtual void nf_logic(bess::Packet* pkt, char* state_ptr) = 0;

  inline size_t get_nf_state_size(){
    return nf_state_size_;
  }

private:

  simple_ring_buffer<char> ring_buf_;
  size_t nf_state_size_;
  char* array_;
};

#endif
