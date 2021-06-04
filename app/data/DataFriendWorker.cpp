#include "DataFriendWorker.h"

//////////////////////////////////////////////////////////////////
int CDataFriendWorker::Get(DataFriendWorker &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(endts);
	DBCREQ_NEED(pos);
	DBCREQ_NEED(invite_ts);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, endts);
	DBCREQ_GET_INT(data, pos);
	DBCREQ_GET_INT(data, invite_ts);

	return 0;
}

int CDataFriendWorker::Get(vector<DataFriendWorker> &data)
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
	DBCREQ_NEED(endts);
	DBCREQ_NEED(pos);
	DBCREQ_NEED(invite_ts);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, endts);
	DBCREQ_ARRAY_GET_INT(data, pos);
	DBCREQ_ARRAY_GET_INT(data, invite_ts);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataFriendWorker::Add(DataFriendWorker &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, endts);
	DBCREQ_SET_INT(data, pos);
	DBCREQ_SET_INT(data, invite_ts);
	DBCREQ_EXEC;

	return 0;
}

int CDataFriendWorker::Set(DataFriendWorker &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, endts);
	DBCREQ_SET_INT(data, pos);
	DBCREQ_SET_INT(data, invite_ts);
	DBCREQ_EXEC;

	return 0;
}

int CDataFriendWorker::Del(DataFriendWorker &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
