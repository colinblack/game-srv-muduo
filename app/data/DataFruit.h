#include "Kernel.h"

struct DataFruit {
	uint32_t uid;
	uint32_t treeid;
	uint32_t harvest_time;
	uint32_t aid_uid;
	uint16_t id;

	int8_t status;
	int8_t stage;
	int8_t fruit_left_num;

	DataFruit()
	{
		uid = 0;
		id = 0;
		treeid = 0;
		status = 0;
		stage = 0;
		fruit_left_num = 0;
		harvest_time = 0;
		aid_uid = 0;
	}
	void setStatus(int8_t tmpStatus);

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_ud(id);
		msg->set_treeid(treeid);
		msg->set_status(status);
		msg->set_stage(stage);
		msg->set_fruitleftnum(fruit_left_num);
		msg->set_harvesttime(harvest_time);
	}

	void FromMessage(const ProtoUser::FruitCPP * msg)
	{
		treeid = msg->treeid();
		status = msg->status();
		stage = msg->stage();
		fruit_left_num = msg->fruitleftnum();
		harvest_time = msg->harvesttime();
	}
};

class CDataFruit: public DBCBase<DataFruit, DB_FRUIT>
{
public:
	virtual int Get(DataFruit &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();

		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(treeid);
		DBCREQ_NEED(status);
		DBCREQ_NEED(stage);
		DBCREQ_NEED(fruit_left_num);
		DBCREQ_NEED(harvest_time);
		DBCREQ_NEED(aid_uid);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, treeid);
		DBCREQ_GET_INT(data, status);
		DBCREQ_GET_INT(data, stage);
		DBCREQ_GET_INT(data, fruit_left_num);
		DBCREQ_GET_INT(data, harvest_time);
		DBCREQ_GET_INT(data, aid_uid);

		return 0;
	}

	virtual int Get(vector<DataFruit> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(treeid);
		DBCREQ_NEED(status);
		DBCREQ_NEED(stage);
		DBCREQ_NEED(fruit_left_num);
		DBCREQ_NEED(harvest_time);
		DBCREQ_NEED(aid_uid);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, treeid);
		DBCREQ_ARRAY_GET_INT(data, status);
		DBCREQ_ARRAY_GET_INT(data, stage);
		DBCREQ_ARRAY_GET_INT(data, fruit_left_num);
		DBCREQ_ARRAY_GET_INT(data, harvest_time);
		DBCREQ_ARRAY_GET_INT(data, aid_uid);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataFruit &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, treeid);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, stage);
		DBCREQ_SET_INT(data, fruit_left_num);
		DBCREQ_SET_INT(data, harvest_time);
		DBCREQ_SET_INT(data, aid_uid);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataFruit &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, treeid);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, stage);
		DBCREQ_SET_INT(data, fruit_left_num);
		DBCREQ_SET_INT(data, harvest_time);
		DBCREQ_SET_INT(data, aid_uid);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataFruit &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
