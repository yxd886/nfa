#ifndef HANDLEA_H_
#define HANDLEA_H_

#include "../httpparser/Public.h"
#include "../httpparser/BehaviorInfo.h"
#include "../httpparser/FormatPacket.h"
#include "SessionHash.h"
#include "HttpParse.h"

class Ids_CHandle{
public:
	Ids_CHandle();
	~Ids_CHandle();
	void Init();
	void Process(CFormatPacket packet, CSharedBehaviorInfo* pInfo, ids_fsPtr& sesp);
	void Create(IFormatPacket *pPacket,CSharedBehaviorInfo* pInfo,ids_fsPtr& ptr);

private:
	void TimeOutCheck();


	Ids_CHttpParse         _httpParse;


};






#endif
