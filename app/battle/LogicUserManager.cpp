/*
 * LogicUserManager.cpp
 *
 *  Created on: 2016-9-12
 *      Author: dawx62fac
 */

#include "LogicUserManager.h"
#include "BattleServer.h"

OffUserSaveControl::OffUserSaveControl(unsigned uid):
		uid_(uid)
{
	//加载用户的玩家档
	int ret = UserManager::Instance()->LoadArchives(uid_);

	if (ret)
	{
		throw runtime_error("load_other_data_error");
	}
}

OffUserSaveControl::~OffUserSaveControl()
{
	//析构的时候，进行离线用户保存
	UserManager::Instance()->SyncSave(uid_);
}

DBCUserBaseWrap::DBCUserBaseWrap(unsigned uid)
	: container_(BaseManager::Instance())
	, index_(_Index(uid))
	, data_(container_->m_data->data[index_])
{
}

DBCUserBaseWrap::DBCUserBaseWrap(unsigned index, DataBase& data)
	: container_(BaseManager::Instance())
	, index_(index)
	, data_(data)
{

}

int DBCUserBaseWrap::_Index(unsigned uid)
{
	int index = container_->GetIndex(uid);
	if (index < 0)
	{
		int ret = container_->LoadBuff(uid);
		if (0 != ret) {
			error_log("get_user_data_error.uid=%u,ret=%d", uid, ret);
			throw std::runtime_error("get_user_data_error");
		}

		return container_->GetIndex(uid);
	}

	return index;
}

void DBCUserBaseWrap::AddCash(int cash, const std::string& reason)
{
	if (cash <= 0)
	{
		error_log("[AddCash] wrong param. uid=%u, cash=%d", data_.uid, cash);
		throw std::runtime_error("invalid_cash_param");
	}

	COINS_LOG("[%s][uid=%u,ocash=%u,ncash=%u,chgcash=%d]"
			, reason.c_str(), data_.uid, data_.cash, data_.cash+cash, cash);

	data_.cash += cash;

	if(LogicXsgReportManager::Instance()->IsWXPlatform(data_.uid))
		LogicXsgReportManager::Instance()->XSGGetDiamondReport(data_.uid,reason,cash,data_.cash);
	Save();
}

unsigned DBCUserBaseWrap::GetCash()
{
	return data_.cash;
}

void DBCUserBaseWrap::CostCash(int cash, const std::string& reason)
{
	if (cash <= 0)
	{
		error_log("wrong param. uid=%u, cash=%d", data_.uid, cash);
		throw std::runtime_error("invalid_cash_param");
	}

	if (cash > (int)data_.cash)
	{
		error_log("cash not enough uid=%u,chgcash=%d,cash=%u", data_.uid, cash, data_.cash);
		throw std::runtime_error("cash_is_not_enough");
	}

	unsigned old_cash = data_.cash;
	data_.cash -= cash;

	if(LogicXsgReportManager::Instance()->IsWXPlatform(data_.uid))
		LogicXsgReportManager::Instance()->XSGCostDiamondReport(data_.uid,cash,data_.cash,reason);

	COINS_LOG("[%s][uid=%u,ocash=%u,ncash=%u,chgcash=%d]", reason.c_str(), data_.uid, old_cash, data_.cash, -cash);
	Save();
}

void DBCUserBaseWrap::AddCoin(int coin, const std::string& reason)
{
	if (coin <= 0)
	{
		error_log("[AddCoin] wrong param. uid=%u, coin=%d", data_.uid, coin);
		throw std::runtime_error("invalid_coin_param");
	}

	RESOURCE_LOG("[%s][uid=%u,ocoin=%u,ncoin=%u,chgcoin=%d]"
			, reason.c_str(), data_.uid, data_.coin, data_.coin+coin, coin);

	data_.coin += coin;

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(data_.uid)) {
		if("CoinPurchase" == reason)
			LogicXsgReportManager::Instance()->XSGPurchaseCoinReport(data_.uid,coin,data_.coin);
		else
			LogicXsgReportManager::Instance()->XSGGetCoinReport(data_.uid,coin,data_.coin,reason);
	}
	Save();
}

unsigned DBCUserBaseWrap::GetCoin()
{
	return data_.coin;
}

unsigned DBCUserBaseWrap::GetCreateAdTs()
{
	return data_.next_create_ad_ts;
}

void DBCUserBaseWrap::UpdateCreateAdTs(unsigned ts)
{
	data_.next_create_ad_ts = ts;
	Save();
}

unsigned DBCUserBaseWrap::UpdateVIPRewardTs(unsigned ts)
{
	data_.vip_reward_daily_gift_ts = ts;
	Save();
}

unsigned DBCUserBaseWrap::UpdateShareRewardTs(unsigned ts)
{
	data_.share_reward_daily_gift_ts = ts;
	Save();
}

void DBCUserBaseWrap::UpdateVIPFreeSpeedCnt()
{
	data_.vip_daily_speed_product_cnt ++;
	Save();
}

void DBCUserBaseWrap::UpdateVIPRemoveOrderCdCnt()
{
	data_.vip_daily_remove_ordercd_cnt ++;
	Save();
}


void DBCUserBaseWrap::CostCoin(int coin, const std::string& reason)
{
	if (coin <= 0)
	{
		error_log("wrong param. uid=%u, coin=%d", data_.uid, coin);
		throw std::runtime_error("invalid_coin_param");
	}

	if (coin > (int)data_.coin)
	{
		error_log("coin not enough uid=%u,chgcoin=%d,coin=%u", data_.uid, coin, data_.coin);
		throw std::runtime_error("coin_not_enough");
	}

	unsigned old_coin = data_.coin;
	data_.coin -= coin;

	if(LogicXsgReportManager::Instance()->IsWXPlatform(data_.uid))
	{
			unsigned sub = coin;
			LogicXsgReportManager::Instance()->XSGCostCoinReport(data_.uid,sub,data_.coin,reason);
	}

	RESOURCE_LOG("[%s][uid=%u,ocoin=%u,ncoin=%u,chgcoin=%d]", reason.c_str(), data_.uid, old_coin, data_.coin, -coin);

	Save();
}

void DBCUserBaseWrap::FullMessage(User::Base* obj) const
{
	data_.SetMessage(obj);
}

bool DBCUserBaseWrap::existBaseFlag(unsigned id)
{
	if(id >= 32)
	{
		error_log("wrong param. uid=%u, flagId=%u", data_.uid, id);
		throw std::runtime_error("invalid_flag_id_param");
	}
	return ((data_.flag >> id) & 0x1) > 0;
}
int DBCUserBaseWrap::SetBaseFlag(unsigned id)
{
	if(id >= 32)
	{
		error_log("wrong param. uid=%u, flagId=%u", data_.uid, id);
		throw std::runtime_error("invalid_flag_id_param");
	}
	data_.flag |= (1 << id);

	Save();
	return 0;
}

void DBCUserBaseWrap::AccRecharge(int count,  bool isPush)
{
	data_.acccharge += count;

	RefreshVIPLevel(isPush);

	Save();
}

