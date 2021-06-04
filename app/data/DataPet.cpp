#include "DataPet.h"

//////////////////////////////////////////////////////////////////
int CDataPet::Get(DataPet &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(teaseEndts);
	DBCREQ_NEED(normalEndts);
	DBCREQ_NEED(teaseFlag);
	DBCREQ_NEED(name);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, teaseEndts);
	DBCREQ_GET_INT(data, normalEndts);
	DBCREQ_GET_INT(data, teaseFlag);
	DBCREQ_GET_CHAR(data, name,BASE_NAME_LEN);

	return 0;
}

int CDataPet::Get(vector<DataPet> &data)
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
	DBCREQ_NEED(teaseEndts);
	DBCREQ_NEED(normalEndts);
	DBCREQ_NEED(teaseFlag);
	DBCREQ_NEED(name);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, teaseEndts);
	DBCREQ_ARRAY_GET_INT(data, normalEndts);
	DBCREQ_ARRAY_GET_INT(data, teaseFlag);
	DBCREQ_ARRAY_GET_CHAR(data, name,BASE_NAME_LEN);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataPet::Add(DataPet &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, teaseEndts);
	DBCREQ_SET_INT(data, normalEndts);
	DBCREQ_SET_INT(data, teaseFlag);
	DBCREQ_SET_CHAR(data, name,BASE_NAME_LEN);
	DBCREQ_EXEC;

	return 0;
}

int CDataPet::Set(DataPet &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, teaseEndts);
	DBCREQ_SET_INT(data, normalEndts);
	DBCREQ_SET_INT(data, teaseFlag);
	DBCREQ_SET_CHAR(data, name,BASE_NAME_LEN);
	DBCREQ_EXEC;

	return 0;
}

int CDataPet::Del(DataPet &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
