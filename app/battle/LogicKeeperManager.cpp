#include "ServerInc.h"
#include "AppDefine.h"
//查询农场助手
int LogicKeeperManager::Process(unsigned uid, ProtoKeeper::KeeperInfoReq* req, ProtoKeeper::KeeperInfoResp* resp)
{
	FillKeeperInfo(uid, req->id(), resp);
	return 0;
}
//花钻石奖励时间
int LogicKeeperManager::Process(unsigned uid, ProtoKeeper::KeeperBuyTime* req, ProtoKeeper::KeeperInfoResp* resp)
{
	uint32_t now = Time::GetGlobalTime();
	uint32_t keeperId = req->id();
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	if(now < keeper.ts)
	{
		error_log("keeper_still_serve uid=%u,ts=%u", uid, keeper.ts);
		throw runtime_error("keeper_still_serve");
	}
	CommonGiftConfig::CommonModifyItem cost;
	cost.mutable_based()->set_cash(KeeperCfgWrap().GetCfg().upgrade_cost());
	LogicUserManager::Instance()->CommonProcess(uid, cost, "keeper_buy_time", resp->mutable_commons());
	uint32_t rewardTime = KeeperCfgWrap().GetCfg().upgrade_cost_reward_time();
	keeper.ts = now + rewardTime * 3600;
	keeper.exp += rewardTime;
	DataKeeperManager::Instance()->UpdateItem(keeper);
	FillKeeperInfo(uid, req->id(), resp);

	LogicKeeperManager::Instance()->OnAddAnimalHungry(uid);
	return 0;
}
//看广告奖励
int LogicKeeperManager::Process(unsigned uid, ProtoKeeper::KeeperWatchAds* req, ProtoKeeper::KeeperInfoResp* resp)
{
	//如果为钻石消耗、校验钻石是否足够
	DBCUserBaseWrap userwrap(uid);
	bool isNeedCost = false;
	if(req->has_iscostdimaond() && req->iscostdimaond() == 1)
	{
		int needCash = UserCfgWrap().User().diamondcost().zhushou_open_cost().based().cash();
		if(userwrap.Obj().cash < -needCash)
		{
			throw std::runtime_error("diamond_not_enough");
		}
		isNeedCost = true;
	}

	uint32_t keeperId = req->id();
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	uint32_t now = Time::GetGlobalTime();
	if(keeper.ts < now)
	{
		keeper.ts = now;
	}
	uint32_t maxRewardSecond = KeeperCfgWrap().GetCfg().ad_reward_time() * 3600;
	if(keeper.ts > now + maxRewardSecond)	// 超过看广告最长奖励时间
	{
		error_log("keeper_over_ads_reward_second uid=%u,ts=%u", uid, keeper.ts);
		throw runtime_error("keeper_still_serve");
	}
	keeper.ts += 3600;
	if(keeper.ts > now + maxRewardSecond)
	{
		keeper.ts = now + maxRewardSecond;
	}
	else
	{
		keeper.exp += 1;	// 没有超过上限才加经验值
	}
	DataKeeperManager::Instance()->UpdateItem(keeper);

	if(isNeedCost)
		LogicUserManager::Instance()->CommonProcess(uid,UserCfgWrap().User().diamondcost().zhushou_open_cost(),"zhushou_open_cost",resp->mutable_commons());
	FillKeeperInfo(uid, req->id(), resp);

	LogicKeeperManager::Instance()->OnAddAnimalHungry(uid);
	return 0;
}
//设置任务
int LogicKeeperManager::Process(unsigned uid, ProtoKeeper::KeeperSetTask* req, ProtoKeeper::KeeperInfoResp* resp)
{
	uint32_t keeperId = req->keeperid();
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	uint32_t maxTarget = 0;
	KeeperCfgWrap().GetMax(keeper.level, maxTarget);
	map<uint32_t, uint32_t> typeSize;
	set<uint32_t> tid;
	// 检查每个type中的target是否超过上限
	for(uint32_t i = 0; i < req->task_size(); ++i)
	{
		const ProtoKeeper::KeeperTaskItem& taskItem = req->task(i);
		uint32_t type = GetKeeperTaskType(taskItem.taskid());
		if(typeSize.find(type) == typeSize.end())
		{
			typeSize.insert(make_pair(type, 0));
		}
		uint32_t curSize = typeSize[type] + taskItem.need();
		if(curSize > maxTarget)
		{
			error_log("curSize out of limit uid=%u,curSize=%u", uid, curSize);
			throw runtime_error("keeper_curSize_out_of_limit");
		}
		typeSize[type] = curSize;
		tid.insert(taskItem.taskid());
	}
	// 检查是否存在无效tid
	uint32_t invalidTid = 0;
	if((invalidTid = KeeperCfgWrap().ExistTarget(tid)) > 0)
	{
		error_log("invalid tid uid=%u,invalidTid=%u", uid, invalidTid);
		throw runtime_error("keeper_exist_invalid_tid");
	}

	vector<unsigned>result;
	DataKeeperTaskManager::Instance()->GetIndexs(uid,result);
	for(int i = 0; i < result.size(); i++)
	{
		DataKeeperTask& task = DataKeeperTaskManager::Instance()->GetDataByIndex(result[i]);
		if(keeperId == GetKeeperIdByTaskId(task.id) && tid.find(task.id) == tid.end())	// 重置其他任务
		{
			task.need = 0;
			task.finish = 0;
			task.status = keeper_task_status_unknow;
			DataKeeperTaskManager::Instance()->UpdateItem(task);
		}
	}

	for(uint32_t i = 0; i < req->task_size(); ++i)
	{
		const ProtoKeeper::KeeperTaskItem& taskItem = req->task(i);
		DataKeeperTask& task = DataKeeperTaskManager::Instance()->GetData(uid, taskItem.taskid());
		task.need = taskItem.need();
		task.finish = 0;
		task.status = keeper_task_status_ready;
		DataKeeperTaskManager::Instance()->UpdateItem(task);
	}
	FillKeeperInfo(uid, keeperId, resp);

	LogicProductLineManager::Instance()->OnAnimalTask(uid);
	LogicProductLineManager::Instance()->OnProductLineTask(uid);

	return 0;
}
//升级
int LogicKeeperManager::Process(unsigned uid, ProtoKeeper::KeeperUpgrade* req, ProtoKeeper::KeeperInfoResp* resp)
{
	uint32_t keeperId = req->id();
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	uint32_t upgradeExp = 0;
	KeeperCfgWrap().GetExp(keeper.level + 1, upgradeExp);
	if(keeper.exp < upgradeExp)
	{
		error_log("not enough exp uid=%u,exp=%u", uid, keeper.exp);
		throw runtime_error("not_enough_exp");
	}
	CommonGiftConfig::CommonModifyItem cost;
	KeeperCfgWrap().GetUpgradeCost(keeper.level + 1, &cost);
	LogicUserManager::Instance()->CommonProcess(uid, cost, "keeper_upgrade", resp->mutable_commons());
	keeper.level++;
	DataKeeperManager::Instance()->UpdateItem(keeper);
	FillKeeperInfo(uid, req->id(), resp);
	return 0;
}
//设置自动喂养
int LogicKeeperManager::Process(unsigned uid, ProtoKeeper::KeeperSetAutoFeed* req, ProtoKeeper::KeeperSetAutoFeedResp* resp)
{
	uint32_t keeperId = keeper_id_animal;
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	keeper.autofeed = req->autofeed();
	DataKeeperManager::Instance()->UpdateItem(keeper);
	resp->set_ret(0);
	if(keeper.autofeed == 1)
	{
		OnAnimalAutoFeed(uid);
	}
	return 0;
}
//材料变多
int LogicKeeperManager::OnAddMaterial(unsigned uid, uint32_t mId)
{
	material[uid].insert(mId);
	return 0;
}
//生产线空闲
int LogicKeeperManager::OnAddProductLine(unsigned uid, uint32_t bud)
{
	productLine[uid].insert(bud);
	return 0;
}
//动物空闲
int LogicKeeperManager::OnAddAnimalFull(unsigned uid)
{
	animalFull.insert(uid);
	return 0;
}

