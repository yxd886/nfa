#ifndef MP_TCPFS_H
#define MP_TCPFS_H

#include <cstdint>

struct mp_tcp_fs{
  bool need_to_migrate;
  int32_t migration_target_id;
};

#endif
