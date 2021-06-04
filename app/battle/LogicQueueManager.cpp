#include "LogicQueueManager.h"
#include "ServerInc.h"
#include "BattleServer.h"

//加速。立即完成系列
void DataRoutineBase::CheckUd(unsigned ud)
{
	if(ud_ != ud)
	{
		error_log("vector can't find ud. uid=%u,ud=%u,type=%u", uid_, ud, type);
		throw runtime_error("cannot_find_routine_ud");
	}
}
int DataRoutineBase::DoEnd()
{
	//debug_log("routine DoEnd begin. uid=%u,rid=%llu", uid_, rid);
	//判断是否在线，如果离线，则不做处理
	if (!UserManager::Instance()->IsOnline(uid_))
	{
		return R_ERROR;
	}

	ProtoPush::PushBuildingsCPP * msg = new ProtoPush::PushBuildingsCPP;

	SingleRoutineEnd(ud_, msg);

	//推送
	bool issuccess = LogicManager::Instance()->sendMsg(uid_, msg);

	debug_log("routine DoEnd end. uid=%u,result=%u", uid_, issuccess);

	return 0;
}
uint32_t DataRoutineBase::GetBuildType()
{
	return QUEUE_BUILD_TYPE_UNKNOW;
}
int DataRoutineBase::GetLeftTime()
{
	uint32_t now = Time::GetGlobalTime();
	return (endts_ > now) ? (endts_ - now) : 0;
}

int LogicQueueManager::OnInit(){
	BattleServer::Instance()->SetTimerCB(std::bind(&LogicQueueManager::OnTimer1, this), 1.0);
}


void LogicQueueManager::OnTimer1()
{
	unsigned now = Time::GetGlobalTime();
//	info_log("now=%d", now);
	list<DataRoutineBase*> overList;
	debug_log("task size=%d", userTask.size());
	for(map<unsigned, list<DataRoutineBase*> >::iterator uIter = userTask.begin(); uIter != userTask.end(); ++uIter)
	{
		uint32_t uid = uIter->first;
		list<DataRoutineBase*>& rList = uIter->second;
		for(list<DataRoutineBase*>::iterator rIter = rList.begin(); rIter != rList.end();)
		{
			DataRoutineBase* pRoutine = *rIter;
			if(pRoutine == NULL)
			{
				rIter = rList.erase(rIter);
				continue;
			}
			debug_log("uid=%u, now=%u, endts=%u", uid, now, pRoutine->endts_);
			if(pRoutine->endts_ <= now)
			{
				overList.push_back(pRoutine);
				rIter = rList.erase(rIter);	// 删除映射
				continue;
			}
			break;
		}
	}
	for(list<DataRoutineBase*>::iterator oIter = overList.begin(); oIter != overList.end(); ++oIter)
	{
		DataRoutineBase* pRoutine = *oIter;
		if(pRoutine != NULL)
		{
	        try
	        {
	        	TaskDone(pRoutine);
	        }catch (exception & e)
	        {
	            error_log("task done fail msg=%s", e.what());
	        }
		}
	}
	BattleServer::Instance()->SetTimerCB(std::bind(&LogicQueueManager::OnTimer1, this), 1.0);
}
void LogicQueueManager::Offline(unsigned uid)
{
	map<unsigned, list<DataRoutineBase*> >::iterator uIter = userTask.find(uid);
	if(uIter == userTask.end())
	{
		return;
	}
	list<DataRoutineBase*>& routineList = uIter->second;
	for(list<DataRoutineBase*>::iterator iter = routineList.begin(); iter != routineList.end(); ++iter)
	{
		DataRoutineBase* pRoutine = *iter;
//		debug_log("queue_routine_timer_del_old uid=%u ud=%u type=%u", pRoutine->uid_, pRoutine->ud_, pRoutine->type);
		delete pRoutine;
	}
	userTask.erase(uIter);
}

int LogicQueueManager::Process(unsigned uid, User::SpeedUpReq* req, User::SpeedUpResp* resp)
{
	unsigned type = req->type();
	unsigned ud = req->ud();
	unsigned method = 0;
	if(req->has_method())
		method = req->method();

	SpeedUp(uid, type, method,ud, resp);

	return 0;
}

DataRoutineBase * LogicQueueManager::GetRoutineObj(unsigned uid, unsigned ud, uint8_t type)
{
	map<unsigned, list<DataRoutineBase*> >::iterator uIter = userTask.find(uid);
	if(uIter == userTask.end())
	{
		return NULL;
	}
	list<DataRoutineBase*>& routineList = uIter->second;
	for(list<DataRoutineBase*>::iterator iter = routineList.begin(); iter != routineList.end(); ++iter)
	{
		if(uid == (*iter)->uid_ && ud == (*iter)->ud_ && type == (*iter)->type)
		{
			return *iter;
		}
	}
	return NULL;
}