void LogicKeeperManager::OnTimer2()
{
	// 材料依赖
	for(map<uint32_t, set<uint32_t> >::iterator uIter = material.begin(); uIter != material.end(); ++uIter)
	{
		uint32_t uid = uIter->first;
		if (!UserManager::Instance()->IsOnline(uid))
		{
			continue;
		}
		bool autoFeed = LogicKeeperManager::Instance()->AutoFeed(uid);
		DBCUserBaseWrap userwrap(uid);
		set<uint32_t>& mc = uIter->second;
		map<uint32_t, map<uint32_t, uint32_t> > building;	// buildingId -> productId -> taskId
		// 依赖材料的生产线
		for(set<uint32_t>::iterator mIter = mc.begin(); mIter != mc.end(); ++mIter)
		{
			uint32_t mId = *mIter;
			if(!KeeperCfgWrap().ExistDependProduct(mId))
			{
				continue;
			}
			const map<unsigned, KeeperMaterial>& km = KeeperCfgWrap().GetDependProduct(mId);
			for(map<unsigned, KeeperMaterial>::const_iterator kIter = km.begin(); kIter != km.end(); ++kIter)
			{
				unsigned productId = kIter->first;
				const KeeperMaterial& item = kIter->second;
				if(userwrap.Obj().level >= item.unlockLevel)
				{
					if(ProductlineCfgWrap().ExistAnimal(item.buildId))	// 动物任务
					{
						if(autoFeed)
						{
							building[item.buildId][productId] = item.taskId;
						}
					}
					else if(LogicKeeperManager::Instance()->CheckKeeperTaskNeed(uid, item.taskId))
					{
						building[item.buildId][productId] = item.taskId;
					}
				}
			}
		}
		OnProduce(uid, building);
	}
//	debug_log("on_material_change size=%u", material.size());
	material.clear();

	// 空闲建筑
	for(map<uint32_t, set<uint32_t> >::iterator pIter = productLine.begin(); pIter != productLine.end(); ++pIter)
	{
		uint32_t uid = pIter->first;
		if (!UserManager::Instance()->IsOnline(uid))
		{
			continue;
		}
		DBCUserBaseWrap userwrap(uid);
		set<uint32_t>& mc = pIter->second;
		for(set<uint32_t>::iterator mIter = mc.begin(); mIter != mc.end(); ++mIter)
		{
			uint32_t id = *mIter;
			OnProductLine(uid, id);
		}
	}
	productLine.clear();
	// 空闲动物
	for(set<uint32_t>::iterator aIter = animalFull.begin(); aIter != animalFull.end(); ++aIter)
	{
		uint32_t uid = *aIter;
		OnAnimalAutoObtain(uid);
	}
	animalFull.clear();

	for(set<uint32_t>::iterator iter = animalHungry.begin(); iter != animalHungry.end(); ++iter)
	{
		OnAnimalAutoFeed(*iter);
	}
	animalHungry.clear();
}
uint32_t LogicKeeperManager::GetAnimalKeeperTs(uint32_t uid)
{
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, KEEPER_ID_ANIMAL);
	return keeper.ts;
}
uint32_t LogicKeeperManager::GetProductLineKeeperTs(uint32_t uid)
{
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, KEEPER_ID_PRODUCT);
	return keeper.ts;
}
bool LogicKeeperManager::IsKeeperOn(uint32_t uid, uint32_t keeperId)
{
//	uint32_t keeperId = KeeperCfgWrap().GetProductKeeperId(productId);
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	return keeper.ts > Time::GetGlobalTime();
}
bool LogicKeeperManager::AutoFeed(uint32_t uid)
{
	uint32_t keeperId = keeper_id_animal;
	if(!DataKeeperManager::Instance()->IsExistItem(uid, keeperId))
	{
		return false;
	}
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	return (keeper.autofeed == 1) && (keeper.ts > Time::GetGlobalTime());
}

