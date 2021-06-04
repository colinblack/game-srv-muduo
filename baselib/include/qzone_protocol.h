/*
 * qzone_protocol.h : Qzone协议统一接口
 * 
 *  Copyright (c) Tencent 2007.
 * 
 * History: 
  * 0.04, 9 -- 2008, geminiguan modified
 	Note:	1.增加AppendLongLong和FetchLongLong程序函数，打包和解包longlong型数据
 * 0.09, 10--2007, ianyang modified
 	Note:	1. 增加m_iErrno成员变量记录打包/解包是否出错, 并提供相应接口
 * 0.07, 31-may-2007, ianyang modified
 	Note:	1. 对于AppendData() & FetchData() 完善unsigned 类型支持, 无须强制转换
 			2. [FIXBUG] 修改AppendData() & FetchData() 的DataLen为unsigned
 * 0.05, 28-may-2007, ianyang modified
	Note:	1. 重新整理packet接口, 提供全黑盒式接口	
			2. 提供MapPacketBuffer() & UnmapPacketBuffer()作为映射数据包buffer的接口
			3. 提供映射协议头buffer获取协议头信息的接口
			4. 修改FetchData(), 改用memcpy传出数据
			5. 修改Input() & Output(), 避免多次重复调用风险
			6. 对于QzoneServerResponse增加QzoneClient标示协议为客户端请求, 区分协议类型
 * 0.03, 16-may-2007, ianyang modified
	Note: 	1. 去除client信息, 增加染色信息
 			2. 去除分包信息, 由系统设计人员保证若使用UDP则不使用分包设计
 			3. 考虑到64位和32位机器兼容, 修改所有ulong为uint
 			4. 去除重定向信息, 是否重定向合并到serverResponseFlag, 具体的重定向ip,port存放在协议体中
 			5. 增加类似CQzonePacket接口
 * 0.01, 15-may-2007, ianyang create
 * 
 */

#ifndef _QZONE_PROTOCOL_H_
#define _QZONE_PROTOCOL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string>

////add by geminiguan
#include <sys/types.h>
#include <endian.h>
#include <byteswap.h>
#include <netinet/in.h>


#if BYTE_ORDER == LITTLE_ENDIAN
#define htonq(x) bswap_64(x)
#elif BYTE_ORDER == BIG_ENDIAN
#define htonq(x)  (x)
#endif
///end add by geminiguan

using namespace std;

// -----------------------------------------------------------------------------
//
// server response 定义
//
// -----------------------------------------------------------------------------
typedef enum
{
	QzoneServerSucc = 0,	// 0 - [正常数据, 处理成功]
	QzoneServerFailed = 1,	// 1 - [正常数据, 处理失败]
	QzoneServerExc = 2,		// 2 - [异常数据, 服务器拒绝处理]
	QzoneServerBusy = 3,	// 3 - [正常数据, 服务器忙, 可重试]	
	QzoneServerRedirected = 10,	// 10 - [服务器重定向]
	QzoneServerAck = 20,	// 20 - [回执包]
	QzoneClient = 100,		// 100 - [client请求, 非server回应]
}QzoneServerResponse;

#define DefaultServResInfo 0



/*
 * protocol head
 ------------------------------------------------------------------------------------------------------------
| 版本(1byte) | 命令字(4 bytes) | 效验和(2 bytes) | 序列号(4 bytes) | 序列号(4 bytes) | 染色信息(4 bytes) | 
 ------------------------------------------------------------------------------------------------------------
| server回应标识(1 byte) | server回应信息(2 bytes) | 协议总长度(4bytes) | 协议体|
----------------------------------------------------------------------------------------
 */ 
