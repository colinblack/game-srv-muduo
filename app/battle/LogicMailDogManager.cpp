#include "ServerInc.h"

int LogicMailDogManager::NewUser(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	const ConfigMailDog::MailDogInfoCPP maildog = ConfigManager::Instance()->maildog.m_config.maildog();

	//初始化数据
	for(int i = 0; i < maildog.staticid_size(); i++)
	{
		DataMailDog &mailDog = DataMailDogManager::Instance()->GetData(uid,maildog.staticid(i));
		DataMailDogManager::Instance()->UpdateItem(mailDog);
	}

	return 0;
}

int LogicMailDogManager::CheckLogin(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	const ConfigMailDog::MailDogInfoCPP maildog = ConfigManager::Instance()->maildog.m_config.maildog();

	//登录校验配置
	for(int i = 0; i < maildog.staticid_size(); i++)
	{
		bool is_exsit = DataMailDogManager::Instance()->IsExistItem(uid,maildog.staticid(i));
		if(!is_exsit)
		{
			DataMailDog &mailDog = DataMailDogManager::Instance()->GetData(uid,maildog.staticid(i));
			DataMailDogManager::Instance()->UpdateItem(mailDog);
		}
	}

	return 0;
}

int LogicMailDogManager::UpdateMailDogData(unsigned uid,int type,unsigned count)
{
	//加载玩家数据
	OffUserSaveControl offuserctl(uid);

	bool is_exsit = DataMailDogManager::Instance()->IsExistItem(uid,type);
	if(!is_exsit)
		return 0;
	DataMailDog &mailDog = DataMailDogManager::Instance()->GetData(uid,type);
	mailDog.value += count;
	DataMailDogManager::Instance()->UpdateItem(mailDog);
	return 0;
}

int LogicMailDogManager::ResetMailDogData(unsigned uid)
{
	vector<unsigned>lists;
	DataMailDogManager::Instance()->GetIndexs(uid,lists);
	for(int i = 0; i < lists.size(); i++)
	{
		DataMailDog &mailDog = DataMailDogManager::Instance()->GetDataByIndex(lists[i]);
		mailDog.value = 0;
		DataMailDogManager::Instance()->UpdateItem(mailDog);
	}
	return 0;
}

int LogicMailDogManager::Process(unsigned uid,ProtoMailDog::GetMailDogInfoReq *req,ProtoMailDog::GetMailDogInfoResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	const ConfigMailDog::MailDogInfoCPP maildog = ConfigManager::Instance()->maildog.m_config.maildog();

	if(userwrap.Obj().level < maildog.unlocklevel())
	{
		throw std::runtime_error("level_unlock");
	}

	//获取报纸狗统计信息
	vector<unsigned>lists;
	DataMailDogManager::Instance()->GetIndexs(uid,lists);
	for(int i = 0; i < lists.size(); i++)
	{
		DataMailDog &mailDog = DataMailDogManager::Instance()->GetDataByIndex(lists[i]);
		ProtoMailDog::MaidDogCPP *dog = resp->add_dog();
		dog->set_id(mailDog.id);
		dog->set_value(mailDog.value);
	}

	//繁荣度信息处理
	unsigned prosperity = GetProsperity(uid);
	unsigned exp = 0;
	unsigned coin = 0;
	GetReward(uid,prosperity,coin,exp);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	if(coin != 0)
	{
		//处理金币奖励
		CommonGiftConfig::CommonModifyItem common;
		CommonGiftConfig::BaseItem * base = common.mutable_based();
		base->set_coin(coin);
		base->set_exp(exp);
		LogicUserManager::Instance()->CommonProcess(uid,common,"propsperity_reward",resp->mutable_commons());

		//存储领取奖励的ts
		activity.actdata[activiy_table_save_index_3] = GetHourTs();
		DataGameActivityManager::Instance()->UpdateActivity(activity);
	}
	resp->mutable_prosperity()->set_rewardts(activity.actdata[activiy_table_save_index_3]);
	resp->mutable_prosperity()->set_prosperity(prosperity);

	return 0;
}

