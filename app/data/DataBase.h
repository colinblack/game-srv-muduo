#ifndef DATABASE_H_
#define DATABASE_H_
#include "Kernel.h"

struct DataBase{
	uint32_t uid;
	uint32_t register_platform;//注册平台
	uint32_t register_time;//注册时间
	uint32_t last_login_platform;//(上次登录平台)废弃， 作为累计在线秒数
	uint32_t last_login_time;//上次登录时间
	uint32_t login_times;//连续登录次数
	uint32_t login_days;//累计登录天数
	uint32_t last_active_time;//上次活跃时间
	uint32_t last_off_time;//上次下线时间
	uint32_t forbid_ts;//封号到期时间
	char forbid_reason[BASE_FORBID_REASON_LEN];//封号原因
	uint32_t tutorial_stage;//新手教程步骤
	char name[BASE_NAME_LEN];//名字
	char fig[BASE_FIG_LEN];//头像url
	uint64_t exp;//经验
	uint32_t level;//等级
	uint32_t acccharge;//累计充值
	uint32_t viplevel;//vip等级
	uint32_t cash;//二级货币
	uint32_t coin;//代币
	uint32_t first_recharge;  	//首充领取标志。按位表示第一次，第二次首充状态
	uint32_t alliance_id;//加入的联盟
	uint32_t count; //收获次数
	char barrier[BARRIER_LENGTH];   //障碍物状态
	uint32_t next_create_ad_ts;//下一次可以发广告的时间
	uint32_t vip_reward_daily_gift_ts;//VIP用户每日领取奖励的ts
	uint32_t vip_daily_speed_product_cnt;//VIP用户每日已加速生产的次数
	uint32_t vip_daily_remove_ordercd_cnt;//VIP用户每日已祛除订单cd的次数
	uint32_t allian_allow_ts;  //允许入商会时间
	uint32_t next_donation_ts;  //下次可捐收时间
	uint32_t helptimes;  //帮助次数
	uint32_t npc_shop_update_ts; //npc商店更新ts
	uint32_t switch_status; //开关状态
	uint32_t share_reward_daily_gift_ts;//用户每日领取分享奖励的ts
	uint32_t vip_daily_gift_refresh_ts;//VIP礼包刷新的ts
	uint32_t assist_start_ts; //物品助手开始时间
	uint32_t assist_end_ts; //物品助手截止时间
	char expand_map_1[EXPAND_MAPGRID_LENGTH];   //扩充地图1每个格子解锁状态(0:未解锁,1:解锁)
	char expand_map_2[EXPAND_MAPGRID_LENGTH];   //扩充地图2每个格子解锁状态(0:未解锁,1:解锁)
	char expand_map_3[EXPAND_MAPGRID_LENGTH];   //扩充地图3每个格子解锁状态(0:未解锁,1:解锁)
	uint32_t npc_customer1_propsid;//NPC顾客1需要物品id
	uint32_t npc_customer1_propscnt;//NPC顾客1需要物品数量
	uint32_t npc_customer2_propsid;//NPC顾客2需要物品id
	uint32_t npc_customer2_propscnt;//NPC顾客1需要物品数量
	uint32_t next_random_box_refresh_ts;//随机宝箱下一次刷新时间
	uint32_t flag;	// 标志位
	uint32_t npc_customer1_next_visit_ts;//npc顾客1下一次访问的ts
	uint32_t npc_customer2_next_visit_ts;//npc顾客2下一次访问的ts
	uint32_t friendly_value;//友情值
	uint32_t blue_info;	//蓝钻信息(高16位表示蓝钻等级,低16位不同位表示不同蓝钻类型是否开启--1:普通，2:豪华，3:年费)
	uint32_t version;	//版本号
	uint32_t prosperity; //繁荣度
	uint32_t accthumbsup;//累积被点赞
	uint32_t inviteuid;//邀请的uid,指明玩家是被谁邀请进来的
	uint32_t isUnlockPetResidence;//宠物住所是否已解锁(0:未解锁,1:解锁)
	char pxgksChannel[64];   //微信平台用户的具体来源渠道
	DataBase(){
		uid = 0;
		register_platform = 0;
		register_time = 0;
		last_login_platform = 0;
		last_login_time = 0;
		login_times = 0;
		login_days = 0;
		last_active_time = 0;
		last_off_time = 0;
		forbid_ts = 0;
		tutorial_stage = 0;
		exp = 0;
		level = 0;
		acccharge = 0;
		viplevel = 0;
		cash = 0;
		coin = 0;
		first_recharge = 0;
		alliance_id = 0;
		count = 0;
		next_create_ad_ts = 0;
		vip_reward_daily_gift_ts = 0;
		vip_daily_speed_product_cnt = 0;
		vip_daily_remove_ordercd_cnt = 0;
		allian_allow_ts = 0;
		next_donation_ts = 0;
		helptimes = 0;
		npc_shop_update_ts = 0;
		switch_status = 0;
		share_reward_daily_gift_ts = 0;
		vip_daily_gift_refresh_ts = 0;
		assist_start_ts = 0;
		assist_end_ts = 0;
		npc_customer1_propsid = 0;
		npc_customer1_propscnt = 0;
		npc_customer2_propsid = 0;
		npc_customer2_propscnt = 0;
		next_random_box_refresh_ts = 0;
		flag = 0;
		npc_customer1_next_visit_ts = 0;
		npc_customer2_next_visit_ts = 0;
		friendly_value = 0;
		blue_info = 0;
		version = 0;
		prosperity = 0;
		accthumbsup = 0;
		inviteuid = 0;
		isUnlockPetResidence = 0;
		memset(forbid_reason, 0, sizeof(forbid_reason));
		memset(name, 0, sizeof(name));
		memset(fig, 0, sizeof(fig));
		memset(barrier, 0, sizeof(barrier));
		memset(expand_map_1, 0, sizeof(expand_map_1));
		memset(expand_map_2, 0, sizeof(expand_map_2));
		memset(expand_map_3, 0, sizeof(expand_map_3));
		memset(pxgksChannel, 0, sizeof(pxgksChannel));
	}

