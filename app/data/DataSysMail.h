/*
 * DataSysMail.h
 *
 *  Created on: 2018-2-8
 *      Author: Ralf
 */

#ifndef DATASYSMAIL_H_
#define DATASYSMAIL_H_

#include "Kernel.h"

#define Data_sysmail_sys_COUNT 512
#define Data_sysmail_reward_COUNT 256
enum mail_stat
{
	e_mail_stat_unread		= 0,
	e_mail_stat_unget		= 1,
	e_mail_stat_over		= 2,
	e_mail_stat_del			= 3,
};
struct DataSysmail {
	uint32_t uid;
	uint32_t id;
	uint32_t stat;
	char sys[Data_sysmail_sys_COUNT];
	char reward[Data_sysmail_reward_COUNT];
	DataSysmail() {
		uid = 0;
		id = 0;
		stat = e_mail_stat_unread;
		memset(sys, 0, sizeof(sys));
		memset(reward, 0, sizeof(reward));
	}
	void SetMessage(User::SysMail* msg)
	{
		msg->set_ts(id);
		msg->set_stat(stat);
		msg->set_sys(string(sys));
		msg->set_reward(string(reward));
	}
};
class CDataSysmail: public DBCBase<DataSysmail, DB_SYSMAIL> {
public:
	virtual int Get(DataSysmail &data) {
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(stat);
		DBCREQ_NEED(sys);
		DBCREQ_NEED(reward);
		DBCREQ_EXEC;
		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, stat);
		DBCREQ_GET_CHAR(data, sys, Data_sysmail_sys_COUNT);
		DBCREQ_GET_CHAR(data, reward, Data_sysmail_reward_COUNT);
		return 0;
	}
	virtual int Get(vector<DataSysmail> &data) {
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(stat);
		DBCREQ_NEED(sys);
		DBCREQ_NEED(reward);
		DBCREQ_EXEC;
		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, stat);
		DBCREQ_ARRAY_GET_CHAR(data, sys, Data_sysmail_sys_COUNT);
		DBCREQ_ARRAY_GET_CHAR(data, reward, Data_sysmail_reward_COUNT);
		DBCREQ_ARRAY_GET_END();
		return 0;
	}
	virtual int Add(DataSysmail &data) {
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, stat);
		DBCREQ_SET_CHAR(data, sys, Data_sysmail_sys_COUNT);
		DBCREQ_SET_CHAR(data, reward, Data_sysmail_reward_COUNT);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Set(DataSysmail &data) {
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, stat);
		DBCREQ_SET_CHAR(data, sys, Data_sysmail_sys_COUNT);
		DBCREQ_SET_CHAR(data, reward, Data_sysmail_reward_COUNT);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Del(DataSysmail &data) {
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_EXEC;
		return 0;
	}
};

#endif /* DATASYSMAIL_H_ */