int LogicKeeperManager::CheckProductLineLogin(unsigned uid, map<uint32_t, uint32_t>& finishTime, map<unsigned, set<unsigned> > &tobuild)
{
//	uint32_t now = Time::GetGlobalTime();

	for(map<uint32_t, uint32_t>::iterator iter = finishTime.begin(); iter != finishTime.end(); ++iter)
	{
		uint32_t eud = iter->first;
		uint32_t endTs = iter->second;
		OnKeeperProduce(uid, eud, endTs, tobuild);
	}
	return 0;
}
int LogicKeeperManager::CheckAnimalLogin(unsigned uid, DataAnimal & animal, map<unsigned, set<unsigned> >& tobuild)
{
	DBCUserBaseWrap userwrap(uid);
	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::AnimalLine &animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animal.animal_id);
	//获取动物产品的等待时间
	unsigned productId = animalcfg.product().props(0u).id();
	uint32_t spendTime = LogicProductLineManager::Instance()->GetFeedAnimalSpendTime(uid, animal.animal_id);

	uint32_t keeperOverTs = LogicKeeperManager::Instance()->GetAnimalKeeperTs(uid);
	for(; animal.full_time <= now && animal.full_time <= keeperOverTs; animal.full_time += spendTime)
	{
		animal.Full();
		uint32_t taskId = 0;
		// 收获农产品
		if(KeeperCfgWrap().GetProductTaskId(productId, taskId)
			&& LogicKeeperManager::Instance()->CheckKeeperTaskNeed(uid, taskId)
			&& LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse) > 0)	// 助手开启
		{
			ProtoProduce::ObtainProductResp resp;
			LogicProductLineManager::Instance()->Obtain(uid, animal.id, &resp);
			LogicKeeperManager::Instance()->AddKeeperTaskFinish(uid, taskId, 1);
			uint32_t materialUd = 0;
			if(LogicProductLineManager::Instance()->CheckAnimalMaterial(uid, animal.animal_id, materialUd))
			{
				LogicProductLineManager::Instance()->OnFinishFeedInstant(uid, animal.id, materialUd);
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
		// 喂饱动物
	}
	if(animal.full_time > now || animal.full_time > keeperOverTs)
	{
		tobuild[animal.uid].insert(animal.id);
	}
	DataAnimalManager::Instance()->UpdateItem(animal);
	return 0;
}
bool LogicKeeperManager::GetKeeperTaskBuildId(uint32_t uid, set<uint32_t>& taskBuild)
{
	vector<unsigned>result;
	DataKeeperTaskManager::Instance()->GetIndexs(uid,result);
	for(int i = 0; i < result.size(); i++)
	{
		DataKeeperTask& data = DataKeeperTaskManager::Instance()->GetDataByIndex(result[i]);
		uint32_t taskBuildId = 0;
		if(data.need > 0 && data.finish < data.need
			&& IsKeeperOn(uid, GetKeeperIdByTaskId(data.id))
			&& KeeperCfgWrap().GetProductBuild(data.id, taskBuildId))
		{
			taskBuild.insert(taskBuildId);
		}
	}
	return true;
}
bool LogicKeeperManager::OnKeeperProduce(uint32_t uid, uint32_t eud, uint32_t endTs, map<unsigned, set<unsigned> > &tobuild)
{
	const string reason = "AutoEquipProduce";
	uint32_t now = Time::GetGlobalTime();
	DBCUserBaseWrap userwrap(uid);
	ItemCfgWrap itemcfgwrap;
	set<uint32_t> productId;
	uint32_t buildId = LogicProductLineManager::Instance()->GetBuildId(uid, eud);
	ProductlineCfgWrap().GetProductProduce(buildId, productId);
	if(productId.empty())
	{
		error_log("productId is empty uid=%u eud=%u buildId=%u", uid, eud, buildId);
		return false;
	}
	uint32_t keeperOverTs = LogicKeeperManager::Instance()->GetProductLineKeeperTs(uid);
	int32_t loginMaxProduct = 500;	// 最多生产500个，防止死循环
	for(set<uint32_t>::iterator iter = productId.begin(); iter != productId.end(); ++iter)
	{
		uint32_t pId = *iter;
		uint32_t spendTime = LogicProductLineManager::Instance()->GetProduceSpendTime(uid, eud, pId);
		if(spendTime == 0)
		{
			continue;
		}
		uint32_t taskId = 0;
		if(!KeeperCfgWrap().GetProductTaskId(pId, taskId))
		{
			continue;
		}
		const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(pId);
		while(CheckKeeperTaskNeed(uid, taskId) && userwrap.CheckResBeforeCost(uid, propscfg.material()))
		{
			if(--loginMaxProduct <= 0)
			{
				return true;
			}
			endTs += spendTime;
			LogicKeeperManager::Instance()->AddKeeperTaskFinish(uid, taskId, 1);

			if(endTs <= now && LogicProductLineManager::Instance()->CheckStorage(uid, eud, pId))
			{
				// 生产消耗
				DataCommon::CommonItemsCPP resp;
				LogicUserManager::Instance()->CommonProcess(uid, propscfg.material(), reason, &resp);
				LogicProductLineManager::Instance()->OnFinishJobInstant(uid, eud, pId);
				ProductlineCfgWrap productlinecfg;

				//获取物品
				unsigned index = productlinecfg.GetEquipProductIndex(buildId, pId);
				const ConfigProductLine::EquipLine & equipcfg = productlinecfg.GetEquiplineCfg(buildId);
				const CommonGiftConfig::CommonModifyItem& product = equipcfg.product_list(index);
				LogicProductLineManager::Instance()->AddFetchReward(uid, product, reason, &resp);
			}
			else
			{
				ProtoProduce::JoinQueueResp resp;
				LogicProductLineManager::Instance()->JoinEquipQueue(uid, eud, pId, PRODUCE_TYPE_KEEPER, &resp);
				tobuild[endTs].insert(eud);
				return true;
			}
		}
	}
	return false;
}
int LogicKeeperManager::FillKeeperInfo(uint32_t uid, uint32_t keeperId, ProtoKeeper::KeeperInfoResp* resp)
{
	DataKeeper& keeper = DataKeeperManager::Instance()->GetData(uid, keeperId);
	keeper.SetMessage(resp);

	vector<unsigned>result;
	DataKeeperTaskManager::Instance()->GetIndexs(uid,result);
	for(int i = 0; i < result.size(); i++)
	{
		DataKeeperTask& data = DataKeeperTaskManager::Instance()->GetDataByIndex(result[i]);
		if(keeperId == GetKeeperIdByTaskId(data.id))
		{
			data.SetMessage(resp->add_task());
		}
	}
	return 0;
}
/*
void LogicKeeperManager::OnMaterialChange(uint32_t uid, uint32_t mId)
{
	if(!KeeperCfgWrap().ExistDependProduct(mId))
	{
		return;
	}
	const map<unsigned, KeeperMaterial>& km = KeeperCfgWrap().GetDependProduct(mId);
	for(map<unsigned, KeeperMaterial>::const_iterator iter = km.begin(); iter != km.end(); ++iter)
	{
		unsigned productId = iter->first;
		const KeeperMaterial& item = iter->second;
		if(count >= item.need)	// 可能足够满足多个生产线生产
		{
			count -= item.need;
		}
	}


	uint32_t productId = 0;
	if(CheckTaskStatus(productId) && CheckProductLineIdle(productId) && CheckMaterial(productId))
	{
		OnProduce(productId);
	}
}
*/
void LogicKeeperManager::AddKeeperTaskFinish(uint32_t uid, uint32_t taskId, uint32_t count)
{
	DataKeeperTask& task = DataKeeperTaskManager::Instance()->GetData(uid, taskId);
	if(task.need == 0)
	{
		return;
	}
	if(task.status == keeper_task_status_finish)
	{
		return;
	}
	task.finish += count;
	if(task.finish >= task.need)
	{
		task.finish = task.need;
		task.status = keeper_task_status_finish;
	}
	DataKeeperTaskManager::Instance()->UpdateItem(task);
}
bool LogicKeeperManager::CheckKeeperTaskNeed(uint32_t uid, uint32_t taskId)
{
	if(!IsKeeperOn(uid, GetKeeperIdByTaskId(taskId)))
	{
		return false;
	}
	DataKeeperTask& task = DataKeeperTaskManager::Instance()->GetData(uid, taskId);
	if(task.need == 0)
	{
		return false;
	}
	if(task.finish >= task.need)
	{
		return false;
	}
	return true;
}
void LogicKeeperManager::ChangeKeeperTaskStatus(uint32_t uid, uint32_t taskId, uint8_t status)
{
	DataKeeperTask& task = DataKeeperTaskManager::Instance()->GetData(uid, taskId);
	task.status = status;
	DataKeeperTaskManager::Instance()->UpdateItem(task);
}
bool LogicKeeperManager::CheckProductLineIdle(uint32_t productId) // 检查多台生产设备
{
	return true;
}
bool LogicKeeperManager::CheckMaterial(uint32_t productId)
{
	return true;
}
bool LogicKeeperManager::CheckTaskStatus(uint32_t productId)
{
	return true;
}
void LogicKeeperManager::OnProduce(uint32_t uid, map<uint32_t, map<uint32_t, uint32_t> >& building)
{
	DBCUserBaseWrap userwrap(uid);
	ItemCfgWrap itemcfgwrap;
	for(map<uint32_t, map<uint32_t, uint32_t> >::iterator bIter = building.begin(); bIter != building.end(); ++bIter)
	{
		uint32_t bId = bIter->first;
		map<uint32_t, uint32_t>& pSet = bIter->second;
		for(map<uint32_t, uint32_t>::iterator pIter = pSet.begin(); pIter != pSet.end(); ++pIter)
		{
			try{
				// check building free
				uint32_t productId = pIter->first;
				uint32_t taskId = pIter->second;
				const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(productId);
				if(userwrap.CheckResBeforeCost(uid, propscfg.material()))
				{
					if(ProductlineCfgWrap().ExistAnimal(bId))	// 动物任务
					{
						OnAnimalAutoFeed(uid);
					}
					else
					{

						uint32_t bUd = 0;
						if(LogicProductLineManager::Instance()->GetFreeProductLine(uid, bId, bUd))
						{
							DataProduceequip & equip = DataProduceequipManager::Instance()->GetData(uid, bUd);
							if(equip.status == status_free)
							{
								ProtoProduce::JoinQueueResp* resp = new ProtoProduce::JoinQueueResp;
								LogicProductLineManager::Instance()->JoinEquipQueue(uid, bUd, productId, PRODUCE_TYPE_KEEPER, resp);
								LogicKeeperManager::Instance()->AddKeeperTaskFinish(uid, taskId, 1);
								LMI->sendMsg(uid, resp);
							}
						}
					}
				}
			}catch(const std::exception& e)
			{
				error_log("auto produce fail uid=%u,reason=%s", uid, e.what());
			}
		}
	}

}
int LogicKeeperManager::GetKeeperIdByTaskId(uint32_t taskId)
{
	uint32_t type = GetKeeperTaskType(taskId);
	if(type == 1 || type == 2)
	{
		return KEEPER_ID_ANIMAL;
	}
	else
	{
		return KEEPER_ID_PRODUCT;
	}
}
int LogicKeeperManager::OnProductLine(uint32_t uid, uint32_t eud)
{
	if(!DataProduceequipManager::Instance()->IsExistItem(uid, eud))
	{
		return 0;
	}
	const string reason = "AutoEquipProduce";
	DataProduceequip & equip = DataProduceequipManager::Instance()->GetData(uid, eud);

	if(equip.status != status_free)
	{
		return 0;
	}
	/*
	for(int pos = 0; pos < equip.queuenum; ++pos)
	{
		int productId = equip.GetPosValue(equip.shelfdata, pos);
		if(productId > 0 && LogicProductLineManager::Instance()->CheckStorage(uid, eud, productId))
		{
			ProtoProduce::FetchProductResp resp;
			LogicProductLineManager::Instance()->FetchBackStorage(uid, eud, pos, &resp);
		}
	}
	*/

	uint32_t now = Time::GetGlobalTime();
	DBCUserBaseWrap userwrap(uid);
	ItemCfgWrap itemcfgwrap;
	set<uint32_t> productId;
	uint32_t buildId = LogicProductLineManager::Instance()->GetBuildId(uid, eud);
	ProductlineCfgWrap().GetProductProduce(buildId, productId);
	for(set<uint32_t>::iterator iter = productId.begin(); iter != productId.end(); ++iter)
	{
		uint32_t pId = *iter;
		uint32_t taskId = 0;
		const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(pId);
		if(KeeperCfgWrap().GetProductTaskId(pId, taskId)
				&& CheckKeeperTaskNeed(uid, taskId)
				&& userwrap.CheckResBeforeCost(uid, propscfg.material()))
		{
			uint32_t spendTime = LogicProductLineManager::Instance()->GetProduceSpendTime(uid, eud, pId);
			if(spendTime == 0)
			{
				continue;
			}
			ProtoProduce::JoinQueueResp* resp = new ProtoProduce::JoinQueueResp;
			LogicProductLineManager::Instance()->JoinEquipQueue(uid, eud, pId, PRODUCE_TYPE_KEEPER, resp);
			LogicKeeperManager::Instance()->AddKeeperTaskFinish(uid, taskId, 1);
			LMI->sendMsg(uid, resp);
			return true;
		}
	}

	return 0;
}
int LogicKeeperManager::OnAddAnimalHungry(uint32_t uid)
{
	animalHungry.insert(uid);
	return 0;
}
int LogicKeeperManager::OnAnimalAutoFeed(uint32_t uid)
{
	if (!UserManager::Instance()->IsOnline(uid))
	{
		return 0;
	}
	if(!LogicKeeperManager::Instance()->AutoFeed(uid))
	{
		return 0;
	}
	vector<uint32_t> vIdx;
	DataAnimalManager::Instance()->GetIndexs(uid, vIdx);
	for(uint32_t i = 0; i < vIdx.size(); ++i)
	{
		DataAnimal & animal = DataAnimalManager::Instance()->GetDataByIndex(vIdx[i]);
		if(animal.status != status_hungry)
		{
			continue;
		}
		uint32_t animalud = animal.id;
		uint32_t materialUd = 0;
		if(!LogicProductLineManager::Instance()->CheckAnimalMaterial(uid, animal.animal_id, materialUd))
		{
			continue;
		}

		ProtoProduce::FeedAnimalResp *feedResp = new ProtoProduce::FeedAnimalResp;
		LogicProductLineManager::Instance()->Feed(uid, animalud, materialUd, PRODUCE_TYPE_KEEPER, feedResp);

		LMI->sendMsg(uid,feedResp);
	}
	return 0;
}
int LogicKeeperManager::OnAnimalAutoObtain(uint32_t uid)	// 自动收获
{
	if (!UserManager::Instance()->IsOnline(uid))
	{
		return 0;
	}
	if(!IsKeeperOn(uid, KEEPER_ID_ANIMAL))
	{
		return 0;
	}
	vector<uint32_t> vIdx;
	DataAnimalManager::Instance()->GetIndexs(uid, vIdx);
	for(uint32_t i = 0; i < vIdx.size(); ++i)
	{
		DataAnimal & animal = DataAnimalManager::Instance()->GetDataByIndex(vIdx[i]);
		uint32_t animalud = animal.id;

		//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
		const ConfigProductLine::AnimalLine &animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animal.animal_id);
		//获取动物产品的等待时间
		unsigned productId = animalcfg.product().props(0u).id();
		uint32_t taskId = 0;
		if(animal.status != status_full
			|| !KeeperCfgWrap().GetProductTaskId(productId, taskId)
			|| !LogicKeeperManager::Instance()->CheckKeeperTaskNeed(uid, taskId)
			|| LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse) == 0)
		{
			continue;
		}

		ProtoProduce::ObtainProductResp *obtainResp = new ProtoProduce::ObtainProductResp;
		LogicProductLineManager::Instance()->Obtain(uid, animalud, obtainResp);
		LogicKeeperManager::Instance()->AddKeeperTaskFinish(uid, taskId, 1);

		LMI->sendMsg(uid,obtainResp);
	}
	return 0;
}

int LogicKeeperManager::OnStorageChange(uint32_t uid)
{
	LogicProductLineManager::Instance()->OnKeeperSuspendEquip(uid);
	return 0;
}
