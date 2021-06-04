#include "ServerInc.h"

int LogicFriendlyTreeManager::Process(unsigned uid, ProtoFriendlyTree::GetFriendlyTreeReq* req, ProtoFriendlyTree::GetFriendlyTreeResp* resp)
{
	//--------获取信息之前、判定是否需要重置友情树信息
	//获取友情树状态数据
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned nextwaterts = activity.actdata[activiy_table_save_index_1];
	unsigned treestatus = activity.actdata[activiy_table_save_index_2];
	unsigned cur_ts = Time::GetGlobalTime();

	//判定是否需要重置数据
	if(treestatus == status_regrowth && cur_ts >= nextwaterts)
	{
		activity.actdata[activiy_table_save_index_1] = 0;
		activity.actdata[activiy_table_save_index_2] = status_growth;
		DataGameActivityManager::Instance()->UpdateActivity(activity);

		vector<unsigned>results;
		results.clear();
		DataFriendlyTreeManager::Instance()->GetIndexs(uid,results);
		for(int i = 0; i < results.size(); i++)
		{
			DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
			DataFriendlyTreeManager::Instance()->DelItem(uid,tree.id);
		}
	}

	//----------获取友情树浇水者信息
	vector<unsigned>results;
	DataFriendlyTreeManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendlyTree &tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
		SetWaterTreeMessage(tree,resp->add_basictreeinfo());
	}

	SetTreeStatusMessage(uid,resp->mutable_statustreeinfo());
	return 0;
}


int LogicFriendlyTreeManager::Process(unsigned uid, ProtoFriendlyTree::WaterFriendlyTreeReq* req)
{
	DBCUserBaseWrap userwrap(uid);
	const ConfigFriendlyTree::FriendlyTreeCPP  & treecfg = ConfigManager::Instance()->friendlytree.m_config.friendlytree();

	//校验等级
	unsigned unlock_level = treecfg.unlocklevel();
	unsigned user_level = userwrap.Obj().level;
	if(user_level < unlock_level)
	{
		ProtoFriendlyTree::WaterFriendlyTreeResp* resp = new ProtoFriendlyTree::WaterFriendlyTreeResp;
		resp->set_code(water_code_level_error);
		return LMI->sendMsg(uid,resp)?0:R_ERROR;
	}

	//判定个人友情值是否已到上线
	unsigned friendly_value_max = treecfg.friendlyvaluemax();
	unsigned friendly_value = userwrap.Obj().friendly_value;
	if(friendly_value >= friendly_value_max)
	{
		ProtoFriendlyTree::WaterFriendlyTreeResp* resp = new ProtoFriendlyTree::WaterFriendlyTreeResp;
		resp->set_code(water_code_friendly_max_error);
		return LMI->sendMsg(uid,resp)?0:R_ERROR;
	}

	//----校验通过、是否需跨服处理
	unsigned othuid = req->othuid();
	if(CMI->IsNeedConnectByUID(othuid))
	{

		ProtoFriendlyTree::CSWaterFriendlyTreeReq* m = new ProtoFriendlyTree::CSWaterFriendlyTreeReq;
		m->set_othuid(othuid);
		m->set_myuid(uid);
		m->set_myname(userwrap.Obj().name);
		m->set_myhead(userwrap.Obj().fig);
		int ret = BMI->BattleConnectNoReplyByUID(othuid, m);
		AddWaterDyInfoOverServer(uid,othuid);	//跨服浇水增加一条动态
		return ret;
	}

	ProtoFriendlyTree::WaterFriendlyTreeResp* resp = new ProtoFriendlyTree::WaterFriendlyTreeResp;
	try{
		WaterTree(uid, othuid, resp);
		AddWaterDyInfo(uid,othuid);				//同服浇水增加一条动态
	}catch(const std::exception &e)
	{
		delete resp;
		error_log("failed:%s",e.what());
		throw std::runtime_error(e.what());
		return R_ERROR;
	}
	return LMI->sendMsg(uid,resp)?0:R_ERROR;

	return 0;
}

