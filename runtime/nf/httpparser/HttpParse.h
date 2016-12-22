#ifndef HTTPPARSE_H_
#define HTTPPARSE_H_

#include "Public.h"
#include "SessionHash.h"
#include "FormatPacket.h"
#include <string>
#include <arpa/inet.h>
#include <memory.h>

using namespace std;
union iptrans{
	struct{
		uint8_t x1;
		uint8_t x2;
		uint8_t x3;
		uint8_t x4;
	};
	uint32_t ip;
};

class CHttpParse
{
public:
    CHttpParse();
    ~CHttpParse();

    void  Init();
    void  Parse(http_parser_fsPtr& sesptr);
private:
    bool  ParseMethodAndUri(const char* pBuf, const uint32_t len,uint32_t& pos, CResult& result);
    bool  ParseRspState(const char* pBuf, const uint32_t len, uint32_t& pos, CResult& result);
    bool  ParseHeader(const char* pBuf, const uint32_t len, uint32_t& pos,  HeaderMap& headmap);
    bool  ParseReqData(const char* pBuf, const uint32_t len, uint32_t& pos, CResult& result);
    bool  ParseRspData(const char* pBuf, const uint32_t len, uint32_t& pos, CResult& result);
    uint32_t  GetMethod(string method);
    bool  GetVersion(string version, uint32_t& ver);

    void  Send(http_parser_fsPtr&  sesptr);


    int GetBufByTag(const char* in, const int len, const char* tag, const int tagsize, string& out)
    {
        int i;
        for(i = 0; i< len; i++)
        {
            if(strncmp(in + i, tag, tagsize) == 0)
            {
                out.append(in,i);
                return i;
            }
        }

        return -1;
    }

};



CHttpParse::CHttpParse()
{
}

CHttpParse::~CHttpParse()
{

}

void CHttpParse::Init()
{
}

void CHttpParse::Parse(http_parser_fsPtr& sesptr)
{
    uint32_t reqLen = 0;
    uint32_t rspLen = 0;
    const char*  reqBuf = GetBuf(sesptr->ReqBuf,reqLen);
    const char*  rspBuf = GetBuf(sesptr->RspBuf,rspLen);

    if(!reqBuf || !rspBuf || !reqLen || !rspLen)
    {
        http_parser_fs_Reset(sesptr);
        return;
    }


    uint32_t pos = 0;

    if(reqLen > 10)  //parse request packet
    {

    	printf("parsing MethodAndUri !\n");
    	if(!ParseMethodAndUri(reqBuf,reqLen,pos,sesptr->Result))  //parse method, uri, version
        {
    		printf("parsing MethodAndUri failure!\n");
    		http_parser_fs_Reset(sesptr);
				return;
        }

    	printf("parsing Request header !\n");
    	if(!ParseHeader(reqBuf, reqLen,pos, sesptr->Result.RequestHeader))  //parse request header
        {
    		printf("parsing Request header failure!\n");
    		http_parser_fs_Reset(sesptr);
            return;
        }
    	printf("parsing Request data !\n");
        if(!ParseReqData(reqBuf,reqLen,pos,sesptr->Result))  //parse request data
        {
        	printf("parsing Request data failure!\n");
        	http_parser_fs_Reset(sesptr);
            return;
        }
    }

    if(rspLen > 10)  //parse response packet
    {
        pos = 0;
        printf("parsing Response state !\n");
        if(!ParseRspState(rspBuf,rspLen,pos,sesptr->Result)) //parse reponse state
        {
        	printf("parsing Response state failure!\n");
        	http_parser_fs_Reset(sesptr);
            return;
        }

        printf("parsing Response header !\n");
        if(!ParseHeader(rspBuf,rspLen, pos,sesptr->Result.ResponseHeader)) //parse reponse header
        {
        	printf("parsing Response header failure!\n");
        	http_parser_fs_Reset(sesptr);
            return;
        }

        printf("parsing Response data !\n");
        if(!ParseRspData(rspBuf,rspLen, pos,sesptr->Result)) //parse reponse data
        {
        	printf("parsing Response data failure!\n");
        	http_parser_fs_Reset(sesptr);
            return;
        }
    }



    Send(sesptr);
    http_parser_fs_Reset(sesptr);

    return;
}

bool CHttpParse::ParseMethodAndUri(const char* pBuf, const uint32_t len, uint32_t& pos, CResult& result)
{
    //get method
    string method;
    int ret = GetBufByTag(pBuf+pos,len-pos," ",1,method);
    if( ret == -1)
    {
        //log get method error
    	printf("method prasing failure\n");
        return false;
    }
    pos += ret;
    pos += 1;  //skip the space
    result.Method = GetMethod(method);
    cout<<"Method:"<<method<<endl;


    //get url
    std::string url(result.Url);
    ret = GetBufByTag(pBuf+pos,len-pos," ",1,url);
    memset(result.Url,0,sizeof(result.Url));
    strcpy(result.Url,url.c_str());
    if(ret == -1)
    {
        //log get url error
    	printf("get url failure\n");
        return false;
    }
    pos += ret;
    pos += 1;  //skip the space
    cout<<"Url:"<<url<<endl;


    //get http version
    string version;
    ret = GetBufByTag(pBuf+pos,len-pos,"\n",1,version);
    if( ret == -1)
    {
        //log get version error
    	printf("get version failure\n");
        return false;
    }
    pos += ret;
    pos += 1;  //skip the \r\n
    cout<<"Version:"<<version<<endl;
    if(!GetVersion(version,result.Version))
    {
    	printf(" version compare failure\n");
    	return false;
    }


    return true;

}

