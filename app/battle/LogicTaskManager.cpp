#include "ServerInc.h"


int LogicTaskManager::NewUser(unsigned uid)
{

	//读取任务配置
	const ConfigTask::TaskInfo& task_cfg   = TaskCfgWrap().GetAllTaskCfg();

	//添加初始任务
	for(int i = 0; i < task_cfg.task_size(); i++)
	{
		int taskid  = task_cfg.task(i).id();
		bool is_exsit  = DataTaskManager::Instance()->IsExistItem(uid,taskid);
		if(is_exsit)
		{
			error_log("data error!,uid = %u,taskid = %u",uid,taskid);
			throw std::runtime_error("data error");
		}

		//校验通过、添加此任务
		DataTask &task   = DataTaskManager::Instance()->GetData(uid,taskid);
		DataTaskManager::Instance()->UpdateItem(task);
	}
	return 0;
}

int LogicTaskManager::CheckLogin(unsigned uid)
{
	//读取任务配置
	const ConfigTask::TaskInfo& task_cfg   = TaskCfgWrap().GetAllTaskCfg();
	//添加未初始化的任务
	for(int i = 0; i < task_cfg.task_size(); i++)
	{
		int taskid  = task_cfg.task(i).id();
		bool is_exsit  = DataTaskManager::Instance()->IsExistItem(uid,taskid);
		if(!is_exsit)
		{
			//校验通过、添加此任务
			DataTask &task   = DataTaskManager::Instance()->GetData(uid,taskid);
			DataTaskManager::Instance()->UpdateItem(task);
		}


		//-----修改bug;任务配置数据扩增、但配置未生效之前，玩家已完成此任务线.导致数据库中存储的当前最大任务值与配置中的最大进度值不匹配、进而引起前端显示错误
		//-----解决方案:回退一级任务、使当前任务可以继续执行、直至完成
		DataTask &task = DataTaskManager::Instance()->GetData(uid,taskid);
		int condition_value   = GetNextTaskMaxValue(taskid,task.cur_task_star - 1);
		if(task_cfg.task(i).condition_size() == task.cur_task_star && task.cur_task_value < condition_value)
		{
			//表明任务已完成、但任务未更新到最大值
			task.cur_task_star = task.cur_task_star - 1;
			DataTaskManager::Instance()->UpdateItem(task);
		}
	}

	//-----修改bug;对于某些任务、如建造鸡窝,原本最多只能建三个,而玩家已经建了三个,但因为前期bug添加任务的时候只加了一个个任务值,致使后面再无法完成此任务
	//------解决方案:遍历某些指定的任务数据,使之与任务数据中的当前进度值一样
	/*
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned version = activity.actdata[activiy_table_save_index_5];
	if(version != repair_task_debug_vesion)
	{
		try{
			vector<unsigned>results;
			results.clear();
			DataTaskManager::Instance()->GetIndexs(uid,results);
			for(int i = 0; i < results.size(); i++)
			{
				DataTask & task = DataTaskManager::Instance()->GetDataByIndex(results[i]);
				unsigned taskid = task.id;
				unsigned tasktype = TaskCfgWrap().GetTaskInfoCfg(taskid).tasktype();
				if(task_of_build_building == tasktype)
				{
					unsigned build_id =  TaskCfgWrap().GetTaskInfoCfg(taskid).condition(0).value(0);
					unsigned build_num = DataBuildingMgr::Instance()->GetBuildNum(uid,build_id);
					task.cur_task_value = build_num;
				}else if(task_of_product_device_cnt == tasktype)
				{
					unsigned build_id =  TaskCfgWrap().GetTaskInfoCfg(taskid).condition(0).value(0);
					int type = BuildCfgWrap().GetBuildType(build_id);
					unsigned num = LogicBuildManager::Instance()->GetAllProductBuildNum(uid);
					task.cur_task_value = num;

				}else if(task_of_product_device_star == tasktype)
				{
					vector<unsigned>indexs;
					indexs.clear();
					unsigned all_star_num = 0;
					DataEquipmentStarManager::Instance()->GetIndexs(uid,indexs);
					for(int i = 0; i < indexs.size(); i++)
					{
						DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetDataByIndex(indexs[i]);
						all_star_num += datastar.star;
					}
					task.cur_task_value = all_star_num;

				}else if(task_of_adopt_animal == tasktype)
				{
					unsigned animal_id =  TaskCfgWrap().GetTaskInfoCfg(taskid).condition(0).value(0);
					unsigned animal_num = DataAnimalManager::Instance()->GetAdoptedNum(uid,animal_id);
					task.cur_task_value = animal_num;
				}
				DataTaskManager::Instance()->UpdateItem(task);
			}
		}catch(runtime_error &e){
			error_log("%s",e.what());
		}

		//设置修复的版本数据
		activity.actdata[activiy_table_save_index_5] = repair_task_debug_vesion;
		DataGameActivityManager::Instance()->UpdateActivity(activity);
	}

	*/
	return 0;
}

int LogicTaskManager::Process(unsigned uid, ProtoTask::GetTaskReq* req, ProtoTask::GetTaskResp* resp)
{
	vector<unsigned > taskList;
	DataTaskManager::Instance()->GetIndexs(uid,taskList);

	for(int i = 0; i < taskList.size(); i++)
	{
		DataTask& task   = DataTaskManager::Instance()->GetDataByIndex(taskList[i]);
		task.SetMessage(resp->add_task());
	}
	return 0;
}