int LogicFriendlyTreeManager::Process(ProtoFriendlyTree::CSWaterFriendlyTreeReq* req)
{
	unsigned uid = req->myuid();
	unsigned othuid = req->othuid();

	//加载玩家数据
	OffUserSaveControl offuserctl(othuid);

	const ConfigFriendlyTree::FriendlyTreeCPP & treecfg = ConfigManager::Instance()->friendlytree.m_config.friendlytree();

	//消息构造返回
	ProtoFriendlyTree::CSWaterFriendlyTreeResp* resp = new ProtoFriendlyTree::CSWaterFriendlyTreeResp;

	//获取友情树状态数据
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(othuid, actid);
	unsigned nextwaterts = activity.actdata[activiy_table_save_index_1];
	unsigned treestatus = activity.actdata[activiy_table_save_index_2];

	//获取友情树浇水者信息
	vector<unsigned>results;
	results.clear();
	DataFriendlyTreeManager::Instance()->GetIndexs(othuid,results);

	//首先判定是否需要重置玩家友情树信息(如果树处于再生长阶段,并且再生长的冷却cd已到,则重置数据)
	unsigned maxud = 0;//清空表之前先记录当前友情树表中的最大可用ud
	unsigned cur_ts = Time::GetGlobalTime();
	if(treestatus == status_regrowth && cur_ts >= nextwaterts)
	{
		activity.actdata[activiy_table_save_index_1] = 0;
		activity.actdata[activiy_table_save_index_2] = status_growth;
		DataGameActivityManager::Instance()->UpdateActivity(activity);
		nextwaterts = activity.actdata[activiy_table_save_index_1];
		treestatus = activity.actdata[activiy_table_save_index_2];

		maxud = DataFriendlyTreeManager::Instance()->GetNextWaterUd(othuid);
		for(int i = 0; i < results.size(); i++)
		{
			DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
			DataFriendlyTreeManager::Instance()->DelItem(othuid,tree.id);
		}
	}else if(treestatus == status_regrowth && cur_ts < nextwaterts)
	{
		resp->set_code(water_code_tree_regrowth);
		return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);
	}else if(treestatus == status_haverst)
	{
		resp->set_code(water_code_tree_harvest);
		return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);
	}

	//校验是否可浇水
	unsigned waterts = GetWaterTS(othuid,uid);
	if(waterts > Time::GetGlobalTime())
	{
		resp->set_code(water_code_tree_cd_error);
		return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);
	}

	//--------------校验通过、处理浇树逻辑
	if(maxud == 0)
		maxud = DataFriendlyTreeManager::Instance()->GetNextWaterUd(othuid);
	DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetData(othuid,maxud);
	//1.往对方友情树档里添加记录
	strncpy(tree.fig, req->myhead().c_str(), BASE_FIG_LEN-1);
	strncpy(tree.name, req->myname().c_str(), BASE_NAME_LEN-1);
	tree.othuid = uid;
	tree.ts = Time::GetGlobalTime() + treecfg.treecdtime();
	DataFriendlyTreeManager::Instance()->UpdateItem(tree);
	//2.如果浇水次数已满，则更新友情树状态
	bool is_full = isFull(othuid);
	if(is_full)
	{
		activity.actdata[activiy_table_save_index_2] = status_haverst;
		DataGameActivityManager::Instance()->UpdateActivity(activity);
		PushMessage(othuid);
	}

	//消息返回
	resp->set_myuid(uid);
	SetTreeStatusMessage(othuid,resp->mutable_statustreeinfo());
	SetWaterTreeMessage(tree,resp->mutable_basictreeinfo());
	resp->set_code(water_code_success);
	return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);
}

int LogicFriendlyTreeManager::Process(ProtoFriendlyTree::CSWaterFriendlyTreeResp* req)
{
	ProtoFriendlyTree::WaterFriendlyTreeResp *resp = new ProtoFriendlyTree::WaterFriendlyTreeResp;
	unsigned uid = req->myuid();

	//判定是否执行成功
	unsigned result_code = req->code();
	if(result_code != water_code_success)
	{
		resp->set_code(result_code);
		return LMI->sendMsg(uid,resp,false)?0:R_ERROR;
	}

	const ConfigFriendlyTree::FriendlyTreeCPP & treecfg = ConfigManager::Instance()->friendlytree.m_config.friendlytree();
	DBCUserBaseWrap userwrap(uid);
	userwrap.Obj().friendly_value += treecfg.watertreereward();


	resp->mutable_statustreeinfo()->MergeFrom(*(req->mutable_statustreeinfo()));
	resp->mutable_basictreeinfo()->MergeFrom(*(req->mutable_basictreeinfo()));
	resp->set_curfriendlyvalue(userwrap.Obj().friendly_value);
	resp->set_code(result_code);

	DataBase &base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);

	return LMI->sendMsg(uid,resp,false)?0:R_ERROR;
}