bool DBCUserBaseWrap::IsGetFirstRechargeReward() const
{
	//检测第1位是否为0
	return data_.acccharge >= 100 && (0 == (data_.first_recharge & 1));
}

void DBCUserBaseWrap::SetFirstRechargeRewardStatus()
{
	//设置第1位为1
	data_.first_recharge |= 1;

	Save();
}

bool DBCUserBaseWrap::IsGetSecondRechargeReward() const
{
	//是否能够领取
	return data_.acccharge >= 888 && (0 == (data_.first_recharge & 2));
}

void DBCUserBaseWrap::SetSecondRechargeRewardStatus()
{
	data_.first_recharge |= 2;

	Save();
}

int DBCUserBaseWrap::AsynType2ResourceType(ASYN_TYPE type)
{
	switch(type)
	{
		case e_asyn_coin: return 0;
		default:
		{
			error_log("unknow_asyn_type: ", type);
			throw std::runtime_error("unknow_asyn_type");
		}
	}
}

void DBCUserBaseWrap::AddAsynItem(unsigned id, unsigned count, const std::string& op)
{
	if ((ASYN_TYPE)id == e_asyn_cash)
	{
		this->AddCash(count, op);

		Common::NoticePay* m = new Common::NoticePay;
		m->set_cash(data_.cash);
		m->set_viplevel(data_.viplevel);
		m->set_accrecharge(data_.acccharge);
		LogicManager::Instance()->sendMsg(data_.uid, m);

		Recharge4399ActivityManager::Instance()->OnCharge(data_.uid,count);
		Daily4399ActivityManager::Instance()->OnCharge(data_.uid,count);
	}
	else if ((ASYN_TYPE)id == e_asyn_accrecharge)
	{
		data_.acccharge += count;
		//LogicUserManager::Instance()->NotifyRecharge(data_.uid, count);
		Save();

		RefreshVIPLevel();

		Common::NoticePay* m = new Common::NoticePay;
		m->set_cash(data_.cash);
		m->set_viplevel(data_.viplevel);
		m->set_accrecharge(data_.acccharge);
		LogicManager::Instance()->sendMsg(data_.uid, m);
	}
	else if(id == e_asyn_month_card)
	{
		//月卡处理
		const ConfigCard::MonthCardCPP &monthcard = ConfigManager::Instance()->card.m_config.monthcard();

		//1.添加钻石奖励
		unsigned cash = monthcard.first_reward().based().cash();
		this->AddCash(cash, op);

		//2.添加累积充值
		data_.acccharge += cash;
		LogicUserManager::Instance()->NotifyRecharge(data_.uid, cash);

		//3.添加月卡处理
		LogicCardManager::Instance()->HandleCardPurchase(data_.uid,MONTH_CARD_ID);
	}
	else if(id == e_asyn_life_card)
	{
		//终生卡处理
		const ConfigCard::LifeCardCPP &liftcard = ConfigManager::Instance()->card.m_config.lifecard();

		//1.添加钻石奖励
		unsigned cash = liftcard.first_reward().based().cash();
		this->AddCash(cash, op);

		//2.添加累积充值
		data_.acccharge += cash;
		LogicUserManager::Instance()->NotifyRecharge(data_.uid, cash);

		//3.添加终生卡处理
		LogicCardManager::Instance()->HandleCardPurchase(data_.uid,LIFE_CARD_ID);
	}
	else if (id == e_asyn_coin)
	{
	}
	else if(id == e_asyn_exp)
	{
	}
	else if (id >= e_asyn_max)
	{
	}

}

void DBCUserBaseWrap::FinalAsynData()
{
	unsigned uid = data_.uid;
	const AsynMap& map_data = AsynManager::Instance()->GetMap();

	if (map_data.count(uid) <= 0)
	{
		return ;
	}

	const AsynMap::const_iterator asyn_it = map_data.find(uid);

	if (asyn_it == map_data.end())
	{
		return ;
	}

	const std::map<unsigned, unsigned>& user_map = asyn_it->second;

	for (std::map<unsigned, unsigned>::const_iterator it = user_map.begin();
			it != user_map.end();
			++it)
	{
		try
		{
			const AsynItem& as_item = AsynManager::Instance()->m_data->item[it->second];
			AddAsynItem(as_item.id, as_item.count);
		}
		catch(const std::exception& e)
		{
			error_log("%s", e.what());
		}

	}

	AsynManager::Instance()->Del(uid);
}

unsigned DBCUserBaseWrap::GetRegisterHours() const
{
	unsigned now = Time::GetGlobalTime();
	if (now > data_.register_time)
	{
		return (now - data_.register_time) / (60*60);
	}
	else
	{
		return 0;
	}
}

bool DBCUserBaseWrap::AddExp(int exp)
{
	unsigned old_lvl = data_.level;

	if (exp > 0)
	{
		data_.AddExp(exp);
		Save();

		if (data_.level > old_lvl)
		{
			LogicAllianceManager::Instance()->UpdateMemberLevel(data_.uid);
			//用户升级推送
			OnUserUpgradeReward(old_lvl);

			//用户等级更新、添加至任务系统
			unsigned value = data_.level - old_lvl;
			LogicMissionManager::Instance()->AddMission(data_.uid,mission_of_level,value);

			//首次推送NPC商人信息
			LogicNPCSellerManager::Instance()->PushNPCSellerInfo(data_.uid,old_lvl,data_.level);

			//西山居上报
			if(LogicXsgReportManager::Instance()->IsWXPlatform(data_.uid))
				LogicXsgReportManager::Instance()->XSGLevelUpReport(data_.uid);

			std::string open_id = UserManager::Instance()->GetOpenId(data_.uid);
			USER_LOG("[upgrade]uid=%u,open_id=%s,name:%s,old_level=%d,new_level:%d"
					, data_.uid, open_id.c_str(),data_.name, old_lvl, data_.level);
		}

		return true;
	}

	return false;
}

void DBCUserBaseWrap::OnUserUpgradeReward(unsigned old_level)
{
	LogicResourceManager::Instance()->SyncUserLevel(data_.uid, data_.level);

	ProtoPush::PushUserUpLevel* msg = new ProtoPush::PushUserUpLevel;
	DataCommon::BaseItemCPP* basemsg = msg->mutable_commons()->mutable_userbase()->add_baseitem();

	basemsg->set_type(type_level);
	basemsg->set_change(data_.level - old_level);
	basemsg->set_totalvalue(data_.level);

	//等级达到16级开通物品助手
	const ConfigAssistor::AssistCfg& assistCfg =AssistorCfgWrap().GetAssistorCfg();
	if(data_.level >= assistCfg.assistinfo().level() && 0 == data_.assist_end_ts)
	{
		data_.assist_start_ts = 0;
		data_.assist_end_ts = Time::GetGlobalTime() + 3600*24;

		ProtoAssistor::OpenAssistorResp* msg_1 = new ProtoAssistor::OpenAssistorResp;
		msg_1->set_endts(data_.assist_end_ts );
		LogicManager::Instance()->sendMsg(data_.uid, msg_1);

		//debug_log("[OpenAssistor] uid: %u, start_time: %u, end_time: %u, level: %u", data_.uid, data_.assist_start_ts,  data_.assist_end_ts,data_.level);
	}


	LogicManager::Instance()->sendMsg(data_.uid, msg);
}