bool CHttpParse::ParseRspState(const char* pBuf, const uint32_t len,uint32_t& pos, CResult& result)
{
    int ret = 0;

    //check the version with reqeust version
    string version;
    uint32_t ver;
    ret = GetBufByTag(pBuf+pos,len-pos," ",1,version);
    if( ret == -1)
    {
        //log reponse get version error
        return false;
    }
    pos += ret;
    pos += 1;  //skip the space

    if(!GetVersion(version,ver))
    {
        return false;
    }

    if( ver != result.Version)
    {
        //log response version is not equal to request version
        return false;
    }


    //get the response code
    string rspCode;
    ret = GetBufByTag(pBuf+pos,len-pos," ",1,rspCode);
    if( ret == -1)
    {
        //log reponse get version error
        return false;
    }
    pos += ret;
    pos += 1;  //skip the space
    result.RetCode  = atoi(rspCode.c_str());
    string retnote(result.RetNote);
    ret = GetBufByTag(pBuf+pos,len-pos,"\r\n",2,retnote);
    memset(result.RetNote,0,sizeof(result.RetNote));
    strcpy(result.RetNote,retnote.c_str());
    if( ret == -1)
    {
        //log reponse get version error
        return false;
    }
    pos += ret;
    pos += 2;  //skip the \r\n


    return true;
}

bool CHttpParse::ParseHeader(const char* pBuf, const uint32_t len,uint32_t& pos,  HeaderMap& headmap)
{
    int ret = 0;


    while(true)
    {
        string key = "";
        string value = "";

        if(!(pBuf+pos) || !(len-pos))
        {
            //log buf modified  %d <=> len
            return false;
        }
        cout<<"pBuf + pos=";
        putchar(*(pBuf + pos));
        cout<<endl;
        if(strncmp(pBuf + pos,"\n",1) == 0)
        {
            pos += 1;
            return true;
        }
        if(strncmp(pBuf + pos,"\r\n",2) == 0)
                {
                    pos += 2;
                    return true;
                }

        ret = GetBufByTag(pBuf + pos,len - pos,":",1,key);
        if( ret == -1)
        {
            //log parse header key error
            return false;
            printf("find key failure\n");
        }
        pos += ret;
        pos += 1; //skip the :
        cout<<"key:"<<key<<endl;

        ret = GetBufByTag(pBuf + pos,len - pos,"\n",1,value);
        if( ret == -1)
        {
            //log parse header value error
        	printf("find value failure\n");
            return false;
        }
        pos += ret;
        pos += 1; //skip the \r\n
        cout<<"value:"<<value<<endl;
        ElemType key_value;
        memset(key_value.key,0,sizeof(key_value.key));
        memset(key_value.value,0,sizeof(key_value.value));
        strcpy(key_value.key,key.c_str());
        strcpy(key_value.value,value.c_str());
        InsertHash(&headmap,key_value);

    }
    return true;
}

bool CHttpParse::ParseReqData(const char* pBuf, const uint32_t len,uint32_t& pos, CResult& result)
{
    return true;
}

bool CHttpParse::ParseRspData(const char* pBuf, const uint32_t len,uint32_t& pos, CResult& result)
{
    return true;
}

void CHttpParse::Send(http_parser_fsPtr&  sesptr)
{
    string file("../src/network_function/http_parser/result/");

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
      if(sesptr->Result.RequestHeader.elem[i].key!=NULLKEY) // 有数据
       print(i,sesptr->Result.RequestHeader.elem[i]);
      output<<std::string(sesptr->Result.RequestHeader.elem[i].key)<<":\t"<<std::string(sesptr->Result.RequestHeader.elem[i].value)<<std::endl;
    }

    output << "Request Context:\n";
    unsigned int length;
    char*tmp = GetBuf(sesptr->ReqBuf,length);
    output.write(tmp,length);
    output<<std::endl;
    output << "Response Header:\n";
    for(int i=0;i<m;i++){
      if(sesptr->Result.ResponseHeader.elem[i].key!=NULLKEY) // 有数据
       print(i,sesptr->Result.ResponseHeader.elem[i]);
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

bool CHttpParse::GetVersion(string version, uint32_t& ver)
{
    if(version.size() != 8)
    {
        //log error to get version. %s <=> version
        return false;
    }

    if(version.find("HTTP/") == string::npos)
    {
        //log error to get version. %s <=> version
        return false;
    }

    string tmp = version.substr(5,3);

    float i;

    i = atof(tmp.c_str());


    ver = (uint32_t) (i * 10);

    return true;
}

uint32_t CHttpParse::GetMethod(string method)
{
    if(method.compare("GET") == 0)
    {
        return GET;
    }
    else if(method.compare("POST") == 0)
    {
        return POST;
    }
    else if(method.compare("OPTIONS") == 0)
    {
        return OPTIONS;
    }
    else if(method.compare("HEAD") == 0)
    {
        return HEAD;
    }
    else if(method.compare("PUT") == 0)
    {
        return PUT;
    }
    else if(method.compare("DELETE") == 0)
    {
        return DELETE;
    }
    else if(method.compare("TRACE") == 0)
    {
        return TRACE;
    }
    else if(method.compare("CONNECT") == 0)
    {
        return CONNECT;
    }
    else
    {
        return METUNKNOWN;
    }
}

#endif
