#include "HttpParse.h"


extern int m;

void Ids_CHttpParse::Send(ids_fsPtr&  sesptr){
	string file("/home/net/nfa/runtime/nf/ids/result/");

	file += to_string(sesptr->ClientPort);

	std::ofstream output(file.c_str(), std::ios::out | std::ios::app);
	// FILE *fp=fopen(file,)

	union iptrans p,q;
	// uint32_t a=htonl(sesptr->ClientIp);
	//  uint32_t b=htonl(sesptr->ServerIp);
	p.ip=sesptr->ClientIp;q.ip=sesptr->ServerIp;
	//output <<inet_ntoa(*((struct in_addr*)&a))<< ":" << sesptr->ClientPort << " ===> " << inet_ntoa(*(struct in_addr*)(&b)) << ":" << sesptr->ServerPort << std::endl;
	output <<(int)p.x4<<"."<<(int)p.x3<<"."<<(int)p.x2<<"."<<(int)p.x1 << ":" << sesptr->ClientPort << " ===> " <<(int)q.x4<<"."<<(int)q.x3<<"."<<(int)q.x2<<"."<<(int)q.x1  << ":" << sesptr->ServerPort << std::endl;
	// output <<sesptr->ClientIpstr<< ":" << sesptr->ClientPort << " ===> " << sesptr->ServerIpstr << ":" << sesptr->ServerPort << std::endl;
	output << "method: " << sesptr->Result.Method << "\tversion: " << sesptr->Result.Version << std::endl;
	output << "retrun code : " << sesptr->Result.RetCode << sesptr->Result.RetNote << std::endl;
	output << "url : " << sesptr->Result.Url << std::endl;
	output << "Request Header:\n";
	for(int i=0;i<m;i++){
		if(*(sesptr->Result.RequestHeader.elem[i].key)!=NULLKEY) // 有数据
		output<<std::string(sesptr->Result.RequestHeader.elem[i].key)<<":\t"<<std::string(sesptr->Result.RequestHeader.elem[i].value)<<std::endl;
	}

	output << "Request Context:\n";
	unsigned int length;
	char*tmp = GetBuf(sesptr->ReqBuf,length);
	output.write(tmp,length);
	output<<std::endl;
	output << "Response Header:\n";
	for(int i=0;i<m;i++){
		if(*(sesptr->Result.ResponseHeader.elem[i].key)!=NULLKEY) // 有数据
		output<<std::string(sesptr->Result.ResponseHeader.elem[i].key)<<":\t"<<std::string(sesptr->Result.ResponseHeader.elem[i].value)<<std::endl;
	}
	output << "Response Context:\n";

	tmp = GetBuf(sesptr->RspBuf,length);
	output.write(tmp,length);
	output<<std::endl;
	output << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
	output.close();

	return;
	}
