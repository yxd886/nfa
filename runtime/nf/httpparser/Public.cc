
#include "Public.h"



void CResult_Reset(struct CResult& r){
	r.RequestTimeStamp = 0;
	r.ResponseTimeStamp = 0;
	r.Method = METUNKNOWN;
	r.RetCode = 0;
	memset(r.Url,0,sizeof(r.Url));
	memset(r.RetNote,0,sizeof(r.RetNote));
	r.Version = VERUNKNOWN;
	InitHashTable(&(r.RequestHeader));
	InitHashTable(&(r.ResponseHeader));
	memset(r.RequestData,0,sizeof(r.RequestData));
	memset(r.ResponseData,0,sizeof(r.ResponseData));
}
