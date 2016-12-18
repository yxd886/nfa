// This module is where we poll input/output port and schedule
// execution_context actors.

#ifndef EC_SCHEDULER_H
#define EC_SCHEDULER_H

#include <string>

#include <rte_config.h>
#include <rte_hash_crc.h>

#include "../port/sn_port.h"
#include "../bessport/module.h"
#include "../bessport/module_msg.pb.h"
#include "../bessport/message.h"
#include "../bessport/utils/htable.h"
#include "../actor/flow_actor_allocator.h"

using std::string;

inline int flow_keycmp(const void *key, const void *key_stored, size_t key_len) {
  const uint64_t *a = ((flow_key_t *)key)->field;
  const uint64_t *b = ((flow_key_t *)key_stored)->field;

  switch (key_len >> 3) {
    default:
      promise_unreachable();
    case 8:
      if (unlikely(a[7] != b[7]))
        return 1;
    case 7:
      if (unlikely(a[6] != b[6]))
        return 1;
    case 6:
      if (unlikely(a[5] != b[5]))
        return 1;
    case 5:
      if (unlikely(a[4] != b[4]))
        return 1;
    case 4:
      if (unlikely(a[3] != b[3]))
        return 1;
    case 3:
      if (unlikely(a[2] != b[2]))
        return 1;
    case 2:
      if (unlikely(a[1] != b[1]))
        return 1;
    case 1:
      if (unlikely(a[0] != b[0]))
        return 1;
  }

  return 0;
}

inline uint32_t flow_hash(const void *key, uint32_t key_len, uint32_t init_val) {
#if __SSE4_2__ && __x86_64
  const uint64_t *a = ((flow_key_t *)key)->field;

  switch (key_len >> 3) {
    default:
      promise_unreachable();
    case 8:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 7:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 6:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 5:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 4:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 3:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 2:
      init_val = crc32c_sse42_u64(*a++, init_val);
    case 1:
      init_val = crc32c_sse42_u64(*a++, init_val);
  }

  return init_val;
#else
  return rte_hash_crc(key, key_len, init_val);
#endif
}

class ec_scheduler final : public Module {
public:
  using htable_t = HTable<flow_key_t, flow_actor*, flow_keycmp, flow_hash>;

  static const gate_idx_t kNumOGates = 1;
  static const gate_idx_t kNumIGates = 1;

  ec_scheduler() : Module(), allocator_(0), htable_(), deadend_flow_actor_(0) {}

  // Fake init function.
  pb_error_t Init(const bess::pb::PortIncArg &arg);

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init(flow_actor_allocator* allocator);

private:
  flow_actor_allocator* allocator_;

  htable_t htable_;

  flow_actor* deadend_flow_actor_;

};

#endif
