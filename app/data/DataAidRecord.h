#include "Kernel.h"

struct DataAidRecord
{
	uint32_t uid;
	uint32_t ts;
	uint32_t aid_id;
	uint32_t aid_times;

	DataAidRecord()
	{
		uid = 0;
		ts = 0;
		aid_id = 0;
		aid_times = 0;
	}
};

class CDataAidRecord: public DBCBase<DataAidRecord, DB_AID_RECORD>
{
public:
	virtual int Get(DataAidRecord &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, ts, data.ts);
		DBCREQ_SET_CONDITION(EQ, aid_id, data.aid_id);

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(ts);
		DBCREQ_NEED(aid_id);
		DBCREQ_NEED(aid_times);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, ts);
		DBCREQ_GET_INT(data, aid_id);
		DBCREQ_GET_INT(data, aid_times);

		return 0;
	}

	virtual int Get(vector<DataAidRecord> &data)
	{
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(ts);
		DBCREQ_NEED(aid_id);
		DBCREQ_NEED(aid_times);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, ts);
		DBCREQ_ARRAY_GET_INT(data, aid_id);
		DBCREQ_ARRAY_GET_INT(data, aid_times);
		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataAidRecord &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_SET_INT(data, ts);
		DBCREQ_SET_INT(data, aid_id);
		DBCREQ_SET_INT(data, aid_times);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAidRecord &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, ts, data.ts);
		DBCREQ_SET_CONDITION(EQ, aid_id, data.aid_id);

		DBCREQ_SET_INT(data, ts);
		DBCREQ_SET_INT(data, aid_id);
		DBCREQ_SET_INT(data, aid_times);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAidRecord &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, ts, data.ts);
		DBCREQ_SET_CONDITION(EQ, aid_id, data.aid_id);

		DBCREQ_EXEC;

		return 0;
	}
};