void LogicQueueManager::TaskDone(DataRoutineBase * task)
{
//	调用TaskDone时,已经从映射中删除定时任务,但是还未释放空间
	task->DoEnd();
	delete task;
	task = NULL;
}
void LogicQueueManager::FinishInstant(uint32_t uid, uint32_t buildType)
{
	map<unsigned, list<DataRoutineBase*> >::iterator uIter = userTask.find(uid); // 玩家任务
	if(uIter == userTask.end())
	{
		return;
	}
	list<DataRoutineBase*> routineList;
	list<DataRoutineBase*>& task = uIter->second;
	for(list<DataRoutineBase*>::iterator tIter = task.begin(); tIter != task.end(); ++tIter)
	{
		DataRoutineBase *pRoutine = *tIter;
		if(pRoutine != NULL && pRoutine->GetBuildType() == buildType)
		{
			routineList.push_back(pRoutine);
		}
	}

	for(list<DataRoutineBase*>::iterator tIter = routineList.begin(); tIter != routineList.end(); ++tIter)
	{
		DataRoutineBase *pRoutine = *tIter;
		FinishRoutine(pRoutine, pRoutine->ud_, pRoutine->GetLeftTime());
	}
}
int LogicQueueManager::SpeedUp(unsigned uid, unsigned type, unsigned method,unsigned ud, User::SpeedUpResp * resp)
{
	if(0 == method || 1 == method)
	{
		/*************单个加速******************/
		DataRoutineBase * proutine = GetRoutineObj(uid, ud, type);
		if (NULL == proutine)
		{
			error_log("routine's maps can't find routine. uid=%u,type=%u,ud=%u", uid, type, ud);
			throw runtime_error("queue_no_routine");
		}

		//检查队列是否存在
		proutine->CheckUd(ud);

		int cash = 0;
		int diffts = 0;

		int speedType = routine_speed_cash;
		//获取消耗的钻石以及钻石减去的时间
		proutine->GetPriceAndATime(ud, cash, diffts, speedType);

		if(0 == method)
		{
			/********不是通过看广告加速,进行以下资源扣除处理****************/

			//钻石消耗
			CommonGiftConfig::CommonModifyItem cfg;
			if(speedType == routine_speed_cash)
				cfg.mutable_based()->set_cash(-cash);
			else
			{
				CommonGiftConfig::PropsItem* k = cfg.add_props();
				k->set_id(60089);
				k->set_count(-cash);
			}

			LogicUserManager::Instance()->CommonProcess(proutine->uid_, cfg, "SpeedUpRoutine", resp->mutable_commons());
		}

		//结束当前ud的定时任务
		FinishRoutine(proutine, ud, diffts);
	}
	else if(2 == method)
	{
		/*************************一键加速(动物跟农地加速)************************/

		//1.获取一键加速的物品类型(只有动物跟农地加速,因为动物ud不可能跟农地的ud重复，所以可以直接可以用ud来区分两种类型)
		bool is_animal = false;
		bool is_exist = DataAnimalManager::Instance()->IsExistItem(uid,ud);
		if(!is_exist)
		{
			is_exist = DataCroplandManager::Instance()->IsExistItem(uid,ud);
			if(!is_exist)
				throw std::runtime_error("not_exist_speed_type");
			else
				is_animal = false;
		}
		else
		{
			is_animal = true;
		}

		//2.获取一键加速需要所需的资源
		unsigned total_cash = 0;
		vector<DataRoutineBase *> proutineList;
		vector<unsigned>udlist,speedUdList,difftsList;
		udlist.clear();
		speedUdList.clear();
		difftsList.clear();
		proutineList.clear();

		if(is_animal)
		{
			DataAnimal &animalDemo = DataAnimalManager::Instance()->GetData(uid,ud);
			DataAnimalManager::Instance()->GetIndexs(uid,udlist);
			for(int i = 0; i < udlist.size(); i++)
			{
				DataAnimal & animal = DataAnimalManager::Instance()->GetDataByIndex(udlist[i]);
				if(animalDemo.animal_id == animal.animal_id && animal.full_time > Time::GetGlobalTime() && animal.status == status_growup)
				{
					DataRoutineBase * proutine = GetRoutineObj(uid, animal.id, type);
					if (NULL == proutine)
						continue;

					//检查队列是否存在
					try{
						proutine->CheckUd(animal.id);
					}catch(const std::exception& e){
						continue;
					}

					int cash = 0;
					int diffts = 0;

					int speedType = routine_speed_cash;
					//获取消耗的钻石以及钻石减去的时间
					proutine->GetPriceAndATime(animal.id, cash, diffts, speedType);
					total_cash += cash;
					speedUdList.push_back(animal.id);
					difftsList.push_back(diffts);
					proutineList.push_back(proutine);
				}
			}
		}
		else
		{
			DataCropland &croplandDemo = DataCroplandManager::Instance()->GetData(uid,ud);
			DataCroplandManager::Instance()->GetIndexs(uid,udlist);
			for(int i = 0; i < udlist.size(); i++)
			{
				DataCropland & cropland = DataCroplandManager::Instance()->GetDataByIndex(udlist[i]);
				if(croplandDemo.plant == cropland.plant && cropland.harvest_time > Time::GetGlobalTime() && cropland.status == status_growup)
				{
					DataRoutineBase * proutine = GetRoutineObj(uid, cropland.id, type);
					if (NULL == proutine)
						continue;

					//检查队列是否存在
					try{
						proutine->CheckUd(cropland.id);
					}catch(const std::exception& e){
						continue;
					}

					int cash = 0;
					int diffts = 0;

					int speedType = routine_speed_cash;
					//获取消耗的钻石以及钻石减去的时间
					proutine->GetPriceAndATime(cropland.id, cash, diffts, speedType);
					total_cash += cash;
					speedUdList.push_back(cropland.id);
					difftsList.push_back(diffts);
					proutineList.push_back(proutine);
				}
			}
		}

		//3.扣除资源
		CommonGiftConfig::CommonModifyItem cfg;
		CommonGiftConfig::PropsItem* k = cfg.add_props();
		k->set_id(60089);
		k->set_count(-total_cash);
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "SpeedUpRoutine", resp->mutable_commons());

		//4.结束定时任务
		for(int i = 0; i < speedUdList.size(); i++)
		{
			FinishRoutine(proutineList[i], speedUdList[i], difftsList[i]);
		}
	}
	else
	{
		throw std::runtime_error("method_param_error");
	}

	return 0;
}

