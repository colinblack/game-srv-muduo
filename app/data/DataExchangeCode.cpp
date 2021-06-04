/*
 * DataExchangeCode.cpp
 *
 *  Created on: 2012-12-5
 *      Author: Administrator
 */

#include "DataExchangeCode.h"


CDataExchangeCode::~CDataExchangeCode() {
	// TODO Auto-generated destructor stub
}

int CDataExchangeCode::AddExchageCode( const DataExchangeCode &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.code.c_str());
	DBCREQ_SET_INT(data, type);
	DBCREQ_SET_INT(data, uid);
	DBCREQ_SET_INT(data, gentime);
	DBCREQ_SET_INT(data, deadline);
	DBCREQ_SET_INT(data, usetime);
	DBCREQ_EXEC;
	return 0;

}
int CDataExchangeCode::GetExchageCode( DataExchangeCode &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.code.c_str());

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(code);
	DBCREQ_NEED(type);
	DBCREQ_NEED(uid);
	DBCREQ_NEED(gentime);
	DBCREQ_NEED(deadline);
	DBCREQ_NEED(usetime);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_STR(data, code);
	DBCREQ_GET_INT(data, type);
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, gentime);
	DBCREQ_GET_INT(data, deadline);
	DBCREQ_GET_INT(data, usetime);

	return 0;
}
int CDataExchangeCode::SetExchageCode( const DataExchangeCode &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.code.c_str());
	DBCREQ_SET_INT(data, type);
	DBCREQ_SET_INT(data, uid);
	DBCREQ_SET_INT(data, gentime);
	DBCREQ_SET_INT(data, deadline);
	DBCREQ_SET_INT(data, usetime);
	DBCREQ_EXEC;
	return 0;
}
