/*
 * Packet.h
 *
 *  Created on: 2012-2-10
 *      Author: dada
 */

#ifndef _LIP_FIRE_PACKET_H_
#define _LIP_FIRE_PACKET_H_

#include "Common.h"
#include "IPacket.h"

#define PACKET_HEADER_SIZE sizeof(BattleHead)
#define MAX_PROTOCOL_BODY_SIZE 0x20000
#define MAX_PACKET_SIZE (PACKET_HEADER_SIZE + MAX_PROTOCOL_BODY_SIZE + sizeof(byte))


// 协议包头数据
class CFirePacket{
public:
	uint16_t head;
	uint32_t bodyLen;
	uint16_t cmd;
	uint32_t fd;
	uint32_t time;
	uint32_t microTime;
	uint32_t clientIp;	//原始客户端的ip

	uint32_t uid;
	bool delmsg;
	int32_t channelId;
	set<unsigned>* group;

	CFirePacket(uint16_t c = 0, bool d = true);
	~CFirePacket();

	bool Encode(Buffer *pBuffer);
	bool Decode(Buffer *pBuffer);

	uint32_t GetDecodeSize() {
		return m_decodeSize;
	}

	IMPLEMENT_TO_STRING("cid=%d&len=%u&cmd=%u&fd=%u&time=%u&microTime=%u",channelId, bodyLen,cmd,fd,time,microTime);
/* 	auto f = [&]{
		std::cout << uid << std::endl;
	}; */


	Message* m_msg;
	bool EncodePB(Buffer *pBuffer);
	bool DecodePB(Buffer *pBuffer, uint32_t size);

private:
	uint32_t m_decodeSize;

};


class CReceiveProtocol{
public:
	Message* msg;
	bool Decode(Buffer *pBuffer, uint32_t size)	
	{	
		if(pBuffer == NULL)	
		{	
			error_log("buff is null ");
			return false;	
		}	
		CBufferReader reader(pBuffer);	
		if(!reader.GetPBMsg(msg, size)) 
			return false;

		return true;
	}
};

class CSendCProtocol{
public:
	Message* msg;
	set<unsigned>* group;
	bool Encode(Buffer *pBuffer)	
	{	
		if(pBuffer == NULL)	
		{	
			return false;	
		}	
		CBufferWriter writer(pBuffer);	
		if(group && group->size())
		{
			if(!writer.AppendUInt32(group->size())) 
				return false;
			for(set<unsigned>::iterator it=group->begin();it!=group->end();++it)
			{
				if(!writer.AppendUInt32(*it)) 
					return false;
			}
		}
		if(!writer.AppendPBMsg(msg)) 
			return false;

		return true;
	}
};



#endif /* PACKET_H_ */