void DBCUserBaseWrap::BaseProcess(const CommonGiftConfig::BaseItem & base, DataCommon::UserBaseChangeCPP* obj,
		  const std::string& reason, double nMul)
{
	CheckBaseBeforeCost(data_.uid, reason, base);

	string strlog;

	char szchgtemp[1000] = {0};
	sprintf(szchgtemp, "[%s] uid=%u,", reason.c_str(), data_.uid);
	strlog += szchgtemp;

	if (base.has_coin())
	{
		int coin = base.coin() * nMul;
		if (coin > 0)
		{
			AddCoin(coin, reason);
		}
		else if (coin < 0)
		{
			CostCoin(-coin, reason);
		}

		sprintf(szchgtemp, "chgcoin=%d,coin=%u,", coin, data_.coin);
		strlog += szchgtemp;

		DataCommon::BaseItemCPP * item = obj->add_baseitem();

		item->set_type(type_coin);
		//item->set_id(id);
		item->set_change(coin);
		item->set_totalvalue(data_.coin);
	}

	//只支持加经验
	if (base.has_exp() && base.exp() > 0)
	{
		//经验没有扣除这一说法，所以AddExp方法内，排除了经验小于0的情况
		int exp = nMul * base.exp();
		unsigned old_level = data_.level;
		bool success = AddExp(exp);

		if (success)
		{
			sprintf(szchgtemp, "chgexp=%d,exp=%llu,", exp, data_.exp);
			strlog += szchgtemp;

			DataCommon::BaseItemCPP * item = obj->add_baseitem();

			item->set_type(type_exp);
			//item->set_id(id);
			item->set_change(exp);
			item->set_totalvalue(data_.exp);
			try
			{
				LogicUserManager::Instance()->UpdateCriticalExpReward(old_level, exp, data_);
			}
			catch (const std::exception& e)
			{
				error_log("uid: %u, %s", data_.uid, e.what());
			}
			try
			{
				LogicUserManager::Instance()->UpdateLevelReward(old_level, data_);
			}
			catch (const std::exception& e)
			{
				error_log("uid: %u, %s", data_.uid, e.what());
			}
		}
	}

	if (base.has_cash())
	{
		//调用加钻石的接口
		int cash = base.cash() * nMul;

		if (cash > 0)
		{
			AddCash(cash, reason);
		}
		else if (cash < 0)
		{
			CostCash(-cash, reason);
		}

		DataCommon::BaseItemCPP * item = obj->add_baseitem();

		item->set_type(type_cash);
		//item->set_id(id);
		item->set_change(cash);
		item->set_totalvalue(data_.cash);
	}

	if (base.has_friend_())
	{
		//调用加钻石的接口
		int friend_ = base.friend_() * nMul;
		if (friend_ > 0)
		{
			data_.friendly_value += friend_;

			sprintf(szchgtemp, "chgfriend=%d,friend=%u,", friend_, data_.friendly_value);
			strlog += szchgtemp;
			DataCommon::BaseItemCPP * item = obj->add_baseitem();

			item->set_type(type_friend_value);
			//item->set_id(id);
			item->set_change(friend_);
			item->set_totalvalue(data_.friendly_value);
		}

	}
	Save();

	RESOURCE_LOG(strlog.c_str());
}

