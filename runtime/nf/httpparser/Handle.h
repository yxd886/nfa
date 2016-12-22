#ifndef HANDLEA_H_
#define HANDLEA_H_

#include "Public.h"
#include "BehaviorInfo.h"
#include "FormatPacket.h"
#include "SessionHash.h"
#include "HttpParse.h"

class CHandle
{
public:
    CHandle();
    ~CHandle();
    void Init();
    void Process(CFormatPacket packet, CSharedBehaviorInfo* pInfo, http_parser_fsPtr& sesp);
    void Create(IFormatPacket *pPacket,CSharedBehaviorInfo* pInfo,http_parser_fsPtr& ptr);

private:
    void TimeOutCheck();


    CHttpParse         _httpParse;


};






#endif