#pragma pack(1)
typedef struct _QzoneProtocolHead_
{
	unsigned char version;
	unsigned int cmd;
	unsigned short checksum;
	
	unsigned int serialNo;				// 4 bytes, Protocol Serial Number, 由client生成,client效验	
	unsigned int colorration;			// 4 bytes, 染色信息	
	unsigned char serverResponseFlag;	// 1 byte, Server端回应标识 : 
										/* 	0 - [正常数据, 处理成功], 
											1 - [正常数据, 处理失败]
											2 - [异常数据, 服务器拒绝处理]
											3 - [正常数据, 服务器忙, 可重试]											
											10 - [服务器重定向]
											20 - [回执包],
											100 - [client请求, 非server回应]
										*/
	unsigned short serverResponseInfo;	// 2 bytes, Server端回应附加信息 
										/*	对于处理失败(1):  表示处理失败的错误号errcode
											对于服务器忙(3):  表示重试时间(网络字节序)
											对于服务器拒绝服务(2): 表示拒绝原因(网络字节序)
											其中, 服务器拒绝服务原因定义如下:
											使用的每bit表示不同的拒绝理由, 由低位字节至高分别定义为:		
												0x1: 当前协议版本
												0x2: 当前协议命令字
												0x4: 当前client类型
												0x8: 当前client版本
												0x10: 当前client子系统
												相应的位置1表示拒绝, 置0表示不拒绝, 如5位全为0表示无理由拒绝.
											例如, 服务器拒绝当前client类型的当前client版本, 则ServerResponseInfo的取值为0x12.
										*/
	char reserved[1];					// 预留
	
	unsigned int len;					// 协议总长度
	
	_QzoneProtocolHead_()
	{
		version = 0x0;
		cmd = 0;
		checksum = 0;
		serialNo = 0;
		colorration = 0;
		serverResponseFlag = 0;
		serverResponseInfo = 0;
		len = 0;
	}

	void Encode()
	{
		version = version;
		cmd = htonl(cmd);		
		serialNo = htonl(serialNo);
		colorration = htonl(colorration);
		serverResponseFlag = serverResponseFlag;
		serverResponseInfo = htons(serverResponseInfo);
		len = htonl(len);

		return ;
	}
	void Decode()
	{
		version = version;
		cmd = ntohl(cmd);		
		serialNo = ntohl(serialNo);
		colorration = ntohl(colorration);
		serverResponseFlag = serverResponseFlag;
		serverResponseInfo = ntohs(serverResponseInfo);
		len = ntohl(len);

		return ;
	}

	inline void operator = (_QzoneProtocolHead_ &head)
	{
		version = head.version;
		cmd = head.cmd;
		serialNo = head.serialNo;
		colorration = head.colorration;
		serverResponseFlag = head.serverResponseFlag;
		serverResponseInfo = head.serverResponseInfo;
		len = head.len;

		return ;
	}

	/*
	@brief:	设置协议头基本信息
	@param:	版本, 命令字
	*/
	inline void SetHead(unsigned char version_, unsigned int cmd_)
	{
		version = version_;
		cmd = cmd_;
		return ;
	}

	/*
	@brief:	设置协议头序列号
	@param:	序列号
	*/
	inline void SetSerialNo(unsigned int serialNo_)
	{
		serialNo = serialNo_;
		return ;
	}	

	/*
	@brief:	设置染色信息
	@param:	染色信息
	*/
	inline void SetColorration(unsigned int colorration_)
	{
		colorration = colorration_;
		return ;
	}

	/*
	@brief:	Server设置应答结果
	@param:	回应标识, 回应信息
	*/
	inline void SetServerResponse(QzoneServerResponse serverResponseFlag_, unsigned short serverResponseInfo_)
	{
		serverResponseFlag = serverResponseFlag_;
		serverResponseInfo = serverResponseInfo_;		
		return ;
	}

	/*
	@brief:	设置协议长度
	@param:	协议长度(包的长度)
	*/
	inline void SetLen(unsigned int len_)
	{
		len = len_;
		return ;
	}

	/*
	@brief:	效验合
	@param:	协议头+协议体的sendbuf, sendbuf长度
	*/
	inline unsigned short CheckSum(const void* buf, unsigned int bufLen)
	{
		unsigned int sum = 0;
		unsigned short* data = (unsigned short*) buf;
		
		int len = bufLen / 2;
		int mod = bufLen % 2;
		for(int i = 0;i < len; i++)
			sum += data[i];
		
		unsigned short nshort = 0;
		if (mod == 1)
		{
			/* bugfix, 2008-05-28, ianyang modified, char* => unsigned char* */
			nshort = (unsigned short) ((unsigned char*) buf)[bufLen - 1];
			sum += nshort;
		}		
		sum = (sum >> 16) + (sum & 0xffff);
		sum += (sum >> 16);
		nshort = ~((unsigned short)sum);		
		
		return nshort;		
	}
} QzoneProtocolHead, *QzoneProtocolHeadPtr;
#pragma pack()

// -----------------------------------------------------------------------------
//
// 协议结构
//
// -----------------------------------------------------------------------------

/*
 * 数据包的头尾标识
 */
#define QzoneProtocolSOH			0x04
#define QzoneProtocolEOT			0x05