int LogicFriendlyTreeManager::WaterTree(unsigned uid,unsigned othuid,ProtoFriendlyTree::WaterFriendlyTreeResp* resp)
{
	//加载玩家数据
	OffUserSaveControl offuserctl(othuid);

	const ConfigFriendlyTree::FriendlyTreeCPP & treecfg = ConfigManager::Instance()->friendlytree.m_config.friendlytree();

	//获取友情树状态数据
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(othuid, actid);
	unsigned nextwaterts = activity.actdata[activiy_table_save_index_1];
	unsigned treestatus = activity.actdata[activiy_table_save_index_2];

	//获取友情树浇水者信息
	vector<unsigned>results;
	results.clear();
	DataFriendlyTreeManager::Instance()->GetIndexs(othuid,results);

	//首先判定是否需要重置玩家友情树信息(如果树处于再生长阶段,并且再生长的冷却cd已到,则重置数据)

	unsigned maxud = 0;//清空表之前先记录当前友情树表中的最大可用ud
	unsigned cur_ts = Time::GetGlobalTime();
	if(treestatus == status_regrowth && cur_ts >= nextwaterts)
	{
		activity.actdata[activiy_table_save_index_1] = 0;
		activity.actdata[activiy_table_save_index_2] = status_growth;
		DataGameActivityManager::Instance()->UpdateActivity(activity);
		nextwaterts = activity.actdata[activiy_table_save_index_1];
		treestatus = activity.actdata[activiy_table_save_index_2];

		maxud = DataFriendlyTreeManager::Instance()->GetNextWaterUd(othuid);
		for(int i = 0; i < results.size(); i++)
		{
			DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
			DataFriendlyTreeManager::Instance()->DelItem(othuid,tree.id);
		}
	}else if(treestatus == status_regrowth && cur_ts < nextwaterts)
	{
		resp->set_code(water_code_tree_regrowth);
		return 0;
	}else if(treestatus == status_haverst)
	{
		resp->set_code(water_code_tree_harvest);
		return 0;
	}

	//校验是否可浇水
	unsigned waterts = GetWaterTS(othuid,uid);
	if(waterts > Time::GetGlobalTime())
	{
		resp->set_code(water_code_tree_cd_error);
		return 0;
	}

	//--------------校验通过、处理浇树逻辑
	if(maxud == 0)
		maxud = DataFriendlyTreeManager::Instance()->GetNextWaterUd(othuid);
	DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetData(othuid,maxud);
	//1.给自己加友情值
	DBCUserBaseWrap userwrap(uid);
	userwrap.Obj().friendly_value += treecfg.watertreereward();
	DataBase &base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);

	//2.往对方友情树档里更新记录
	memcpy(tree.fig, userwrap.Obj().fig, BASE_FIG_LEN);
	memcpy(tree.name, userwrap.Obj().name, BASE_NAME_LEN);
	tree.othuid = uid;
	tree.ts = Time::GetGlobalTime() + treecfg.treecdtime();
	DataFriendlyTreeManager::Instance()->UpdateItem(tree);
	//3.如果浇水次数已满，则更新友情树状态
	bool is_full = isFull(othuid);
	if(is_full)
	{
		activity.actdata[activiy_table_save_index_2] = status_haverst;
		DataGameActivityManager::Instance()->UpdateActivity(activity);

		//推送消息
		PushMessage(othuid);
	}

	//-------------设置返回信息
	resp->set_curfriendlyvalue(userwrap.Obj().friendly_value);
	SetTreeStatusMessage(othuid,resp->mutable_statustreeinfo());
	SetWaterTreeMessage(tree,resp->mutable_basictreeinfo());
	resp->set_code(water_code_success);

	return 0;
}

int LogicFriendlyTreeManager::PushMessage(unsigned uid)
{
	if(!UserManager::Instance()->IsOnline(uid))
		return 0;

	ProtoFriendlyTree::PushFriendlyTreeMsg *msg = new ProtoFriendlyTree::PushFriendlyTreeMsg;

	vector<unsigned>results;
	DataFriendlyTreeManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendlyTree &tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
		SetWaterTreeMessage(tree,msg->add_basictreeinfo());
	}

	SetTreeStatusMessage(uid,msg->mutable_statustreeinfo());

	return LMI->sendMsg(uid,msg,false)?0:R_ERROR;
}

