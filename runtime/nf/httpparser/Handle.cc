#include "Handle.h"


CHandle::CHandle()
{
}

CHandle::~CHandle()
{
}

void CHandle::Init()
{

    _httpParse.Init();

}



void CHandle::Create(IFormatPacket *pPacket,CSharedBehaviorInfo* pInfo,http_parser_fsPtr& ptr)
{



    if(pInfo->m_nIP == ntohl(pPacket->GetSrcIp()) && pInfo->m_nPort == ntohs(pPacket->GetSrcPort()))
    {
        ptr->ServerIp   = ntohl(pPacket->GetSrcIp());
        ptr->ClientIp   = ntohl(pPacket->GetDstIp());
        ptr->ServerPort = ntohs(pPacket->GetSrcPort());
        ptr->ClientPort = ntohs(pPacket->GetDstPort());
        ptr->ServerMac  = pPacket->GetSrcMac();
        ptr->ClientMac  = pPacket->GetDstMac();
        ptr->Protocol   = pInfo->m_nBehaviorId;

        ptr->CreatedTime = time(0);
        ptr->RefreshTime = ptr->CreatedTime;
        ptr->counter=0;
    }
    else
    {
        ptr->ServerIp   = ntohl(pPacket->GetDstIp());
        ptr->ClientIp   = ntohl(pPacket->GetSrcIp());
        ptr->ServerPort = ntohs(pPacket->GetDstPort());
        ptr->ClientPort = ntohs(pPacket->GetSrcPort());
        ptr->ServerMac  = pPacket->GetDstMac();
        ptr->ClientMac  = pPacket->GetSrcMac();
        ptr->Protocol   = pInfo->m_nBehaviorId;

        ptr->CreatedTime = time(0);
        ptr->RefreshTime = ptr->CreatedTime;
        ptr->counter=0;

    }



}


void CHandle::Process(CFormatPacket packet, CSharedBehaviorInfo* pInfo, http_parser_fsPtr& fhs)
{



    if( !pInfo)
    {
        //log handle.process arguament is null
        return;
    }

    uint32_t srcIp = ntohl(packet.GetSrcIp());
    uint32_t dstIp = ntohl(packet.GetDstIp());
    uint16_t srcPort = ntohs(packet.GetSrcPort());
    uint16_t dstPort = ntohs(packet.GetDstPort());

    //从会话链中查找该会话


    if(fhs->counter==0)
    {
        //如果不存在，则创建新会话
    	printf("new session created!\n");
    	//getchar();
        Create(&packet,pInfo,fhs);
        fhs->counter++;



    }

    //开始组包

    if(packet.GetTcphdr()->fin == 1 || packet.GetTcphdr()->rst == 1)
    {
        //log 会话结束,session over
    	printf("session over\n");
        _httpParse.Parse(fhs);

        return;
    }

    if(fhs->SeqNo == ntohl(packet.GetTcphdr()->seq))
    {
        //log repeated packet  重复的包
    	printf("repeated packet!\n");
        return;
    }

    if(packet.GetDataLen() == 0)
    {
        //log zero length
    	printf("data zero!\n");
        return;
    }

    fhs->SeqNo=ntohl(packet.GetTcphdr()->seq);
    fhs->AckSeqNo=ntohl(packet.GetTcphdr()->ack_seq);

    if(pInfo->m_nIdtMatchWay == UNK_MATCH)
    {
        //log unknown match
    	printf("unknow match!\n");
        return;
    }
    else if(pInfo->m_nIdtMatchWay == C2S_MATCH)
    {
        printf("C2S\n");
    	if(fhs->ReqBuf.GetBufLen() > 0 && fhs->RspBuf.GetBufLen() > 0)
        {
            printf("enter sesptr->ReqBuf.GetBufLen() > 0 && sesptr->RspBuf.GetBufLen() > 0\n");
        	_httpParse.Parse(fhs);

        }

        if(fhs->ReqBuf.GetBufLen() == 0 && fhs->Result.RequestTimeStamp == 0)
        {
            //the first request packet. we will get timestamp from this packet
        	printf("first request packet!\n");
            fhs->Result.RequestTimeStamp=packet.GetPacketTime()->tv_sec * 1000000LL + packet.GetPacketTime()->tv_usec;;
        }
        printf("appending request buffer!\n");
        if(!Append(fhs->ReqBuf,(char*) packet.GetData(), (size_t) packet.GetDataLen()))
        {

        	//log  c2s append date error
            return;
        }
    }
    else if(pInfo->m_nIdtMatchWay == S2C_MATCH)
    {
    	printf("S2C\n");
    	if(GetBufLen(fhs->RspBuf) == 0 && fhs->Result.ResponseTimeStamp == 0)
        {
            //the first response packet. we will get timestamp from this packet
    		printf("first response packet!\n");
            fhs->Result.ResponseTimeStamp = packet.GetPacketTime()->tv_sec * 1000000LL + packet.GetPacketTime()->tv_usec;

        }

    	printf("appending respone buffer!\n");
    	if(!Append(fhs->RspBuf,(char*) packet.GetData(), (size_t) packet.GetDataLen()))
        {
            //log  c2s append date error
            return;
        }
    }
    unsigned int i;
    printf("session request buffer:%s\n\n\n\n",GetBuf(fhs->ReqBuf,i));
   // getchar();
    printf("session response buffer:%s\n",GetBuf(fhs->RspBuf,i));
    return;
}

