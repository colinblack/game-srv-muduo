#include "ServerInc.h"

int LogicMissionManager::NewUser(unsigned uid)
{
	//获取任务配置中的第一条配置作为新手开始任务
	const ConfigTask::MissionCPP &cfg = ConfigManager::Instance()->task.m_config.missions(0);

	bool is_exsit =  DataMissionManager::Instance()->IsExistItem(uid,cfg.id());
	if(!is_exsit) {
		DataMission & mission = DataMissionManager::Instance()->GetData(uid,cfg.id());
		InitMission(mission,cfg);
		DataMissionManager::Instance()->UpdateItem(mission);
	}

	return 0;
}

int LogicMissionManager::Process(unsigned uid, ProtoMission::GetCurMissionReq* req, ProtoMission::GetCurMissionResp* resp)
{
	GetCurMission(uid,resp);
	return 0;
}

void LogicMissionManager::GetCurMission(unsigned uid,ProtoMission::GetCurMissionResp* resp)
{
	vector<unsigned>result;
	DataMissionManager::Instance()->GetIndexs(uid,result);
	for(int i = 0; i < result.size(); i++)
	{
		DataMission & mission = DataMissionManager::Instance()->GetDataByIndex(result[i]);
		mission.SetMessage(resp->add_misssion());
	}
}

int LogicMissionManager::Process(unsigned uid, ProtoMission::RewardMissionReq* req, ProtoMission::RewardMissionResp* resp)
{
	unsigned ud = req->ud();
	unsigned type = req->type();

	if(type >= type_of_errorcode_reward || type < type_of_common_reward)
	{
		throw std::runtime_error("param_error");
	}
	bool is_exsit = DataMissionManager::Instance()->IsExistItem(uid,ud);
	if(!is_exsit){
		throw std::runtime_error("task_is_not_exsit");
	}
	RewardMission(uid,ud,type,resp);
	return 0;
}

void LogicMissionManager::RewardMission(unsigned uid,unsigned ud,unsigned type,ProtoMission::RewardMissionResp* resp)
{
	DataMission & mission = DataMissionManager::Instance()->GetData(uid,ud);

	//获取配置信息
	const ConfigTask::MissionCPP & cfg = TaskCfgWrap().GetMissionInfoCfg(ud);
	unsigned condition_y = 0;
	if(cfg.condition_size() == 2)
		condition_y = cfg.condition(1);
	else if(cfg.condition_size() == 1)
		condition_y = cfg.condition(0);
	else
		throw std::runtime_error("config_error");

	//判定任务是否已可领取
	if(mission.cur_task_value < condition_y)
	{
		error_log("task_can_not_reward.cur_value=%u,target_value=%u",mission.cur_task_value,condition_y);
		throw std::runtime_error("task_can_not_reward");
	}

	if(type_of_common_reward == type) {
		//获取奖励
		LogicUserManager::Instance()->CommonProcess(uid,cfg.reward(),"mission_reward",resp->mutable_commons());
	} else if(type_of_viewad_reward == type) {
		//构造两倍奖励
		CommonGiftConfig::CommonModifyItem common;
		CommonGiftConfig::BaseItem * new_baseitem = common.mutable_based();
		if(cfg.reward().has_based()) {
			if(cfg.reward().based().has_exp())
				new_baseitem->set_exp(cfg.reward().based().exp() * 2);
			if(cfg.reward().based().has_coin())
				new_baseitem->set_coin(cfg.reward().based().coin() * 2);
			if(cfg.reward().based().has_cash())
				new_baseitem->set_cash(cfg.reward().based().cash() * 2);
		}
		LogicUserManager::Instance()->CommonProcess(uid,common,"mission_reward",resp->mutable_commons());
	}

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(mission.uid))
		LogicXsgReportManager::Instance()->XSGMissionEndReport(mission.uid,mission.id,type);

	//获取下一任务
	GetNextMission(uid,cfg,resp);

}

void LogicMissionManager::GetNextMission(unsigned uid,const ConfigTask::MissionCPP &curMissionCfg,ProtoMission::RewardMissionResp* resp)
{
	//先删除当前任务
	unsigned curMissionUd = curMissionCfg.id();
	DataMissionManager::Instance()->DelItem(uid,curMissionUd);

	//添加新任务
	for(int i = 0; i < curMissionCfg.next_size(); i++)
	{
		DataMission & mission = DataMissionManager::Instance()->GetData(uid,curMissionCfg.next(i));
		const ConfigTask::MissionCPP & nextCfg = TaskCfgWrap().GetMissionInfoCfg(curMissionCfg.next(i));
		InitMission(mission,nextCfg);
		DataMissionManager::Instance()->UpdateItem(mission);
		mission.SetMessage(resp->add_missions());
	}
}

