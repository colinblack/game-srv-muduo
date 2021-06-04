#include "ServerInc.h"
#include <sys/timeb.h>

DcLogger* LogicXsgReportManager::m_dclogger = 0;

LogicXsgReportManager::LogicXsgReportManager ()
{
	if(m_dclogger == NULL)
	{
		m_dclogger = new DcLogger("99917");
		m_dclogger->Start();
	}
}

void LogicXsgReportManager::CallDestroy()
{
	if(m_dclogger != NULL)
	{
		m_dclogger->Quit();
		delete m_dclogger;
	}
	Destroy();
}

bool LogicXsgReportManager::IsWXPlatform(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	bool IsValid = false;
	//if(uid == 10000012)
	if(userwrap.Obj().register_platform == PT_WX || userwrap.Obj().register_platform == PT_XMFOUR || userwrap.Obj().register_platform == PT_XMZZ || userwrap.Obj().register_platform == PT_Mi2 || userwrap.Obj().register_platform == PT_VIVO || userwrap.Obj().register_platform == PT_OPPO)
		IsValid = true;
	return IsValid;
}


template<class T>
void LogicXsgReportManager::SetBaseParam(unsigned uid,string openid,T &base)
{
	if(openid == "")
	{
		map<unsigned,string>::iterator it = user_openid_map.find(uid);
		if(it == user_openid_map.end())
			return;
		else
			openid = it->second;
	}

	DBCUserBaseWrap userwrap(uid);
	//BaseMsg
	base.deviceId = "";
	struct  timeb   pt;
	ftime(&pt);
	struct tm ptm;
	localtime_r(&pt.time, &ptm);
	char str[100];
	snprintf(str, sizeof(str), "%u-%u-%u %u:%u:%u.%u", ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec,pt.millitm);
	base.timestamp = str;
	//error_log("ts:%s,ts2:%s",str,base->timestamp.c_str());
	//RoleInfo
	string channel = "seasun",channelDesc = "微信";
	if(userwrap.Obj().register_platform == PT_XMZZ)
	{
		channel = "xiaomi4";
		channelDesc = "小米4部(赚赚小游戏)";
	}
	else if(userwrap.Obj().register_platform == PT_XMFOUR)
	{
		channel = "xiaomi4";
		channelDesc = "小米4部(疯狂小游戏)";
	}
	else if(userwrap.Obj().register_platform == PT_Mi2)
	{
		channel = "xiaomi2";
		channelDesc = "小米2部";
	}
	else if(userwrap.Obj().register_platform == PT_VIVO)
	{
		channel = "vivo";
		channelDesc = "vivo快游戏";
	}
	else if(userwrap.Obj().register_platform == PT_OPPO)
	{
		channel = "oppo";
		channelDesc = "oppo快游戏";
	}

	base.channel = channel;
	base.accountId = openid;
	base.roleId = CTrans::UTOS(uid);
	base.roleName = userwrap.Obj().name;
	base.roleType = "";
	base.roleLevel = userwrap.Obj().level;
	base.roleVipLevel = userwrap.Obj().viplevel;
	base.server = CTrans::UTOS(Config::GetZoneByUID(uid));
	base.serverName = "田园物语";
	base.pxgksChannel = userwrap.Obj().pxgksChannel;
	base.partyId = CTrans::UTOS(userwrap.Obj().alliance_id);
	base.gender = "";
	base.deviceId = openid;
	base.channelDesc = channelDesc;

}

void LogicXsgReportManager::XSGLoginReport(unsigned uid,const string openid)
{
	DBCUserBaseWrap userwrap(uid);

	LoginInfo loginInfo ;
	SetBaseParam<LoginInfo>(uid,openid,loginInfo);

	m_dclogger->OnRoleLogin(&loginInfo);

	user_openid_map[uid] = openid;
}

