#ifndef FLOW_ACTOR_ALLOCATOR_H
#define FLOW_ACTOR_ALLOCATOR_H

#include <memory>
#include <cassert>

#include "flow_actor.h"
#include "../bessport/utils/simple_ring_buffer.h"

class flow_actor_allocator{

public:
  static void create(int max_actors){
    assert(ptr_==nullptr);
    ptr_.reset(new flow_actor_allocator(max_actors));
  }

  static flow_actor_allocator* get(){
    return ptr_.get();
  }

  ~flow_actor_allocator(){
    delete[] flow_actor_array_;
  }

  inline flow_actor* allocate(){
    return ring_buf_.pop();
  }

  inline bool deallocate(flow_actor* flow_actor_ptr){
    return ring_buf_.push(flow_actor_ptr);
  }

  inline size_t get_max_actor(){
    return max_actors_;
  }

private:
  static std::unique_ptr<flow_actor_allocator> ptr_;

  flow_actor_allocator(size_t max_actors) : max_actors_(max_actors), ring_buf_(max_actors) {
    flow_actor_array_ = new flow_actor[max_actors];
    for(size_t i=0; i<max_actors; i++){
      flow_actor_array_[i].set_id(i);
      ring_buf_.push(&flow_actor_array_[i]);
    }
  }

  size_t max_actors_;

  simple_ring_buffer<flow_actor> ring_buf_;

  flow_actor* flow_actor_array_;
};

#endif
