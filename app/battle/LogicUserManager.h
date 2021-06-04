/*
 * LogicUserMangager.h
 *
 *  Created on: 2016-9-12
 *      Author: dawx62fac
 */

#ifndef LOGICUSERMANGAGER_H_
#define LOGICUSERMANGAGER_H_

#include "ServerInc.h"
#define USER_MAX_LEVEL 120

//离线用户数据保存控制
class OffUserSaveControl
{
public:
	OffUserSaveControl(unsigned uid);

	~OffUserSaveControl();
private:
	unsigned uid_;
};

class DBCUserBaseWrap
{
public:
	DBCUserBaseWrap(unsigned uid);
	DBCUserBaseWrap(unsigned index, DataBase& data);

	void Save()
	{
		container_->m_data->MarkChange(index_);
	}

	void MarkLoad()
	{
		container_->m_data->SetPlusTS(index_);
	}

	DataBase& Obj()
	{
		return data_;
	}

	const DataBase& Obj() const
	{
		return data_;
	}

	unsigned Id() const { return data_.uid; }

	/////////////////////////////////////////////////////////////////////////
	void CostCash(int cash, const std::string& reason);
	void AddCash(int cash, const std::string& reason);
	void CostCoin(int cash, const std::string& reason);
	void AddCoin(int cash, const std::string& reason);
	unsigned GetCash();
	unsigned GetCoin();
	//更新当前时间为玩家发广告的时间
	void UpdateCreateAdTs(unsigned ts);
	//获取玩家发广告的时间
	unsigned GetCreateAdTs();
	//更新VIP领取礼包的时间
	unsigned UpdateVIPRewardTs(unsigned ts);
	//更新领取分享奖励的时间
	unsigned UpdateShareRewardTs(unsigned ts);
	//更新VIP生产设备秒钻次数
	void UpdateVIPFreeSpeedCnt();
	//更新VIP祛除订单cd的次数
	void UpdateVIPRemoveOrderCdCnt();
	//累计充值
	void AccRecharge(int count, bool isPush = true);
	//
	void FinalAsynData();
	void AddAsynItem(unsigned id, unsigned count, const std::string& op = "asyn_data");

	//首充
	bool IsGetFirstRechargeReward() const;
	void SetFirstRechargeRewardStatus();

	//再次充值
	bool IsGetSecondRechargeReward() const;
	void SetSecondRechargeRewardStatus();

	//零点逻辑，解决登录跨天导致的bug
	void EveryDayAction(int di = 1);

	void 	RefreshVIPLevel(bool isPush = true);

	//获取用户注册小时数
	unsigned GetRegisterHours() const;

	bool AddExp(int exp);

	//通用属性操作
	void BaseProcess(const ::CommonGiftConfig::BaseItem& base, DataCommon::UserBaseChangeCPP* obj,
			const std::string& reason, double factor = 1.0);

	//检查条件
	int CheckBaseBeforeCost(unsigned uid, const string & reason, const CommonGiftConfig::BaseItem & base);
	//助手检查资源消耗
	bool CheckResBeforeCost(unsigned uid, const CommonGiftConfig::CommonModifyItem& cfg);
	//产资源所依赖的原料数量
	bool GetMaterialLeft(unsigned uid, uint32_t productId, uint32_t& mUd, uint32_t& mCount);

	void FullMessage(User::Base* obj) const;
	bool existBaseFlag(unsigned id);
	int SetBaseFlag(unsigned id);
private:
	int _Index(unsigned uid);

	int AsynType2ResourceType(ASYN_TYPE type);

	void OnUserUpgradeReward(unsigned old_level);
private:
	BaseManager*	container_;
	unsigned 		index_;
	DataBase& 		data_;
};

class LogicUserManager : public BattleSingleton, public CSingleton<LogicUserManager>
{
	enum{
		daily_rank_thumbs_up_max  = 3,//每日最大点赞次数

		index_of_thumbs_up_init = 10,//用来存储排行榜点赞的活动表初始位置索引,总共用活动表的三个字段来存储点赞信息
	};

	enum{
		e_Activity_UserData_1_index_0 = 0,//用于存储世界频道请求帮助次数

		daily_world_channel_help_max = 5,//世界频道每日求助最大次数
	};
	class _RechargeRecord
	{
	public:
		_RechargeRecord(unsigned nUid, unsigned nCash,unsigned nItemid)
			: uid(nUid), cash(nCash),itemid(nItemid)
		{

		}

		unsigned uid;
		unsigned cash;
		unsigned itemid;
	};
	class _FetchAdsReward
	{
	public:
		_FetchAdsReward(): propsId(0), propsCount(0), count(0)
		{
		}
		unsigned propsId;	// 奖励的物品ID
		unsigned propsCount;// 奖励数量
		unsigned count;	// 获取次数
	};

	class _RewardsRecord
	{
	public:
		_RewardsRecord(unsigned nUid, const CommonGiftConfig::CommonModifyItem& user_item)
			: uid(nUid), item(user_item)
	{
	}

