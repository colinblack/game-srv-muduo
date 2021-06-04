/*=============================================================================
#  Author:          DanteZhu
#  Email:           dantezhu@vip.qq.com
#  FileName:        dbc_route_table_ex.h
#  Version:         1.0
#  LastChange:      2010-01-12 20:18:40
#  Description:     DBC的路由控制类
#  History:         
=============================================================================*/
#ifndef _DBC_ROUTE_TABLE_EX_H_
#define _DBC_ROUTE_TABLE_EX_H_
#include "dbcapi.h"
#include "dbc_conn_pool.h"
#include "markupstl.h"

#define DBCROUTETABLEEX_MAXERRLEN 512
//=============================================================================
typedef struct _dbc_route_node_tag_
{
	_dbc_route_node_tag_()
	{
		beginUin = 0;
		endUin = 0;
		strIp="";
		strPort="";
		strProtocol="";
		strTable="";
		iTimeOut = 0;
		iMTimeOut = 0;
		pServer = NULL;
	}
	unsigned int beginUin;
	unsigned int endUin;
	string strIp;
	string strPort;
	string strProtocol;
	string strTable;
	int  iTimeOut;
	int iMTimeOut;
	DBC::Server* pServer;
}DBCRouteNode;
//=============================================================================
class CDBCRouteInfo
{
	public:
		CDBCRouteInfo();
		~CDBCRouteInfo();
		int Init(int div,int mod);
		void push_back(DBCRouteNode node);
		int GetClient(unsigned int iUin,DBC::Server *&pServer);
		int GetHostInfo(unsigned int iUin,DBCRouteNode& node);
	private:
		void FreeSelf();
	private:
		int					m_Div_Num;			//	被除数
		int 				m_Mod_Num;			//	模数
		std::vector<DBCRouteNode>	m_vec_host;	//	对象列表,可能是各种对象
};
//=============================================================================
class CDBCRouteMng
{
	public:
		static auto_ptr<CDBCRouteMng> m_auto_ptr;
		static CDBCRouteMng* _ins;      //0 没有初始化
		~CDBCRouteMng();

	public:
		static CDBCRouteMng* Ins();
		int AddNewRoute(string strFilePath);//当要读取一个新的路由配置文件的时候
		int GetClient(string strFilePath,unsigned int iUin,DBC::Server *&pServer);
		int GetHostInfo(string strFilePath,unsigned int iUin,DBCRouteNode& node);
		char* GetErrMsg()
		{
			return m_StrErrMsg;
		}

	private:
		CDBCRouteMng();
	private:
		char m_StrErrMsg[DBCROUTETABLEEX_MAXERRLEN];
		map<string,CDBCRouteInfo*> m_MapDBCRoute;
};
//=============================================================================
#endif
