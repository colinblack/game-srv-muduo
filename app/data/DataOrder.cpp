/*
 * DataOrder.cpp
 *
 *  Created on: 2018-2-28
 *      Author: Administrator
 */
#include "DataOrder.h"

int CDataOrder::Get(DataOrder &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(storage_id);
	DBCREQ_NEED(level_id);
	DBCREQ_NEED(order_id);
	DBCREQ_NEED(coin);
	DBCREQ_NEED(exp);
	DBCREQ_NEED(end_ts);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, storage_id);
	DBCREQ_GET_INT(data, level_id);
	DBCREQ_GET_BINARY(data, order_id, ORDER_LENGTH);
	DBCREQ_GET_INT(data, coin);
	DBCREQ_GET_INT(data, exp);
	DBCREQ_GET_INT(data, end_ts);

	return 0;
}

int CDataOrder::Get(vector<DataOrder> &data)
{
	if (0 == data.size())
	{
		return R_ERR_PARAM;
	}

	DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
	DBCREQ_SET_KEY(data[0].uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(storage_id);
	DBCREQ_NEED(level_id);
	DBCREQ_NEED(order_id);
	DBCREQ_NEED(coin);
	DBCREQ_NEED(exp);
	DBCREQ_NEED(end_ts);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, storage_id);
	DBCREQ_ARRAY_GET_INT(data, level_id);
	DBCREQ_ARRAY_GET_BINARY(data, order_id, ORDER_LENGTH);
	DBCREQ_ARRAY_GET_INT(data, coin);
	DBCREQ_ARRAY_GET_INT(data, exp);
	DBCREQ_ARRAY_GET_INT(data, end_ts);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataOrder::Add(DataOrder &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, storage_id);
	DBCREQ_SET_INT(data, level_id);
	DBCREQ_SET_BINARY(data, order_id, ORDER_LENGTH);
	DBCREQ_SET_INT(data, coin);
	DBCREQ_SET_INT(data, exp);
	DBCREQ_SET_INT(data, end_ts);
	DBCREQ_EXEC;

	return 0;
}

int CDataOrder::Set(DataOrder &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, storage_id);
	DBCREQ_SET_INT(data, level_id);
	DBCREQ_SET_BINARY(data, order_id, ORDER_LENGTH);
	DBCREQ_SET_INT(data, coin);
	DBCREQ_SET_INT(data, exp);
	DBCREQ_SET_INT(data, end_ts);
	DBCREQ_EXEC;

	return 0;
}

int CDataOrder::Del(DataOrder &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}

