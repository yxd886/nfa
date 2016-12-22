#include "Receiver.h"
#include "BehaviorInfo.h"
#include "FormatPacket.h"
#include "FormatPacket.h"

Receiver::Receiver()

{
    _handle.Init();

}

Receiver::~Receiver()
{

}


void Receiver::Work(char* msg, http_parser_fsPtr& sesp)
{

	printf("packet %d processing!\n",++counter);
	HandleMessage( msg,sesp);


}


void Receiver::HandleMessage(char* msg, http_parser_fsPtr& sesp)
{
    if(msg == NULL)
    {
        cout<<"message is empty, return"<<endl;
    	return;
    }
    //格式化一个二进制包
		 CFormatPacket packet;
		 packet.Format(msg);
		 printf("packet.GetDstPort:%x\n",packet.GetDstPort());
		 printf("ntoh packet.GetDstPort:%x\n",ntohs(packet.GetDstPort()));
		 CSharedBehaviorInfo info;
		 if(packet.GetIpProtocol()==IPPROTO_TCP&&ntohs(packet.GetDstPort())==0x50)//if destport is 80
		 {
			// info.CSharedBehaviorInfo(ntohl(packet.GetDstIp()),ntohl((uint32_t)packet.GetDstPort()),packet.GetIpProtocol());
			 info.m_nIP=ntohl(packet.GetDstIp());
			 info.m_nPort=ntohs(packet.GetDstPort());
			 info.m_nBehaviorId=packet.GetIpProtocol();
			 info.m_nIdtMatchWay=C2S_MATCH;
		 }
		 else if(packet.GetIpProtocol()==IPPROTO_TCP&&ntohs(packet.GetSrcPort())==0x50)
		 {
			 //info.CSharedBehaviorInfo(ntohl(packet.GetSrcIp()),ntohs(packet.GetSrcPort()),packet.GetIpProtocol());
			 info.m_nIP=ntohl(packet.GetSrcIp());
			 info.m_nPort=ntohs(packet.GetSrcPort());
			 info.m_nBehaviorId=packet.GetIpProtocol();
			 info.m_nIdtMatchWay=S2C_MATCH;
		 }else{


			 return;//not http packet
		 }
		 CSharedBehaviorInfo* pInfo=&info;

    _handle.Process(packet,pInfo,sesp);

    return;
}
