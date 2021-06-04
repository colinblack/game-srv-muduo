#ifndef DATA_INVITED_LIST_H_
#define DATA_INVITED_LIST_H_

#include "Kernel.h"

struct DataInvitedList
{
	uint32_t uid;
	uint32_t id;  //邀请的商会id
	uint32_t invite_uid;  //邀请者uid
	uint32_t invitets;  //邀请时间

	uint8_t flag;  //商会标志
	char alliance_name[DataAlliance_name_LENG];
	char invite_name[BASE_NAME_LEN];

	DataInvitedList()
	{
		uid = 0;
		id = 0;
		invite_uid = 0;
		invitets = 0;
		flag = 0;

		memset(alliance_name, 0, sizeof(alliance_name));
		memset(invite_name, 0, sizeof(invite_name));
	}

	void SetMessage(ProtoAlliance::AllianceInvitedCPP * msg)
	{
		msg->set_allianceid(id);
		msg->set_inviteuid(invite_uid);
		msg->set_alliancename(alliance_name);
		msg->set_flag(flag);
		msg->set_invitename(invite_name);
		msg->set_invitets(invitets);
	}
};

class CDataInvitedList: public DBCBase<DataInvitedList, DB_INVITED_LIST>
{
public:
	virtual int Get(DataInvitedList &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(invite_uid);
		DBCREQ_NEED(alliance_name);
		DBCREQ_NEED(flag);
		DBCREQ_NEED(invite_name);
		DBCREQ_NEED(invitets);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, invite_uid);
		DBCREQ_GET_CHAR(data, alliance_name, DataAlliance_name_LENG);
		DBCREQ_GET_INT(data, flag);
		DBCREQ_GET_CHAR(data, invite_name, BASE_NAME_LEN);
		DBCREQ_GET_INT(data, invitets);

		return 0;
	}

	virtual int Get(vector<DataInvitedList> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);

		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(invite_uid);
		DBCREQ_NEED(alliance_name);
		DBCREQ_NEED(flag);
		DBCREQ_NEED(invite_name);
		DBCREQ_NEED(invitets);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, invite_uid);
		DBCREQ_ARRAY_GET_CHAR(data, alliance_name, DataAlliance_name_LENG);
		DBCREQ_ARRAY_GET_INT(data, flag);
		DBCREQ_ARRAY_GET_CHAR(data, invite_name, BASE_NAME_LEN);
		DBCREQ_ARRAY_GET_INT(data, invitets);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataInvitedList &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, invite_uid);
		DBCREQ_SET_CHAR(data, alliance_name, DataAlliance_name_LENG);
		DBCREQ_SET_INT(data, flag);
		DBCREQ_SET_CHAR(data, invite_name, BASE_NAME_LEN);
		DBCREQ_SET_INT(data, invitets);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataInvitedList &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, invite_uid);
		DBCREQ_SET_CHAR(data, alliance_name, DataAlliance_name_LENG);
		DBCREQ_SET_INT(data, flag);
		DBCREQ_SET_CHAR(data, invite_name, BASE_NAME_LEN);
		DBCREQ_SET_INT(data, invitets);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataInvitedList &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};

#endif //DATA_INVITED_LIST_H_