/*
 *  protocol packet
 */
#pragma pack(1)

typedef struct 
{
	char soh;
	QzoneProtocolHead head;
	char body[];
	// ... char eot; 包结束
}QzoneProtocol, *QzoneProtocolPtr;

#pragma pack()



// -----------------------------------------------------------------------------
//
// qzone protocol packet; 提供类似CQZonePackage的接口
//
// -----------------------------------------------------------------------------
class QzoneProtocolPacket
{
public:
	QzoneProtocolPtr m_pPacket;
	
private:	
	unsigned int m_iPos;
	unsigned int m_iTail;
	unsigned int m_iBodyMaxLen;

	bool m_bInput;
	bool m_bOutput;

	bool m_bCreated;
    bool m_bNew;

	int m_iErrno;
	
public:
	QzoneProtocolPacket()
		:m_pPacket(NULL),
		m_iPos(0),
		m_iTail(0),
		m_iBodyMaxLen(0),
		m_bInput(false),
		m_bOutput(false),
		m_bCreated(false),
		m_bNew(false),
		m_iErrno(0)
	{
	}
	~QzoneProtocolPacket()
	{
		FreePacket();
	}
	

/// 设置协议头信息的接口
public:
	inline void SetHead(unsigned char version_, unsigned int cmd_)
	{		
		m_pPacket->head.SetHead(version_, cmd_);
		return ;
	}
	inline void SetSerialNo(unsigned int serialNo_)
	{
		m_pPacket->head.SetSerialNo(serialNo_);
		return ;
	}
	inline void SetColorration(unsigned int colorration_)
	{
		m_pPacket->head.SetColorration(colorration_);
		return ;
	}
	inline void SetServerResponse(QzoneServerResponse serverResponseFlag, unsigned short serverResponseInfo)
	{
		m_pPacket->head.SetServerResponse(serverResponseFlag, serverResponseInfo);
		return ;
	}
	inline void SetLen(unsigned int len_)
	{
		m_pPacket->head.SetLen(len_);
		return ;
	}
	inline void SetHead(QzoneProtocolHead &head_)
	{		
		memcpy((void*) &(m_pPacket->head), (void*) &head_, sizeof(QzoneProtocolHead));
		return ;
	}
	

/// 取协议头信息的接口
public:	
	inline unsigned char version()
	{
		return m_pPacket->head.version;
	}
	inline unsigned int cmd()
	{
		return m_pPacket->head.cmd;
	}
	inline unsigned int serialNo()
	{
		return m_pPacket->head.serialNo;
	}
	inline unsigned int colorration()
	{
		return m_pPacket->head.colorration;
	}
	inline unsigned char serverResponseFlag()
	{
		return m_pPacket->head.serverResponseFlag;
	}
	inline unsigned short serverResponseInfo()
	{
		return m_pPacket->head.serverResponseInfo;
	}
	inline unsigned int len()
	{
		return m_pPacket->head.len;
	}

/// 取协议长度、位置相关的接口
public:
	inline int bodySize()
	{
		return m_iTail;
	}
	inline int bodySize(unsigned int iPacketLen)
	{
		return (iPacketLen - sizeof(char) - sizeof(QzoneProtocolHead) - sizeof(char));
	}
	inline int bodyMaxLen()
	{
		return m_iBodyMaxLen;
	}
	inline int bodyPos()
	{
		return m_iPos;
	}
	inline int headSize()
	{
		return (sizeof(char) + sizeof(QzoneProtocolHead));
	}
	/*
	@brief: 得到packet的指针, 可以对其进行外部操作, 请谨慎使用
	*/
    inline char* packet()
    {
        return (char*)m_pPacket;
    }

