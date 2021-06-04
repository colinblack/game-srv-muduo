#include "DataBuildings.h"

int CDataBuildings::Get(DataBuildings &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(build_id);
	DBCREQ_NEED(position);
	DBCREQ_NEED(direct);
	DBCREQ_NEED(done_time);
	DBCREQ_NEED(level);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, build_id);
	DBCREQ_GET_INT(data, position);
	DBCREQ_GET_INT(data, direct);
	DBCREQ_GET_INT(data, done_time);
	DBCREQ_GET_INT(data, level);

	return 0;
}

int CDataBuildings::Get(vector<DataBuildings> &data)
{
	if (data.empty())
	{
		return R_ERROR;
	}

	unsigned uid = data.begin()->uid;
	data.clear();

	DBCREQ_DECLARE(DBC::GetRequest, uid);
	DBCREQ_SET_KEY(uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(build_id);
	DBCREQ_NEED(position);
	DBCREQ_NEED(direct);
	DBCREQ_NEED(done_time);
	DBCREQ_NEED(level);

	DBCREQ_EXEC;
	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, build_id);
	DBCREQ_ARRAY_GET_INT(data, position);
	DBCREQ_ARRAY_GET_INT(data, direct);
	DBCREQ_ARRAY_GET_INT(data, done_time);
	DBCREQ_ARRAY_GET_INT(data, level);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataBuildings::Add(DataBuildings &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, build_id);
	DBCREQ_SET_INT(data, position);
	DBCREQ_SET_INT(data, direct);
	DBCREQ_SET_INT(data, done_time);
	DBCREQ_SET_INT(data, level);

	DBCREQ_EXEC;

	return 0;
}

int CDataBuildings::Set(DataBuildings &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, build_id);
	DBCREQ_SET_INT(data, position);
	DBCREQ_SET_INT(data, direct);
	DBCREQ_SET_INT(data, done_time);
	DBCREQ_SET_INT(data, level);

	DBCREQ_EXEC;

	return 0;
}

int CDataBuildings::Del(DataBuildings &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}