int LogicTaskManager::Process(unsigned uid, ProtoTask::RewardTaskReq* req, ProtoTask::RewardTaskResp* resp)
{
	unsigned taskid  = req->id();

	//校验是否存在
	bool is_exsit   = DataTaskManager::Instance()->IsExistItem(uid,taskid);
	if(!is_exsit)
	{
		error_log("param error. taskid = %u",taskid);
		throw std::runtime_error("param error");
	}

	//获取任务配置信息
	const ConfigTask::TaskCPP & task_cfg = TaskCfgWrap().GetTaskInfoCfg(taskid);
	int max_star   = task_cfg.condition_size();

	//获取对应任务id的信息
	DataTask& task  = DataTaskManager::Instance()->GetData(uid,taskid);
	unsigned cur_progress   =  task.cur_task_value;
	unsigned cur_task_star  =  task.cur_task_star;

	//校验任务是否已完成
	if(cur_task_star == max_star)
	{
		error_log("task is complted.taskid=%u,cur_task_star=%u,max_star=%u",taskid,cur_task_star,max_star);
		throw std::runtime_error("task is complted");
	}

	//读取配置文件中的条件值
	int next_condition_value   = GetNextTaskMaxValue(taskid,cur_task_star);

	//校验是否达到领取条件
	if(cur_progress < next_condition_value)
	{
		error_log("param error.can't reward,cur_progress=%u,condition_value=%u",cur_progress,next_condition_value);
		throw std::runtime_error("param error");
	}
	else
	{
		task.cur_task_star  = task.cur_task_star + 1;
		if(max_star == cur_task_star) {
			task.reward_status  = 1;
		}

		DataTaskManager::Instance()->UpdateItem(task);
		task.SetMessage(resp->mutable_task());

		//添加奖励
		LogicUserManager::Instance()->CommonProcess(uid,task_cfg.reward(cur_task_star),"task_reward",resp->mutable_commons());
	}
	return 0;
}

int LogicTaskManager::GetNextTaskMaxValue(unsigned taskid,unsigned star)
{
	int condition_cnt = 0;

	const ConfigTask::TaskCPP & task_cfg = TaskCfgWrap().GetTaskInfoCfg(taskid);
	unsigned condition_value = 0;

	//验证星级是否合法
	if(star >= task_cfg.condition_size())
		return -1;

	//获取配置中的任务值
	condition_cnt        =  task_cfg.condition(star).value_size();

	//根据condition[x,y]中的参数个数，来选定condtion_value的值
	if(condition_cnt > 2 || condition_cnt < 0){
		error_log("config error, taskid=%u",taskid);
		return -1;
	} else if(condition_cnt == 2)
		condition_value  = task_cfg.condition(star).value(1);
	else if(condition_cnt == 1)
		condition_value  = task_cfg.condition(star).value(0);

	return condition_value;
}

int LogicTaskManager::AddTaskData(unsigned uid,int tasktype,int value,int var)
{
	if(tasktype >= task_of_max || tasktype <= task_of_min)
	{
		error_log("param erro. add task failed,tasktype=%d",tasktype);
		return -1;
	}

	try
	{
		//获取任务类型对应的任务索引配置
		set<unsigned> indexs;
		TaskCfgWrap().GetTaskIndexsCfg(tasktype,indexs);
		unsigned taskid = 0;

		if(indexs.size() > 1)//代表一个任务类型下有多种任务
		{
			if(var == 0)
			{
				error_log("config error,tasktype = %d",tasktype);
				return -1;
			}
			else
			{
				//根据配置中的condtion[x,y]中的x与var进行匹配，获取对应的任务id
				set<unsigned>::iterator it = indexs.begin();
				for(; it != indexs.end(); it++)
				{
					const ConfigTask::TaskCPP & task_cfg  = TaskCfgWrap().GetAllTaskCfg().task(*it);
					if(var == task_cfg.condition(0).value(0))
						break;
				}
				if(it == indexs.end())
					return -1;
				taskid = TaskCfgWrap().GetAllTaskCfg().task(*it).id();
			}
		}
		else if(indexs.size() == 1)//代表一个任务类型下只有一种任务
		{
			if(var != 0)
			{
				error_log("config error,tasktype = %d",tasktype);
				return -1;
			}
			else
			{
				set<unsigned>::iterator it = indexs.begin();
				const ConfigTask::TaskCPP & task_cfg  = TaskCfgWrap().GetAllTaskCfg().task(*it);
				taskid = TaskCfgWrap().GetAllTaskCfg().task(*it).id();
			}
		}
		else
		{
			error_log("config error,tasktype = %d",tasktype);
			return -1;
		}

		UpdateTaskData(uid,taskid,value);
	}
	catch(runtime_error &e)
	{
		error_log("exception happen. uid=%u,reason=%s", uid, e.what());
		return -1;
	}

	return 0;
}

int LogicTaskManager::UpdateTaskData(unsigned uid,int taskid,int value)
{
	DataTask &task  = DataTaskManager::Instance()->GetData(uid,taskid);

	int condition_value   = GetNextTaskMaxValue(taskid,task.cur_task_star);
	if(condition_value == -1)
		return R_ERROR;

	//获取任务配置信息
	int max_star = TaskCfgWrap().GetTaskInfoCfg(taskid).condition_size();

	if(task.cur_task_star < max_star)
	{
		//更新当前进度值
		task.cur_task_value = task.cur_task_value + value;
		DataTaskManager::Instance()->UpdateItem(task);

		//任务完成进行推送
		if(task.cur_task_value >= condition_value)
		{
			ProtoTask::PushComplteTask *msg = new ProtoTask::PushComplteTask;
			task.SetMessage(msg->mutable_task());
			LogicManager::Instance()->sendMsg(uid,msg);
		}
	}
	return 0;
}
