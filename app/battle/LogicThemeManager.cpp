#include "LogicThemeManager.h"

int LogicThemeManager::CheckLogin(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned acccharge = userwrap.Obj().acccharge;
	unsigned first_recharge = userwrap.Obj().first_recharge;
	bool valid = acccharge >= 1 && (0 == (first_recharge & 1));

	//如果有首充，则送首充皮肤
	if(valid)
	{
		//获取首充皮肤配置
		const ConfigTheme::Conf& cfg = ConfigManager::Instance()->theme.m_config;
		unsigned themeId = cfg.theme(1).themeid();
		unsigned itemType = cfg.theme(1).item(0).type();

		//如果未送过，则赠送首充皮肤
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
		uint32_t status = activity.actdata[themeId + theme_id_use_max - 1];
		if(((status >> itemType) & 0x1) == 0)
		{
			activity.actdata[themeId + theme_id_use_max - 1] |= (0x1 << itemType);
		}
	}
	return 0;
}

int LogicThemeManager::FirstChargeRewardTheme(unsigned uid,ProtoTheme::ThemeInfoResp *resp)
{
	const ConfigTheme::Conf& cfg = ConfigManager::Instance()->theme.m_config;
	unsigned themeId = cfg.theme(1).themeid();
	unsigned itemType = cfg.theme(1).item(0).type();

	//如果未送过，则赠送首充皮肤
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	uint32_t status = activity.actdata[themeId + theme_id_use_max - 1];
	if(((status >> itemType) & 0x1) == 0)
	{
		activity.actdata[themeId + theme_id_use_max - 1] |= (0x1 << itemType);
	}

	FillTheme(activity, resp);
	return 0;
}


//查询
int LogicThemeManager::Process(unsigned uid,ProtoTheme::ThemeInfoReq *req,ProtoTheme::ThemeInfoResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	FillTheme(activity, resp);
	return 0;
}
//购买
int LogicThemeManager::Process(unsigned uid,ProtoTheme::ThemeBuyReq *req,ProtoTheme::ThemeBuyResp *resp)
{
	uint32_t themeId = req->themeid();
	uint32_t itemType = req->itemtype();
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	CommonGiftConfig::CommonModifyItem cost;
	ThemeCfgWrap().GetThemeCost(req->themeid(), req->itemtype(), &cost);
	if(themeId == 0 || themeId + theme_id_use_max > DB_GAME_DATA_NUM)
	{
		error_log("invalid param uid=%u,themeId=%u", uid, themeId);
		throw runtime_error("invalid_themeId");
	}
	if(itemType == 0 || itemType >= 32)	// 第0位弃用
	{
		error_log("invalid param uid=%u,itemType=%u", uid, itemType);
		throw runtime_error("invalid_itemType");
	}
	uint32_t status = activity.actdata[themeId + theme_id_use_max - 1];
	if(((status >> itemType) & 0x1) == 1)
	{
		error_log("already buy uid=%u,themeId=%u,itemType=%u", uid, themeId, itemType);
		throw runtime_error("already_buy");
	}
	status |= (0x1 << itemType);

	string reason;
	String::Format(reason, "BuyTheme_%u_%u", themeId, itemType);
	//扣除建造商会的费用
	LogicUserManager::Instance()->CommonProcess(uid, cost, reason, resp->mutable_commons());
	activity.actdata[themeId + theme_id_use_max - 1] = status;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	resp->set_themeid(themeId);
	resp->set_itemtype(itemType);
	FillTheme(activity, resp->mutable_themeinfo());
	return 0;
}
//应用
int LogicThemeManager::Process(unsigned uid,ProtoTheme::ThemeUseReq *req,ProtoTheme::ThemeInfoResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	uint32_t themeId = req->themeid();
	if(themeId + theme_id_use_max > DB_GAME_DATA_NUM)
	{
		error_log("invalid param uid=%u,themeId=%u", uid, themeId);
		throw runtime_error("invalid_themeId");
	}
	for(uint32_t i = 0; i < req->itemtype_size(); ++i)
	{
		uint32_t itemType = req->itemtype(i);
		if(itemType == 0 || itemType >= 32)	// 第0位弃用
		{
			error_log("invalid param uid=%u,itemType=%u", uid, itemType);
			break;
		}
		if(themeId > 0)
		{
			uint32_t status = activity.actdata[themeId + theme_id_use_max - 1];
			if(((status >> itemType) & 0x1) == 0)
			{
				error_log("not exist item uid=%u,themeId=%u,itemType=%u", uid, themeId, itemType);
				break;
			}
		}
		uint32_t blockSize = sizeof(uint32_t);
		uint32_t idx = itemType / blockSize;
		uint32_t shift = itemType % blockSize;
		if(idx >= theme_id_use_max)
		{
			error_log("invalid param uid=%u,itemType=%u,idx=%u", uid, itemType,idx);
			break;
		}
		uint32_t newThemeId =  (activity.actdata[idx] & (~(0xFF << (shift << 3)))) | ((themeId & 0xFF) << (shift << 3));
		activity.actdata[idx] = newThemeId;
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FillTheme(activity, resp);
	return 0;
}
int LogicThemeManager::FillTheme(DataGameActivity& activity, ProtoTheme::ThemeInfoResp *resp)
{
	for(uint32_t i = 0; i < theme_id_use_max && i < DB_GAME_DATA_NUM; ++i)
	{
		for(uint32_t shift = 0; shift < sizeof(uint32_t); ++shift)
		{
			resp->add_use((activity.actdata[i] >> (shift << 3)) & 0xFF);
		}
	}
	for(uint32_t i = theme_id_use_max; i < DB_GAME_DATA_NUM; ++i)
	{
		resp->add_own(activity.actdata[i]);
	}
	return 0;
}