int DBCUserBaseWrap::CheckBaseBeforeCost(unsigned uid, const string & reason, const CommonGiftConfig::BaseItem & base)
{
	if (base.has_coin() && base.coin() < 0 && static_cast<unsigned>(-base.coin()) > data_.coin)
	{
		error_log("coin not enough. uid=%u,need=%d,code=%s", uid, base.coin(), reason.c_str());
		throw runtime_error("coin_not_enough");
	}

	if (base.has_cash() && base.cash() < 0 && static_cast<unsigned>(-base.cash()) > data_.cash)
	{
		error_log("cash not enough. uid=%u,need=%d,code=%s", uid, base.cash(), reason.c_str());
		throw runtime_error("cash_is_not_enough");
	}

	return 0;
}
bool DBCUserBaseWrap::CheckResBeforeCost(unsigned uid, const CommonGiftConfig::CommonModifyItem& cfg)
{
	if (cfg.based().has_coin() && cfg.based().coin() < 0 && static_cast<unsigned>(-cfg.based().coin()) > data_.coin)
	{
		return false;
	}

	if (cfg.based().has_cash() && cfg.based().cash() < 0 && static_cast<unsigned>(-cfg.based().cash()) > data_.cash)
	{
		return false;
	}
	if (cfg.props_size() > 0)
	{
		for(int i = 0; i < cfg.props_size(); ++i)
		{
			if (cfg.props(i).count() < 0)
			{
				unsigned int propsid = cfg.props(i).id();

				//只支持可叠加装备
				bool isOverlay = LogicPropsManager::Instance()->IsAllowOverlay(propsid);

				if (!isOverlay)
				{
					return false;
				}

				int ud = DataItemManager::Instance()->GetPropsUd(uid, propsid);

				if (-1 == ud)
				{
					return false;
				}
				if((propsid >= 60001 && propsid <= 60012) || propsid == 60104)	// 至少保留10个农作物
				{
					int left = cfg.props(i).count() + DataItemManager::Instance()->GetData(uid, ud).item_cnt;
					if(left < 10)
					{
						return false;
					}
				}
				if ( static_cast<unsigned>(-cfg.props(i).count()) > DataItemManager::Instance()->GetData(uid, ud).item_cnt)
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool DBCUserBaseWrap::GetMaterialLeft(unsigned uid, uint32_t productId, uint32_t& mUd, uint32_t& mCount)
{
	mUd = 0;
	mCount = 0;
	ItemCfgWrap itemcfgwrap;
	const CommonGiftConfig::CommonModifyItem& cfg = itemcfgwrap.GetPropsItem(productId).material();
	for(int i = 0; i < cfg.props_size(); ++i)
	{
		if (cfg.props(i).count() < 0)
		{
			unsigned int propsid = cfg.props(i).id();

			//只支持可叠加装备
			bool isOverlay = LogicPropsManager::Instance()->IsAllowOverlay(propsid);

			if (!isOverlay)
			{
				return 0;
			}

			mUd = DataItemManager::Instance()->GetPropsUd(uid, propsid);

			if (-1 == mUd)
			{
				return 0;
			}
			mCount = DataItemManager::Instance()->GetData(uid, mUd).item_cnt;
		}
	}
	return 0;
}
void DBCUserBaseWrap::EveryDayAction(int di)
{
	//day interval
	if(di == 1)
	{
		++data_.login_times;
	}
	else
	{
		data_.login_times = 0;
	}

	data_.login_days ++;

	//每日重置VIP生产设备的秒钻次数
	if(data_.viplevel >= 1)
	{
		data_.vip_daily_speed_product_cnt = 0;
		data_.vip_daily_remove_ordercd_cnt  = 0;
	}

	//每日重置邮件狗相关数据
	LogicMailDogManager::Instance()->ResetMailDogData(data_.uid);

	//每日重置转盘数据
	RotaryTableActivity::Instance()->ResetActivity(data_.uid);

	//每日重置大厅活动数据
	LogicActivityTencent::Instance()->ResetActivity(data_.uid);

	//每日重置钻石加成使用次数
	LogicOrderManager::Instance()->ResetOrderBonusUsedCnt(data_.uid);

	//重置月卡与终生卡领取标记
	LogicCardManager::Instance()->ResetCardReward(data_.uid);

	//重置每日点赞次数
	LogicUserManager::Instance()->ResetDailyThumbsUpCnt(data_.uid);

	//重置世界频道求助次数
	LogicUserManager::Instance()->ResetWorldChannelHelpCnt(data_.uid);

	//重置航运加成次数
	LogicShippingManager::Instance()->ResetShipBonusCnt(data_.uid);

	//重置气球看广告次数
	LogicAccessAdManager::Instance()->ResetViewAdCnt(data_.uid);

	//重置新分享活动相关数据
	LogicNewShareActivity::Instance()->ResetNewShareData(data_.uid);

	//重置每日看广告减少建筑cd的次数
	LogicBuildManager::Instance()->ResetViewAdReduceBuildTime(data_.uid);

	Save();
}


void DBCUserBaseWrap::RefreshVIPLevel(bool isPush)
{
	uint8_t level = data_.viplevel;
	data_.viplevel = LogicVIPManager::Instance()->GetVIPLevelByCharge(data_.uid);

	if (data_.viplevel > level)
	{
		Save();

		unsigned uid = data_.uid;

		if (isPush)
		{
		}
		OfflineResourceItem& resItem = LogicResourceManager::Instance()->Get(uid);
		resItem.viplevel = data_.viplevel;

		//推送VIP等级信息
		ProtoVIP::PushVIPLevel *msg = new ProtoVIP::PushVIPLevel;
		msg->set_newlevel(data_.viplevel);
		LogicManager::Instance()->sendMsg(data_.uid,msg);
		LogicAllianceManager::Instance()->UpdateMemberNow(uid);

		BaseManager::Instance()->DoSave(uid);	//存档以便聊天服务器访问到最新的vip信息
	}
}
///////////////////////////////////////////////////////////////////////////////////
LogicUserManager::LogicUserManager()
	: baseData_(BaseManager::Instance())
{
	if (baseData_ == NULL)
	{
		error_log("base_data_manager_instance_error");
		throw std::runtime_error("base_data_manager_instance_error");
	}

	start_time = Time::GetGlobalTime();

	//初始化用于存储排行榜点赞的信息
	actid = e_Activity_FriendlyTree;
	for(int i = index_of_thumbs_up_init; i < index_of_thumbs_up_init + daily_rank_thumbs_up_max; i++)
	{
		thumbsup[i - index_of_thumbs_up_init] = i;
	}

	//初始化,用于存储世界频道点赞请求
	userActId = e_Activity_UserData_1;
}

int LogicUserManager::OnInit(){
	BattleServer::Instance()->SetTimerCB(std::bind(&LogicUserManager::OnTimer1, this), 1.0);
}

void LogicUserManager::OnTimer1()
{
	//unsigned now = Time::GetGlobalTime();

	while(! recharge_records_.empty())
	{
		const _RechargeRecord& record = recharge_records_.front();
		try
		{
			DBCUserBaseWrap user(record.uid);

			//unsigned oldvip = user.Obj().viplevel;


			//购买月卡与终生卡处理
			if(record.itemid == MONTH_CARD_ID || record.itemid == LIFE_CARD_ID)
			{
				LogicCardManager::Instance()->HandleCardPurchase(record.uid,record.itemid);
			}
			/*
			{
				LogicCardManager::Instance()->HandleCardPurchase(record.uid,record.cash);
			}
			*/
			//添加充值记录到玩家的档里
			DataChargeHistoryManager::Instance()->AddChargeHistory(record.uid, record.cash);

			if(record.itemid != MONTH_CARD_ID && record.itemid != LIFE_CARD_ID) {
				//首充双倍活动
				RechargeActivity::Instance()->AddDoubleCash(record.uid, record.cash);
				//4399首冲翻倍
				Recharge4399ActivityManager::Instance()->OnCharge(record.uid, record.cash);
				//4399每日充值
				Daily4399ActivityManager::Instance()->OnCharge(record.uid, record.cash);
			}

			user.RefreshVIPLevel();

			Common::NoticePay* m = new Common::NoticePay;
			m->set_cash(user.Obj().cash);
			m->set_viplevel(user.Obj().viplevel);
			m->set_accrecharge(user.Obj().acccharge);

			LogicManager::Instance()->sendMsg(record.uid, m);

			//推送最新的累积充值数据
			User::PushAccumulateChangeReq * pushmsg = new User::PushAccumulateChangeReq;
			//GET_RMI(record.uid).SetMessage(pushmsg->mutable_change_acccharge());
			//改为使用玩家档中的充值数据
			DataChargeHistoryManager::Instance()->FullMessage(record.uid, pushmsg->mutable_changeacccharge());

			LogicManager::Instance()->sendMsg(record.uid, pushmsg);

			//小米平台充值邮件提醒
			if(LogicSysMailManager::Instance()->ValidPlatForm(record.uid))
			{
				LogicSysMailManager::Instance()->FirstRechargeAddMail(record.uid);
			}
		}
		catch (const std::exception& e)
		{
			error_log("uid: %u, %s", record.uid, e.what());
		}

		BattleServer::Instance()->SetTimerCB(std::bind(&LogicUserManager::OnTimer1, this), 1.0);
		recharge_records_.pop_front();
	}

	while(! critical_reward_.empty())
	{
		const _RewardsRecord & record = critical_reward_.front();
		try
		{
			User::CriticalRewardsResp* critical_reward_msg = new User::CriticalRewardsResp;
			LogicUserManager::Instance()->CommonProcess(record.uid, record.item, "CritiaclRewards", critical_reward_msg->mutable_commons());
			LogicManager::Instance()->sendMsg(record.uid, critical_reward_msg);
		}
		catch(const std::exception& e)
		{
			error_log("uid: %u, %s", record.uid, e.what());
		}
		critical_reward_.pop_front();
	}

	while(!level_reward_.empty())
	{
		const _RewardsRecord & record = level_reward_.front();
		try
		{
			User::LevelRewardsResp* level_reward_msg = new User::LevelRewardsResp;
			LogicUserManager::Instance()->CommonProcess(record.uid, record.item, "LevelRewards", level_reward_msg->mutable_commons());
			LogicManager::Instance()->sendMsg(record.uid, level_reward_msg);
		}
		catch(const std::exception& e)
		{
			error_log("uid: %u, %s", record.uid, e.what());
		}
		level_reward_.pop_front();
	}
}
// 返回物品数量
uint32_t LogicUserManager::OnWatchAdsReward(uint32_t uid, uint32_t propsId)
{
	if(propsId == 0)
	{
		return 0;
	}
	uint32_t count = 0;
	map<uint32_t, _FetchAdsReward>::iterator iter = watchAdsReward.find(uid);
	if(iter == watchAdsReward.end())
	{
		_FetchAdsReward reward;
		reward.propsId = propsId;
		reward.count = 0;
		watchAdsReward[uid] = reward;
	}
	count = ++watchAdsReward[uid].count;
	const ConfigReward::FetchProductWatchAds &ads = ConfigManager::Instance()->reward.m_config.fetchproductwatchads();

	if(count < ads.mintime())	// 收获产品小于指定次数不能触发随机奖励
	{
		return 0;
	}
	for(uint32_t i = 0; i < ads.reward_size(); ++i)
	{
		const ConfigReward::FetchProductWatchAdsItem& item = ads.reward(i);
		if(propsId == item.id())
		{
			if(Math::GetRandomInt(10000) < (100 * item.rate()))
			{
				watchAdsReward[uid].propsId = propsId;
				watchAdsReward[uid].propsCount = item.count();
				watchAdsReward[uid].count = 0;	// 重置次数

				ProtoReward::FetchProductWatchAdsReward *resp = new ProtoReward::FetchProductWatchAdsReward;
				resp->set_propsid(propsId);
				resp->set_count(item.count());
				LMI->sendMsg(uid, resp);

				return item.count();
			}
		}
	}
	return 0;
}

bool LogicUserManager::FetchWatchAdsReward(uint32_t uid, uint32_t& propsId, uint32_t& count)
{
	map<uint32_t, _FetchAdsReward>::iterator iter = watchAdsReward.find(uid);
	if(iter == watchAdsReward.end())
	{
		return false;
	}
	propsId = iter->second.propsId;
	count = iter->second.propsCount;
	if(propsId == 0)
	{
		return false;
	}
	// 重置奖励
	iter->second.propsId = 0;
	iter->second.propsCount = 0;

	return true;
}
void LogicUserManager::NotifyRecharge(unsigned uid, unsigned cash,unsigned itemid)
{
	recharge_records_.push_back(_RechargeRecord(uid, cash,itemid));
}

void LogicUserManager::UpdateCriticalExpReward(unsigned level, int add_exp, DataBase& data)
{
	const UserCfg::User& userCfg = UserCfgWrap().User();
	//加经验前玩家等级
	unsigned pre_level = level;
	if(pre_level >= userCfg.user_exp_size())
	{
		return;
	}

	//加经验后玩家等级
	unsigned cur_level = data.level;
	unsigned pre_exp= data.exp - add_exp;

	while(pre_level <= cur_level)
	{
		if(pre_level == USER_MAX_LEVEL)
		{
			return;
		}
		unsigned pre_level_exp 		= userCfg.user_exp(pre_level-1);
		unsigned next_level_exp 	= userCfg.user_exp(pre_level);
		unsigned index = 1;
		if(pre_level < 10)
		{
			index = 1;
		}
		else if(10 <= pre_level && pre_level < 50)
		{
			index =  pre_level / 10;
		}
		else if(pre_level >= 50)
		{
			index = 5;
		}

		unsigned gap = (unsigned)ceil(static_cast<double>(next_level_exp - pre_level_exp) / (index + 1));

		for(unsigned point = pre_level_exp + gap, i = 0; point <= data.exp && i < index; point += gap, ++i)
		{
			if(point > pre_exp && point <= data.exp)
			{
				critical_reward_.push_back(_RewardsRecord(data.uid, userCfg.criticalpoint(pre_level-1).prize(i)));
			}
		}
		++pre_level;

	}
}

int LogicUserManager::UpdateLevelReward(unsigned level, DataBase& data)
{

	const LevelupUnlock::LevelupUnlockCfg & level_reward = LevelupUnlockCfgWrap().GetLevelupUnlockCfg();
	unsigned inc = level + 1;
	while(inc <= data.level)
	{
		level_reward_.push_back(_RewardsRecord(data.uid, level_reward.levels(inc-1).prize(0)));
		++inc;
	}

	return 0;
}

void LogicUserManager::CostCash(unsigned uid, int cash, string reason, unsigned & newcash)
{
	DBCUserBaseWrap userWrap(uid);
	userWrap.CostCash(cash, reason);

	newcash = userWrap.Obj().cash;
}

int LogicUserManager::EveryDayAction()
{
	std::vector<unsigned> vUsers;
	UserManager::Instance()->GetOnlineUsers(vUsers);

	for (size_t i = 0; i < vUsers.size(); ++i)
	{
		try
		{
			DBCUserBaseWrap user(vUsers[i]);
			user.EveryDayAction();
		}
		catch(const std::exception& e)
		{
			error_log("uid: %u, %s", vUsers[i], e.what());
		}
	}

	return R_SUCCESS;
}

bool LogicUserManager::IsUserNeedHelp(unsigned uid)
{
	//判断用户是否在线
	int ret = UserManager::Instance()->LoadArchives(uid);

	if (ret)
	{
		return false;
	}

	//到这里，该用户的数据应该都是在内存中了
	//遍历果树，查询果树是否有需要援助
	vector<unsigned> indexs;
	DataFruitManager::Instance()->GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataFruit & fruit = DataFruitManager::Instance()->GetDataByIndex(indexs[i]);

		if (fruit.status == status_seek_help)
		{
			return true;
		}
	}

	//查询航运订单，是否有援助
	indexs.clear();
	DataShippingboxManager::Instance()->GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataShippingbox & box = DataShippingboxManager::Instance()->GetDataByIndex(indexs[i]);

		if (box.aid_status == LogicShippingManager::aid_stauts_public ||
				box.aid_status == LogicShippingManager::aid_status_commercial)
		{
			return true;
		}
	}

	return false;
}

int LogicUserManager::UpdateHelpTimes(unsigned uid)
{
	unsigned aid = 0;
	UpdateUserHelpTimes(uid, aid);
	UpdateAllianceHelpTimes(uid, aid);

	return 0;
}
int LogicUserManager::UpdateUserHelpTimes(unsigned uid, unsigned& aid)
{
	//帮助，都是在线操作的
	DBCUserBaseWrap userwrap(uid);
	userwrap.Obj().helptimes += 1;
	aid = userwrap.Obj().alliance_id;
	return 0;
}
int LogicUserManager::UpdateAllianceHelpTimes(unsigned uid, unsigned aid)
{
	if(!IsAllianceId(aid))
		return 0;
	if(CMI->IsNeedConnectByAID(aid))
	{
		ProtoAlliance::RequestAddMemberHelpTimesBC* m = new ProtoAlliance::RequestAddMemberHelpTimesBC;
		m->set_aid(aid);
		m->set_uid(uid);
		return BMI->BattleConnectNoReplyByAID(aid, m);
	}
	else
	{
		LogicAllianceManager::Instance()->AddMemberHelpTimesLocal(uid, aid);
	}
	return 0;
}

int LogicUserManager::Process(unsigned uid, User::CostCashReq* req, User::CostCashResp* resp)
{
	DBCUserBaseWrap user(uid);
	if (req->cash() > user.Obj().cash)
	{
		throw std::runtime_error("cash_is_not_enough");
	}

	if (req->opcode().empty())
	{
		throw std::runtime_error("need_op_code");
	}

	user.CostCash(req->cash(), req->opcode());

	if (resp)
	{
		resp->set_cash(user.Obj().cash);
		resp->set_operation(req->operation());
	}

	return R_SUCCESS;
}

int  LogicUserManager::Process(unsigned uid, ProtoReward::GetFirstRechargeRewardReq* req, ProtoReward::GetFirstRechargeRewardResp* resp)
{
	//验证
	DBCUserBaseWrap userwrap(uid);
	unsigned acccharge = userwrap.Obj().acccharge;
	unsigned first_recharge = userwrap.Obj().first_recharge;
	bool valid = acccharge >= 1 && (0 == (first_recharge & 1));
	if(!valid)
	{
		throw std::runtime_error("status_error");
	}

	//更新首充标记
	DataBase &base = BaseManager::Instance()->Get(uid);
	base.first_recharge |= 1;
	BaseManager::Instance()->UpdateDatabase(base);

	//添加返回信息
	resp->set_firstrecharge(base.first_recharge);

	//领取奖励
	const ConfigReward::FirstRechargeRewardCPP & reward = ConfigManager::Instance()->reward.m_config.firstrecharge();
	const CommonGiftConfig::CommonModifyItem& modifyitem = reward.common();
	CommonProcess(uid,modifyitem,"first_recharge_reward",resp->mutable_common());

	//首充主题奖励
	LogicThemeManager::Instance()->FirstChargeRewardTheme(uid,resp->mutable_theme());

	return 0;
}

int  LogicUserManager::Process(unsigned uid, ProtoReward::GetFollowPublicRewardReq* req, ProtoReward::GetFollowPublicRewardResp* resp)
{
	//验证
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.existBaseFlag(base_flag_id_follow_public_reward))
	{
		error_log("already_get_reward uid=%u", uid);
		throw std::runtime_error("already_get_reward");
	}
	userwrap.SetBaseFlag(base_flag_id_follow_public_reward);

	//领取奖励
	const ConfigReward::FollowPublicRewardCPP & reward = ConfigManager::Instance()->reward.m_config.followpublic();
	const CommonGiftConfig::CommonModifyItem& modifyitem = reward.common();
	CommonProcess(uid,modifyitem,"follow_public_reward",resp->mutable_common());

	return 0;
}
int LogicUserManager::Process(unsigned uid, ProtoReward::FetchProductWatchAdsRewardReq* req, ProtoReward::FetchProductWatchAdsRewardResp* resp)
{
	//验证
	DBCUserBaseWrap userwrap(uid);
	uint32_t propsId = 0;
	uint32_t count = 0;
	if(!FetchWatchAdsReward(uid, propsId, count))
	{
		error_log("not exist ads reward uid=%u", uid);
		throw std::runtime_error("not_exist_ads_reward");
	}
	ItemCfgWrap itemcfgwrap;
	int leftSpace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, itemcfgwrap.GetPropsItem(propsId).location());
	if(leftSpace <= 0)
	{
		error_log("no left space uid=%u", uid);
		throw std::runtime_error("no_left_space");
	}
	if(count > leftSpace)	// 不可超出容量上限
	{
		count = leftSpace;
	}

	CommonGiftConfig::CommonModifyItem reward;

	CommonGiftConfig::PropsItem *prop = reward.add_props();
	prop->set_id(propsId);
	prop->set_count(count);

	LogicUserManager::Instance()->CommonProcess(uid, reward, "fetch_product_watch_ads_reward", resp->mutable_common());

	return 0;
}


