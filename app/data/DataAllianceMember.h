#include "Kernel.h"
struct DataAllianceMemberRaceOrderLog
{
	uint16_t id;	//
	uint16_t status;//订单状态
};
struct DataAllianceMemberRaceReward
{
//	uint8_t levelId;	//等级ID
	uint8_t rewardId;	//奖励ID
};
struct DataAllianceMember
{
	uint32_t alliance_id;  //商会id
	uint32_t id;  //用户id
	uint32_t authority;  //权限
	uint32_t helptimes; //帮助次数
	uint32_t userlevel;  //用户等级
	uint32_t join_ts;	// 加入时间

	uint32_t race_user_level;	// 上一轮商会竞赛玩家等级
	uint32_t race_point; // 玩家积分
	uint32_t race_order_ts; // 任务超时
	uint16_t race_order_id; // 任务ID
	uint8_t race_order_recv; // 已领任务数
	uint8_t race_order_finish; // 完成任务数
	uint8_t race_order_cancel; // 放弃任务数
	uint8_t flag;	// 标志位
	uint8_t position; //成员职位
	char username[BASE_NAME_LEN];   //成员名称
	char fig[BASE_FIG_LEN];	// 成员头像

	DataAllianceMemberRaceReward race_grade_reward[DataAllianceMember_race_grade_reward_LENG]; // 等级奖励
	DataAllianceMemberRaceReward race_stage_reward[DataAllianceMember_race_stage_reward_LENG]; // 阶段奖励
	DataAllianceMemberRaceOrderLog race_order_log[DataAllianceMember_race_order_log_LENG]; // 任务日志
	uint16_t race_order_progress[DataAllianceMember_race_order_progress_LENG]; // 任务进度

	uint32_t helpTs; //帮助时间戳(不存数据库)---------
	uint32_t onlineTs;	//在线时间戳(不存数据库)-----------
	uint32_t vipLevel;	// vip等级不存档

	DataAllianceMember()
	{
		alliance_id = 0;
		id = 0;
		authority = 0;
		helptimes = 0;
		userlevel = 0;
		join_ts = 0;

		race_user_level = 0;
		race_point= 0;
		race_order_id= 0;
		race_order_ts= 0;
		race_order_recv= 0;
		race_order_finish= 0;
		race_order_cancel= 0;

		flag = 0;
		position = 0;
		helpTs = 0;
		onlineTs = 0;
		vipLevel = 0;

		memset(username, 0, sizeof(username));
		memset(fig, 0, sizeof(fig));
		memset(race_grade_reward, 0, sizeof(race_grade_reward));
		memset(race_stage_reward, 0, sizeof(race_stage_reward));
		memset(race_order_log, 0, sizeof(race_order_log));
		memset(race_order_progress, 0, sizeof(race_order_progress));
	}

	void SetMessage(ProtoAlliance::AllianceMemberCPP * msg)
	{
		if(onlineTs + 172800 < Time::GetGlobalTime()) // 更新时间戳超过两天表示无效
		{
			onlineTs = 0;
		}
		if(helpTs + 172800 < Time::GetGlobalTime()) // 更新时间戳超过两天表示无效
		{
			helpTs = 0;
		}
		msg->set_memberuid(id);
		msg->set_position(position);
		msg->set_authority(authority);
		msg->set_helptimes(helptimes);
		msg->set_name(username);
		msg->set_level(userlevel);
		msg->set_onlinets(onlineTs);
		msg->set_helpts(helpTs);
		msg->set_fig(fig);
		msg->set_joints(join_ts);
	}
	void SetData(unsigned aid, ProtoAlliance::AllianceMemberCPP * msg)
	{
		alliance_id = aid;
		id = msg->memberuid();
		position = msg->position();
		authority = msg->authority();
		helptimes = msg->helptimes();
		userlevel = msg->level();
		onlineTs = msg->onlinets();
		helpTs = msg->helpts();
		snprintf(username, sizeof(username), msg->name().c_str());
		snprintf(fig, sizeof(fig), msg->fig().c_str());
		join_ts = msg->joints();
	}
	void SetMessage(ProtoAlliance::ReplyAllianceRaceOrder * msg)
	{
		ProtoAlliance::AllianceRaceMemberOrder* pOrder = msg->add_memberorder();
		pOrder->set_uid(id);
		pOrder->set_id(race_order_id);
		pOrder->set_ts(race_order_ts);
		pOrder->set_level(userlevel);
		pOrder->set_name(username);
		pOrder->set_fig(fig);
	}
};

