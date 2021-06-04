#ifndef __ONLINE_COUNT_H_SHAWXIAO_2009_08_31__
#define __ONLINE_COUNT_H_SHAWXIAO_2009_08_31__

#include <stdint.h>
#include "route.h"

#define ONLINE_VERSION_V2          0x10
#define ONLINE_CMD_LOGIN           20
#define ONLINE_MQZONE_APPID    35    // 手机农场在线统计


class COnlineCount
{
public:
	COnlineCount(){ _status = -1; }

public:
	/*包头，len表示发送内容的长度,包含OnlineHead长度*/
	typedef struct {
	    uint32_t       ver;
	    uint32_t       len;
	    uint32_t       cmd;
	} __attribute__((packed)) OnlineHead;
	/* time: 登陆/验证的时间
	 * qq: 登陆的QQ
	 * ip: 用户IP
	 * appid: 业务的appid
	 * extend: 保留扩展字段*/
	typedef struct {
	    uint32_t   time;
	    uint32_t   qq;
	    uint32_t   ip;
	    uint32_t   appid; /* 来源，校友为9，空间为15 */
	    int32_t    extend;
	} __attribute__((packed)) LoginReqBody;
	typedef struct {
    	OnlineHead head;
    	LoginReqBody body;
	} LoginReq;

	enum emAppId
	{
		car = 1,
	};

	enum emExtend
	{
		writeRequest = 0,
		readRequest = 1,
		writeOther = 2,
		readOther = 3,
	};

public:
	/* app: 1-农场, 2-牧场 */
	int Report( int uin, int domain, int clientip, int app=car, int extend=writeRequest,
				const int pn_TJ2AppId = 0 );
	static COnlineCount* GetInstance()
	{
		if ( _pInstance == 0 )
		{
			try
			{
				_pInstance = new COnlineCount();
			}
			catch(...)
			{
				_pInstance = NULL;
			}
		}
		return _pInstance;
	};

protected:
	int Init( int app );
	int Source( int domain, int app );

public:
	char _err[128];
	int _status;
	CRoute _route;

public:
	static COnlineCount* _pInstance;
};

#endif

