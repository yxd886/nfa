#ifndef RUNTIME_CONFIG_ALLOCATOR_H
#define RUNTIME_CONFIG_ALLOCATOR_H

#include <memory>
#include <cassert>

#include "ring_msg.h"
#include "../bessport/utils/simple_ring_buffer.h"

class runtime_config_allocator{

public:
  static_assert(std::is_pod<runtime_config>::value, "runtime_config is not a pod");

  static void create(int max_rt_configs){
    assert(ptr_==nullptr);
    ptr_.reset(new runtime_config_allocator(max_rt_configs));
  }

  static runtime_config_allocator* get(){
    return ptr_.get();
  }

  ~runtime_config_allocator(){
    delete[] rt_config_array_;
  }

  inline runtime_config* allocate(){
    return ring_buf_.pop();
  }

  inline bool deallocate(runtime_config* runtime_config_ptr){
    return ring_buf_.push(runtime_config_ptr);
  }

  inline size_t get_max_rt_configs(){
    return max_rt_configs_;
  }

private:
  static std::unique_ptr<runtime_config_allocator> ptr_;

  runtime_config_allocator(size_t max_rt_configs) : max_rt_configs_(max_rt_configs), ring_buf_(max_rt_configs) {
    rt_config_array_ = static_cast<runtime_config*>(mem_alloc(sizeof(runtime_config)*max_rt_configs));
    for(size_t i=0; i<max_rt_configs; i++){
      ring_buf_.push(&rt_config_array_[i]);
    }
  }

  size_t max_rt_configs_;

  simple_ring_buffer<runtime_config> ring_buf_;

  runtime_config* rt_config_array_;
};

#endif
