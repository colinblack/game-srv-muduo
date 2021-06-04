#ifndef DATA_KEEPER_TASK_H_
#define DATA_KEEPER_TASK_H_
#include "Kernel.h"

struct DataKeeperTask {
	uint32_t uid;
	uint32_t id;
	uint32_t need;
	uint32_t finish;
	uint8_t status;	// 状态(不存档)
	DataKeeperTask() {
		uid = 0;
		id = 0;
		need = 0;
		finish = 0;
		status = 0;
	}
	void SetMessage(ProtoKeeper::KeeperTask* msg) {
		msg->set_id(id);
		msg->set_need(need);
		msg->set_finish(finish);
		msg->set_status(status);
	}
};
class CDataKeeperTask: public DBCBase<DataKeeperTask, DB_KEEPER_TASK> {
public:
	virtual int Get(DataKeeperTask &data) {
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(need);
		DBCREQ_NEED(finish);
		DBCREQ_EXEC;
		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, need);
		DBCREQ_GET_INT(data, finish);
		return 0;
	}
	virtual int Get(vector<DataKeeperTask> &data) {
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(need);
		DBCREQ_NEED(finish);
		DBCREQ_EXEC;
		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, need);DBCREQ_ARRAY_GET_INT(data, finish);DBCREQ_ARRAY_GET_END();
		return 0;
	}
	virtual int Add(DataKeeperTask &data) {
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, need);
		DBCREQ_SET_INT(data, finish);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Set(DataKeeperTask &data) {
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, need);
		DBCREQ_SET_INT(data, finish);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Del(DataKeeperTask &data) {
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_EXEC;
		return 0;
	}
};

#endif //DATA_KEEPER_TASK_H_