	void SetMessage(User::Base* msg)
	{
		msg->set_uid(uid);
		msg->set_registerplatform(register_platform);
		msg->set_registertime(register_time);
		msg->set_lastloginplatform(last_login_platform);
		msg->set_lastlogintime(last_login_time);
		msg->set_logintimes(login_times);
		msg->set_logindays(login_days);
		msg->set_lastactivetime(last_active_time);
		msg->set_lastofftime(last_off_time);
		msg->set_forbidts(forbid_ts);
		msg->set_forbidreason(string(forbid_reason));
		msg->set_tutorialstage(tutorial_stage);
		msg->set_name(string(name));
		msg->set_fig(string(fig));
		msg->set_exp(exp);
		msg->set_level(level);
		msg->set_acccharge(acccharge);
		msg->set_viplevel(viplevel);
		msg->set_cash(cash);
		msg->set_coin(coin);
		msg->set_firstrecharge(first_recharge);
		msg->set_allianceid(alliance_id);
		msg->set_barrier(barrier, BARRIER_LENGTH);
		msg->set_lastcreateadts(next_create_ad_ts);
		msg->set_allianallowts(allian_allow_ts);
		msg->set_nextdonationts(next_donation_ts);
		msg->set_helptimes(helptimes);
		msg->set_viprewarddailygiftts(vip_reward_daily_gift_ts);
		msg->set_vipdailyspeedproductcnt(vip_daily_speed_product_cnt);
		msg->set_vipdailyremoveordercdcnt(vip_daily_remove_ordercd_cnt);
		msg->set_switchstatus(switch_status);
		msg->set_sharerewarddailygiftts(share_reward_daily_gift_ts);
		msg->set_assiststartts(assist_start_ts);
		msg->set_assistendts(assist_end_ts);
		msg->set_expandmap1(expand_map_1,EXPAND_MAPGRID_LENGTH);
		msg->set_expandmap2(expand_map_2,EXPAND_MAPGRID_LENGTH);
		msg->set_expandmap3(expand_map_3,EXPAND_MAPGRID_LENGTH);
		msg->set_nextrandomboxrefreshts(next_random_box_refresh_ts);
		msg->set_flag(flag);
		msg->set_friendlyvalue(friendly_value);
		msg->set_version(version);
		msg->set_accthumbsup(accthumbsup);
		msg->set_isunlockpetresidence(isUnlockPetResidence);
		msg->set_wxchannel(string(pxgksChannel));
	}

	void SetMessage(ProtoUser::Base* msg)
	{
		msg->set_uid(uid);
		msg->set_exp(exp);
		msg->set_level(level);
		msg->set_acccharge(acccharge);
		msg->set_viplevel(viplevel);
		msg->set_cash(cash);
		msg->set_coin(coin);
		msg->set_firstrecharge(first_recharge);
		msg->set_allianceid(alliance_id);
		msg->set_lastcreateadts(next_create_ad_ts);
		msg->set_flag(flag);

		for(int i = 0; i < BARRIER_LENGTH; i++)
		{
			unsigned char data = (unsigned char )barrier[i];
			unsigned int value = (unsigned int) data;
			msg->add_barrier(value);
		}
	}

	void FromMessage(const ProtoUser::Base* msg)
	{
		exp = msg->exp();
		level = msg->level();
		acccharge = msg->acccharge();
		viplevel = msg->viplevel();
		cash = msg->cash();
		coin = msg->coin();
		first_recharge = msg->firstrecharge();
		alliance_id = msg->allianceid();
		flag = msg->flag();

		memset(barrier, 0, sizeof(barrier));
		for(int i = 0; i < msg->barrier_size(); i++)
		{
			unsigned int data = msg->barrier(i);
			barrier[i] = data;
		}

		next_create_ad_ts = msg->lastcreateadts();
	}

	bool IsOnline()
	{
		return last_off_time < last_login_time;
	}

	bool CanOff()
	{
		return IsOnline() && last_active_time + 12 * 3600 < Time::GetGlobalTime();
	}

	bool CanClear()
	{
		return !IsOnline() && last_off_time + 300 < Time::GetGlobalTime();
	}

	void AddExp(int exp_);
};

class CDataBase :public DBCBase<DataBase, DB_BASE>
{
public:
	virtual int Get(DataBase &data);
	virtual int Add(DataBase &data);
	virtual int Set(DataBase &data);
	virtual int Del(DataBase &data);
};

#endif /* DATABASE_H_ */