unsigned LogicMailDogManager::GetProsperity(unsigned uid)
{
	unsigned prosperity = 0;

	//获取动物住所与生产设备总数
	unsigned build_all_num = 0;
	vector<unsigned>buildlist;
	buildlist.clear();
	DataBuildingMgr::Instance()->GetIndexs(uid,buildlist);
	for(int i= 0; i < buildlist.size(); i++)
	{
		DataBuildings &build = DataBuildingMgr::Instance()->GetDataByIndex(buildlist[i]);
		int type = BuildCfgWrap().GetBuildType(build.build_id);
		if(build_type_animal_residence == type || build_type_produce_equipment == type)
		{
			build_all_num++;
		}
	}

	//获取地块数
	unsigned cropland_id = 1;
	unsigned cropland_num = DataBuildingMgr::Instance()->GetBuildNum(uid,cropland_id);

	//生产设备使用的总时间
	vector<unsigned>indexs;
	indexs.clear();
	unsigned usetime = 0;
	DataEquipmentStarManager::Instance()->GetIndexs(uid,indexs);
	for(int i = 0; i < indexs.size(); i++)
	{
		DataEquipmentStar & equipment = DataEquipmentStarManager::Instance()->GetDataByIndex(indexs[i]);
		usetime += equipment.usedtime;
	}

	//根据公式计算繁荣度值
	const ConfigMailDog::ProsperityInfoCPP & prosperitycfg = ConfigManager::Instance()->maildog.m_config.prosperity();

	prosperity = build_all_num  * prosperitycfg.buildk() + cropland_num * prosperitycfg.croplandk() + (usetime / 3600)/prosperitycfg.buildbase();

	//更新繁荣度信息
	DBCUserBaseWrap userwrap(uid);
	userwrap.Obj().prosperity = prosperity;
	DataBase &userbase = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(userbase);


	return prosperity;
}

unsigned LogicMailDogManager::GetReward(unsigned uid,unsigned prosperity,unsigned & coin,unsigned & exp)
{
	const ConfigMailDog::ProsperityInfoCPP & prosperitycfg = ConfigManager::Instance()->maildog.m_config.prosperity();

	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;

	//------获取小时差
	unsigned hour_diff = 0;

	//获取上次领取的整点ts
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned next_hour_ts = activity.actdata[activiy_table_save_index_3];

	//获取当前的整点ts
	unsigned cur_hour_ts = GetHourTs();

	if(next_hour_ts == 0)
	{
		//第一次领取
		hour_diff = prosperitycfg.firstrewardhours();
	}else {
		hour_diff = (cur_hour_ts - next_hour_ts)/3600;
		hour_diff = (hour_diff > prosperitycfg.maxhourdiff()) ? prosperitycfg.maxhourdiff() : hour_diff;
	}

	if(hour_diff > 1)
	{
		float K = (float) prosperity / prosperitycfg.prosperitybase();
		float reward = (K * level *  hour_diff) / prosperitycfg.rewardbase();
		unsigned total = prosperitycfg.cointoexp(0) +  prosperitycfg.cointoexp(1);
		coin = ceil((reward / total) * prosperitycfg.cointoexp(0));
		exp = ceil((reward / total) * prosperitycfg.cointoexp(1));
	}
	return 0;
}

unsigned LogicMailDogManager::GetHourTs()
{
	//获取当前的整点ts
	unsigned cur_hour_ts = Time::GetGlobalTime();
	time_t nts = cur_hour_ts;
	struct tm ptm;
	if(localtime_r(&nts, &ptm) != NULL)
	{
		cur_hour_ts  -= ptm.tm_min * 60;
		cur_hour_ts  -= ptm.tm_sec;
	}
	return cur_hour_ts;
}