int LogicUserManager::Process(unsigned uid, User::PurchaseCoinReq* req, User::PurchaseCoinResp* resp)
{
	unsigned id = req->id();

	//获取金币购买的配置
	const UserCfg::CoinPurchase & coincfg = UserCfgWrap().GetCoinPurchaseCfg(id);

	//先进行钻石消耗
	CommonProcess(uid, coincfg.cost(), "CoinPurchase", resp->mutable_commons());

	//金币获得
	CommonProcess(uid, coincfg.reward(), "CoinPurchase", resp->mutable_commons());

	return 0;
}

int LogicUserManager::Process(unsigned uid, User::GetThumbsUpReq* req, User::GetThumbsUpResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		resp->mutable_thumbsup()->add_index(activity.actdata[thumbsup[i]]);
	}
	return 0;
}

int LogicUserManager::Process(unsigned uid, User::RankThumbsUpReq* req)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned othuid = req->othuid();
	unsigned index1 = req->index1();//榜单索引
	unsigned index2 = req->index2();//对应帮当下的索引

	//校验今日是否已超过点赞次数
	unsigned thumbsupcnt = 0;
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		if(activity.actdata[thumbsup[i]] != 0)
			thumbsupcnt ++;
	}
	if(thumbsupcnt >= daily_rank_thumbs_up_max)
	{
		throw std::runtime_error("thumbs_up_is_maxed");
	}

	//校验对应位置是否已点过赞
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		if(activity.actdata[thumbsup[i]] != 0)
		{
			unsigned highdata = activity.actdata[thumbsup[i]] >> 16;
			unsigned lowdata =  activity.actdata[thumbsup[i]] & 0x0000ffff;
			if(highdata == index1 && lowdata == index2)
			{
				throw std::runtime_error("already_thumbs_up");
			}
		}
	}

	//处理被点赞
	if(CMI->IsNeedConnectByUID(othuid))		//跨服记录点赞信息
	{
		User::CSRankThumbsUpReq* m = new User::CSRankThumbsUpReq;
		m->set_othuid(othuid);
		m->set_myuid(uid);
		m->set_index1(index1);
		m->set_index2(index2);
		int ret = BMI->BattleConnectNoReplyByUID(othuid, m);
		return ret;
	}
	User::RankThumbsUpResp* resp = new User::RankThumbsUpResp;
	//1.累积被点赞次数
	DBCUserBaseWrap othuserwrap(othuid);
	othuserwrap.Obj().accthumbsup += 1;
	DataBase & base = BaseManager::Instance()->Get(othuid);
	BaseManager::Instance()->UpdateDatabase(base);
	//2.存储被点赞的索引位置
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		if(activity.actdata[thumbsup[i]] == 0)
		{
			activity.actdata[thumbsup[i]] = ((activity.actdata[thumbsup[i]] >> 16) | index1) << 16;
			activity.actdata[thumbsup[i]] = activity.actdata[thumbsup[i]] | index2;
			DataGameActivityManager::Instance()->UpdateActivity(activity);
			break;
		}
	}
	//3.处理点赞奖励
	DBCUserBaseWrap myuserwrap(uid);
	unsigned level = myuserwrap.Obj().level;
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::BaseItem *baseitem = common.mutable_based();
	baseitem->set_coin(level * 5);
	baseitem->set_exp(level * 3);
	CommonProcess(uid,common,"thumbs_up_reward",resp->mutable_commons());

	//4.设置返回
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		resp->mutable_thumbsup()->add_index(activity.actdata[thumbsup[i]]);
	}
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}

