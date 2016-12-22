
#include "Buffer.h"

void CBuffer_Reset(struct CBuffer& Cbuf)
{
    if(!Cbuf.buf)
    {
    	Cbuf.buf = (char*) malloc(BUFFER_SIZE);
        memset(Cbuf.buf,0x00,Cbuf._free);
    }

    if(Cbuf.len > BUFFER_SIZE * 2 && Cbuf.buf)
    {
        //如果目前buf的大小是默认值的2倍，则对其裁剪内存，保持buf的大小为默认值，减小内存耗费
        char* newbuf = (char*) realloc(Cbuf.buf,BUFFER_SIZE);
        if(newbuf != Cbuf.buf)
        	Cbuf.buf = newbuf;
    }

    Cbuf.len = 0;
    Cbuf._free = BUFFER_SIZE;
}

bool Append(struct CBuffer& Cbuf,char* p, size_t size)
{
    if(!p || !size)
        return true;
    if(size < Cbuf._free)
    {
        memcpy(Cbuf.buf + Cbuf.len, p , size);
        Cbuf.len += size;
        Cbuf._free -= size;
    }
    else
    {
    	return false;
    }

    return true;
}

char* GetBuf(struct CBuffer Cbuf,uint32_t& size)
{
    size = Cbuf.len;
    return Cbuf.buf;
}

uint32_t GetBufLen(struct CBuffer Cbuf){

	return Cbuf.len;

}


void Buf_init(struct CBuffer& Cbuf)
{


	Cbuf.len=0;
	Cbuf._free=0;
	Cbuf.buf=0;
	CBuffer_Reset(Cbuf);
}
