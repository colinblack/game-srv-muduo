#include "LogicPropsManager.h"
#include "ServerInc.h"

int LogicPropsManager::Process(unsigned uid, User::BuyMaterialReq* req, User::BuyMaterialResp* resp)
{
	//校验物品是否已解锁、并计算所需花费的钻石
	DBCUserBaseWrap userwrap(uid);
	int cost_diamond = 0;
	unsigned user_level = userwrap.Obj().level;
	uint32_t buyProductId = 0;
	for(int i = 0; i < req->buyinfo_size(); i++)
	{
		User::BuyMaterialInfo buy_info = req->buyinfo(i);
		unsigned props_id = buy_info.propsid();
		buyProductId = props_id;
		unsigned props_cnt = buy_info.propscnt();
		cost_diamond += props_cnt * ItemCfgWrap().GetPropsItem(props_id).dimond_cost().based().cash();

		unsigned props_unlock_level = ItemCfgWrap().GetPropsItem(props_id).unlock_level();
		if(user_level < props_unlock_level)
		{
			error_log("some props don't unlock.user_level = %u,unlock_level=%u,props_id=%u",user_level,props_unlock_level,props_id);
			throw std::runtime_error("some props don't unlock");
		}
	}

	//校验钻石是否足够
	if(-cost_diamond > userwrap.GetCash())
	{
		error_log("cash is not enough.cur_cash=%u,cost_cash=%u", userwrap.GetCash(), -cost_diamond);
		throw runtime_error("cash_is_not_enough");
	}

	//扣除钻石
	string reason;
	String::Format(reason, "CostCashBuyMaterial_%u", buyProductId);
	userwrap.CostCash(-cost_diamond,reason);
	DataCommon::CommonItemsCPP *common = resp->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
	base->set_type(type_cash);
	base->set_change(cost_diamond);
	base->set_totalvalue(userwrap.GetCash());

	//添加物品
	for(int i = 0; i < req->buyinfo_size(); i++)
	{
		User::BuyMaterialInfo buy_info = req->buyinfo(i);
		unsigned props_id = buy_info.propsid();
		unsigned props_cnt = buy_info.propscnt();

		AddProps(uid,props_id,props_cnt,reason,resp->mutable_commons()->mutable_props());
	}

	//添加返回序列号
	resp->set_seq(req->seq());
	return 0;
}

int LogicPropsManager::Process(unsigned uid, User::ViewAdGetSpeedUpCardReq* req, User::ViewAdGetSpeedUpCardResp* resp)
{
	const ConfigItem::Items & item_cfg = ConfigManager::Instance()->propsitem.m_config;
	LogicUserManager::Instance()->CommonProcess(uid,item_cfg.view_ad_reward(),"view_ad_reward_speedup_card",resp->mutable_commons());
	return 0;
}

int LogicPropsManager::NewUser(unsigned uid)
{
	DataItemManager::Instance()->Init(uid);

	//读取新手文档中的配置，进行新道具的放送
	const UserCfg::User& userCfg = UserCfgWrap().User();

	//发放物品
	DataCommon::CommonItemsCPP reward;
	LogicUserManager::Instance()->CommonProcess(uid, userCfg.initprops(), "NewUser", &reward);

	return 0;
}

int LogicPropsManager::CheckLogin(unsigned uid)
{
	DataItemManager::Instance()->Init(uid);

	return 0;
}

int LogicPropsManager::CostProps(unsigned uid, unsigned propsud, unsigned propsid, unsigned count, string reason, DataCommon::PropsAllChangeCPP * msg)
{
	//判断道具是否存在
	if (!DataItemManager::Instance()->IsExistItem(uid, propsud))
	{
		error_log("props's ud is not exist. uid=%u,ud=%u", uid, propsud);
		throw runtime_error("props_not_exist");
	}

	//获取道具
	DataItem & dataprops = DataItemManager::Instance()->GetData(uid, propsud);

	if (dataprops.props_id != propsid)
	{
		error_log("props's id is not same. uid=%u,ud=%u.needeqid=%u", uid, propsud, propsid);
		throw std::runtime_error("props_not_same");
	}

	//判断道具数量是否足够
	if (dataprops.item_cnt < count)
	{
		error_log("props not enough. uid=%u,ud=%u,count=%u", uid, propsud, count);
		throw std::runtime_error("props_not_enough");
	}

	dataprops.item_cnt -= count;
	string logtype;

	int restcount = dataprops.item_cnt;

	DataCommon::PropsChangeCPP* costpropsmsg = msg->add_propsitem();
	costpropsmsg->set_change(-count);
	dataprops.SetMessage(costpropsmsg->mutable_props());

	if (0 == dataprops.item_cnt)  //道具已用完，删除
	{
		logtype = "Del";
		DataItemManager::Instance()->DelProps(dataprops);
	}
	else
	{
		logtype = "Sub";
		DataItemManager::Instance()->UpdateItem(dataprops);
	}

	PROPS_LOG("uid=%u,id=%u,propsid=%d,act=%s,chg=%d,count=%d,code=%s.",
			uid, propsud, propsid, logtype.c_str(), count, restcount, reason.c_str());

	LogicKeeperManager::Instance()->OnStorageChange(uid);

	return 0;
}

