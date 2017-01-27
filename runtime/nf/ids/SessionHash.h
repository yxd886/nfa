#ifndef SESSIONHASH_H_
#define SESSIONHASH_H_

#include "../httpparser/Public.h"
#include "../httpparser/Buffer.h"
#include "../httpparser/FormatPacket.h"
#include "../httpparser/BehaviorInfo.h"
#include <rte_config.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ethdev.h>

struct ids_fs{

	uint32_t ServerIp;
	uint32_t ClientIp;
	uint16_t ServerPort;
	uint16_t ClientPort;
	uint64_t ServerMac;
	uint64_t ClientMac;

	time_t   CreatedTime;
	time_t   RefreshTime;
	uint32_t SeqNo;
	uint32_t AckSeqNo;

	uint8_t Protocol;
	CBuffer  ReqBuf;
	CBuffer  RspBuf;
	CResult  Result;
	int counter;
};
typedef ids_fs*    ids_fsPtr;

void ids_fs_Reset(ids_fsPtr& ptr);







#endif
