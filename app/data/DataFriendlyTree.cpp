#include "DataFriendlyTree.h"

//////////////////////////////////////////////////////////////////
int CDataFriendlyTree::Get(DataFriendlyTree &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(othuid);
	DBCREQ_NEED(ts);
	DBCREQ_NEED(name);
	DBCREQ_NEED(fig);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, othuid);
	DBCREQ_GET_INT(data, ts);
	DBCREQ_GET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_GET_CHAR(data, fig, BASE_FIG_LEN);

	return 0;
}

int CDataFriendlyTree::Get(vector<DataFriendlyTree> &data)
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
	DBCREQ_NEED(othuid);
	DBCREQ_NEED(ts);
	DBCREQ_NEED(name);
	DBCREQ_NEED(fig);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, othuid);
	DBCREQ_ARRAY_GET_INT(data, ts);
	DBCREQ_ARRAY_GET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_ARRAY_GET_CHAR(data, fig, BASE_FIG_LEN);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataFriendlyTree::Add(DataFriendlyTree &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, othuid);
	DBCREQ_SET_INT(data, ts);
	DBCREQ_SET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_EXEC;

	return 0;
}

int CDataFriendlyTree::Set(DataFriendlyTree &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, othuid);
	DBCREQ_SET_INT(data, ts);
	DBCREQ_SET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_EXEC;

	return 0;
}

int CDataFriendlyTree::Del(DataFriendlyTree &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
