#include "DataNPCShop.h"

//////////////////////////////////////////////////////////////////
int CDataNPCShop::Get(DataNPCShop &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(props_id);
	DBCREQ_NEED(props_cnt);
	DBCREQ_NEED(sell_flag);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, props_id);
	DBCREQ_GET_INT(data, props_cnt);
	DBCREQ_GET_INT(data, sell_flag);

	return 0;
}

int CDataNPCShop::Get(vector<DataNPCShop> &data)
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
	DBCREQ_NEED(props_id);
	DBCREQ_NEED(props_cnt);
	DBCREQ_NEED(sell_flag);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, props_id);
	DBCREQ_ARRAY_GET_INT(data, props_cnt);
	DBCREQ_ARRAY_GET_INT(data, sell_flag);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataNPCShop::Add(DataNPCShop &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, props_id);
	DBCREQ_SET_INT(data, props_cnt);
	DBCREQ_SET_INT(data, sell_flag);
	DBCREQ_EXEC;

	return 0;
}

int CDataNPCShop::Set(DataNPCShop &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, props_id);
	DBCREQ_SET_INT(data, props_cnt);
	DBCREQ_SET_INT(data, sell_flag);
	DBCREQ_EXEC;

	return 0;
}

int CDataNPCShop::Del(DataNPCShop &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
