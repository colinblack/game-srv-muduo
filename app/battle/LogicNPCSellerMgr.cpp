#include "ServerInc.h"


int LogicNPCSellerManager::NewUser(unsigned uid)
{
	bool is_exsit = DataNPCSellerManager::Instance()->IsExist(uid);
	if(!is_exsit)
	{
		DataNPCSeller &npcSeller = DataNPCSellerManager::Instance()->GetData(uid);
		npcSeller.props_cnt = GetRandomPropsCnt();
		npcSeller.props_id  = GetRandomPropsId(uid);
		DataNPCSellerManager::Instance()->UpdateItem(npcSeller);
	}
	return 0;
}

int LogicNPCSellerManager::CheckLogin(unsigned uid)
{
	NewUser(uid);
	if(CheckLevelValid(uid))
	{
		ProtoNPCSeller::PushNPCSellerInfo *msg = new ProtoNPCSeller::PushNPCSellerInfo;
		DataNPCSeller &npcseller = DataNPCSellerManager::Instance()->GetData(uid);
		npcseller.SetMessage(msg->mutable_npcseller());
	}
	return 0;
}

int LogicNPCSellerManager::Process(unsigned uid, ProtoNPCSeller::GetPropsDiscountReq* req, ProtoNPCSeller::GetPropsDiscountResp* resp)
{
	//验证等级是否合法
	if(!CheckLevelValid(uid))
	{
		error_log("user level is not valid");
		throw std::runtime_error("user_level_is_not_valid");
	}

	//验证是否为合法用户
	bool is_valid = DataNPCSellerManager::Instance()->IsExist(uid);
	if(!is_valid)
	{
		error_log("user is not valid");
		throw std::runtime_error("user_is_not_valid");
	}

	//验证npc 是否已cd
	DataNPCSeller &npcSeller = DataNPCSellerManager::Instance()->GetData(uid);
	if(npcSeller.npc_next_visit_ts > Time::GetGlobalTime())
	{
		error_log("param error.npcSeller cd isn't complted");
		throw std::runtime_error("npcSeller_cd_isn't_complted");
	}

	//抽取折扣
	unsigned discount = GetRandomDiscount();
	npcSeller.props_discount = discount;
	DataNPCSellerManager::Instance()->UpdateItem(npcSeller);

	//设置返回信息
	npcSeller.SetMessage(resp->mutable_npcseller());

	return 0;
}

int LogicNPCSellerManager::Process(unsigned uid, ProtoNPCSeller::ResponseNPCSellerReq* req, ProtoNPCSeller::ResponseNPCSellerResp* resp)
{
	//验证等级是否合法
	if(!CheckLevelValid(uid))
	{
		error_log("user level is not valid");
		throw std::runtime_error("user_level_is_not_valid");
	}

	//验证是否为合法用户
	bool is_valid = DataNPCSellerManager::Instance()->IsExist(uid);
	if(!is_valid)
	{
		error_log("user is not valid");
		throw std::runtime_error("user_is_not_valid");
	}

	//验证npc 是否已cd
	DataNPCSeller &npcSeller = DataNPCSellerManager::Instance()->GetData(uid);
	if(npcSeller.npc_next_visit_ts > Time::GetGlobalTime())
	{
		error_log("param error.npcSeller cd isn't complted");
		throw std::runtime_error("npcSeller_cd_isn't_complted");
	}

	//校验通过
	unsigned flag = req->responseflag();
	if(buy_npc_seller_props == flag)//购买npc商人物品
	{
		//获取玩家金币
		DBCUserBaseWrap userWrap(uid);
		unsigned user_coin = userWrap.GetCoin();

		//购买物品所需金币
		int init_coin = ItemCfgWrap().GetPropsItem(npcSeller.props_id).price().based().coin();
		float K = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller().pricebase();
		float coin = init_coin * K * npcSeller.props_discount * npcSeller.props_cnt / 100;

		//花费金币向上取整(因为coin值为负数,所以向上取整时-1)
		int cost_coin = coin > (int)coin? (int)coin - 1 : (int)coin;

		//校验金币是否符合条件
		if(user_coin < -cost_coin)
		{
			error_log("coin_isn't_enough");
			throw std::runtime_error("coin_isn't_enough");
		}

		//校验通过
		CommonGiftConfig::CommonModifyItem modifyitem;
		CommonGiftConfig::BaseItem *baseItem = modifyitem.mutable_based();
		CommonGiftConfig::PropsItem *propsItem = modifyitem.add_props();

		baseItem->set_coin(cost_coin);
		propsItem->set_id(npcSeller.props_id);
		propsItem->set_count(npcSeller.props_cnt);
		LogicUserManager::Instance()->CommonProcess(uid,modifyitem,"npcSeller_buy",resp->mutable_commons());
	}

	//无论购买与否、重新生成新的NPC商人信息
	npcSeller.props_id = GetRandomPropsId(uid);
	npcSeller.props_cnt = GetRandomPropsCnt();
	npcSeller.props_discount = 0;
	npcSeller.npc_next_visit_ts = GetNPCSellerVisitTs(uid);
	npcSeller.npc_seller_status = 0;
	DataNPCSellerManager::Instance()->UpdateItem(npcSeller);

	npcSeller.SetMessage(resp->mutable_npcseller());

	//推送NPC商人信息
	ProtoNPCSeller::PushNPCSellerInfo *msg = new ProtoNPCSeller::PushNPCSellerInfo;
	DataNPCSeller &npcseller = DataNPCSellerManager::Instance()->GetData(uid);
	npcseller.SetMessage(msg->mutable_npcseller());
	return 0;
}

