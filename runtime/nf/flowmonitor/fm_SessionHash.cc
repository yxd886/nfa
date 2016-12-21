#include "fm_SessionHash.h"
#include "fm_headinfo.h"
#include <arpa/inet.h>


flow_monitor_fsHashTable::flow_monitor_fsHashTable()
{
}

flow_monitor_fsHashTable::~flow_monitor_fsHashTable()
{
}

void flow_monitor_fsHashTable::Init()
{
    _cseshash.clear();
    for(uint32_t i = 0; i < 65536; i++)
    {
        _cseshash.push_back(flow_monitor_fsPtrListPtr(new flow_monitor_fsPtrList()));
    }
}

flow_monitor_fsPtr flow_monitor_fsHashTable::Find(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort, uint16_t dstPort)
{
    uint32_t tsNow = time(0);
    uint16_t index = HashVal(srcIp,dstIp,srcPort,dstPort);
    flow_monitor_fsPtrListPtr listPtr = _cseshash[index];

    deque<flow_monitor_fsPtr>::iterator iter = listPtr->begin();
    for(;iter != listPtr->end();)
    {
        if(IsSameSes(srcIp,dstIp,srcPort,dstPort,*iter))
        {
            (*iter)->RefreshTime = tsNow;
            return *iter;
        }
        else
        {
            iter++;
        }
    }

    return flow_monitor_fsPtr();
}

flow_monitor_fsPtr flow_monitor_fsHashTable::Create(struct head_info* hd)
{
    flow_monitor_fsPtr   ptr = flow_monitor_fsPtr(new flow_monitor_fs());

        ptr->SrcIp   = ntohl(hd->m_pIphdr->saddr);
        ptr->DstIp   = ntohl(hd->m_pIphdr->daddr);
        ptr->SrcPort = ntohs((hd->m_pTcphdr!=NULL)?hd->m_pTcphdr->source:0);
        ptr->DstPort = ntohs((hd->m_pTcphdr!=NULL)?hd->m_pTcphdr->dest:0);
        ptr->protocol   = hd->m_pIphdr->protocol;
        ptr->CreatedTime = time(0);
        ptr->RefreshTime = ptr->CreatedTime;
        ptr->no_icmp = 0;
        ptr->no_icmp = 0;
        ptr->no_tcp = 0;
        ptr->no_total = 0;
        ptr->counter=0;

    uint16_t index = HashVal(ptr->SrcIp,ptr->DstIp,ptr->SrcPort,ptr->DstPort);

    _cseshash[index]->push_back(ptr);

    return ptr;
}

void flow_monitor_fsHashTable::Remove(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort,uint16_t dstPort)
{

    uint16_t index = HashVal(srcIp,dstIp,srcPort,dstPort);
    flow_monitor_fsPtrListPtr listPtr = _cseshash[index];

    deque<flow_monitor_fsPtr>::iterator iter = listPtr->begin();
    for(;iter != listPtr->end();)
    {
        if(IsSameSes(srcIp,dstIp,srcPort,dstPort,*iter))
        {
            iter = listPtr->erase(iter);
            return ;
        }
        else
        {
            iter++;
        }
    }
}

uint16_t  flow_monitor_fsHashTable::HashVal(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort,uint16_t dstPort)
{
    uint32_t h = ((srcIp ^ dstIp) ^ (srcPort ^ dstPort) );
    h ^= h >> 16;
    h ^= h >> 8;
    h &= 0x0000ffff;
    uint16_t r = h;
    return r;
}

bool flow_monitor_fsHashTable::IsSameSes(uint32_t srcIp,uint32_t dstIp, uint16_t srcPort, uint16_t dstPort,const flow_monitor_fsPtr& ptr)
{
    return   (
                (ptr->SrcIp == srcIp)     &&
                (ptr->DstIp == dstIp)     &&
                (ptr->SrcPort == srcPort) &&
                (ptr->DstPort == dstPort)
             )
             ||
             (
                (ptr->SrcIp == dstIp)     &&
                (ptr->DstIp == srcIp)     &&
                (ptr->SrcPort == dstPort) &&
                (ptr->DstPort == srcPort)
             );
}