int LogicUserManager::Process(User::CSRankThumbsUpReq* req)
{
	unsigned othuid = req->othuid();
	DBCUserBaseWrap userwrap(othuid);

	//处理被点赞次数
	userwrap.Obj().accthumbsup += 1;
	DataBase & base = BaseManager::Instance()->Get(othuid);
	BaseManager::Instance()->UpdateDatabase(base);

	User::CSRankThumbsUpResp* resp = new User::CSRankThumbsUpResp;
	resp->set_myuid(req->myuid());
	resp->set_index1(req->index1());
	resp->set_index2(req->index2());
	return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);
}

int LogicUserManager::Process(User::CSRankThumbsUpResp* resp)
{
	unsigned uid = resp->myuid();
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned index1 = resp->index1();
	unsigned index2 = resp->index2();

	User::RankThumbsUpResp* msg = new User::RankThumbsUpResp;
	//1.处理点赞奖励
	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::BaseItem *baseitem = common.mutable_based();
	baseitem->set_coin(level * 5);
	baseitem->set_exp(level * 3);
	CommonProcess(uid,common,"thumbs_up_reward",msg->mutable_commons());

	//2.存储被点赞的索引位置
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		if(activity.actdata[thumbsup[i]] == 0)
		{
			activity.actdata[thumbsup[i]] = ((activity.actdata[thumbsup[i]] >> 16) | index1) << 16;
			activity.actdata[thumbsup[i]] = activity.actdata[thumbsup[i]] | index2;
			DataGameActivityManager::Instance()->UpdateActivity(activity);
			break;
		}
	}

	//3.设置返回
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		msg->mutable_thumbsup()->add_index(activity.actdata[thumbsup[i]]);
	}
	return LMI->sendMsg(uid, msg) ? 0 : R_ERROR;
}

