/*
 * Packet.cpp
 *
 *  Created on: 2012-2-10
 *      Author: dada
 */

#include "FirePacket.h"
#include "AppDefine.h"
#include "Util.h"

#define PACKET_HEAD_MAGIC_NUMBER 21961

CFirePacket::CFirePacket(uint16_t c, bool d)
{
	channelId = -1;
	head = PACKET_HEAD_MAGIC_NUMBER;
	bodyLen = 0;
	cmd = c;
	fd = 0;
	time = 0;
	microTime = 0;
	clientIp = 0;
	m_decodeSize = 0;
	uid = 0;
	delmsg = d;

	m_msg = NULL;
	group = NULL;
}

CFirePacket::~CFirePacket()
{
	if(delmsg && m_msg)
		delete m_msg;
	if(group)
		delete group;
}

bool CFirePacket::Encode(Buffer *pBuffer)
{
	if(pBuffer == NULL)
	{
		return false;
	}
	CBufferWriter writer(pBuffer);
//	writer.SetSwapByte(true);

	assert(m_msg != nullptr);
	bodyLen = m_msg->GetTypeName().size() + 2 + m_msg->ByteSize() + sizeof(m_msg->ByteSize());
	PACKET_ENCODE(UInt16, PACKET_HEAD_MAGIC_NUMBER);

	debug_log("head=%x", PACKET_HEAD_MAGIC_NUMBER);
	PACKET_ENCODE(UInt32,bodyLen);
		debug_log("bodyLen=%x", bodyLen);
	PACKET_ENCODE(UInt16,cmd);
		debug_log("cmd=%x", cmd);
	PACKET_ENCODE(UInt32,fd);
		debug_log("fd=%x", fd);
	PACKET_ENCODE(UInt32,time);
		debug_log("time=%x", time);
	PACKET_ENCODE(UInt32,microTime);
		debug_log("microTime=%x", microTime);
	PACKET_ENCODE(UInt32,clientIp); //client ip
		debug_log("clientIp=%x", clientIp);

   if(cmd == PROTOCOL_ACCESS_ANSWER
	|| cmd == PROTOCOL_ACCESS_BROAD_CAST
	|| cmd == PROTOCOL_ACCESS_SEND
	|| cmd == PROTOCOL_ACCESS_GROUP_SEND
	|| cmd == PROTOCOL_DELIVER
	|| cmd == PROTOCOL_ADMIN
	|| cmd == PROTOCOL_BOT
	|| cmd == PROTOCOL_EVENT_BATTLE_CONNECT
	|| cmd == PROTOCOL_EVENT_BATTLE_FORWARD)
	{
		if(!EncodePB(pBuffer))
			return false;
	}


	debug_log("%s", String::b2s((const char*)pBuffer->peek(), pBuffer->readableBytes()).c_str());
	//info_log("%s",String::b2s((const char*)pBuffer->GetConstBuffer(),pBuffer->GetSize()).c_str());

	return true;
}

bool CFirePacket::Decode(Buffer *pBuffer)
{
	m_decodeSize = 0;
	if(pBuffer == NULL)
	{
		return false;
	}
	CBufferReader reader(pBuffer);
	size_t readableSize = pBuffer->readableBytes();
	if(readableSize == 0){
		error_log("no data");
		return false;
	}

	PACKET_DECODE(UInt16, head);
	debug_log("readableSize=%d, head=%d", readableSize, head);

	if(head != PACKET_HEAD_MAGIC_NUMBER)
	{
		error_log("get head failed:%x",head);
		return false;
	}

	if(PACKET_HEADER_SIZE > readableSize)
	{
		error_log("readable data short");
		return false;
	}


	PACKET_DECODE(UInt32, bodyLen);
	debug_log("bodyLen=%d", bodyLen);
	
	PACKET_DECODE(UInt16, cmd);
	debug_log("cmd=%d", cmd);

	PACKET_DECODE(UInt32, fd);
	debug_log("fd=%d", fd);

	PACKET_DECODE(UInt32, time);
	debug_log("time=%d", time);

	PACKET_DECODE(UInt32, microTime);
	debug_log("microTime=%d", microTime);

	PACKET_DECODE(UInt32, clientIp);
	debug_log("clientIp=%d", clientIp);

	debug_log("readable=%d,  readableSize=%d", PACKET_HEADER_SIZE + bodyLen, readableSize);
	if((size_t)(PACKET_HEADER_SIZE + bodyLen) > readableSize){
		error_log("package is invalid");
		return false;
	}	

	m_decodeSize = bodyLen + PACKET_HEADER_SIZE;
	debug_log("decodesize=%d", m_decodeSize);

//	debug_log("body len=%u-%u:\n%s",bodyLen,body.GetSize(),body.ToString().c_str());

	if(cmd == PROTOCOL_ACCESS_TRANSFER
	|| cmd == PROTOCOL_DELIVER
	|| cmd == PROTOCOL_ADMIN
	|| cmd == PROTOCOL_BOT
	|| cmd == PROTOCOL_EVENT_BATTLE_CONNECT
	|| cmd == PROTOCOL_EVENT_BATTLE_FORWARD){
		return DecodePB(pBuffer, m_decodeSize-PACKET_HEADER_SIZE);	
	}

	return true;
}


bool CFirePacket::EncodePB(Buffer *pBuffer)
{
	CSendCProtocol sp;
	sp.msg = m_msg;
	sp.group = group;
	return sp.Encode(pBuffer);
}

bool CFirePacket::DecodePB(Buffer *pBuffer, uint32_t size)
{
	CReceiveProtocol rp;
	if(!rp.Decode(pBuffer, size))
		return false;
	m_msg = rp.msg;
	return true;
}