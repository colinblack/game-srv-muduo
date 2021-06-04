/*
 * BufferReader.h
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#ifndef BUFFERREADER_H_
#define BUFFERREADER_H_

#include "Common.h"

class CBufferReader
{
public:
	CBufferReader();
	CBufferReader(Buffer *pBuffer);
	uint32_t GetReadSize();
	bool Skip(int offset);

	bool GetByte(byte &value);
	bool GetInt16(int16_t &value);
	bool GetUInt16(uint16_t &value);
	bool GetInt32(int32_t &value);
	bool GetUInt32(uint32_t &value);
	bool GetInt64(int64_t &value);
	bool GetUInt64(uint64_t &value);
	bool GetFloat(float &value);
	bool GetDouble(double &value);
	bool GetString(string &value);
	bool GetPBMsg(Message* &msg, uint32_t decodesize);
	bool GetBuffer(Buffer *value);
	bool GetBytes(byte *value, uint32_t size);

private:
	Buffer *m_pBuffer;
	uint32_t m_index;
};


#endif /* BUFFERREADER_H_ */
