#ifndef _WX_COMM_H_
#define _WX_COMM_H_

#define MAGIC_HEADER 21961
#define MAGIC_ADDER	5834

#define PROTOCOL_ACCESS_TRANSFER 1 //access转发client
#define PROTOCOL_ACCESS_ANSWER 2 //发access回复clinet
#define PROTOCOL_ACCESS_HEARBEAT 3 //心跳，似乎无用
#define PROTOCOL_ACCESS_COFFLINE 4 //客户端下线
#define PROTOCOL_ACCESS_LOFFLINE 5 //服务器强制客户端下线
#define PROTOCOL_ACCESS_CHANGE_LOGIN 6 //无用
#define PROTOCOL_ACCESS_KICK_CROSS 7 //无用
#define PROTOCOL_ACCESS_BROAD_CAST 8 //广播信息用
#define PROTOCOL_ACCESS_SEND 9 //主动推送
#define PROTOCOL_DELIVER 10 //支付
#define PROTOCOL_ADMIN 11 //GM
#define PROTOCOL_BOT 12 //bot
#define PROTOCOL_ACCESS_GROUP_SEND 13 //群发

#define DAWX_RALF_KEY "954d7110c0d2b072565ffdbcbc4a0e2b"


#pragma pack(1)

struct ClientHead{
	uint16_t head;
	uint32_t bodyLen;
	uint16_t crc1;
	uint16_t crc2;
	int32_t padding1;
	int32_t padding2;
};

struct BattleHead{
	uint16_t head;
	uint32_t bodyLen;
	uint16_t cmd;
	uint32_t fd;
	uint32_t time;
	uint32_t microTime;
	uint32_t clientIp;
};

#pragma pack()

#include "Admin.pb.h"
#include "Common.pb.h"
#include "ProtoArchive.pb.h"
#include "ProtoUser.pb.h"

#endif