void LogicXsgReportManager::XSGLogOutReport(unsigned uid)
{
	string openid = "";
	LogoutInfo logoutInfo;
	SetBaseParam<LogoutInfo>(uid,openid,logoutInfo);

	m_dclogger->OnRoleLogout(&logoutInfo);

	map<unsigned,string>::iterator it = user_openid_map.find(uid);
	if(it != user_openid_map.end())
		user_openid_map.erase(it);

}

void LogicXsgReportManager::XSGRechargeReport(unsigned uid,string itemid,unsigned charge,string orderid,string channelOrderId)
{
	string openid = "";
	RechargeInfo rechargeInfo;
	SetBaseParam<RechargeInfo>(uid,openid,rechargeInfo);

	rechargeInfo.currency = "CNY";
	rechargeInfo.money = charge;
	rechargeInfo.tradeNo = orderid;
	rechargeInfo.channelTradeNo = channelOrderId;
	rechargeInfo.ext["rechargeType"] = itemid;
	rechargeInfo.ext["rechargeItem"] = itemid;
	m_dclogger->OnRoleRecharge(&rechargeInfo);
}

void LogicXsgReportManager::XSGRechargeGetDiamondReport(unsigned uid,unsigned cash,unsigned total)
{
	string openid = "";
	TokenPurchaseInfo tokenPurchaseInfo;
	SetBaseParam<TokenPurchaseInfo>(uid,openid,tokenPurchaseInfo);

	tokenPurchaseInfo.gold = cash;
	tokenPurchaseInfo.virtualCurrencyTotal = total;
	m_dclogger->OnTokenPurchase(&tokenPurchaseInfo);

}

void LogicXsgReportManager::XSGGetDiamondReport(unsigned uid,string reason,unsigned add,unsigned total,bool isbind)
{
	string openid = "";
	TokenRewardInfo tokenRewardInfo;
	SetBaseParam<TokenRewardInfo>(uid,openid,tokenRewardInfo);

	tokenRewardInfo.gainChannel = reason;
	tokenRewardInfo.gold = add;
	tokenRewardInfo.virtualCurrencyTotal = total;
	tokenRewardInfo.isBinding = isbind;

	m_dclogger->OnTokenReward(&tokenRewardInfo);
}

void LogicXsgReportManager::XSGCostDiamondReport(unsigned uid,unsigned sub,unsigned total,string reason)
{
	string openid = "";
	TokenConsumeInfo tokenConsumeInfo ;
	SetBaseParam<TokenConsumeInfo>(uid,openid,tokenConsumeInfo);

	tokenConsumeInfo.gold = sub;
	tokenConsumeInfo.itemType = reason;
	tokenConsumeInfo.itemId = "cash";
	tokenConsumeInfo.itemName = "钻石";
	tokenConsumeInfo.itemNum = sub;
	tokenConsumeInfo.virtualCurrencyTotal = total;
	tokenConsumeInfo.isBinding = false;

	m_dclogger->OnTokenConsume(&tokenConsumeInfo);
}

void LogicXsgReportManager::XSGPurchaseCoinReport(unsigned uid,unsigned purchase_count,unsigned total)
{
	string openid = "";
	VirtualCurrencyPurchaseInfo virtualCurrencyPurchaseInfo;
	SetBaseParam<VirtualCurrencyPurchaseInfo>(uid,openid,virtualCurrencyPurchaseInfo);

	virtualCurrencyPurchaseInfo.gold = purchase_count;
	virtualCurrencyPurchaseInfo.virtualCurrencyType = "jinbi";
	virtualCurrencyPurchaseInfo.virtualCurrencyTotal = total;

	m_dclogger->OnVirtualCurrencyPurchase(&virtualCurrencyPurchaseInfo);
}

