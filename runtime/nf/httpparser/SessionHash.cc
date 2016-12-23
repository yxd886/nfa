#include "SessionHash.h"

void http_parser_fs_Reset(http_parser_fsPtr& ptr){
	CBuffer_Reset(ptr->ReqBuf);
	CBuffer_Reset(ptr->RspBuf);
	CResult_Reset(ptr->Result);

}
