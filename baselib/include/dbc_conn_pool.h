/*=============================================================================
#  Author:          DanteZhu
#  Email:           dantezhu@vip.qq.com
#  FileName:        dbc_conn_pool.h
#  Version:         1.0
#  LastChange:      2010-01-12 18:43:21
#  Description:     DBC链接池管理
#  History:         
=============================================================================*/
#ifndef _DBC_CONN_POOL_H_
#define _DBC_CONN_POOL_H_
#include <iostream>
#include <list>
#include <string>
#include <memory>
#include "dbcapi.h"
using namespace std;
class CDBCConnPool
{
	public:
		static auto_ptr<CDBCConnPool> m_auto_ptr;
		static CDBCConnPool* _ins;      //0 没有初始化

	public:
		static CDBCConnPool* Ins()
		{
			if ( !_ins )
			{
				try
				{
					_ins = new CDBCConnPool();
				}
				catch(...)
				{
					return NULL;
				}
			}
			return _ins;	
		}
		void SetMaxConnNum(int num);

		int push_back(DBC::Server *pServer);

	private:
		CDBCConnPool();
	private:
		list<DBC::Server *> m_pDBCConList;
		int m_MaxConnNum;
};
#endif
