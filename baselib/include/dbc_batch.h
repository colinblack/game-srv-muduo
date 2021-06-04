#ifndef __DBC_BATCH_H_SHAWXIAO_2009_09_10__
#define __DBC_BATCH_H_SHAWXIAO_2009_09_10__

#include "dbcapi.h"
#include "route.h"

typedef void (*FUNC_FILLDATA)( DBC::Result& dbcret, int argc, void** argv );

class CDBCBatch
{
public:
	CDBCBatch(){}
	virtual ~CDBCBatch();
	
public:
	int Init( const char* path, const char* key, int maxkey );
	DBC::Request* GetRequest( int uin );
    
	 /* 
	 note: 调用前需要调用SetLsVoteId，以确保模块间调用数据正常上报
	 */
	int Exec( FUNC_FILLDATA func, int argc, void** argv );

	/*
       note: 设置模块间调用ID
       */
	void SetLsVoteId(int masterid, int slaveid, int interfaceid);
	void Reset();


public:
	CRoute _route;
	std::vector<DBC::Server*> _dbcsrv;
	std::vector<DBC::Request*> _dbcreq;
	std::vector<DBC::Result*> _dbcret;
	std::vector<int> _reqcnt;
};


#endif