int LogicPropsManager::AddProps(unsigned uid, unsigned propsid, unsigned count, string reason, DataCommon::PropsAllChangeCPP * msg)
{
	//先判断是否可叠加
	if (IsAllowOverlay(propsid))
	{
		//可叠加道具
		DataItem additem;
		additem.uid = uid;
		additem.item_cnt = count;
		additem.props_id = propsid;

		AddPropsImpl(additem, reason, msg);
	}
	else
	{
		//不可叠加道具，批量添加
		for(unsigned i = 0; i < count; ++i)
		{
			DataItem additem;
			additem.uid = uid;
			additem.item_cnt = 1;
			additem.props_id = propsid;

			AddPropsImpl(additem, reason, msg);
		}
	}
	if(count > 0)
	{
		LogicKeeperManager::Instance()->OnAddMaterial(uid, propsid);
	}
	return 0;
}

int LogicPropsManager::AddPropsImpl(DataItem & props, string & reason, DataCommon::PropsAllChangeCPP * msg)
{
	unsigned uid = props.uid;
	unsigned propsid = props.props_id;

	if (IsAllowOverlay(propsid) && DataItemManager::Instance()->IsPropsExist(uid, propsid))
	{
		//道具可叠加，并且道具存在
		unsigned ud = DataItemManager::Instance()->GetPropsUd(uid, propsid);

		//获取道具
		DataItem & dataprops = DataItemManager::Instance()->GetData(uid, ud);

		//更新旧有道具
		dataprops.item_cnt += props.item_cnt;
		DataItemManager::Instance()->UpdateItem(dataprops);

		DataCommon::PropsChangeCPP* addpropsmsg = msg->add_propsitem();

		addpropsmsg->set_change(props.item_cnt);
		dataprops.SetMessage(addpropsmsg->mutable_props());

		PROPS_LOG("uid=%u,id=%u,propsid=%d,act=%s,chg=%d,count=%d,code=%s.",
				uid, dataprops.id, propsid, "Add", props.item_cnt, dataprops.item_cnt, reason.c_str());
	}
	else
	{
		//新增道具
		unsigned ud = DataItemManager::Instance()->GetUserNextUd(uid);
		props.id = ud;

		DataItemManager::Instance()->AddNewProps(props);

		DataCommon::PropsChangeCPP* addpropsmsg = msg->add_propsitem();

		addpropsmsg->set_change(props.item_cnt);
		props.SetMessage(addpropsmsg->mutable_props());

		PROPS_LOG("uid=%u,id=%u,propsid=%d,act=%s,chg=%d,count=%d,code=%s.",
				uid, props.id, propsid, "Add", props.item_cnt, props.item_cnt, reason.c_str());
	}

	return 0;
}

bool LogicPropsManager::IsAllowOverlay(unsigned propsid)
{
	//根据eqid获取配置文件中的叠加配置选项
	int limit = ItemCfgWrap().GetPropsItem(propsid).limitnum();

	return (limit > 1 ? true : false);
}

int LogicPropsManager::GetStorageUsedSpace(unsigned uid, unsigned type) const
{
	//遍历所有道具，根据location计算
	vector<unsigned> indexs;
	DataItemManager::Instance()->GetIndexs(uid, indexs);
	ItemCfgWrap itemcfgwrap;
	int count = 0;

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataItem & item = DataItemManager::Instance()->GetDataByIndex(indexs[i]);

		const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(item.props_id);

		if (propscfg.location() == type)
		{
			//类型一致
			count += item.item_cnt;
		}
	}

	return count;
}