	inline char* body()
	{
		return (char*)m_pPacket->body;
	}
	

/// 映射协议头buffer获取协议头信息的接口
public:
	inline int MapPacketHeadBuffer(const char* buf, unsigned int bufLen)
	{
		if (m_pPacket)
			return -1;

		if (bufLen < (sizeof(char) + sizeof(QzoneProtocolHead)))
			return -2;
		
		m_pPacket = (QzoneProtocolPtr) buf;
		m_iBodyMaxLen = bufLen - sizeof(char) - sizeof(QzoneProtocolHead);
		m_iTail = 0;
		m_iPos = 0;	
			
		return 0;
	}
	inline void UnmapPacketHeadBuffer()
	{
		m_pPacket = NULL;
		return ;
	}
	inline unsigned char mappedVersion()
	{
		return m_pPacket->head.version;
	}
	inline unsigned int mappedCmd()
	{
		return ntohl(m_pPacket->head.cmd);
	}
	inline unsigned int mappedSerialNo()
	{
		return ntohl(m_pPacket->head.serialNo);
	}
	inline unsigned int mappedColorration()
	{
		return ntohl(m_pPacket->head.colorration);
	}
	inline unsigned char mappedServerResponseFlag()
	{
		return m_pPacket->head.serverResponseFlag;
	}
	inline unsigned short mappedServerResponseInfo()
	{
		return ntohs(m_pPacket->head.serverResponseInfo);
	}
	inline unsigned int mappedLen()
	{
		return ntohl(m_pPacket->head.len);
	}	

/// map对应的packet buffer
public:
	inline int CheckPacketBuffer(const char* buf)
	{
		if (buf[0] != QzoneProtocolSOH)
			return -1;
		
		return 0;
	}
	inline int MapPacketBuffer(const char* buf, unsigned int bufLen)
	{
		if (m_pPacket)
			return -1;

		if (buf == NULL)
			return -2;
		
		m_pPacket = (QzoneProtocolPtr) buf;
		m_iBodyMaxLen = bufLen - sizeof(char) - sizeof(QzoneProtocolHead) - sizeof(char);
		
		return 0;
	}
	inline void UnmapPacketBuffer()
	{		
		m_pPacket = NULL;
		
		return ;
	}

/// 创建& 释放& 重置packet
public:
	inline int CreatePacket(unsigned int iBodyMaxLen)
	{
		if (m_pPacket)
			return -1;
		
		unsigned int len = sizeof(char) + sizeof(QzoneProtocolHead) + iBodyMaxLen + sizeof(char);
		m_pPacket = (QzoneProtocolPtr) malloc(len * sizeof(char));
		if (m_pPacket)
			memset(m_pPacket, 0, len);
		else
			return -2;
		
		m_iBodyMaxLen = iBodyMaxLen;
		m_iTail = 0;
		m_iPos = 0;

		m_bCreated = true;
        m_bNew = true;
		
		return 0;
	}	

    inline int CreatePacketEx(unsigned int iBodyMaxLen, char* pbuffer)
	{
		if (m_pPacket)
			return -1;
		
		unsigned int len = sizeof(char) + sizeof(QzoneProtocolHead) + iBodyMaxLen + sizeof(char);
		m_pPacket = (QzoneProtocolPtr)pbuffer;
		if (m_pPacket)
			memset(m_pPacket, 0, len);
		else
			return -2;
		
		m_iBodyMaxLen = iBodyMaxLen;
		m_iTail = 0;
		m_iPos = 0;

		m_bCreated = true;
        m_bNew = false;
		
		return 0;
	}	
	inline void FreePacket()
	{
		if (m_bCreated)
		{			
			if(m_pPacket != NULL && m_bNew)
			{
				free(m_pPacket);
				m_pPacket = NULL;				
			}
			m_bCreated = false;
		}

		return ;
	}

    bool GetOutput()
    {
        return m_bOutput;
    }
    
