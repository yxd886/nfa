#include "SessionHash.h"

void ids_fs_Reset(ids_fsPtr& ptr){
	CBuffer_Reset(ptr->ReqBuf);
	CBuffer_Reset(ptr->RspBuf);
	CResult_Reset(ptr->Result);

}
