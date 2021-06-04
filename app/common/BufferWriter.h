/*
 * BufferWriter.h
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#ifndef BUFFERWRITER_H_
#define BUFFERWRITER_H_

#include "Common.h"

class CBufferWriter
{
public:
	CBufferWriter();
	CBufferWriter(Buffer *pBuffer);
	bool Attach(Buffer *pBuffer);
	bool Detach();

	bool AppendByte(byte value);
	bool AppendInt16(int16_t value);
	bool AppendUInt16(uint16_t value);
	bool AppendInt32(int32_t value);
	bool AppendUInt32(uint32_t value);
	bool AppendInt64(int64_t value);
	bool AppendUInt64(uint64_t value);
	bool AppendFloat(float value);
	bool AppendDouble(double value);
	bool AppendString(const string &value);
	bool AppendPBMsg(const Message* msg);
	bool AppendBuffer(const IBuffer *value);
	bool AppendBytes(const byte *value, uint32_t size);

private:
	Buffer *m_pBuffer;
};

#endif /* BUFFERWRITER_H_ */