int LogicUserManager::ResetDailyThumbsUpCnt(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	for(int i = 0; i < daily_rank_thumbs_up_max; i++)
	{
		activity.actdata[thumbsup[i]] = 0;
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	return 0;
}

int LogicUserManager::Process(unsigned uid, User::GetWorldChannelHelpCntReq* req, User::GetWorldChannelHelpCntResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	unsigned remaincnt = daily_world_channel_help_max - activity.actdata[e_Activity_UserData_1_index_0];
	resp->set_remaincnt(remaincnt);
	return 0;
}

int LogicUserManager::Process(unsigned uid, User::WorldChannelHelpReq* req, User::WorldChannelHelpResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	unsigned helpcnt = activity.actdata[e_Activity_UserData_1_index_0];
	if(helpcnt >= daily_world_channel_help_max)
	{
		throw std::runtime_error("today_help_cnt_is_over");
	}

	activity.actdata[e_Activity_UserData_1_index_0] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	unsigned remaincnt = daily_world_channel_help_max - activity.actdata[e_Activity_UserData_1_index_0];
	resp->set_remaincnt(remaincnt);
	return 0;
}

int LogicUserManager::Process(unsigned uid, User::NewUserGuideShareReq* req, User::NewUserGuideShareResp* resp)
{
	unsigned type = req->type();
	unsigned id = req->id();
	const UserCfg::User & user_cfg = ConfigManager::Instance()->user.m_config;

	if(id > user_cfg.newuserguideshare_size())
	{
		throw std::runtime_error("id_param_error");
	}

	if(1 == type)
	{
		//单倍分享，直接读配置，发放奖励
		CommonProcess(uid,user_cfg.newuserguideshare(id - 1).reward(),"new_user_guide_share_reward",resp->mutable_commons());
	}
	else if(2 == type)
	{
		//双倍奖励
		CommonGiftConfig::CommonModifyItem common;
		for(int i = 0; i < user_cfg.newuserguideshare(id - 1).reward().props_size(); i++)
		{
			CommonGiftConfig::PropsItem * propsbase = common.add_props();
			propsbase->set_id(user_cfg.newuserguideshare(id - 1).reward().props(i).id());
			propsbase->set_count(user_cfg.newuserguideshare(id - 1).reward().props(i).count() * 2);
		}
		if(user_cfg.newuserguideshare(id - 1).reward().has_based())
		{
			CommonGiftConfig::BaseItem * baseitem = common.mutable_based();
			if(user_cfg.newuserguideshare(id - 1).reward().based().has_cash())
				baseitem->set_cash(user_cfg.newuserguideshare(id - 1).reward().based().cash() * 2);
			if(user_cfg.newuserguideshare(id - 1).reward().based().has_coin())
				baseitem->set_coin(user_cfg.newuserguideshare(id - 1).reward().based().coin() * 2);
			if(user_cfg.newuserguideshare(id - 1).reward().based().has_exp())
				baseitem->set_exp(user_cfg.newuserguideshare(id - 1).reward().based().exp() * 2);
			if(user_cfg.newuserguideshare(id - 1).reward().based().has_friend_())
				baseitem->set_friend_(user_cfg.newuserguideshare(id - 1).reward().based().friend_() * 2);
		}
		CommonProcess(uid,common,"new_user_guide_share_reward",resp->mutable_commons());
	}
	else
	{
		throw std::runtime_error("type_param_error");
	}

	return 0;
}

int LogicUserManager::ResetWorldChannelHelpCnt(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	activity.actdata[e_Activity_UserData_1_index_0] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	return 0;
}

int LogicUserManager::PushLoginInfo(unsigned uid,unsigned logints)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned inviteUid = userwrap.Obj().inviteuid;
	if(inviteUid != 0)
	{
		if(CMI->IsNeedConnectByUID(inviteUid))
		{
			User::CSPushLoginMsg* m = new User::CSPushLoginMsg;
			m->set_inviteuid(inviteUid);
			m->set_logints(logints);
			m->set_uid(uid);
			int ret = BMI->BattleConnectNoReplyByUID(inviteUid, m);
			return ret;
		}
	}
	HandlePushLoginInfo(uid,inviteUid,logints);
	return 0;
}

int LogicUserManager::Process(User::CSPushLoginMsg* msg)
{
	unsigned inviteUid = msg->inviteuid();
	unsigned logints = msg->logints();
	unsigned uid = msg->uid();
	HandlePushLoginInfo(uid,inviteUid,logints);
	return 0;
}