int LogicNPCSellerManager::Process(unsigned uid, ProtoNPCSeller::ChangeNPCSellerStatusReq* req, ProtoNPCSeller::ChangeNPCSellerStatusResp* resp)
{
	//验证等级是否合法
	if(!CheckLevelValid(uid))
	{
		error_log("user level is not valid");
		throw std::runtime_error("user_level_is_not_valid");
	}

	//验证是否为合法用户
	bool is_valid = DataNPCSellerManager::Instance()->IsExist(uid);
	if(!is_valid)
	{
		error_log("user is not valid");
		throw std::runtime_error("user_is_not_valid");
	}

	//验证npc 是否已cd
	DataNPCSeller &npcSeller = DataNPCSellerManager::Instance()->GetData(uid);
	if(npcSeller.npc_next_visit_ts > Time::GetGlobalTime())
	{
		error_log("param error.npcSeller cd isn't complted");
		throw std::runtime_error("npcSeller_cd_isn't_complted");
	}

	//----验证通过、改变状态
	npcSeller.npc_seller_status = 1;
	npcSeller.SetMessage(resp->mutable_npcseller());
	return 0;
}

int LogicNPCSellerManager::PushNPCSellerInfo(unsigned uid,unsigned old_level,unsigned new_level)
{
	unsigned unlock_level = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller().unlocklevel();
	if(old_level < unlock_level && new_level >= unlock_level)
	{
		ProtoNPCSeller::PushNPCSellerInfo *msg = new ProtoNPCSeller::PushNPCSellerInfo;
		DataNPCSeller &npcseller = DataNPCSellerManager::Instance()->GetData(uid);
		npcseller.SetMessage(msg->mutable_npcseller());
		LMI->sendMsg(uid,msg);
	}
	return 0;
}

int LogicNPCSellerManager::GetRandom(int start,int end)
{
	int random = 0;

	//在合法的区域中生成随机数
	srand((unsigned)time(NULL));
	random = rand()%(end- start + 1) + start;//[start-end]区间的随机数

	return random;
}

unsigned LogicNPCSellerManager::GetRandomPropsId(unsigned uid)
{
	unsigned propsId = 0;

	//获取配置
	const ConfigNPCSeller::NPCSellerCPP& npc_cfg = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller();

	//获取玩家等级
	DBCUserBaseWrap userwrap(uid);
	unsigned user_level = userwrap.Obj().level;

	//根据玩家等级判定应该从哪个物品随机库中随机物品
	int index = 0;
	if(user_level > npc_cfg.level())
		index = 1;

	//获取对应随机库大小
	int size = npc_cfg.material(index).id_size();

	//从对应库中随机抽出对应的物品索引
	int random_index = 0;
	if(size >= 1)
		random_index = GetRandom(1,size) - 1;

	//获取随机的物品Id
	propsId = npc_cfg.material(index).id(random_index);

	return propsId;
}

unsigned LogicNPCSellerManager::GetRandomPropsCnt()
{
	unsigned count = 0;

	//获取配置
	const ConfigNPCSeller::NPCSellerCPP& npc_cfg = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller();
	unsigned min = npc_cfg.sellcount(0);
	unsigned max = npc_cfg.sellcount(1);

	count = GetRandom(min,max);

	return count;
}

unsigned LogicNPCSellerManager::GetRandomDiscount()
{
	unsigned discount = 0;
	//获取配置
	const ConfigNPCSeller::NPCSellerCPP& npc_cfg = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller();

	//总共有多少种折扣
	int size = npc_cfg.pricediscount_size();

	//抽出随机折扣索引
	int random_index = 0;
	if(size >= 1)
		random_index = GetRandom(1,size) - 1;

	//获取随机折扣
	discount = npc_cfg.pricediscount(random_index);
	return discount;
}

unsigned LogicNPCSellerManager::GetNPCSellerVisitTs(unsigned uid)
{
	unsigned cur_ts = Time::GetGlobalTime();

	//获取玩家等级
	DBCUserBaseWrap userWrap(uid);
	unsigned level = userWrap.Obj().level;

	//获取配置
	const ConfigNPCSeller::NPCSellerCPP& npc_cfg = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller();
	unsigned wait_time_min = npc_cfg.visitinterval(0);
	unsigned wait_time_max = npc_cfg.visitinterval(1);

	unsigned wait_time = GetRandom(wait_time_min,wait_time_max) * level;
	wait_time = wait_time > npc_cfg.maxvisitinterval()? npc_cfg.maxvisitinterval() : wait_time;

	unsigned next_visit_ts = cur_ts + wait_time;

	return next_visit_ts ;
}

bool LogicNPCSellerManager::CheckLevelValid(unsigned uid)
{
	bool valid = false;

	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;

	unsigned unlock_level = NPCSellerCfgWrap().GetNPCSellerCfg().npcseller().unlocklevel();
	if(level >= unlock_level)
		valid = true;

	return valid;
}
