#ifndef RECEIVE_H_
#define RECEIVE_H_

#include "Public.h"
#include "Handle.h"
#include "BehaviorInfo.h"
#include "FormatPacket.h"

class Receiver
{
public:
    Receiver(){
      _handle.Init();

  }

    ~Receiver(){}
    void Work(char* msg, http_parser_fsPtr& sesp){

    	HandleMessage( msg,sesp);


    }


private:
    void HandleMessage(char* msg, http_parser_fsPtr& sesp)
    {
        if(msg == NULL)
        {
            cout<<"message is empty, return"<<endl;
        	return;
        }
        //格式化一个二进制包
    		 CFormatPacket packet;
    		 packet.Format(msg);
    	//	 printf("packet.GetDstPort:%x\n",packet.GetDstPort());
    	//	 printf("ntoh packet.GetDstPort:%x\n",ntohs(packet.GetDstPort()));
    	//	 printf("packet.GetSrcPort:%x\n",packet.GetSrcPort());
    	//	 printf("ntoh packet.GetSrcPort:%x\n",ntohs(packet.GetSrcPort()));
    		 CSharedBehaviorInfo info;
    		 if(packet.GetIpProtocol()==IPPROTO_TCP&&ntohs(packet.GetDstPort())==0x50)//if destport is 80
    		 {
    			// info.CSharedBehaviorInfo(ntohl(packet.GetDstIp()),ntohl((uint32_t)packet.GetDstPort()),packet.GetIpProtocol());
    			 info.m_nIP=ntohl(packet.GetDstIp());
    			 info.m_nPort=ntohs(packet.GetDstPort());
    			 info.m_nBehaviorId=packet.GetIpProtocol();
    			 info.m_nIdtMatchWay=C2S_MATCH;
    		//	 printf("Client to server/n");
    		 }
    		 else if(packet.GetIpProtocol()==IPPROTO_TCP&&ntohs(packet.GetSrcPort())==0x50)
    		 {
    			 //info.CSharedBehaviorInfo(ntohl(packet.GetSrcIp()),ntohs(packet.GetSrcPort()),packet.GetIpProtocol());
    			 info.m_nIP=ntohl(packet.GetSrcIp());
    			 info.m_nPort=ntohs(packet.GetSrcPort());
    			 info.m_nBehaviorId=packet.GetIpProtocol();
    			 info.m_nIdtMatchWay=S2C_MATCH;
    	//		 printf("Server to client\n");
    		 }else{


    			 return;//not http packet
    		 }
    		 CSharedBehaviorInfo* pInfo=&info;

        _handle.Process(packet,pInfo,sesp);

        return;
    }

    //zmq::context_t _ctx;
    //zmq::socket_t _socket;
    CHandle  _handle;
};












#endif
