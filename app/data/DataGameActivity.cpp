#include "DataGameActivity.h"

int CDataGameActivity::Get(DataGameActivity &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(version);

	//声明需要的参数类型
	char fieldname[50] = {0};

	for(int i = 1; i <= DB_GAME_DATA_NUM; ++i)
	{
		sprintf(fieldname, "actdata%d", i);
		req.Need(fieldname, ++reqItemIndex);
	}

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, version);

	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		data.actdata[i] = m_dbcret.IntValue(++reqItemIndex);
	}

	return 0;
}

int CDataGameActivity::Get(vector<DataGameActivity> &data)
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
	DBCREQ_NEED(version);

	//声明需要的参数类型
	char fieldname[50] = {0};

	for(int i = 1; i <= DB_GAME_DATA_NUM; ++i)
	{
		sprintf(fieldname, "actdata%d", i);
		req.Need(fieldname, ++reqItemIndex);
	}

	data.clear();
	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, version);

	for(int j = 0; j < DB_GAME_DATA_NUM; ++j)
	{
		data[i].actdata[j] = m_dbcret.IntValue(++reqItemIndex);
	}

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataGameActivity::Add(DataGameActivity &data)
{
	DBCREQ_DECLARE(DBC::ReplaceRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, version);

	char fieldname[50] = {0};

	for(int i = 1; i <= DB_GAME_DATA_NUM; ++i)
	{
		sprintf(fieldname, "actdata%d", i);
		req.Set(fieldname, data.actdata[i-1]);
	}

	DBCREQ_EXEC;

	return 0;
}

int CDataGameActivity::Set(DataGameActivity &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, version);

	char fieldname[50] = {0};

	for(int i = 1; i <= DB_GAME_DATA_NUM; ++i)
	{
		sprintf(fieldname, "actdata%d", i);
		req.Set(fieldname, data.actdata[i-1]);
	}

	DBCREQ_EXEC;

	return 0;
}

int CDataGameActivity::Del(DataGameActivity &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
