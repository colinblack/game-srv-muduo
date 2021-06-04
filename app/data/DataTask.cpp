#include "DataTask.h"

//////////////////////////////////////////////////////////////////
int CDataTask::Get(DataTask &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(cur_task_value);
	DBCREQ_NEED(cur_task_star);
	DBCREQ_NEED(reward_status);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, cur_task_value);
	DBCREQ_GET_INT(data, cur_task_star);
	DBCREQ_GET_INT(data, reward_status);

	return 0;
}

int CDataTask::Get(vector<DataTask> &data)
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
	DBCREQ_NEED(cur_task_value);
	DBCREQ_NEED(cur_task_star);
	DBCREQ_NEED(reward_status);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, cur_task_value);
	DBCREQ_ARRAY_GET_INT(data, cur_task_star);
	DBCREQ_ARRAY_GET_INT(data, reward_status);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataTask::Add(DataTask &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, cur_task_value);
	DBCREQ_SET_INT(data, cur_task_star);
	DBCREQ_SET_INT(data, reward_status);
	DBCREQ_EXEC;

	return 0;
}

int CDataTask::Set(DataTask &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, cur_task_value);
	DBCREQ_SET_INT(data, cur_task_star);
	DBCREQ_SET_INT(data, reward_status);
	DBCREQ_EXEC;

	return 0;
}

int CDataTask::Del(DataTask &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
