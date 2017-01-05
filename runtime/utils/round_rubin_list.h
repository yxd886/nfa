#ifndef ROUND_RUBIN_LIST_H
#define ROUND_RUBIN_LIST_H

#include "cdlist.h"

template<class T>
class round_rubin_list{
public:
  round_rubin_list(){
    cdlist_head_init(&rr_list_head_);
    cnt_ = 0;
  }

  inline void add_to_tail(T* obj_ptr){
    cdlist_add_tail(&rr_list_head_, reinterpret_cast<struct cdlist_item*>(obj_ptr));
    cnt_ += 1;
  }

  inline T* peek_head(){
    struct cdlist_item* item = cdlist_peek_first_item(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline T* peek_tail(){
    struct cdlist_item* item = cdlist_peek_last_item(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline T* pop_head(){
    struct cdlist_item* item = cdlist_pop_head(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      cnt_ -= 1;
      return reinterpret_cast<T*>(item);
    }
  }

  inline T* rotate(){
    struct cdlist_item* item = cdlist_rotate_left(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline struct cdlist_head* get_list_head(){
    return &rr_list_head_;
  }

  inline void list_item_delete(struct cdlist_item* list_item){
    cdlist_del(list_item);
    cnt_ -= 1;
  }

  inline uint64_t size(){
    return cnt_;
  }

private:
  struct cdlist_head rr_list_head_;
  uint64_t cnt_;

  static_assert(std::is_pod<T>::value, "The type argument passed to round_rubin_list is not POD");
};

#endif
