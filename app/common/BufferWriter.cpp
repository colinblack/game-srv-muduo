/*
 * BufferWriter.cpp
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#include "BufferWriter.h"

CBufferWriter::CBufferWriter() :
	m_pBuffer(NULL)
{
}

CBufferWriter::CBufferWriter(Buffer *pBuffer) :
	m_pBuffer(pBuffer)
{
}

bool CBufferWriter::Attach(Buffer *pBuffer)
{
	m_pBuffer = pBuffer;
	return true;
}

bool CBufferWriter::Detach()
{
	m_pBuffer = NULL;
	return true;
}

#define NOTHING(value) value

#define IMPLEMENT_BUFFER_WRITE(TypeName, Type)	\
	bool CBufferWriter::Append##TypeName(Type value)	\
	{	\
		if(m_pBuffer == NULL)	\
		{	\
			return false;	\
		}	\
		if(sizeof(Type) == 1)\
		{\
			m_pBuffer->appendInt8(value);\
		}\
		else if(sizeof(Type) == 2)\
		{\
			m_pBuffer->appendInt16(value);\
		}\
		else if(sizeof(Type) == 4)\
		{\
			m_pBuffer->appendInt32(value);\
		}\
		else if(sizeof(Type) == 8)\
		{\
			m_pBuffer->appendInt64(value);\
		}\
		else\
		{\
			return false; \
		}\
			return true;\
	}	

IMPLEMENT_BUFFER_WRITE(Byte, byte)
IMPLEMENT_BUFFER_WRITE(Int16, int16_t)
IMPLEMENT_BUFFER_WRITE(UInt16, uint16_t)
IMPLEMENT_BUFFER_WRITE(Int32, int32_t)
IMPLEMENT_BUFFER_WRITE(UInt32, uint32_t)
IMPLEMENT_BUFFER_WRITE(Int64, int64_t)
IMPLEMENT_BUFFER_WRITE(UInt64, uint64_t)
//IMPLEMENT_BUFFER_WRITE(Double, double, hton64)

bool CBufferWriter::AppendFloat(float value){
	if(m_pBuffer == NULL)
	{
		return false;
	}

	m_pBuffer->append((byte *)&value, sizeof(float));
	return true;
}

bool CBufferWriter::AppendDouble(double value){
	if(m_pBuffer == NULL)
	{
		return false;
	}

	m_pBuffer->append((byte *)&value, sizeof(double));
	return true;
}


bool CBufferWriter::AppendString(const string &value)
{
	if(m_pBuffer == NULL)
	{
		return false;
	}
	debug_log("value=%s", value.c_str());
	uint16_t size = value.size();
	m_pBuffer->append((const char*)&size, sizeof(size));
	m_pBuffer->append(value);
	return true;
}
bool CBufferWriter::AppendPBMsg(const Message* msg)
{
	if(msg == NULL)
		return false;

	if(!AppendString(msg->GetTypeName()))
	{
		error_log("append name:%s", msg->GetTypeName().c_str());
		return false;
	}

	uint32_t size = msg->ByteSize();
	debug_log("size=%d", size);
	m_pBuffer->append(reinterpret_cast<const char*>(&size), sizeof(size));
	m_pBuffer->ensureWritableBytes(size);
	if(!msg->SerializeToArray(m_pBuffer->beginWrite(), size))
	{
		error_log("append array:%u,%s", size, msg->GetTypeName().c_str());
		return false;
	}
	m_pBuffer->hasWritten(size);
	return true;
}

bool CBufferWriter::AppendBuffer(const IBuffer *value)
{
#if 0
	if(m_pBuffer == NULL || value == NULL)
	{
		return false;
	}
	uint16_t size = (uint16_t)value->GetSize();
	if(m_bSwapByte)
	{
		uint16_t tempSize = htons(size);
		if(!m_pBuffer->Append((byte *)&tempSize, sizeof(tempSize)))
		{
			return false;
		}
	}
	else
	{
		if(!m_pBuffer->Append((byte *)&size, sizeof(size)))
		{
			return false;
		}
	}
	if(!m_pBuffer->AppendBuffer(value))
	{
		m_pBuffer->SetSize(m_pBuffer->GetSize() - sizeof(size));
		return false;
	}
#endif 
	return true;
}

bool CBufferWriter::AppendBytes(const byte *value, uint32_t size)
{
	if(m_pBuffer == NULL || value == NULL)
	{
		return false;
	}
	m_pBuffer->append(value, size);
	return true;
}
