#ifndef DATA_ALLIANCE_H_
#define DATA_ALLIANCE_H_

#include "Kernel.h"
#include "AppDefine.h"

struct DataAlliance
{
	uint32_t alliance_id;  //商会uid
	uint32_t create_uid;   //创建者uid
	uint32_t create_time; /*创建时间*/
	uint32_t qqgroup;  //qq群号
	uint32_t active_ts; // 活跃时间戳
	uint32_t race_ts;	// 竞赛时间戳
	uint32_t race_point; // 竞赛积分
	uint32_t race_opoint;	// 上一轮竞赛积分
	uint32_t race_total_point;	// 竞赛历史积分
	int8_t flag;   /*商会标志*/
	int8_t apply_type;  //入会申请类型.0-无需申请 1-申请进入 2-仅接受邀请进入
	int8_t apply_level_limit;  //入会等级限制
	int8_t race_level;	// 商会竞赛等级
	int8_t race_olevel;	// 上一轮竞赛等级
	int8_t race_rank_id;	// 上一轮商会竞赛排行


	char name[DataAlliance_name_LENG];   /*商会名称*/
	char description[DataAlliance_description_LENG];   //商会描述
	char create_username[BASE_NAME_LEN];   //创建者名称
	uint16_t race_order_id[DataAlliance_race_order_id_LENG]; // 竞赛订单ID
	uint32_t race_order_cd[DataAlliance_race_order_id_LENG]; // 竞赛订单CD

	DataAlliance()
	{
		alliance_id = 0;
		memset(name, 0, sizeof(name));
		flag = 0;
		create_uid = 0;
		create_time = 0;
		apply_type = 0;
		apply_level_limit = 0;
		race_level = 0;
		race_olevel = 0;
		race_rank_id = 0;
		qqgroup = 0;
		active_ts = 0;
		race_ts = 0;
		race_point = 0;
		race_opoint = 0;
		race_total_point = 0;
		memset(description, 0, sizeof(description));
		memset(create_username, 0, sizeof(create_username));
		memset(race_order_id, 0, sizeof(race_order_id));
		memset(race_order_cd, 0, sizeof(race_order_cd));
	}

	void SetMessage(ProtoAlliance::AllianceCPP * msg)
	{
		msg->set_allianceid(alliance_id);
		msg->set_flag(flag);
		msg->set_createuid(create_uid);
		msg->set_createtime(create_time);
		msg->set_qqgroup(qqgroup);
		msg->set_applytype(apply_type);
		msg->set_applylevellimit(apply_level_limit);
		msg->set_name(name);
		msg->set_description(description);
		msg->set_username(create_username);
	}

	void SetMessage(ProtoAlliance::PartAllianceCPP * msg)
	{
		msg->set_allianceid(alliance_id);
		msg->set_flag(flag);
		msg->set_name(name);
		msg->set_applytype(apply_type);
		msg->set_applylevellimit(apply_level_limit);
		msg->set_description(description);
	}
	void SetMessage(ProtoAlliance::ReplyAllianceRaceOrder * msg)
	{
		for(uint32_t i = 0; i < DataAlliance_race_order_id_LENG; ++i)
		{
			ProtoAlliance::AllianceRaceOrder* pOrder = msg->add_order();
			pOrder->set_id(race_order_id[i]);
			pOrder->set_cdts(race_order_cd[i]);
		}
	}
};