void LogicXsgReportManager::XSGGetCoinReport(unsigned uid,unsigned count,unsigned total,string reason)
{
	string openid = "";
	VirtualCurrencyRewardInfo virtualCurrencyRewardInfo;
	SetBaseParam<VirtualCurrencyRewardInfo>(uid,openid,virtualCurrencyRewardInfo);

	virtualCurrencyRewardInfo.gainChannel = reason;
	virtualCurrencyRewardInfo.gainChannelType = reason;
	virtualCurrencyRewardInfo.gold = count;
	virtualCurrencyRewardInfo.virtualCurrencyType = "jinbi";
	virtualCurrencyRewardInfo.virtualCurrencyTotal = total;
	virtualCurrencyRewardInfo.isBinding = false;

	m_dclogger->OnVirtualCurrencyReward(&virtualCurrencyRewardInfo);
}

void LogicXsgReportManager::XSGCostCoinReport(unsigned uid,unsigned sub,unsigned total,string reason)
{
	string openid = "";
	VirtualCurrencyConsumeInfo virtualCurrencyConsumeInfo;
	SetBaseParam<VirtualCurrencyConsumeInfo>(uid,openid,virtualCurrencyConsumeInfo);

	virtualCurrencyConsumeInfo.gold = sub;
	virtualCurrencyConsumeInfo.itemType = reason;
	virtualCurrencyConsumeInfo.itemId = "coin";
	virtualCurrencyConsumeInfo.itemName = "金币";
	virtualCurrencyConsumeInfo.itemNum = sub;
	virtualCurrencyConsumeInfo.virtualCurrencyTotal = total;
	virtualCurrencyConsumeInfo.isBinding = false;

	m_dclogger->OnVirtualCurrencyConsume(&virtualCurrencyConsumeInfo);
}

void LogicXsgReportManager::XSGLevelUpReport(unsigned uid)
{
	string openid = "";
	RoleLevelUpInfo roleLevelUpInfo ;
	SetBaseParam<RoleLevelUpInfo>(uid,openid,roleLevelUpInfo);

	m_dclogger->OnRoleLeveUp(&roleLevelUpInfo);
}

void LogicXsgReportManager::XSGMissionStartReport(unsigned uid,unsigned missionid,unsigned missiontype)
{
	string openid = "";
	MissionInfo missionInfo;
	SetBaseParam<MissionInfo>(uid,openid,missionInfo);

	missionInfo.missionId = CTrans::UTOS(missionid);
	missionInfo.missionType = CTrans::UTOS(missiontype);

	m_dclogger->OnMissionBegin(&missionInfo);
}

void LogicXsgReportManager::XSGMissionEndReport(unsigned uid,unsigned missionid,unsigned missiontype)
{
	string openid = "";
	MissionInfo missionInfo;
	SetBaseParam<MissionInfo>(uid,openid,missionInfo);

	missionInfo.missionId = CTrans::UTOS(missionid);
	missionInfo.missionType = CTrans::UTOS(missiontype);

	m_dclogger->OnMissionSuccess(&missionInfo);
}

void LogicXsgReportManager::XSGShopTradeReport(unsigned uid,string tradeid,string tradetype,unsigned coin,string coinType,unsigned itemid,unsigned itemnum)
{
	string openid = "";
	TradeInfo tradeInfo;
	SetBaseParam<TradeInfo>(uid,openid,tradeInfo);

	tradeInfo.tradeId = tradeid;
	tradeInfo.tradeType = tradetype;
	tradeInfo.gold = coin;
	tradeInfo.virtualCurrencyType = coinType;
	tradeInfo.itemId = CTrans::UTOS(itemid);
	tradeInfo.itemName = "";
	tradeInfo.itemType = "";
	tradeInfo.itemNum = itemnum;

	m_dclogger->OnRoleTrade(&tradeInfo);
}