class CDataAllianceMember: public DBCBase<DataAllianceMember,DB_ALLIANCE_MEMBER>
{
public:
	virtual int Get(DataAllianceMember &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(flag);
		DBCREQ_NEED(position);
		DBCREQ_NEED(race_user_level);
		DBCREQ_NEED(authority);
		DBCREQ_NEED(helptimes);
		DBCREQ_NEED(userlevel);
		DBCREQ_NEED(join_ts);
		DBCREQ_NEED(race_point);
		DBCREQ_NEED(race_order_id);
		DBCREQ_NEED(race_order_ts);
		DBCREQ_NEED(race_order_recv);
		DBCREQ_NEED(race_order_finish);
		DBCREQ_NEED(race_order_cancel);

		DBCREQ_NEED(username);
		DBCREQ_NEED(fig);
		DBCREQ_NEED(race_grade_reward);
		DBCREQ_NEED(race_stage_reward);
		DBCREQ_NEED(race_order_log);
		DBCREQ_NEED(race_order_progress);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, alliance_id);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, flag);
		DBCREQ_GET_INT(data, position);
		DBCREQ_GET_INT(data, race_user_level);
		DBCREQ_GET_INT(data, authority);
		DBCREQ_GET_INT(data, helptimes);
		DBCREQ_GET_INT(data, userlevel);
		DBCREQ_GET_INT(data, join_ts);
		DBCREQ_GET_INT(data, race_point);
		DBCREQ_GET_INT(data, race_order_id);
		DBCREQ_GET_INT(data, race_order_ts);
		DBCREQ_GET_INT(data, race_order_recv);
		DBCREQ_GET_INT(data, race_order_finish);
		DBCREQ_GET_INT(data, race_order_cancel);

		DBCREQ_GET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_GET_CHAR(data, fig, BASE_FIG_LEN);
		DBCREQ_GET_BIN_SIZE(data, race_grade_reward);
		DBCREQ_GET_BIN_SIZE(data, race_stage_reward);
		DBCREQ_GET_BIN_SIZE(data, race_order_log);
		DBCREQ_GET_BIN_SIZE(data, race_order_progress);


		return 0;
	}

	virtual int Get(vector<DataAllianceMember> &data)
	{
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].alliance_id);
		DBCREQ_SET_KEY(data[0].alliance_id);

		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(flag);
		DBCREQ_NEED(position);
		DBCREQ_NEED(race_user_level);
		DBCREQ_NEED(authority);
		DBCREQ_NEED(helptimes);
		DBCREQ_NEED(userlevel);
		DBCREQ_NEED(join_ts);
		DBCREQ_NEED(race_point);
		DBCREQ_NEED(race_order_id);
		DBCREQ_NEED(race_order_ts);
		DBCREQ_NEED(race_order_recv);
		DBCREQ_NEED(race_order_finish);
		DBCREQ_NEED(race_order_cancel);
		DBCREQ_NEED(username);
		DBCREQ_NEED(fig);
		DBCREQ_NEED(race_grade_reward);
		DBCREQ_NEED(race_stage_reward);
		DBCREQ_NEED(race_order_log);
		DBCREQ_NEED(race_order_progress);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, alliance_id);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, flag);
		DBCREQ_ARRAY_GET_INT(data, position);
		DBCREQ_ARRAY_GET_INT(data, race_user_level);
		DBCREQ_ARRAY_GET_INT(data, authority);
		DBCREQ_ARRAY_GET_INT(data, helptimes);
		DBCREQ_ARRAY_GET_INT(data, userlevel);
		DBCREQ_ARRAY_GET_INT(data, join_ts);
		DBCREQ_ARRAY_GET_INT(data, race_point);
		DBCREQ_ARRAY_GET_INT(data, race_order_id);
		DBCREQ_ARRAY_GET_INT(data, race_order_ts);
		DBCREQ_ARRAY_GET_INT(data, race_order_recv);
		DBCREQ_ARRAY_GET_INT(data, race_order_finish);
		DBCREQ_ARRAY_GET_INT(data, race_order_cancel);
		DBCREQ_ARRAY_GET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_ARRAY_GET_CHAR(data, fig, BASE_FIG_LEN);
		DBCREQ_ARRAY_GET_BIN_SIZE(data, race_grade_reward);
		DBCREQ_ARRAY_GET_BIN_SIZE(data, race_stage_reward);
		DBCREQ_ARRAY_GET_BIN_SIZE(data, race_order_log);
		DBCREQ_ARRAY_GET_BIN_SIZE(data, race_order_progress);
		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataAllianceMember &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, flag);
		DBCREQ_SET_INT(data, position);
		DBCREQ_SET_INT(data, race_user_level);
		DBCREQ_SET_INT(data, authority);
		DBCREQ_SET_INT(data, helptimes);
		DBCREQ_SET_INT(data, userlevel);
		DBCREQ_SET_INT(data, join_ts);
		DBCREQ_SET_INT(data, race_point);
		DBCREQ_SET_INT(data, race_order_id);
		DBCREQ_SET_INT(data, race_order_ts);
		DBCREQ_SET_INT(data, race_order_recv);
		DBCREQ_SET_INT(data, race_order_finish);
		DBCREQ_SET_INT(data, race_order_cancel);
		DBCREQ_SET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
		DBCREQ_SET_BIN_SIZE(data, race_grade_reward);
		DBCREQ_SET_BIN_SIZE(data, race_stage_reward);
		DBCREQ_SET_BIN_SIZE(data, race_order_log);
		DBCREQ_SET_BIN_SIZE(data, race_order_progress);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAllianceMember &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, flag);
		DBCREQ_SET_INT(data, position);
		DBCREQ_SET_INT(data, race_user_level);
		DBCREQ_SET_INT(data, authority);
		DBCREQ_SET_INT(data, helptimes);
		DBCREQ_SET_INT(data, userlevel);
		DBCREQ_SET_INT(data, join_ts);
		DBCREQ_SET_INT(data, race_point);
		DBCREQ_SET_INT(data, race_order_id);
		DBCREQ_SET_INT(data, race_order_ts);
		DBCREQ_SET_INT(data, race_order_recv);
		DBCREQ_SET_INT(data, race_order_finish);
		DBCREQ_SET_INT(data, race_order_cancel);
		DBCREQ_SET_CHAR(data, username, BASE_NAME_LEN);
		DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
		DBCREQ_SET_BIN_SIZE(data, race_grade_reward);
		DBCREQ_SET_BIN_SIZE(data, race_stage_reward);
		DBCREQ_SET_BIN_SIZE(data, race_order_log);
		DBCREQ_SET_BIN_SIZE(data, race_order_progress);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAllianceMember &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
