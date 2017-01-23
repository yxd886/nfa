#ifndef PKT_COUNTER_FS_H
#define PKT_COUNTER_FS_H

#include <cstdint>
#include <netinet/in.h>

struct load_balancer_fs{

	bool is_init;
	uint32_t ip;
};

#endif
