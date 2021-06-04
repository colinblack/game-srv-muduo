#include "ServerInc.h"

int LogicCardManager::HandleCardPurchase(unsigned uid,unsigned itemid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	DBCUserBaseWrap userwrap(uid);
	if(MONTH_CARD_ID == itemid)
	{
		/****************处理月卡购买***************/
		const ConfigCard::MonthCardCPP & monthcpp_cfg = ConfigManager::Instance()->card.m_config.monthcard();
		ProtoCard::PushBuyMonthCardMsg *msg = new ProtoCard::PushBuyMonthCardMsg;


		//1.更新用户月卡信息
		activity.actdata[month_card_end_ts_index] = monthcpp_cfg.effective_day() * 24 * 3600 + GetCurDateStartTs();//更新endts
		userwrap.Obj().flag |= (1 << base_falg_id_is_have_month_card);//是否拥有月卡
		userwrap.Obj().flag &= ~(1 << base_falg_id_is_reward_month_card);//重置领取标记

		//2.设置返回
		unsigned endts = activity.actdata[month_card_end_ts_index];
		unsigned isHave= (userwrap.Obj().flag >> base_falg_id_is_have_month_card) & 1;
		unsigned isreward = (userwrap.Obj().flag >> base_falg_id_is_reward_month_card) & 1;
		msg->mutable_monthcard()->set_ishavemonthcard(isHave);
		msg->mutable_monthcard()->set_isreward(isreward);
		msg->mutable_monthcard()->set_monthcardendts(endts);

		LMI->sendMsg(uid,msg);

	}else if(LIFE_CARD_ID == itemid)
	{
		/****************处理终生卡购买***************/
		const ConfigCard::LifeCardCPP & lifecpp_cfg = ConfigManager::Instance()->card.m_config.lifecard();
		ProtoCard::PushBuyLifeCardMsg *msg = new ProtoCard::PushBuyLifeCardMsg;


		//1.更新用户终生卡信息
		userwrap.Obj().flag |= (1 << base_falg_id_is_have_life_card);//是否拥有年卡
		userwrap.Obj().flag &= ~(1 << base_falg_id_is_reward_life_card);//重置领取标记

		//2.设置返回
		unsigned isHave= (userwrap.Obj().flag >> base_falg_id_is_have_life_card) & 1;
		unsigned isreward = (userwrap.Obj().flag >> base_falg_id_is_reward_life_card) & 1;
		msg->mutable_lifecard()->set_ishavelifecard(isHave);
		msg->mutable_lifecard()->set_isreward(isreward);

		LMI->sendMsg(uid,msg);
	}
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	return 0;
}

unsigned LogicCardManager::GetCurDateStartTs()
{
	//获取当天凌晨ts
	unsigned cur_hour_ts = Time::GetGlobalTime();
	time_t nts = cur_hour_ts;
	struct tm ptm;
	if(localtime_r(&nts, &ptm) != NULL)
	{
		cur_hour_ts  -= ptm.tm_hour * 3600;
		cur_hour_ts  -= ptm.tm_min * 60;
		cur_hour_ts  -= ptm.tm_sec;
	}
	return cur_hour_ts;
}

int LogicCardManager::Process(unsigned uid, ProtoCard::GetCardReq* req, ProtoCard::GetCardResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	DBCUserBaseWrap userwrap(uid);
	//--------获取月卡信息
	unsigned endts = activity.actdata[month_card_end_ts_index];
	unsigned isHaveMonthCard= (userwrap.Obj().flag >> base_falg_id_is_have_month_card) & 1;
	unsigned isrewardMonthCard = (userwrap.Obj().flag >> base_falg_id_is_reward_month_card) & 1;
	if(endts < Time::GetGlobalTime())
	{
		//月卡过期、重置信息
		activity.actdata[month_card_end_ts_index] = 0;
		userwrap.Obj().flag &= ~(1 << base_falg_id_is_have_month_card);//是否拥有月卡
		userwrap.Obj().flag &= ~(1 << base_falg_id_is_reward_month_card);//重置领取标记

		endts = activity.actdata[month_card_end_ts_index];
		isHaveMonthCard= (userwrap.Obj().flag >> base_falg_id_is_have_month_card) & 1;
		isrewardMonthCard = (userwrap.Obj().flag >> base_falg_id_is_reward_month_card) & 1;
		DataGameActivityManager::Instance()->UpdateActivity(activity);

	}
	resp->mutable_monthcard()->set_ishavemonthcard(isHaveMonthCard);
	resp->mutable_monthcard()->set_isreward(isrewardMonthCard);
	resp->mutable_monthcard()->set_monthcardendts(endts);

	//-------获取终生卡信息
	unsigned isHave= (userwrap.Obj().flag >> base_falg_id_is_have_life_card) & 1;
	unsigned isreward = (userwrap.Obj().flag >> base_falg_id_is_reward_life_card) & 1;
	resp->mutable_lifecard()->set_ishavelifecard(isHave);
	resp->mutable_lifecard()->set_isreward(isreward);
	return 0;
}

