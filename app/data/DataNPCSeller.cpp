#include "DataNPCSeller.h"

//////////////////////////////////////////////////////////////////
int CDataNPCSeller::Get(DataNPCSeller &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(props_id);
	DBCREQ_NEED(props_cnt);
	DBCREQ_NEED(npc_next_visit_ts);
	DBCREQ_NEED(props_discount);
	DBCREQ_NEED(npc_seller_status);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, props_id);
	DBCREQ_GET_INT(data, props_cnt);
	DBCREQ_GET_INT(data, npc_next_visit_ts);
	DBCREQ_GET_INT(data, props_discount);
	DBCREQ_GET_INT(data, npc_seller_status);

	return 0;
}


int CDataNPCSeller::Add(DataNPCSeller &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, props_id);
	DBCREQ_SET_INT(data, props_cnt);
	DBCREQ_SET_INT(data, npc_next_visit_ts);
	DBCREQ_SET_INT(data, props_discount);
	DBCREQ_SET_INT(data, npc_seller_status);
	DBCREQ_EXEC;

	return 0;
}

int CDataNPCSeller::Set(DataNPCSeller &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, props_id);
	DBCREQ_SET_INT(data, props_cnt);
	DBCREQ_SET_INT(data, npc_next_visit_ts);
	DBCREQ_SET_INT(data, props_discount);
	DBCREQ_SET_INT(data, npc_seller_status);
	DBCREQ_EXEC;

	return 0;
}

int CDataNPCSeller::Del(DataNPCSeller &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_EXEC;

	return 0;
}