void LogicXsgReportManager::XSGBuildReport(unsigned uid,unsigned buildtype,unsigned buildid,unsigned buildnum)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_putup";
	customEventInfo.ext["itemtype"] = CTrans::UTOS(buildtype);
	customEventInfo.ext["itemid"] = CTrans::UTOS(buildid);
	customEventInfo.ext["itemname"] = "";
	customEventInfo.ext["itemnum"] = CTrans::UTOS(buildnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGEquipmentReport(unsigned uid,unsigned equipmentId,unsigned itemid,unsigned itemnum)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_product";
	customEventInfo.ext["where"] = CTrans::UTOS(equipmentId);
	customEventInfo.ext["itemid"] = CTrans::UTOS(itemid);
	customEventInfo.ext["itemname"] = "";
	customEventInfo.ext["itemnum"] = CTrans::UTOS(itemnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGGetHaverstReport(unsigned uid,unsigned itemtype,unsigned itemid,unsigned itemnum)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_gain";
	customEventInfo.ext["itemtype"] = CTrans::UTOS(itemtype);
	customEventInfo.ext["itemid"] = CTrans::UTOS(itemid);
	customEventInfo.ext["itemname"] = "";
	customEventInfo.ext["itemnum"] = CTrans::UTOS(itemnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGDestroyBarrierReport(unsigned uid,unsigned item_num,unsigned use_num)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_expand";
	customEventInfo.ext["remove_item"] = "";
	customEventInfo.ext["remove_num"] = CTrans::UTOS(item_num);
	customEventInfo.ext["use_item"] = CTrans::UTOS(use_num);
	customEventInfo.ext["use_num"] = "1";
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGFeedAnimalReport(unsigned uid,unsigned animalid,unsigned itemid,unsigned itemnum)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_feed";
	customEventInfo.ext["animal"] = CTrans::UTOS(animalid);
	customEventInfo.ext["item"] = CTrans::UTOS(itemid);
	customEventInfo.ext["item_num"] = CTrans::UTOS(itemnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGStorageUpReport(unsigned uid,unsigned stroageid,unsigned space,string itemid,string itemnum)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	if(stroageid == 100)
		customEventInfo.ext["type"] = "action_grainup";
	else if(stroageid == 200)
		customEventInfo.ext["type"] = "action_stockup";
	customEventInfo.ext["contain"] = CTrans::UTOS(space);
	customEventInfo.ext["item"] = itemid;
	customEventInfo.ext["item_num"] = itemnum;
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGItemChangeReport(unsigned uid,int flag,unsigned itemid,unsigned itemnum,string reason)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	if(flag > 0)
		customEventInfo.ext["type"] = "action_itemget";
	else
		customEventInfo.ext["type"] = "action_itemuse";
	customEventInfo.ext["itemid"] = CTrans::UTOS(itemid);
	customEventInfo.ext["itemname"] = "";
	customEventInfo.ext["where"] = reason;
	customEventInfo.ext["item_num"] = CTrans::UTOS(itemnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGShopShelfItemReport(unsigned uid,unsigned itemid,unsigned itemnum,unsigned price)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_sale";
	customEventInfo.ext["itemid"] = CTrans::UTOS(itemid);
	customEventInfo.ext["itemname"] = "";
	customEventInfo.ext["gold"] = CTrans::UTOS(price);
	customEventInfo.ext["item_num"] = CTrans::UTOS(itemnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}

void LogicXsgReportManager::XSGRotaryDrawReport(unsigned uid,unsigned itemid,unsigned itemnum)
{
	string openid = "";
	CustomEventInfo customEventInfo;
	SetBaseParam<CustomEventInfo>(uid,openid,customEventInfo);

	customEventInfo.eventId = "action_log";
	customEventInfo.eventDesc = "个性化行为表";
	customEventInfo.ext["type"] = "action_turntable";
	customEventInfo.ext["itemid"] = CTrans::UTOS(itemid);
	customEventInfo.ext["itemname"] = "";
	customEventInfo.ext["item_num"] = CTrans::UTOS(itemnum);
	m_dclogger->OnCustomEvent(&customEventInfo);
}