	inline void ResetPacket()
	{
        m_iErrno = 0;
		m_iTail = 0;
		m_iPos = 0;

		m_bInput = false;
		m_bOutput = false;

		return ;
	}


/// 打包, 解包相关接口
public:
	/*
	@brief:	解包相关接口, 必须先CreatePacket()
	@param:	packet的长度, 是否需要效验
	*/
	inline int Input(unsigned int iPacketLen, bool check = true)
	{
		if (m_bInput)
			return 1;

		if (!m_bCreated)
			return 2;

		if (!m_pPacket)
			return -1;

		if ((sizeof(char) + sizeof(QzoneProtocolHead) + m_iBodyMaxLen + sizeof(char)) < iPacketLen)
			return -2;

		if (packet()[0] != QzoneProtocolSOH)
			return -3;

		if (packet()[iPacketLen - 1] != QzoneProtocolEOT)
			return -4;

		m_iPos = 0;
		m_iTail = iPacketLen - (sizeof(char) + sizeof(QzoneProtocolHead) + sizeof(char));

		if (check)
		{
			// checksum		
			unsigned short checksum = m_pPacket->head.CheckSum((char*)m_pPacket, iPacketLen);
			if (checksum != 0x0 && checksum != 0xffff)
				return -5;
		}
		
		m_pPacket->head.Decode();
		m_bInput = true;

		return 0;
	}
	/*
	@brief:	解包相关接口, 不推荐使用, 因为需要外部保证buffer的存在
	@param:	数据包buffer, buffer长度, 是否需要效验
	*/
	inline int Input(const char* buf, unsigned int bufLen, bool check = true)
	{
		if (m_bInput)
			return 1;

		if (m_bCreated)
			return 2;
		
		if (buf == NULL)
			return -1;
		
		if (bufLen < (sizeof(char) + sizeof(QzoneProtocolHead) + sizeof(char)))
			return -2;

		if (buf[0] != QzoneProtocolSOH)
			return -3;

		if (buf[bufLen - 1] != QzoneProtocolEOT)
			return -4;
		
		m_pPacket = (QzoneProtocolPtr) buf;		
		m_iPos = 0;
		m_iTail = bufLen - sizeof(char) - sizeof(QzoneProtocolHead) - sizeof(char);
		m_iBodyMaxLen = m_iTail;		

		if (check)
		{
			// checksum		
			unsigned short checksum = m_pPacket->head.CheckSum(buf, bufLen);
			if (checksum != 0x0 && checksum != 0xffff)
				return -5;
		}
		
		m_pPacket->head.Decode();
		m_bInput = true;

		return 0;
	}

	/*
	@brief:	打包相关接口
	@param:	打包的buffer指针, buffer的长度, 是否需要效验
	*/
	inline int Output(char* &packetBuf, int &packetLen, bool check = true)
	{
		if (m_bOutput)
		{
			packetBuf = (char*) m_pPacket;
			packetLen = sizeof(char) + sizeof(QzoneProtocolHead) + m_iTail + sizeof(char);
			return 1;
		}
		
		// set soh & eot
		*soh() = QzoneProtocolSOH;
		*eot() = QzoneProtocolEOT;

		// set len
		packetLen = sizeof(char) + sizeof(QzoneProtocolHead) + m_iTail + sizeof(char);		
		m_pPacket->head.SetLen(packetLen);		

		// encode head
		m_pPacket->head.Encode();
		packetBuf = (char*) m_pPacket;
		
		// set checksum
		if (check)
		{
			m_pPacket->head.checksum = 0;
			m_pPacket->head.checksum = m_pPacket->head.CheckSum(packetBuf, packetLen);
		}
		
		m_bOutput = true;

		return 0;
	}

/// 设置/取包数据接口
public:
	inline bool IsProtocolError()
	{
		if (m_iErrno == 0)
			return false;
		else
			return true;
	}
	inline void ResetProtocolError()
	{
		m_iErrno = 0;
		return ;
	}
	inline void SetProtocolError()
	{
		m_iErrno = -100;
		return ;
	}
	
	inline int AppendByte(char cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(char)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		m_pPacket->body[m_iPos] = cByteParam;
		m_iPos += sizeof(char);

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}
		return 0;
	}	
	inline int FetchByte(char &cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(char)) > m_iTail)
			return (m_iErrno = -1);

		cByteParam = m_pPacket->body[m_iPos];
		m_iPos += sizeof(char);

		return 0;
	}
	
	inline int AppendByte(unsigned char cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned char)) > m_iBodyMaxLen)
			return (m_iErrno = -1);		
		
		m_pPacket->body[m_iPos] = cByteParam;
		m_iPos += sizeof(unsigned char);

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}
		return 0;
	}
	inline int FetchByte(unsigned char &cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned char)) > m_iTail)
			return (m_iErrno = -1);

		cByteParam = m_pPacket->body[m_iPos];
		m_iPos += sizeof(unsigned char);

		return 0;
	}

	inline int AppendShort(short usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(short)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		usParam = htons(usParam);
		memcpy(&m_pPacket->body[m_iPos], &usParam, sizeof(short));
		m_iPos += sizeof(short);		

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchShort(short &usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(short)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&usParam, &m_pPacket->body[m_iPos], sizeof(short));
		usParam = ntohs(usParam);
		m_iPos += sizeof(short);

		return 0;
	}

	inline int AppendShort(unsigned short usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned short)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		usParam = htons(usParam);
		memcpy(&m_pPacket->body[m_iPos], &usParam, sizeof(short));
		m_iPos += sizeof(unsigned short);		

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchShort(unsigned short &usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned short)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&usParam, &m_pPacket->body[m_iPos], sizeof(short));
		usParam = ntohs(usParam);
		m_iPos += sizeof(unsigned short);

		return 0;
	}
	
	inline int AppendInt(int uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(int)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		uiParam = htonl(uiParam);
		memcpy(&m_pPacket->body[m_iPos], &uiParam, sizeof(int));
		m_iPos += sizeof(int);

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchInt(int &uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(int)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&uiParam, &m_pPacket->body[m_iPos], sizeof(int));
		uiParam = ntohl(uiParam);
		m_iPos += sizeof(int);

		return 0;
	}

	inline int AppendInt(unsigned int uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned int)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		uiParam = htonl(uiParam);
		memcpy(&m_pPacket->body[m_iPos], &uiParam, sizeof(int));
		m_iPos += sizeof(unsigned int);

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchInt(unsigned int &uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned int)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&uiParam, &m_pPacket->body[m_iPos], sizeof(int));
		uiParam = ntohl(uiParam);
		m_iPos += sizeof(unsigned int);

		return 0;
	}

	//////add by geminiguan
	inline int AppendLongLong(unsigned long long uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned long long)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		uiParam = htonq(uiParam);
		memcpy(&m_pPacket->body[m_iPos], &uiParam, sizeof(unsigned long long));
		m_iPos += sizeof(unsigned long long);

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchLongLong(unsigned long long &uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned long long)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&uiParam, &m_pPacket->body[m_iPos], sizeof(unsigned long long));
		uiParam = htonq(uiParam);
		m_iPos += sizeof(unsigned long long);

		return 0;
	}
	
