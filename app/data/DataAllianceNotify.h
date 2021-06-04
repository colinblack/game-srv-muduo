#ifndef DATA_ALLIANCE_NOTIFY_H_
#define DATA_ALLIANCE_NOTIFY_H_

#include "Kernel.h"

struct DataAllianceNotify {
	uint32_t alliance_id;
	uint32_t id;
	uint32_t announce_uid;
	uint32_t create_ts;
	uint8_t position;

	char content[DataAllianceNotify_content_LENG];
	char username[BASE_NAME_LEN];   //创建者名称

	DataAllianceNotify()
	{
		alliance_id = 0;
		id = 0;
		announce_uid = 0;
		create_ts = 0;
		position = 0;

		memset(content, 0, sizeof(content));
		memset(username, 0, sizeof(username));
	}

	void Reset()
	{
		announce_uid = 0;
		create_ts = 0;
		position = 0;
		memset(content, 0, sizeof(content));
		memset(username, 0, sizeof(username));
	}

	void SetMessage(ProtoAlliance::AllianceNotifyCPP * msg)
	{
		msg->set_id(id);
		msg->set_announcer(announce_uid);
		msg->set_content(content);
		msg->set_createts(create_ts);
		msg->set_name(username);
	}
};

class CDataAllianceNotify: public DBCBase<DataAllianceNotify, DB_ALLIANCE_NOTIFY>
{
public:
	virtual int Get(DataAllianceNotify &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(announce_uid);
		DBCREQ_NEED(content);
		DBCREQ_NEED(create_ts);
		DBCREQ_NEED(position);
		DBCREQ_NEED(username);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, alliance_id);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, announce_uid);
		DBCREQ_GET_CHAR(data, content, DataAllianceNotify_content_LENG);
		DBCREQ_GET_INT(data, create_ts);
		DBCREQ_GET_INT(data, position);
		DBCREQ_GET_CHAR(data, username, BASE_NAME_LEN);

		return 0;
	}

	virtual int Get(vector<DataAllianceNotify> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].alliance_id);
		DBCREQ_SET_KEY(data[0].alliance_id);
		data.clear();

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(announce_uid);
		DBCREQ_NEED(content);
		DBCREQ_NEED(create_ts);
		DBCREQ_NEED(position);
		DBCREQ_NEED(username);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, alliance_id);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, announce_uid);
		DBCREQ_ARRAY_GET_CHAR(data, content, DataAllianceNotify_content_LENG);
		DBCREQ_ARRAY_GET_INT(data, create_ts);
		DBCREQ_ARRAY_GET_INT(data, position);
		DBCREQ_ARRAY_GET_CHAR(data, username, BASE_NAME_LEN);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataAllianceNotify &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, announce_uid);
		DBCREQ_SET_CHAR(data, content, DataAllianceNotify_content_LENG);
		DBCREQ_SET_INT(data, create_ts);
		DBCREQ_SET_INT(data, position);
		DBCREQ_SET_CHAR(data, username, BASE_NAME_LEN);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAllianceNotify &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, announce_uid);
		DBCREQ_SET_CHAR(data, content, DataAllianceNotify_content_LENG);
		DBCREQ_SET_INT(data, create_ts);
		DBCREQ_SET_INT(data, position);
		DBCREQ_SET_CHAR(data, username, BASE_NAME_LEN);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAllianceNotify &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};

#endif //DATA_ALLIANCE_NOTIFY_H_
