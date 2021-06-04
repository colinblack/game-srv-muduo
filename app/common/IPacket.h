#ifndef _IPACKET_H_
#define _IPACKET_H_

#include "Common.h"

class IPacket : public IBase
{
public:
	virtual string ToString() const
	{
		return "";
	}
};

class IEncodable : public IBase
{
public:
	virtual bool Encode(IBuffer *pBuffer) = 0;
};

class IDecodable : public IBase
{
public:
	virtual bool Decode(IBuffer *pBuffer) = 0;
};

class IPacketSend : public IPacket, public IEncodable
{
};

/// 协议编解码
#define DEFINE_SEND_PACKET_BEGIN(Name)	\
	class Name : public IPacketSend	\
	{	\
	public:	\

#define DEFINE_SEND_PACKET_END	\
	};	\

#define DEFINE_EMPTY_SEND_PACKET(Name)	\
	class Name : public IPacketSend	\
	{	\
	public:	\
		virtual bool Encode(IBuffer *pBuffer) { return true; }	\
	};	\

#define IMPLEMENT_PACKET_ENCODE_BEGIN	\
	virtual bool Encode(IBuffer *pBuffer)	\
	{	\
		if(pBuffer == NULL)	\
		{	\
			return false;	\
		}	\
		pBuffer->Clear();	\
		CBufferWriter writer(pBuffer);	\
		writer.SetSwapByte(false);	\

#define IMPLEMENT_PACKET_ENCODE_END	\
		return true;	\
	}	\

#define PACKET_ENCODE(type, ...)	if(!writer.Append##type(__VA_ARGS__)) return false

#define DEFINE_RECEIVE_PACKET_BEGIN(Name)	\
	class Name 	\
	{	\
	public:	\

#define DEFINE_RECEIVE_PACKET_END	\
	};	\

#define DEFINE_EMPTY_RECEIVE_PACKET(Name)	\
	class Name	\
	{	\
	public:	\
		virtual bool Decode(IBuffer *pBuffer) { return true; }	\
	};	\

#define IMPLEMENT_PACKET_DECODE_BEGIN	\
	virtual bool Decode(IBuffer *pBuffer)	\
	{	\
		if(pBuffer == NULL)	\
		{	\
			return false;	\
		}	\
		CBufferReader reader(pBuffer);	\
		reader.SetSwapByte(false);	\

#define IMPLEMENT_PACKET_DECODE_END	\
		return true;	\
	}	\

#define PACKET_DECODE(type, ...)	if(!reader.Get##type(__VA_ARGS__)) return false

#define PACKET_DECODE_SKIP(offset)	if(!reader.Skip(offset)) return false

#define DEFINE_PROTOCOL_TYPE(MainType, SubType)	\
	static const byte MainProtocolType = MainType;	\
	static const byte SubProtocol = SubType;	\
	static const uint16_t ProtocolType = COMBINE_PROTOCOL_TYPE(MainType, SubType);	\

#define IMPLEMENT_TO_STRING(format, ...)	\
	 string ToString() const	\
	{	\
		string s;	\
		String::Format(s, format, __VA_ARGS__);	\
		return s;	\
	}	\


#define PACKET_ENCODE_END_OF_STR PACKET_ENCODE(Byte, 0)

#endif

