#ifndef __DBC_KEEPALIVE_H_
#define __DBC_KEEPALIVE_H_

#define DEFAULT_MAX_DBCHANDLE	100		/* 默认100个DBC长连接 */

#define PATH_XML_KEEPALIVE			"/data/eric/AutoMaker/release/auto/conf/dbc_keepalive.xml"

class CDBCKeepAlive
{
public:
	CDBCKeepAlive() : _max(DEFAULT_MAX_DBCHANDLE){};
	~CDBCKeepAlive(){};

public:
	int LoadConfig( const char* tag, const char* path=PATH_XML_KEEPALIVE );
	int GetMaxHandle(){ return _max; };

public:
	static CDBCKeepAlive _ka;

protected:
	int _max;
};

#endif

