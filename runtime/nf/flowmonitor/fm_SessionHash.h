#ifndef FM_SESSIONHASH_H_
#define FM_SESSIONHASH_H_


#include <boost/shared_ptr.hpp>
#include <asm-generic/int-ll64.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <deque>
#include <set>
#include <map>
#include <list>



using namespace std;
using namespace boost;

struct flow_monitor_fs
{

    uint32_t SrcIp;
    uint32_t DstIp;
    uint16_t SrcPort;
    uint16_t DstPort;
    uint8_t protocol;
    time_t   CreatedTime;
    time_t   RefreshTime;
    uint64_t no_tcp;
    uint64_t no_udp;
    uint64_t no_icmp;
    uint64_t no_total;
    int counter;

};

typedef boost::shared_ptr<flow_monitor_fs>         flow_monitor_fsPtr;
typedef deque<flow_monitor_fsPtr>           flow_monitor_fsPtrList;
typedef boost::shared_ptr<flow_monitor_fsPtrList>  flow_monitor_fsPtrListPtr;
typedef vector<flow_monitor_fsPtrListPtr>   fm_CSesHash;


class flow_monitor_fsHashTable
{
public:
    flow_monitor_fsHashTable();
    ~flow_monitor_fsHashTable();
    flow_monitor_fsPtr Find(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort, uint16_t dstPort);
    flow_monitor_fsPtr Create(struct head_info* hd);
    void        Remove(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort,uint16_t dstPort);
    void        Init();
    fm_CSesHash GetHash()
    {
        return _cseshash;
    }

private:
    uint16_t HashVal(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort, uint16_t dstPort);
    fm_CSesHash _cseshash;
    bool     IsSameSes(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, const flow_monitor_fsPtr& ptr);
};

#endif
