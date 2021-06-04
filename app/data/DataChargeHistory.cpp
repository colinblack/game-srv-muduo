#include "DataChargeHistory.h"

int CDataChargeHistory::Get(DataChargeHistory &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, ts, data.ts);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(ts);
	DBCREQ_NEED(cash);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, ts);
	DBCREQ_GET_INT(data, cash);

	return 0;
}

int CDataChargeHistory::Get(vector<DataChargeHistory> &data)
{
	if (0 == data.size())
	{
		return R_ERR_PARAM;
	}

	DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
	DBCREQ_SET_KEY(data[0].uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(ts);
	DBCREQ_NEED(cash);

	data.clear();
	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, ts);
	DBCREQ_ARRAY_GET_INT(data, cash);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataChargeHistory::Add(DataChargeHistory &data)
{
	DBCREQ_DECLARE(DBC::ReplaceRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, ts);
	DBCREQ_SET_INT(data, cash);

	DBCREQ_EXEC;

	return 0;
}

int CDataChargeHistory::Set(DataChargeHistory &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, ts, data.ts);

	DBCREQ_SET_INT(data, cash);
	DBCREQ_EXEC;

	return 0;
}

int CDataChargeHistory::Del(DataChargeHistory &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, ts, data.ts);

	DBCREQ_EXEC;

	return 0;
}