int LogicUserManager::Process(unsigned uid, User::UseCdKeyReq* req, User::UseCdKeyResp* resp)
{

	string str = req->number();

	//敏感词过滤
	String::Trim(str);

	if (str.empty())
	{
		error_log("words empty. name=%s", str.c_str());
		throw runtime_error("words_empty");
	}

	if(!StringFilter::Check(str))
	{
		error_log("sensitive words. name=%s", str.c_str());
		throw runtime_error("sensitive_words");
	}

	//数据库查询
	DataExchangeCode DbCode;
	DbCode.code = str;
	int ret = m_DbExchangeCode.GetExchageCode(DbCode);
	if(ret != 0)
	{
		throw runtime_error("invalid_card");
	}
	unsigned type = DbCode.type;

	if(DbCode.usetime > 0 || IsValidUid(DbCode.uid))
	{
		throw runtime_error("used_card");
	}

	if(CDKeyManager::Instance()->HasUseThisType(uid,type))	//个人数据查询
	{
		throw runtime_error("same_type_card");
	}

	uint32_t curr_ts = Time::GetGlobalTime();
	if(curr_ts < DbCode.gentime || curr_ts > DbCode.deadline)
	{
		throw runtime_error("overtime_card");
	}

	//个人数据存档
	CDKeyManager::Instance()->UseCard(uid,type);

	//兑换码存档
	DbCode.usetime = curr_ts;
	DbCode.uid = uid;
	m_DbExchangeCode.SetExchageCode(DbCode);

	//派发奖励
	DBCUserBaseWrap userwrap(uid);
	CommonGiftConfig::CommonModifyItem common;
	const CdKey::CdKeyCfg & cdkey = CdKeyCfgWrap().GetCfg();
	for(int idx = 0; idx < cdkey.rewards_size(); idx++)
	{
		if(cdkey.rewards(idx).type() == type)
		{
			common.MergeFrom(cdkey.rewards(idx).prize(0));
			break;
		}
	}
	CommonProcess(uid,common,"cdkey_reward",resp->mutable_commons());

	return 0;
}

int LogicUserManager::Process(unsigned uid, ProtoPush::RewardLevelUpReq* req, ProtoPush::RewardLevelUpResp* resp)
{
	unsigned type = req->type();
	if(type != 0 && type != 1)
	{
		throw std::runtime_error("param_error");
	}

	CommonGiftConfig::CommonModifyItem common;
	DBCUserBaseWrap userwrap(uid);
	const LevelupUnlock::LevelupUnlockCfg & level_reward = LevelupUnlockCfgWrap().GetLevelupUnlockCfg();
	if(0 == type)
	{
		//单倍奖励
		CommonProcess(uid,level_reward.levels(userwrap.Obj().level - 1).prize(0),"level_up_reward",resp->mutable_commons());
	}
	else if(1 == type)
	{
		//双倍奖励
		for(int i = 0; i < level_reward.levels(userwrap.Obj().level - 1).prize(0).props_size(); i++)
		{
			CommonGiftConfig::PropsItem *propsbase = common.add_props();
			propsbase->set_id(level_reward.levels(userwrap.Obj().level - 1).prize(0).props(i).id());
			propsbase->set_count(level_reward.levels(userwrap.Obj().level - 1).prize(0).props(i).count() * 2);
		}

		if(level_reward.levels(userwrap.Obj().level - 1).prize(0).has_based())
		{
			CommonGiftConfig::BaseItem *userbase = common.mutable_based();
			if(level_reward.levels(userwrap.Obj().level - 1).prize(0).based().has_coin())
			{
				userbase->set_coin(level_reward.levels(userwrap.Obj().level - 1).prize(0).based().coin() * 2);
			}
			if(level_reward.levels(userwrap.Obj().level - 1).prize(0).based().has_cash())
			{
				userbase->set_cash(level_reward.levels(userwrap.Obj().level - 1).prize(0).based().cash() * 2);
			}
		}
		CommonProcess(uid,common,"level_up_reward",resp->mutable_commons());
	}

	return 0;
}

int LogicUserManager::HandlePushLoginInfo(unsigned uid,unsigned inviteUid,unsigned loginTs)
{
	if(UserManager::Instance()->IsOnline(inviteUid))
	{
		User::PushLoginMsg * msg = new User::PushLoginMsg;
		msg->set_logints(loginTs);
		msg->set_uid(uid);
		LMI->sendMsg(inviteUid,msg);
	}
	return 0;
}

int LogicUserManager::CommonProcess(unsigned uid, const CommonGiftConfig::CommonModifyItem& cfg, const std::string& reason,
		DataCommon::CommonItemsCPP * obj, double multiple)
{
	DBCUserBaseWrap userwrap(uid);

	//调用底层的通用操作
	CommonUnderlaying(userwrap, cfg, reason, obj, multiple);

	return 0;
}

void LogicUserManager::CommonUnderlaying(DBCUserBaseWrap& user, const CommonGiftConfig::CommonModifyItem& cfg, const std::string& reason,
			DataCommon::CommonItemsCPP* obj, double multiple)
{
	bool sub = false;
	unsigned uid = user.Obj().uid;

	//装备消耗的检查
	CheckPropsBeforeCost(uid, reason, cfg);

	//进行资源的结算
	if (cfg.has_based())
	{
		//处理用户属性的扣除和增加
		user.BaseProcess(cfg.based(), obj->mutable_userbase(), reason, multiple);
	}

	//道具的结算
	for(int i = 0; i < cfg.props_size(); ++i)
	{
		unsigned int propsid = cfg.props(i).id();
		int cnt = cfg.props(i).count();

		if (cnt > 0)
		{
			LogicPropsManager::Instance()->AddProps(uid, propsid, cnt, reason, obj->mutable_props());
		}
		else if (cnt < 0)
		{
			//获取该propid对应的ud
			unsigned ud = DataItemManager::Instance()->GetPropsUd(uid, propsid);

			if (-1 == ud)
			{
				error_log("props not exist. uid=%u,propsid=%u", uid, propsid);
				throw runtime_error("props_not_exist");
			}

			LogicPropsManager::Instance()->CostProps(uid, ud, propsid, -cnt, reason, obj->mutable_props());
			sub = true;
		}
		if(LogicXsgReportManager::Instance()->IsWXPlatform(user.Obj().uid))
		{
			int count = cnt;
			if(count < 0)
				count = -count;
			LogicXsgReportManager::Instance()->XSGItemChangeReport(uid,cnt,propsid,count,reason);
		}
	}
	if(sub)
	{
		LogicKeeperManager::Instance()->OnStorageChange(uid);
	}
}

void LogicUserManager::CheckPropsBeforeCost(unsigned uid, const string & reason, const CommonGiftConfig::CommonModifyItem& cfg)
{
	if (cfg.props_size() > 0)
	{
		for(int i = 0; i < cfg.props_size(); ++i)
		{
			if (cfg.props(i).count() < 0)
			{
				unsigned int propsid = cfg.props(i).id();

				//只支持可叠加装备
				bool isOverlay = LogicPropsManager::Instance()->IsAllowOverlay(propsid);

				if (!isOverlay)
				{
					error_log("props can not overlay. uid=%u,propsid=%u,code=%s", uid, propsid, reason.c_str());
					throw runtime_error("props_cfg_error");
				}

				unsigned ud = DataItemManager::Instance()->GetPropsUd(uid, propsid);

				if (-1 == ud)
				{
					error_log("props not exist. uid=%u,propsid=%u", uid, propsid);
					throw runtime_error("props_not_exist");
				}

				if ( static_cast<unsigned>(-cfg.props(i).count()) > DataItemManager::Instance()->GetData(uid, ud).item_cnt)
				{
					error_log("props not enough. uid=%u,propsid=%u,need=%d,code=%s", uid, propsid, cfg.props(i).count(), reason.c_str());
					throw runtime_error("props_not_enough");
				}
			}
		}
	}
}