/////end add by geminiguan

	inline int AppendData(const char* pData, unsigned int iLen)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + iLen) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		if (pData == NULL)
			return (m_iErrno = -2);

		memcpy(&m_pPacket->body[m_iPos], pData, iLen);
		m_iPos += iLen;

		//如果包尾小于当前位置，则做调整
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchData(char *&pData, unsigned int iLen)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + iLen) > m_iTail)
			return (m_iErrno = -1);
		
		//pData = &m_pPacket->body[m_iPos];
		memcpy(pData, &m_pPacket->body[m_iPos], iLen);
		m_iPos += iLen;

		return 0;
	}
	/// 不推荐使用
	inline int FetchDataPos(char *&pData, unsigned int iLen)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + iLen) > m_iTail)
			return (m_iErrno = -1);
		
		pData = &m_pPacket->body[m_iPos];
		m_iPos += iLen;
		
		return 0;
	}

private:
	inline char* soh()
	{
		return (char*) m_pPacket;
	}
	inline char* eot()
	{
		char *p = (char*) m_pPacket;
		p += (sizeof(char) + sizeof(QzoneProtocolHead) + m_iTail);
		return p;
	}	

/// 取serverResponse错误信息
public:
	inline void TranslateServerResponseInfo(char * pInfo, unsigned int iInfoLen)
	{
		switch (serverResponseFlag())
		{										
			// 正常数据, 处理成功
			case QzoneServerSucc:
				snprintf(pInfo, iInfoLen, "SUCCESS.");
				return ;

			// 正常数据, 处理失败
			case QzoneServerFailed:
				// 对于处理失败(1):  表示处理失败的错误号errcode
				snprintf(pInfo, iInfoLen, "FAILED, ERRCODE=%d.", serverResponseInfo());
				return ;

			// 异常数据, 服务器拒绝处理
			case QzoneServerExc:
				snprintf(pInfo, iInfoLen, "DATA EXC, ERRINFO=%d.", serverResponseInfo());
				return ;

			// 正常数据, 服务器忙, 可重试
			case QzoneServerBusy:
				// 对于服务器忙(3):  表示重试时间
				snprintf(pInfo, iInfoLen, "SERVER BUSY, RETRYTIME=%d.", serverResponseInfo());
				return ;

			// 服务器重定向
			case QzoneServerRedirected:
				// 服务器重定向
				snprintf(pInfo, iInfoLen, "SERVER REDIRECTED.");
				return ;

			// 回执包
			case QzoneServerAck:
				snprintf(pInfo, iInfoLen, "ACK.");
				return ;

			// client请求, 非server回应
			case QzoneClient:
				snprintf(pInfo, iInfoLen, "CLIENT QUERY.");
				return ;

			default:
				snprintf(pInfo, iInfoLen, "UNKNOWN.");
				return ;
		}

		return ;
	}

	
};

#endif

