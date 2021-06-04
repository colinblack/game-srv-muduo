#ifndef _WX_MEM_BUF_H_
#define _WX_MEM_BUF_H_

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

class CMemBuf{
public:
	CMemBuf();
	~CMemBuf();
public:
	bool init(uint32_t size);
	char* getWriteMem(uint32_t size);
	char* getReadMem();
public:
	void setReadPos(uint32_t length){*m_pReadPos += length;};
	void setWritePos(uint32_t length){*m_pWritePos += length;};
private:
	char* m_memStart;
	char* m_dataPos;
	volatile uint32_t* m_pReadPos;
	volatile uint32_t* m_pWritePos;
	uint32_t m_endPos;

};


#endif