int LogicQueueManager::SpeedUpTask(DataRoutineBase *pTask, int sec, bool &taskDone)
{
	taskDone = false;
	if(pTask == NULL)
	{
		throw runtime_error("wrong_routine");
	}
	unsigned currts = Time::GetGlobalTime();
	unsigned oldEndts = pTask->endts_;
	if(pTask->endts_ >= sec)
	{
		pTask->endts_ -= sec;
	}
	else
	{
		pTask->endts_ = 0;
	}
	EraseTask(pTask->uid_, pTask->ud_, pTask->type);
	if(currts >= pTask->endts_)
	{
		//当前时间大于结束时间，表示任务完成，直接在这里处理完成的动作
		taskDone = true;
		return 0;
	}
	else
	{
		AddTask(pTask);
	}
	return 0;
}

void LogicQueueManager::AddTask(DataRoutineBase * task)
{
	debug_log("AddTask");
	list<DataRoutineBase*>& routineList = userTask[task->uid_];
	list<DataRoutineBase*>::iterator iter = routineList.begin();
	for(; iter != routineList.end(); ++iter)
	{
	debug_log("endts=%u, routine endts=%u", task->endts_, (*iter)->endts_);
		if(task->endts_ <= (*iter)->endts_)
		{
			break;
		}
	}
	routineList.insert(iter, task);
}
DataRoutineBase* LogicQueueManager::EraseTask(uint32_t uid, uint32_t ud, uint8_t type)
{
	map<unsigned, list<DataRoutineBase*> >::iterator uIter = userTask.find(uid);
	if(uIter == userTask.end())
	{
		return NULL;
	}
	debug_log("EraseTask,uid=%u, listsize=%u", uIter->first, uIter->second.size());
	list<DataRoutineBase*>& routineList = uIter->second;
	for(list<DataRoutineBase*>::iterator iter = routineList.begin(); iter != routineList.end(); ++iter)
	{
	debug_log("uid=%u, uid_=%u, ud=%u, ud_=%u", uid, (*iter)->uid_, ud, (*iter)->ud_);
		if(uid == (*iter)->uid_ && ud == (*iter)->ud_ && type == (*iter)->type)
		{
			DataRoutineBase* pRoutine = *iter;
			routineList.erase(iter);
			return pRoutine;
		}
	}
	return NULL;
}
bool LogicQueueManager::IsExistBuildRoutine(unsigned uid, unsigned ud) const
{
	return LogicQueueManager::Instance()->GetRoutineObj(uid, ud, routine_type_build) != NULL;
}
int LogicQueueManager::FinishRoutine(DataRoutineBase* proutine, unsigned ud, int diffts)
{
	bool taskDone = false;
	//最后一个加速，则调用routine，用于结束routine
	SpeedUpTask(proutine, diffts, taskDone);
	if(taskDone)
	{
//		debug_log("queue_routine_speed_up_del uid=%u ud=%u type=%u", proutine->uid_, proutine->ud_, proutine->type);
		TaskDone(proutine);
	}
	return 0;
}