int LogicCardManager::Process(unsigned uid, ProtoCard::RewardMonthCardReq* req, ProtoCard::RewardMonthCardResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	DBCUserBaseWrap userwrap(uid);

	//------校验
	//1.月卡是否已过期
	unsigned endts = activity.actdata[month_card_end_ts_index];
	if(endts < Time::GetGlobalTime())
	{
		throw std::runtime_error("month_card_is_overdue");
	}
	//2.今日是否已领过
	unsigned isrewardMonthCard = (userwrap.Obj().flag >> base_falg_id_is_reward_month_card) & 1;
	if(isrewardMonthCard)
	{
		throw std::runtime_error("already_reward");
	}

	//------校验通过
	//1.更新标记位
	userwrap.Obj().flag |= (1 << base_falg_id_is_reward_month_card);
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	//2.发放奖励
	const ConfigCard::MonthCardCPP & monthcpp_cfg = ConfigManager::Instance()->card.m_config.monthcard();
	LogicUserManager::Instance()->CommonProcess(uid,monthcpp_cfg.reward(),"month_card_reward",resp->mutable_commons());
	//3.设置返回信息
	unsigned isHave= (userwrap.Obj().flag >> base_falg_id_is_have_month_card) & 1;
	unsigned isreward = (userwrap.Obj().flag >> base_falg_id_is_reward_month_card) & 1;
	resp->mutable_monthcard()->set_ishavemonthcard(isHave);
	resp->mutable_monthcard()->set_isreward(isreward);
	resp->mutable_monthcard()->set_monthcardendts(endts);

	return 0;
}

int LogicCardManager::Process(unsigned uid, ProtoCard::RewardLifeCardReq* req, ProtoCard::RewardLifeCardResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	DBCUserBaseWrap userwrap(uid);
	//------校验
	//1.今日是否已领过
	unsigned isrewardLifeCard = (userwrap.Obj().flag >> base_falg_id_is_reward_life_card) & 1;
	if(isrewardLifeCard)
	{
		throw std::runtime_error("already_reward");
	}

	//------校验通过
	//1.更新标记位
	userwrap.Obj().flag |= (1 << base_falg_id_is_reward_life_card);
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	//2.发放奖励
	const ConfigCard::LifeCardCPP & lifecpp_cfg = ConfigManager::Instance()->card.m_config.lifecard();
	LogicUserManager::Instance()->CommonProcess(uid,lifecpp_cfg.reward(),"life_card_reward",resp->mutable_commons());
	//3.设置返回信息
	//设置返回
	unsigned isHave= (userwrap.Obj().flag >> base_falg_id_is_have_life_card) & 1;
	unsigned isreward = (userwrap.Obj().flag >> base_falg_id_is_reward_life_card) & 1;
	resp->mutable_lifecard()->set_ishavelifecard(isHave);
	resp->mutable_lifecard()->set_isreward(isreward);
	return 0;
}

int LogicCardManager::ResetCardReward(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	userwrap.Obj().flag &= ~(1 << base_falg_id_is_reward_month_card);//重置月卡每日领取标记为0
	userwrap.Obj().flag &= ~(1 << base_falg_id_is_reward_life_card);//重置终生卡每日领取标记为0
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	return 0;
}
