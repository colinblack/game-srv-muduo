#ifndef __ROUTE_H_SHAWXIAO_2009_08_03__
#define __ROUTE_H_SHAWXIAO_2009_08_03__

#include <vector>
#include <string>

/*
	针对号码采取 code/m%n 的方式路由
*/
#define MAX_ERROR_MSG_LEN  512

class CRoute
{
public:
	typedef struct
	{
	int mod;
	int div;
	int tout;			//秒
	int tout_usec;	//微秒
    int keep_conn;  // 是否保持长连接, 0: 短连接, 非0: 长连接
	}_HEAD;

	typedef struct
	{
	int index;
	int begin;
	int end;
	char ip[20];
	int port;
	char table[20];
	char prot[10];
	}_INFO;


	struct ConfigInfo
	{
	int  mn_BlockNum;            // server 分组数
	int  mn_ServerTotalNum;      // 后端逻辑 server 总数
	int  mn_ServerNum;           // 每组内包含的 server 数量
	int  mn_LastServerNum;       // 最后一组内包含的 server 数量
	};


public:
	int initialize( const int &key, char** tags=NEW_TAG );
	int initialize( const char* path, char** tags=NEW_TAG );
	int InitLogicServer( const char* path);

	_INFO* GetRouteInfo( int code );
	int GetRouteInfoSize(){ return m_info.size(); };
	int RouteID(int uid);
	char* GetErrMsg(){return m_errMsg;}
public:
	_HEAD m_head;
	std::vector<_INFO> m_info;
	ConfigInfo  ms_ConfigInfo;

public:
	char	m_errMsg[MAX_ERROR_MSG_LEN];
	static char* OLD_TAG[3];
	static char* NEW_TAG[3];
};

typedef std::vector<CRoute::_INFO>  _RInfoVector;

#endif