	public:
		unsigned uid;
		const CommonGiftConfig::CommonModifyItem item;
	};

private:
	friend class CSingleton<LogicUserManager>;
	LogicUserManager();

public:
	virtual void CallDestroy() { Destroy();}

	int OnInit();
	void OnTimer1();

	uint32_t OnWatchAdsReward(uint32_t uid, uint32_t propsId);
	bool FetchWatchAdsReward(uint32_t uid, uint32_t& propsId, uint32_t& count);

	//消费钻石
	void CostCash(unsigned uid, int cash, string reason, unsigned & newcash);

	//每日零点推送给所有在线用户奖励
	int EveryDayAction();

	void NotifyRecharge(unsigned uid, unsigned cash,unsigned itemid = 0);
	//玩家经验值达到某一临界值则加入队列
	void UpdateCriticalExpReward(unsigned level, int add_exp, DataBase& data);

	int UpdateLevelReward(unsigned level, DataBase& data);
	//判断某个用户是否需要援助
	bool IsUserNeedHelp(unsigned uid);

	//更新帮助次数
	int UpdateHelpTimes(unsigned uid);
	int UpdateUserHelpTimes(unsigned uid, unsigned& aid);
	int UpdateAllianceHelpTimes(unsigned uid, unsigned aid);

	//消耗钻石
	int Process(unsigned uid, User::CostCashReq* req, User::CostCashResp* resp);

	//首充奖励
	int Process(unsigned uid, ProtoReward::GetFirstRechargeRewardReq* req, ProtoReward::GetFirstRechargeRewardResp* resp);
	//关注公众号奖励
	int Process(unsigned uid, ProtoReward::GetFollowPublicRewardReq* req, ProtoReward::GetFollowPublicRewardResp* resp);
	//收获产品看广告获得奖励
	int Process(unsigned uid, ProtoReward::FetchProductWatchAdsRewardReq* req, ProtoReward::FetchProductWatchAdsRewardResp* resp);

	//金币购买
	int Process(unsigned uid, User::PurchaseCoinReq* req, User::PurchaseCoinResp* resp);

	//获取排行榜点赞信息
	int Process(unsigned uid, User::GetThumbsUpReq* req, User::GetThumbsUpResp* resp);

	//排行榜点赞
	int Process(unsigned uid, User::RankThumbsUpReq* req);
	//跨服记录点赞信息
	int Process(User::CSRankThumbsUpReq* req);
	//处理跨服记录点赞信息
	int Process(User::CSRankThumbsUpResp* resp);

	//重置每日点赞次数
	int ResetDailyThumbsUpCnt(unsigned uid);

	//世界频道请求帮助
	int Process(unsigned uid, User::GetWorldChannelHelpCntReq* req, User::GetWorldChannelHelpCntResp* resp);

	//世界频道请求帮助
	int Process(unsigned uid, User::WorldChannelHelpReq* req, User::WorldChannelHelpResp* resp);

	//新手引导分享
	int Process(unsigned uid, User::NewUserGuideShareReq* req, User::NewUserGuideShareResp* resp);

	//每日重置帮助次数
	int ResetWorldChannelHelpCnt(unsigned uid);

	//向邀请者推送登录信息
	int PushLoginInfo(unsigned uid,unsigned loginTs);

	//向邀请者推送登录信息
	int HandlePushLoginInfo(unsigned uid,unsigned inviteUid,unsigned loginTs);

	//跨服推送登录信息
	int Process(User::CSPushLoginMsg* msg);

	//升级奖励
	int Process(unsigned uid, ProtoPush::RewardLevelUpReq* req, ProtoPush::RewardLevelUpResp* resp);

	//cdkey 兑换码奖励
	int Process(unsigned uid, User::UseCdKeyReq* req, User::UseCdKeyResp* resp);

	//通用处理，包含通用消耗和通用奖励
	int CommonProcess(unsigned uid, const CommonGiftConfig::CommonModifyItem& modifyitem, const std::string& reason,
			DataCommon::CommonItemsCPP* obj, double multiple = 1.0);
private:
	//通用处理底层
	void CommonUnderlaying(DBCUserBaseWrap& user, const CommonGiftConfig::CommonModifyItem& cfg, const std::string& reason,
			DataCommon::CommonItemsCPP* obj, double multiple = 1.0);

	void CheckPropsBeforeCost(unsigned uid, const string & reason, const CommonGiftConfig::CommonModifyItem& cfg);
private:
	BaseManager* baseData_;
	unsigned  start_time;

	std::deque<_RechargeRecord> recharge_records_;
	std::deque<_RewardsRecord>  critical_reward_;
	std::deque<_RewardsRecord> 	level_reward_;
	map<uint32_t, _FetchAdsReward> watchAdsReward;	// uid -> count
	int actid;
	int thumbsup[daily_rank_thumbs_up_max];

	int userActId;

	CDataExchangeCode m_DbExchangeCode;
};

#endif /* LOGICUSERMANGAGER_H_ */
