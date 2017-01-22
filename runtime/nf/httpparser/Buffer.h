#ifndef BUFFERA_H
#define BUFFERA_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "Public.h"

const uint32_t  MAX_BUFFER_SIZE = 1024;
const uint32_t  BUFFER_SIZE = 512;

struct CBuffer{

	uint32_t len;
	uint32_t _free;
	char *   buf;

};


void CBuffer_Reset(struct CBuffer& Cbuf);

bool Append(struct CBuffer& Cbuf,char* p, size_t size);

char* GetBuf(struct CBuffer Cbuf,uint32_t& size);

uint32_t GetBufLen(struct CBuffer Cbuf);


void Buf_init(struct CBuffer& Cbuf);


#endif
