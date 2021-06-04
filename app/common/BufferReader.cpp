/*
 * BufferReader.cpp
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#include "BufferReader.h"


CBufferReader::CBufferReader() :
	m_pBuffer(NULL),
	m_index(0)
{
}

CBufferReader::CBufferReader(Buffer *pBuffer) :
	m_pBuffer(pBuffer),
	m_index(0)
{
}

uint32_t CBufferReader::GetReadSize()
{
	return m_index;
}

bool CBufferReader::Skip(int offset)
{
	m_pBuffer->retrieve(offset);
	return true;
}

#define IMPLEMENT_BUFFER_READ(TypeName, Type)	\
	bool CBufferReader::Get##TypeName(Type &value)	\
	{\
		if(m_pBuffer == NULL)	\
		{	\
			return false;	\
		}	\
		if(sizeof(Type) == 1)\
		{\
			value = m_pBuffer->readInt8();\
		}\
		else if(sizeof(Type) == 2)\
		{\
			value = m_pBuffer->readInt16();\
		}\
		else if(sizeof(Type) == 4)\
		{\
			value = m_pBuffer->readInt32();\
		}\
		else if(sizeof(Type) == 8)\
		{\
			value = m_pBuffer->readInt64();\
		}\
		else\
		{\
			return false;\
		}\
		return true;\
	}

IMPLEMENT_BUFFER_READ(Byte, byte)
IMPLEMENT_BUFFER_READ(Int16, int16_t)
IMPLEMENT_BUFFER_READ(UInt16, uint16_t)
IMPLEMENT_BUFFER_READ(Int32, int32_t)
IMPLEMENT_BUFFER_READ(UInt32, uint32_t)
IMPLEMENT_BUFFER_READ(Int64, int64_t)
IMPLEMENT_BUFFER_READ(UInt64, uint64_t)

bool CBufferReader::GetFloat(float &value)
{
	 value = m_pBuffer->readFloat();
	return true;
}

bool CBufferReader::GetDouble(double &value)
{
	 value = m_pBuffer->readDouble();
	return true;
}


bool CBufferReader::GetString(string &value)
{
	if(m_pBuffer == NULL)
	{
		return false;
	}
	uint16_t size = 0;
    GetUInt16(size);

	value.reserve(size);
	value = m_pBuffer->retrieveAsString(size);

	return true;
}

bool CBufferReader::GetPBMsg(Message* &msg, uint32_t decodesize)
{
	debug_log("decodesize=%d, readablesize=%d", decodesize, m_pBuffer->readableBytes());
	string name;
	GetString(name);
	uint32_t retrievesize=decodesize-name.size()-2;

	debug_log("decodesize=%d, readablesize=%d", retrievesize, m_pBuffer->readableBytes());
	msg = Convert::CreateMessage(name);
	if(msg == NULL)
	{
		error_log("create msg:%s", name.c_str());
		return false;
	}

	uint32_t size = 0;
	GetUInt32(size);	
	retrievesize -= sizeof(size);
	
	
    if(size == 0)
	{
		error_log("get length:%s", name.c_str());
		delete msg;
		return false;
	}

	if(!msg->ParseFromArray(m_pBuffer->peek(), size))
	{
		error_log("get array:%u,%s",size, name.c_str());
		delete msg;
		return false;
	}

	debug_log("retrievesize=%d, readablesize=%d", retrievesize, m_pBuffer->readableBytes());
	m_pBuffer->retrieve(retrievesize);

	if(m_pBuffer->readableBytes() == 0){
		debug_log("no readable bytes");
		m_pBuffer->retrieveAll();
	}

	return true;
}

bool CBufferReader::GetBytes(byte *value, uint32_t size)
{
	if(m_pBuffer == NULL || value == NULL)
	{
		return false;
	}
	m_pBuffer->readBytes(value, size);

	return true;
}
