/*
 * DataTruck.cpp
 *
 *  Created on: 2018-3-5
 *      Author: Administrator
 */

#include "DataTruck.h"
#include "DataInc.h"

int CDataTruck::Get(DataTruck &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(slot);
	DBCREQ_NEED(storage_id);
	DBCREQ_NEED(level_id);
	DBCREQ_NEED(order_id);
	DBCREQ_NEED(state);
	DBCREQ_NEED(coin);
	DBCREQ_NEED(exp);
	DBCREQ_NEED(endts);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, slot);
	DBCREQ_GET_INT(data, storage_id);
	DBCREQ_GET_INT(data, level_id);
	DBCREQ_GET_INT(data, order_id);
	DBCREQ_GET_INT(data, state);
	DBCREQ_GET_INT(data, coin);
	DBCREQ_GET_INT(data, exp);
	DBCREQ_GET_INT(data, endts);

	return 0;
}

int CDataTruck::Add(DataTruck &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_INT(data, slot);
	DBCREQ_SET_INT(data, storage_id);
	DBCREQ_SET_INT(data, level_id);
	DBCREQ_SET_INT(data, order_id);
	DBCREQ_SET_INT(data, state);
	DBCREQ_SET_INT(data, coin);
	DBCREQ_SET_INT(data, exp);
	DBCREQ_SET_INT(data, endts);
	DBCREQ_EXEC;
	return 0;
}

int CDataTruck::Set(DataTruck &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_INT(data, slot);
	DBCREQ_SET_INT(data, storage_id);
	DBCREQ_SET_INT(data, level_id);
	DBCREQ_SET_INT(data, order_id);
	DBCREQ_SET_INT(data, state);
	DBCREQ_SET_INT(data, coin);
	DBCREQ_SET_INT(data, exp);
	DBCREQ_SET_INT(data, endts);
	DBCREQ_EXEC;
	return 0;
}

int CDataTruck::Del(DataTruck &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_EXEC;
	return 0;
}