void LogicMissionManager::InitMission(DataMission & mission,const ConfigTask::MissionCPP &cfg)
{
	//初始化任务的时候、判定此任务是否需要统计之前的数据
	unsigned cur_mission_value = 0;
	unsigned missionType = cfg.type();
	unsigned condtion_x = 0;
	if(missionType == mission_of_have_animal || missionType == mission_of_have_build || missionType == mission_of_plant_product)
	{
		if(cfg.condition_size() != 2){
			throw std::runtime_error("config_error!");
		}
		condtion_x = cfg.condition(0);
		cur_mission_value = GetMissionInitValue(mission.uid,missionType,condtion_x);
	}
	else if(missionType == mission_of_level)
	{
		if(cfg.condition_size() != 1){
			throw std::runtime_error("config_error!");
		}
		condtion_x = cfg.condition(0);
		cur_mission_value = GetMissionInitValue(mission.uid,missionType,condtion_x);
	}
	mission.cur_task_value = cur_mission_value;

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(mission.uid))
		LogicXsgReportManager::Instance()->XSGMissionStartReport(mission.uid,mission.id,missionType);
}

unsigned LogicMissionManager::GetMissionInitValue(unsigned uid ,unsigned missionType,unsigned condition_x)
{
	unsigned initvalue = 0;
	if(missionType == mission_of_have_animal)
	{
		vector<unsigned>result;
		result.clear();
		DataAnimalManager::Instance()->GetIndexs(uid,result);
		for(int i = 0; i < result.size(); i++)
		{
			DataAnimal & animal =  DataAnimalManager::Instance()->GetDataByIndex(result[i]);
			if(animal.animal_id == condition_x)
				initvalue ++;
		}
	}
	else if(missionType == mission_of_have_build)
	{
		vector<unsigned>result;
		result.clear();
		DataBuildingMgr::Instance()->GetIndexs(uid,result);
		for(int i = 0; i < result.size(); i++)
		{
			DataBuildings & build =  DataBuildingMgr::Instance()->GetDataByIndex(result[i]);
			if(build.build_id == condition_x)
				initvalue ++;
		}
	}
	else if(missionType == mission_of_plant_product)
	{
		vector<unsigned>result;
		result.clear();
		DataCroplandManager::Instance()->GetIndexs(uid,result);
		for(int i = 0; i < result.size(); i++)
		{
			DataCropland & cropland =  DataCroplandManager::Instance()->GetDataByIndex(result[i]);
			if(cropland.plant == condition_x)
				initvalue ++;
		}
	}
	else if(missionType == mission_of_level)
	{
		DBCUserBaseWrap userwrap(uid);
		unsigned user_level = userwrap.Obj().level;
		initvalue = user_level;
	}
	return initvalue;
}

int LogicMissionManager::AddMission(unsigned uid,int tasktype,int value,int var)
{
	//校验当前是否存在此任务
	vector<unsigned>indexs;
	DataMissionManager::Instance()->GetIndexs(uid,indexs);
	for(int i = 0; i < indexs.size(); i++)
	{
		DataMission & mission = DataMissionManager::Instance()->GetDataByIndex(indexs[i]);
		unsigned missionid = mission.id;

		//获取配置信息
		const ConfigTask::MissionCPP & cfg = TaskCfgWrap().GetMissionInfoCfg(missionid);
		bool is_exsit = false;
		unsigned target_value = 0;

		//如果对应的任务配置中,condition的格式为'condtion[x]',则校验是否存在此任务的时候，只需校验tasktype是否跟对应任务配置中的type相等即可
		//如果对应的任务配置中,condition的格式为'condtion[x,y]',则校验是否存在此任务的时候，除了校验tasktype跟type相等之外,还需校验var是否等于conditon中的x
		if(cfg.condition_size() == 2)
		{
			if(tasktype == cfg.type() && cfg.condition(0) == var)
			{
				is_exsit = true;
				target_value = cfg.condition(1);
			}

		}
		else if(cfg.condition_size() == 1)
		{
			if(tasktype == cfg.type())
			{
				is_exsit = true;
				target_value = cfg.condition(0);
			}
		}
		else
		{
			error_log("config_error");
				return -1;
		}

		if(is_exsit)
		{
			mission.cur_task_value += value;
			DataMissionManager::Instance()->UpdateItem(mission);
			PushCompletedMission(uid,missionid,mission.cur_task_value,target_value);
			break;
		}
	}
	return 0;
}

void LogicMissionManager::PushCompletedMission(unsigned uid,unsigned missionid,unsigned cur_value,unsigned target_value)
{
	if(cur_value >= target_value)
	{
		ProtoMission::PushMission * msg = new ProtoMission::PushMission;
		DataMission & mission = DataMissionManager::Instance()->GetData(uid,missionid);
		mission.SetMessage(msg->mutable_mission());
		LMI->sendMsg(uid,msg);
	}
}