class CDataAlliance: public DBCBase<DataAlliance, DB_ALLIANCE>
{
public:
	virtual int Get(DataAlliance &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(name);
		DBCREQ_NEED(flag);
		DBCREQ_NEED(create_uid);
		DBCREQ_NEED(create_time);
		DBCREQ_NEED(apply_type);
		DBCREQ_NEED(apply_level_limit);
		DBCREQ_NEED(qqgroup);
		DBCREQ_NEED(description);
		DBCREQ_NEED(create_username);
		DBCREQ_NEED(race_level);
		DBCREQ_NEED(race_olevel);
		DBCREQ_NEED(race_rank_id);
		DBCREQ_NEED(active_ts);
		DBCREQ_NEED(race_ts);
		DBCREQ_NEED(race_point);
		DBCREQ_NEED(race_opoint);
		DBCREQ_NEED(race_total_point);
		DBCREQ_NEED(race_order_id);
		DBCREQ_NEED(race_order_cd);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, alliance_id);
		DBCREQ_GET_CHAR(data, name, DataAlliance_name_LENG);
		DBCREQ_GET_INT(data, flag);
		DBCREQ_GET_INT(data, create_uid);
		DBCREQ_GET_INT(data, create_time);
		DBCREQ_GET_INT(data, apply_type);
		DBCREQ_GET_INT(data, apply_level_limit);
		DBCREQ_GET_INT(data, qqgroup);
		DBCREQ_GET_CHAR(data, description, DataAlliance_description_LENG);
		DBCREQ_GET_CHAR(data, create_username, BASE_NAME_LEN);
		DBCREQ_GET_INT(data, race_level);
		DBCREQ_GET_INT(data, race_olevel);
		DBCREQ_GET_INT(data, race_rank_id);
		DBCREQ_GET_INT(data, active_ts);
		DBCREQ_GET_INT(data, race_ts);
		DBCREQ_GET_INT(data, race_point);
		DBCREQ_GET_INT(data, race_opoint);
		DBCREQ_GET_INT(data, race_total_point);
		DBCREQ_GET_BIN_SIZE(data, race_order_id);
		DBCREQ_GET_BIN_SIZE(data, race_order_cd);

		if(data.race_level == 0)
		{
			data.race_level = 5;
		}

		return 0;
	}

	virtual int Add(DataAlliance &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_SET_CHAR(data, name, DataAlliance_name_LENG);
		DBCREQ_SET_INT(data, flag);
		DBCREQ_SET_INT(data, create_uid);
		DBCREQ_SET_INT(data, create_time);
		DBCREQ_SET_INT(data, apply_type);
		DBCREQ_SET_INT(data, apply_level_limit);
		DBCREQ_SET_INT(data, qqgroup);
		DBCREQ_SET_INT(data, race_level);
		DBCREQ_SET_INT(data, race_olevel);
		DBCREQ_SET_INT(data, race_rank_id);
		DBCREQ_SET_INT(data, active_ts);
		DBCREQ_SET_INT(data, race_ts);
		DBCREQ_SET_INT(data, race_point);
		DBCREQ_SET_INT(data, race_opoint);
		DBCREQ_SET_INT(data, race_total_point);
		DBCREQ_SET_CHAR(data, description, DataAlliance_description_LENG);
		DBCREQ_SET_CHAR(data, create_username, BASE_NAME_LEN);
		DBCREQ_SET_BIN_SIZE(data, race_order_id);
		DBCREQ_SET_BIN_SIZE(data, race_order_cd);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAlliance &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_SET_CHAR(data, name, DataAlliance_name_LENG);
		DBCREQ_SET_INT(data, flag);
		DBCREQ_SET_INT(data, create_uid);
		DBCREQ_SET_INT(data, create_time);
		DBCREQ_SET_INT(data, apply_type);
		DBCREQ_SET_INT(data, apply_level_limit);
		DBCREQ_SET_INT(data, qqgroup);
		DBCREQ_SET_INT(data, race_level);
		DBCREQ_SET_INT(data, race_olevel);
		DBCREQ_SET_INT(data, race_rank_id);
		DBCREQ_SET_INT(data, active_ts);
		DBCREQ_SET_INT(data, race_ts);
		DBCREQ_SET_INT(data, race_point);
		DBCREQ_SET_INT(data, race_opoint);
		DBCREQ_SET_INT(data, race_total_point);
		DBCREQ_SET_CHAR(data, description, DataAlliance_description_LENG);
		DBCREQ_SET_CHAR(data, create_username, BASE_NAME_LEN);
		DBCREQ_SET_BIN_SIZE(data, race_order_id);
		DBCREQ_SET_BIN_SIZE(data, race_order_cd);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAlliance &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_EXEC;

		return 0;
	}
};

#endif //DATA_ALLIANCE_H_
