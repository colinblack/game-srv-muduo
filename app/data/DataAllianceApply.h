#include "Kernel.h"

struct DataAllianceApply
{
	uint32_t alliance_id;  //商会id
	uint32_t id;  //用户id
	uint32_t applyts;  //申请时间
	char reason[DataAlliance_apply_reason_LENG];  //申请理由
	char username[BASE_NAME_LEN];   //创建者名称

	DataAllianceApply()
	{
		alliance_id = 0;
		id = 0;
		applyts = 0;

		memset(reason, 0, sizeof(reason));
		memset(username, 0, sizeof(username));
	}

	void SetMessage(ProtoAlliance::AllianceApplyCPP * msg)
	{
		msg->set_applyuid(id);
		msg->set_reason(reason);
		msg->set_name(username);
		msg->set_applyts(applyts);
	}
};

class CDataAllianceApply: public DBCBase<DataAllianceApply, DB_ALLIANCE_APPLY>
{
public:
	virtual int Get(DataAllianceApply &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(reason);
		DBCREQ_NEED(username);
		DBCREQ_NEED(applyts);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, alliance_id);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_CHAR(data, reason, DataAlliance_apply_reason_LENG);
		DBCREQ_GET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_GET_INT(data, applyts);

		return 0;
	}

	virtual int Get(vector<DataAllianceApply> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].alliance_id);
		DBCREQ_SET_KEY(data[0].alliance_id);
		data.clear();

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(reason);
		DBCREQ_NEED(username);
		DBCREQ_NEED(applyts);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, alliance_id);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_CHAR(data, reason, DataAlliance_apply_reason_LENG);
		DBCREQ_ARRAY_GET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_ARRAY_GET_INT(data, applyts);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataAllianceApply &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_CHAR(data, reason, DataAlliance_apply_reason_LENG);
		DBCREQ_SET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_SET_INT(data, applyts);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAllianceApply &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_CHAR(data, reason, DataAlliance_apply_reason_LENG);
		DBCREQ_SET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_SET_INT(data, applyts);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAllianceApply &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
