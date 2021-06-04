#ifndef DATA_KEEPER_H_
#define DATA_KEEPER_H_
#include "Kernel.h"
struct DataKeeper {
	uint32_t uid;
	uint32_t id;
	uint32_t level;
	uint32_t exp;
	uint32_t ts;
	uint8_t autofeed;
	DataKeeper() {
		uid = 0;
		id = 0;
		level = 1;
		exp = 0;
		ts = 0;
		autofeed = 1;
	}
	void SetMessage(ProtoKeeper::KeeperInfoResp* msg)
	{
		msg->set_id(id);
		msg->set_level(level);
		msg->set_exp(exp);
		msg->set_overts(ts);
		msg->set_flag(autofeed);
	}

};
class CDataKeeper: public DBCBase<DataKeeper, DB_KEEPER> {
public:
	virtual int Get(DataKeeper &data) {
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(level);
		DBCREQ_NEED(exp);
		DBCREQ_NEED(ts);
		DBCREQ_NEED(autofeed);
		DBCREQ_EXEC;
		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, level);
		DBCREQ_GET_INT(data, exp);
		DBCREQ_GET_INT(data, ts);
		DBCREQ_GET_INT(data, autofeed);

		if(data.level == 0)
		{
			data.level = 1;
		}
		return 0;
	}
	virtual int Get(vector<DataKeeper> &data) {
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(level);
		DBCREQ_NEED(exp);
		DBCREQ_NEED(ts);
		DBCREQ_NEED(autofeed);
		DBCREQ_EXEC;
		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, level);
		if(data[i].level == 0)
		{
			data[i].level = 1;
		}
		DBCREQ_ARRAY_GET_INT(data, exp);
		DBCREQ_ARRAY_GET_INT(data, ts);DBCREQ_ARRAY_GET_INT(data, autofeed);DBCREQ_ARRAY_GET_END();
		return 0;
	}
	virtual int Add(DataKeeper &data) {
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, level);
		DBCREQ_SET_INT(data, exp);
		DBCREQ_SET_INT(data, ts);
		DBCREQ_SET_INT(data, autofeed);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Set(DataKeeper &data) {
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, level);
		DBCREQ_SET_INT(data, exp);
		DBCREQ_SET_INT(data, ts);
		DBCREQ_SET_INT(data, autofeed);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Del(DataKeeper &data) {
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_EXEC;
		return 0;
	}
};
#endif //DATA_KEEPER_H_