bool LogicFriendlyTreeManager::isFull(unsigned uid)
{
	bool ret = false;
	unsigned count = 0;
	vector<unsigned>results;
	results.clear();
	DataFriendlyTreeManager::Instance()->GetIndexs(uid,results);

	const ConfigFriendlyTree::FriendlyTreeCPP & treecfg = ConfigManager::Instance()->friendlytree.m_config.friendlytree();
	if(results.size() >= treecfg.wateredpersonmax())
		return true;
	else
		return false;
}

int LogicFriendlyTreeManager::Process(unsigned uid, ProtoFriendlyTree::RewardFriendlyTreeReq* req, ProtoFriendlyTree::RewardFriendlyTreeResp* resp)
{
	const ConfigFriendlyTree::FriendlyTreeCPP & treecfg = ConfigManager::Instance()->friendlytree.m_config.friendlytree();

	//校验状态
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned nextwaterts = activity.actdata[activiy_table_save_index_1];
	unsigned treestatus = activity.actdata[activiy_table_save_index_2];
	if(treestatus != status_haverst)
	{
		throw std::runtime_error("tree_status_error");
	}

	//判定个人友情值是否已到上限
	DBCUserBaseWrap userwrap(uid);
	unsigned friendly_value_max = treecfg.friendlyvaluemax();
	unsigned friendly_value = userwrap.Obj().friendly_value;
	if(friendly_value >= friendly_value_max)
	{
		throw std::runtime_error("friendly_value_is_max");
	}

	//领取奖励
	userwrap.Obj().friendly_value += treecfg.rewardtreereward();
	DataBase &base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	resp->set_friendlyvalue(userwrap.Obj().friendly_value);

	//设置友情树状态信息
	activity.actdata[activiy_table_save_index_1] = Time::GetGlobalTime() + treecfg.regrowthcdtime();
	activity.actdata[activiy_table_save_index_2] = status_regrowth;
	SetTreeStatusMessage(uid,resp->mutable_statustreeinfo());
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	//获取浇水者信息
	vector<unsigned>results;
	results.clear();
	DataFriendlyTreeManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
		SetWaterTreeMessage(tree,resp->add_basictreeinfo());
	}
	return 0;
}

void LogicFriendlyTreeManager::SetWaterTreeMessage(const DataFriendlyTree &tree,ProtoFriendlyTree::FriendlyTreeBasicCPP *msg)
{
	msg->set_id(tree.id);
	msg->set_othuid(tree.othuid);
	msg->set_name(tree.name);
	msg->set_head(tree.fig);
	msg->set_ts(tree.ts);
}

void LogicFriendlyTreeManager::SetTreeStatusMessage(unsigned uid,ProtoFriendlyTree::FriendlyTreeStatusCPP *msg)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned nextwaterts = activity.actdata[activiy_table_save_index_1];
	unsigned treestatus = activity.actdata[activiy_table_save_index_2];

	msg->set_treestatus(treestatus);
	msg->set_nextwaterts(nextwaterts);
}

void LogicFriendlyTreeManager::FullMessage(unsigned uid,ProtoFriendlyTree::FriendlyTreeCPP *msg)
{
	vector<unsigned>results;
	DataFriendlyTreeManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendlyTree &tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(results[i]);
		SetWaterTreeMessage(tree,msg->add_basictreeinfo());
	}

	SetTreeStatusMessage(uid,msg->mutable_statustreeinfo());
}

unsigned LogicFriendlyTreeManager::GetWaterTS(unsigned uid,unsigned othuid)
{
	vector<unsigned>indexs;
	indexs.clear();
	DataFriendlyTreeManager::Instance()->GetIndexs(uid,indexs);

	unsigned latestWaterts = 0;
	for(int i = 0; i < indexs.size(); i++)
	{
		DataFriendlyTree & tree = DataFriendlyTreeManager::Instance()->GetDataByIndex(indexs[i]);
		if(tree.othuid == othuid)
		{
			latestWaterts = latestWaterts > tree.ts ? latestWaterts : tree.ts;
		}
	}
	return latestWaterts;
}

bool LogicFriendlyTreeManager::AddWaterDyInfo(unsigned uid,unsigned other_uid)
{
	//uid:访问者,other_uid:被访问者,访问好友庄园给好友浇水会让被访问者增加一条帮助浇水的动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_WATER,pattach))
	{
		return true;
	}
	return false;
}

bool LogicFriendlyTreeManager::AddWaterDyInfoOverServer(unsigned uid,unsigned other_uid)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_WATER);
	msg->set_productid(0);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
