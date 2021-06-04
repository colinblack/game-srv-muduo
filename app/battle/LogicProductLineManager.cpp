#include "LogicProductLineManager.h"
#include "BattleServer.h"

void DataCropProduceRoutine::CheckUd(unsigned buildud)
{
	//调用基类
	DataRoutineBase::CheckUd(buildud);

	if (!DataCroplandManager::Instance()->IsExistItem(uid_, buildud))
	{
		error_log("invalid data, cropland is not exist. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("cropland_not_exist");
	}

	//获取农地数据
	DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid_, buildud);

	if(status_growup != cropland.status)
	{
		error_log("cropland status error. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("cropland_not_growing");
	}
}

void DataCropProduceRoutine::GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type)
{
	type = routine_speed_item;
	//扣钻石，立即完成
	unsigned nowts = Time::GetGlobalTime();
	diffts = endts_ > nowts ? endts_ - nowts : 0;

	//获取农地数据
	DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid_, buildud);

	//根据作物配置，获取加速价格
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(cropland.plant);

	if (0 == propscfg.speed_price())
	{
		error_log("speed price config error. uid=%u,propsid=%u", uid_, cropland.plant);
		throw runtime_error("config_error");
	}

	cash = ceil(static_cast<double>(diffts)/propscfg.speed_price());
}

void DataCropProduceRoutine::SingleRoutineEnd(unsigned buildud, ProtoPush::PushBuildingsCPP * msg)
{
	//获取农地数据
	DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid_, buildud);

	//成熟
	cropland.Mature();
	cropland.SetMessage(msg->add_croplands());

	//结束队列
//	LogicQueueManager::Instance()->FinishQueue(uid_, buildud, routine_type_build);
}
uint32_t DataCropProduceRoutine::GetBuildType()
{
	return QUEUE_BUILD_TYPE_CROP;
}

void DataProduceEquipRoutine::CheckUd(unsigned buildud)
{
	//调用基类
	DataRoutineBase::CheckUd(buildud);

	if (!DataProduceequipManager::Instance()->IsExistItem(uid_, buildud))
	{
		error_log("invalid data, build is not exist. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("build_not_exist");
	}

	//获取设备数据
	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid_, buildud);

	if(status_procing != equipment.status)
	{
		error_log("equip not producing. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("equip_not_producing");
	}
}

void DataProduceEquipRoutine::GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type)
{
	type = routine_speed_item;
	//扣钻石，立即完成
	unsigned nowts = Time::GetGlobalTime();
	diffts = endts_ > nowts ? endts_ - nowts : 0;

	//获取设备数据
	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid_, buildud);

	//从生产队列中取队头元素，就是正在生产的产品
	unsigned product = equipment.GetQueueFront();

	//根据作物配置，获取加速价格
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(product);

	if (0 == propscfg.speed_price())
	{
		error_log("speed price config error. uid=%u,propsid=%u", uid_, product);
		throw runtime_error("config_error");
	}

	cash = ceil(static_cast<double>(diffts)/propscfg.speed_price());
}

void DataProduceEquipRoutine::SingleRoutineEnd(unsigned buildud, ProtoPush::PushBuildingsCPP * msg)
{
	//获取设备数据
	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid_, buildud);

	//设备升星功能
	EquipmentStarUpgrade(equipment, msg);
	//结束队列.
//	LogicQueueManager::Instance()->FinishQueue(uid_, buildud, routine_type_build);

	if(equipment.produce_type == PRODUCE_TYPE_KEEPER)
	{
		FinishKeeperJob(equipment);
	}
	else
	{
		//生产完成.此时设备可能状态有两种 1 -> 2或 1->0
		equipment.FinishCurrentJob();
	}
	//进行下一步动作，取决于设备状态以及生产队列
	LogicProductLineManager::Instance()->ProduceEquipNextMove(equipment);
	//设备的最终状态
	ProtoProduce::ProduceEquipCPP* pEquip = msg->add_equipments();
	equipment.SetMessage(pEquip);

	DataProduceequipManager::Instance()->UpdateItem(equipment);


	if(equipment.status == status_free)
	{
		LogicKeeperManager::Instance()->OnAddProductLine(uid_, buildud);
	}
}

uint32_t DataProduceEquipRoutine::GetBuildType()
{
	return QUEUE_BUILD_TYPE_PRODUCT_LINE;
}

int DataProduceEquipRoutine::EquipmentStarUpgrade(DataProduceequip & equipment,	ProtoPush::PushBuildingsCPP * msg)
{
	//获取升星的解锁条件
	BuildCfgWrap buildcfgwrap;
	int needlevel = buildcfgwrap.GetUnlockUpgradeStarLevel();
	unsigned uid = equipment.uid;

	//判断用户等级是否满足升星的条件
	if (BaseManager::Instance()->Get(uid).level < needlevel)
	{
		return 0;
	}

	//获取该设备的建筑id，根据id获取建筑的星级属性
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, equipment.id);
	DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, databuild.build_id);

	//获取生产设备的配置
	const ConfigBuilding::ProduceEquipment & equipcfg = buildcfgwrap.GetProduceCfgById(databuild.build_id);

	//判断星级属性是否达到最大
	if (datastar.star >= equipcfg.upgrade_star_size())
	{
		//星级已达到最大，不再增加使用时间
		return 0;
	}

	//获取当前商品的id，以此获得设备生产该商品所需的时间
	int productid = equipment.GetQueueFront();
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productid);

	int percent = LogicProductLineManager::Instance()->GetTimePercent(datastar, equipcfg);

	datastar.usedtime += (100 - percent)/static_cast<double>(100) * propscfg.time_gain();

	//根据设备的使用时间，更新当前的星级属性
	for(int i = equipcfg.upgrade_star_size() - 1; i >= 0 ; --i)
	{
		if (datastar.usedtime >= equipcfg.upgrade_star(i).need_time())
		{
			datastar.star = i+1;

			//将生产设备总星级加到任务系统中
			LogicTaskManager::Instance()->AddTaskData(uid,task_of_product_device_star,1);
			break;
		}
	}

	DataEquipmentStarManager::Instance()->UpdateItem(datastar);
	datastar.SetMessage(msg->add_equipmentstar());

	return 0;
}

int DataProduceEquipRoutine::FinishKeeperJob(DataProduceequip& equip)
{
	//展示架上有位置可以摆放.做如下操作
	//1.将队列头部的数据取出，后续数据往前移动
	int productid = equip.GetPosValue(equip.queuedata, 0);
	if(productid == 0)
	{
		error_log("queue_data_fetch_failed.productid=%d",productid);
		throw std::runtime_error("queue_data_fetch_failed");
	}
	if(LogicProductLineManager::Instance()->CheckStorage(equip.uid, equip.id, productid))
	{
		equip.PopArray(equip.queuedata, 0);
		const string reason = "AutoProductLineFinish";

		ProductlineCfgWrap productlinecfg;
		//产品在产品列表中的位置

		uint32_t buildId = LogicProductLineManager::Instance()->GetBuildId(equip.uid, equip.id);
		unsigned index = productlinecfg.GetEquipProductIndex(buildId, productid);
		const ConfigProductLine::EquipLine & equipcfg = productlinecfg.GetEquiplineCfg(buildId);
		const CommonGiftConfig::CommonModifyItem& product = equipcfg.product_list(index);
//		equip.SetMessage(resp->mutable_equipment());

		ProtoProduce::FetchProductResp * resp = new ProtoProduce::FetchProductResp;
		ProtoProduce::ProduceEquipCPP* pEquip = resp->mutable_equipment();

		LogicProductLineManager::Instance()->AddFetchReward(equip.uid, product, reason, resp->mutable_commons());
		resp->set_isfull(false);
		//状态设置为空闲,结束时间重置为0
		equip.status = status_free;
		equip.finish_time = 0;
		pEquip->set_keeper(equip.produce_type);
		equip.SetMessage(pEquip);

		LMI->sendMsg(equip.uid, resp);
	}
	else
	{
		//仓库已满，暂停
		equip.status = status_suspend;
		equip.finish_time = 0;
	}

	return 0;
}
void DataAnimalRoutine::CheckUd(unsigned animalud)
{
	//先调用基类的方法
	DataRoutineBase::CheckUd(animalud);

	if (!DataAnimalManager::Instance()->IsExistItem(uid_, animalud))
	{
		error_log("invalid data, animal is not exist. uid=%u,animalud=%u", uid_, animalud);
		throw runtime_error("animal_not_exist");
	}

	//获取动物数据
	DataAnimal & animal = DataAnimalManager::Instance()->GetData(uid_, animalud);

	if(status_growup != animal.status)
	{
		error_log("animal not in growing. uid=%u, animalud=%u", uid_, animalud);
		throw runtime_error("animal_not_growing");
	}
}

void DataAnimalRoutine::GetPriceAndATime(unsigned animalud, int & cash, int & diffts, int &type)
{
	type = routine_speed_item;
	//扣钻石，立即完成
	unsigned nowts = Time::GetGlobalTime();
	diffts = endts_ > nowts ? endts_ - nowts : 0;

	//获取动物数据
	DataAnimal & animal = DataAnimalManager::Instance()->GetData(uid_, animalud);

	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::AnimalLine & animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animal.animal_id);
	unsigned productid = animalcfg.product().props(0u).id();

	//根据作物配置，获取加速价格
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productid);

	//根据作物配置，获取加速价格
	if (0 == propscfg.speed_price())
	{
		error_log("speed price config error. uid=%u,propsid=%u", uid_, productid);
		throw runtime_error("config_error");
	}

	cash = ceil(static_cast<double>(diffts)/propscfg.speed_price());
}

void DataAnimalRoutine::SingleRoutineEnd(unsigned animalud, ProtoPush::PushBuildingsCPP * msg)
{
	//获取动物数据
	DataAnimal & animal = DataAnimalManager::Instance()->GetData(uid_, animalud);

	//动物饱腹
	animal.Full();
	DataAnimalManager::Instance()->UpdateItem(animal);

	//结束队列.
//	LogicQueueManager::Instance()->FinishQueue(uid_, animalud, routine_type_build);

	animal.SetMessage(msg->add_animals());
	LogicKeeperManager::Instance()->OnAddAnimalFull(uid_);
}
uint32_t DataAnimalRoutine::GetBuildType()
{
	return QUEUE_BUILD_TYPE_ANIMAL;
}
void DataFruitRoutine::CheckUd(unsigned fruitud)
{
	//先调用基类的方法
	DataRoutineBase::CheckUd(fruitud);

	if (!DataFruitManager::Instance()->IsExistItem(uid_, fruitud))
	{
		error_log("invalid data, fruit is not exist. uid=%u,fruitud=%u", uid_, fruitud);
		throw runtime_error("fruit_not_exist");
	}

	//获取动物数据
	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid_, fruitud);

	if(status_growup != fruit.status)
	{
		error_log("fruit not in growing. uid=%u, fruitud=%u", uid_, fruitud);
		throw runtime_error("fruit_not_growing");
	}
}

void DataFruitRoutine::GetPriceAndATime(unsigned fruitud, int & cash, int & diffts, int &type)
{
	type = routine_speed_item;
	//扣钻石，立即完成
	unsigned nowts = Time::GetGlobalTime();
	diffts = endts_ > nowts ? endts_ - nowts : 0;

	//获取果树数据
	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid_, fruitud);

	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::FruitLine & fruitcfg = ProductlineCfgWrap().GetFruitlineCfg(fruit.treeid);
	unsigned productid = fruitcfg.stage_product(fruit.stage - 1).props(0u).id();

	//根据作物配置，获取加速价格
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productid);

	//根据作物配置，获取加速价格
	if (0 == propscfg.speed_price())
	{
		error_log("speed price config error. uid=%u,propsid=%u", uid_, productid);
		throw runtime_error("config_error");
	}

	cash = ceil(static_cast<double>(diffts)/propscfg.speed_price());
}

void DataFruitRoutine::SingleRoutineEnd(unsigned fruitud, ProtoPush::PushBuildingsCPP * msg)
{
	//获取果树数据
	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid_, fruitud);

	//获取该阶段果树能够收获的个数
	const ConfigProductLine::FruitLine & fruitcfg = ProductlineCfgWrap().GetFruitlineCfg(fruit.treeid);

	//根据阶段，读取配置
	if (fruitcfg.stage_product_size() < fruit.stage)
	{
		error_log("config error. uid=%u,fruitud=%u", uid_, fruitud);
		throw runtime_error("config_error");
	}

	//更新果树的状态
//	fruit.status = status_harvest;  //可收获
	LogicProductLineManager::Instance()->SetFruitStatus(fruit, status_harvest);
	fruit.fruit_left_num = fruitcfg.stage_product(fruit.stage - 1).props(0u).count();

	DataFruitManager::Instance()->UpdateItem(fruit);

	//结束队列.
//	LogicQueueManager::Instance()->FinishQueue(uid_, fruitud, routine_type_build);

	fruit.SetMessage(msg->add_fruits());
}

LogicProductLineManager::LogicProductLineManager()
{

}

int LogicProductLineManager::NewUser(unsigned uid)
{
	//动物的ud初始化
	DataAnimalManager::Instance()->Init(uid);

	return 0;
}

int LogicProductLineManager::CheckLogin(unsigned uid)
{
	//动物的ud初始化
	DataAnimalManager::Instance()->Init(uid);

	//地块生产
	OnlineCropland(uid);

	//设备生产
	OnlineEquipProduce(uid);

	//动物生产
	OnlineAnimal(uid);

	//果树生产
	OnlineFruit(uid);

	return 0;
}

int LogicProductLineManager::OnlineCropland(unsigned uid)
{
	vector<unsigned> croplands;
	//将作物正在成长的地块加入定时任务队列
	DataCroplandManager::Instance()->GetIndexs(uid, croplands);
	map<unsigned, set<unsigned> > tobuild;  //endts -> uds

	for(int i = 0; i < croplands.size(); ++i)
	{
		DataCropland & crop = DataCroplandManager::Instance()->GetDataByIndex(croplands[i]);

		if (crop.harvest_time > 0)
		{
			tobuild[crop.harvest_time].insert(crop.id);
		}
	}

	//遍历map
	for(map<unsigned, set<unsigned> >::iterator viter = tobuild.begin(); viter != tobuild.end(); ++viter)
	{
		LogicQueueManager::Instance()->JoinRoutine<DataCropProduceRoutine>(uid, viter->first, routine_type_build, viter->second);
	}

	return 0;
}

int LogicProductLineManager::OnlineEquipProduce(unsigned uid)
{
	vector<unsigned> equips;
	//将正在生产的设备加入定时任务队列
	DataProduceequipManager::Instance()->GetIndexs(uid, equips);
	map<unsigned, set<unsigned> > tobuild;  //endts -> uds

	uint32_t now = Time::GetGlobalTime();
	map<uint32_t, uint32_t> finishTime;
	set<uint32_t> taskBuild;
	LogicKeeperManager::Instance()->GetKeeperTaskBuildId(uid, taskBuild);
	for(int i = 0; i < equips.size(); ++i)
	{
		DataProduceequip & equip = DataProduceequipManager::Instance()->GetDataByIndex(equips[i]);

		unsigned finish_time = 0;
		if (equip.finish_time > 0)
		{
			RotinueTaskCheck(uid,tobuild,equip,finish_time);
		}
		else
		{
			//修复异常数据(队列中的数据 queuedata 不为空,但status为status_free)
			int worknum = equip.GetWorkQueueNum(equip.queuedata);
			for(int i = 0; i < worknum; i++)
				if(status_free == equip.status)
					equip.FinishCurrentJob();
		}

		if(finish_time > 0 && finish_time <= now && equip.status == status_free)
		{
			uint32_t buildId = LogicProductLineManager::Instance()->GetBuildId(uid, equip.id);
			if(taskBuild.find(buildId) != taskBuild.end())	// 助手任务
			{
				finishTime[equip.id] = finish_time;
			}
		}
	}
	if(!finishTime.empty())
	{
		LogicKeeperManager::Instance()->CheckProductLineLogin(uid, finishTime, tobuild);
	}
	//遍历map
	for(map<unsigned, set<unsigned> >::iterator viter = tobuild.begin(); viter != tobuild.end(); ++viter)
	{
		LogicQueueManager::Instance()->JoinRoutine<DataProduceEquipRoutine>(uid, viter->first, routine_type_build, viter->second);
	}

	return 0;
}

int LogicProductLineManager::RotinueTaskCheck(unsigned uid,map<unsigned, set<unsigned> > &tobuild,DataProduceequip &equip, unsigned& finish_time)
{
	unsigned cur_ts = Time::GetGlobalTime();
	finish_time = equip.finish_time;

	if(cur_ts > equip.finish_time)
	{
		//------------继续判定工作队列是否已完成
		//1.获取工作队列长度
		int queuenum = equip.GetWorkQueueNum(equip.queuedata);
		for(int i = 0; i < queuenum; i++)
		{
			if(equip.status != status_suspend)
			{
				//根据产品id，获取生产时间
				int productid = equip.GetPosValue(equip.queuedata,0);
				uint32_t spendTime = GetProduceSpendTime(equip.uid, equip.id,  productid);
				/*
				ItemCfgWrap itemcfgwrap;
				const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(productid);

				//获取设备的星级属性，判断是否有减速的星级
				unsigned build_id = GetBuildId(equip.uid, equip.id);
				const ConfigBuilding::ProduceEquipment & equipcfg = BuildCfgWrap().GetProduceCfgById(build_id);
				DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(equip.uid, build_id);
				int percent = GetTimePercent(datastar, equipcfg);
				*/
				//根据上一个格子的finish_time加间隔时间、计算下一个工作格子的结束时间
				unsigned endts = 0;
				if(i != 0)
				{
					//endts = finish_time + (100 - percent)/static_cast<double>(100) * propscfg.time_gain();
					endts = finish_time + spendTime;
				}
				else
				{
					endts = finish_time;
				}

				if(cur_ts >= endts)
				{
					//如果当前ts大于endts,则直接处理队列数据、无需假如定时器
					HandleProductRotinueOver(equip);
					equip.FinishCurrentJob();
					finish_time = endts;
				}
				else
				{
					finish_time = endts;
					break;
				}
			}
			else
			{
				break;
			}
		}

		DataProduceequipManager::Instance()->UpdateItem(equip);

		//推送设备星级信息
		DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, equip.id);
		DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, databuild.build_id);
		ProtoBuilding::EquipmentStarCPP *pushMsg = new ProtoBuilding::EquipmentStarCPP;
		datastar.SetMessage(pushMsg);
		LogicManager::Instance()->sendMsg(uid, pushMsg);
	}

	//若还有未完成的任务并且展示架未满、则重置finish_time,加入定时器
	int queuenum = equip.GetWorkQueueNum(equip.queuedata);
	if(queuenum != 0 && equip.status != status_suspend){
		tobuild[finish_time].insert(equip.id);
		equip.finish_time = finish_time;
		equip.status = status_procing;
	}
	return 0;
}

uint32_t LogicProductLineManager::GetProduceSpendTime(uint32_t uid, uint32_t equipud, uint32_t productId)
{
	ItemCfgWrap itemcfgwrap;
	const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(productId);

	//获取设备的星级属性，判断是否有减速的星级
	unsigned build_id = GetBuildId(uid, equipud);
	const ConfigBuilding::ProduceEquipment & equipcfg = BuildCfgWrap().GetProduceCfgById(build_id);
	DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, build_id);
	int percent = GetTimePercent(datastar, equipcfg);
	if(percent > 100)
	{
		percent = 100;
	}
	if(percent < 0)
	{
		percent = 0;
	}

	uint32_t spendTime = (100 - percent)/static_cast<double>(100) * propscfg.time_gain();
	//判定是否有仙人加速
	if(UserManager::Instance()->IsFairySpeedUpEquip(uid))
	{
		spendTime = spendTime * UserManager::Instance()->GetFairySpeedUpEquip(uid);
	}else{
		spendTime = spendTime * (1 - (LogicVIPManager::Instance()->VIPProduceSpeedUp(uid) + LogicFriendWorkerManager::Instance()->GetProductSpeedUpPercent(uid)));
	}

	if(spendTime == 0)
	{
		spendTime = 1;
	}
	return spendTime;
}
//喂养动物花费时间
uint32_t LogicProductLineManager::GetFeedAnimalSpendTime(uint32_t uid, uint32_t animalId)
{
	const ConfigProductLine::AnimalLine & animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animalId);

	//获取动物产品的等待时间
	unsigned productid = animalcfg.product().props(0u).id();
	//根据作物配置，获取等待时长
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productid);

	//是否有仙人加速
	uint32_t spendTime = propscfg.time_gain();
	if(UserManager::Instance()->IsFairySpeedUpFarm(uid))
	{
		spendTime = spendTime * UserManager::Instance()->GetFairySpeedUpFarm(uid) ;
	}else {
		spendTime = spendTime * (1 - (LogicVIPManager::Instance()->VIPAnimalSpeedUp(uid)
									 + LogicFriendWorkerManager::Instance()->GetAnimalSpeedUpPercent(uid)) );
	}

	if(spendTime == 0)
	{
		spendTime = 1;
	}
	return spendTime;
}
int LogicProductLineManager::GetHurgryAnimal(uint32_t uid, uint32_t count, set<uint32_t>& mUd)
{
	vector<uint32_t> idx;
	DataAnimalManager::Instance()->GetIndexs(uid, idx);
	for(uint32_t i = 0; i < idx.size() && mUd.size() < count; ++i)
	{
		DataAnimal &animal = DataAnimalManager::Instance()->GetDataByIndex(idx[i]);
		if(animal.status == status_hungry)
		{
			mUd.insert(animal.id);
		}
	}
	return 0;
}
int LogicProductLineManager::HandleProductRotinueOver(DataProduceequip & equipment, uint32_t productId)
{
	//获取升星的解锁条件
	BuildCfgWrap buildcfgwrap;
	int needlevel = buildcfgwrap.GetUnlockUpgradeStarLevel();
	unsigned uid = equipment.uid;

	//判断用户等级是否满足升星的条件
	if (BaseManager::Instance()->Get(uid).level < needlevel)
	{
		return 0;
	}

	//获取该设备的建筑id，根据id获取建筑的星级属性
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, equipment.id);
	DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, databuild.build_id);

	//获取生产设备的配置
	const ConfigBuilding::ProduceEquipment & equipcfg = buildcfgwrap.GetProduceCfgById(databuild.build_id);

	//判断星级属性是否达到最大
	if (datastar.star >= equipcfg.upgrade_star_size())
	{
		//星级已达到最大，不再增加使用时间
		return 0;
	}

	//获取当前商品的id，以此获得设备生产该商品所需的时间
	if(productId == 0)
	{
		productId = equipment.GetQueueFront();
	}
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productId);

	int percent = LogicProductLineManager::Instance()->GetTimePercent(datastar, equipcfg);

	datastar.usedtime += (100 - percent)/static_cast<double>(100) * propscfg.time_gain();

	//根据设备的使用时间，更新当前的星级属性
	for(int i = equipcfg.upgrade_star_size() - 1; i >= 0 ; --i)
	{
		if (datastar.usedtime >= equipcfg.upgrade_star(i).need_time())
		{
			datastar.star = i+1;

			//将生产设备总星级加到任务系统中
			LogicTaskManager::Instance()->AddTaskData(uid,task_of_product_device_star,1);
			break;
		}
	}
	DataEquipmentStarManager::Instance()->UpdateItem(datastar);

	return 0;
}

int LogicProductLineManager::OnlineAnimal(unsigned uid)
{
	uint32_t now = Time::GetGlobalTime();

	ItemCfgWrap itemcfgwrap;
	ProductlineCfgWrap productlinecfg;
	vector<unsigned> animals;
	//将正在生长的动物加入定时任务队列
	DataAnimalManager::Instance()->GetIndexs(uid, animals);
	map<unsigned, set<unsigned> > tobuild;  //endts -> uds
	for(int i = 0; i < animals.size(); ++i)
	{
		DataAnimal & animal = DataAnimalManager::Instance()->GetDataByIndex(animals[i]);
		if(animal.full_time == 0)
		{
			continue;
		}
		if(LogicKeeperManager::Instance()->AutoFeed(uid))	// 有自动喂养标志
		{
			LogicKeeperManager::Instance()->CheckAnimalLogin(uid, animal, tobuild);
		}
		else
		{
			tobuild[animal.full_time].insert(animal.id);
		}
	}

	//遍历map
	for(map<unsigned, set<unsigned> >::iterator viter = tobuild.begin(); viter != tobuild.end(); ++viter)
	{
		LogicQueueManager::Instance()->JoinRoutine<DataAnimalRoutine>(uid, viter->first, routine_type_build, viter->second);
	}

	return 0;
}

int LogicProductLineManager::OnlineFruit(unsigned uid)
{
	vector<unsigned> fruits;

	//将正在成长的果树加入定时任务队列
	DataFruitManager::Instance()->GetIndexs(uid, fruits);
	map<unsigned, set<unsigned> > tobuild;  //endts -> uds

	for(int i = 0; i < fruits.size(); ++i)
	{
		DataFruit & fruit = DataFruitManager::Instance()->GetDataByIndex(fruits[i]);

		if (fruit.harvest_time > 0)
		{
			tobuild[fruit.harvest_time].insert(fruit.id);
		}
	}

	//遍历map
	for(map<unsigned, set<unsigned> >::iterator viter = tobuild.begin(); viter != tobuild.end(); ++viter)
	{
		LogicQueueManager::Instance()->JoinRoutine<DataFruitRoutine>(uid, viter->first, routine_type_build, viter->second);
	}

	return 0;
}

int LogicProductLineManager::OnAnimalTask(unsigned uid)
{
	vector<unsigned> animals;
	DataAnimalManager::Instance()->GetIndexs(uid, animals);
	for(int i = 0; i < animals.size(); ++i)
	{
		DataAnimal & animal = DataAnimalManager::Instance()->GetDataByIndex(animals[i]);
		if(animal.status == status_full)
		{
			LogicKeeperManager::Instance()->OnAddAnimalFull(animal.uid);
		}
	}
	return 0;
}
int LogicProductLineManager::OnProductLineTask(unsigned uid)
{
	vector<unsigned> equips;
	DataProduceequipManager::Instance()->GetIndexs(uid, equips);
	for(int i = 0; i < equips.size(); ++i)
	{
		DataProduceequip &equip = DataProduceequipManager::Instance()->GetDataByIndex(equips[i]);
		if(equip.status != status_free)
		{
			continue;
		}
		LogicKeeperManager::Instance()->OnAddProductLine(equip.uid, equip.id);
	}
	return 0;
}

int LogicProductLineManager::SetFruitStatus(DataFruit& fruit, int8_t tmpStatus)
{
	fruit.status = tmpStatus;
	if(fruit.status == status_seek_help || fruit.status == status_already_help)
	{
		LogicAllianceManager::Instance()->UpdateMemberHelpTs(fruit.uid);
	}
	return 0;
}
int LogicProductLineManager::ProduceAfterBuild(unsigned uid, unsigned ud, unsigned type, ProtoProduce::ProduceProductCPP * msg)
{
	unsigned index = 0;

	switch(type)
	{
		case build_type_corpland:
			{
				//农地
				//新增地块生产需要的农地信息
				index = DataCroplandManager::Instance()->AddNewCropLand(uid, ud);
				DataCropland & crop = DataCroplandManager::Instance()->GetDataByIndex(index);

				crop.SetMessage(msg->mutable_cropland());
			}
			break;
		case build_type_animal_residence :
			//动物住所
			break;
		case build_type_produce_equipment :
			{
				//生产设备
				unsigned index = DataProduceequipManager::Instance()->AddNewEquip(uid, ud);
				DataProduceequip & equip = DataProduceequipManager::Instance()->GetDataByIndex(index);
				DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, ud);

				//生产设备
				//队列长度初始值读配置
				const ConfigBuilding::ProduceEquipment & producecfg = BuildCfgWrap().GetProduceCfgById(databuild.build_id);
				equip.queuenum = producecfg.init_queue();  //初始化队列长度
				DataProduceequipManager::Instance()->UpdateItem(equip);

				equip.SetMessage(msg->mutable_equipment());
			}
			break;
		case build_type_fruit_tree :
			{
				//果树
				//新增果树生产需要的果树信息
				index = DataFruitManager::Instance()->AddNewFruit(uid, ud);
				DataFruit & fruit = DataFruitManager::Instance()->GetDataByIndex(index);
				DataBuildings & build = DataBuildingMgr::Instance()->GetData(uid, ud);

				fruit.treeid = build.build_id;
				fruit.stage = 0;

				//新建造的果树立即进入成长
				FruitGrowUP(fruit);

				fruit.SetMessage(msg->mutable_fruit());
			}
			break;
		case build_type_decorate:
			break;
		case build_type_house:
			break;
		case build_type_storage:
			break;
		default:
			break;
	}

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::PlantCropReq* req, ProtoProduce::PlantCropResp* resp)
{
	unsigned cropud = req->plant();

	vector<unsigned> lands;

	for(int i = 0; i < req->uds_size(); ++i)
	{
		lands.push_back(req->uds(i));
	}

	PlantCrop(uid, cropud, lands, resp);

	return 0;
}

int LogicProductLineManager::PlantCrop(unsigned uid, unsigned propud, vector<unsigned> &lands, ProtoProduce::PlantCropResp *resp)
{
	//验证参数
	if(!DataItemManager::Instance()->IsExistItem(uid, propud))
	{
		error_log("crop ud is not exist. uid=%u,ud=%u", uid, propud);
		throw runtime_error("cropland_not_exist");
	}

	//获取农作物
	DataItem & crops = DataItemManager::Instance()->GetData(uid, propud);

	//判断农作物是否解锁
	DBCUserBaseWrap userwrap(uid);

	//获取农作物的配置
	ItemCfgWrap propscfgwrap;
	const ConfigItem::PropItem & propscfg = propscfgwrap.GetPropsItem(crops.props_id);

	if (propscfg.unlock_level() > userwrap.Obj().level)
	{
		error_log("product locked. uid=%u,producud=%u,productid=%u", uid, propud, crops.props_id);
		throw runtime_error("product_locked");
	}

	//判断作物类型是否农作物
	if (propscfg.type() != item_type_cropland)
	{
		error_log("product not plant. uid=%u,producud=%u", uid, propud);
		throw runtime_error("product_not_plant");
	}

	//debug_log("plant request. uid=%u,propud=%u,cropland=%u", uid, propud, lands[0]);

	//可以种植的地块
	set<unsigned> validlands;

	//批量种植，批量消耗
	for(size_t i = 0; i < lands.size(); ++i)
	{
		//判断地块是否存在，如果不存在，则记录错误日志，不做功能处理
		unsigned ud = lands[i];

		if (!DataCroplandManager::Instance()->IsExistItem(uid, ud))
		{
			error_log("cropland is not exist. uid=%u,ud=%u", uid, ud);
			continue;
		}

		//判断地块是否已有作物
		DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid, ud);

		//修复有些空农地不能种植的问题(理论上不会出现这种不正常的数据)
		if(cropland.plant == 0 && (cropland.status == status_harvest || cropland.status == status_growup))
		{
			cropland.status = status_free;
		}

		if (status_free != cropland.status)
		{
			error_log("cropland is unavailable. uid=%u,ud=%u", uid, ud);
			continue;
		}

		validlands.insert(lands[i]);
	}

	//判断作物是否为空
	if (0 == validlands.size())
	{
		error_log("no lands can plant. uid=%u,propud=%u", uid, propud);
		throw runtime_error("none_land_plant");
	}

	//判断道具是否满足条件
	if (0 == crops.item_cnt)
	{
		error_log("no crop can plant. uid=%u,propud=%u", uid, propud);
		throw runtime_error("none_crop_plant");
	}

	int realnum = crops.item_cnt;
	realnum = min((size_t)realnum, validlands.size());

	unsigned need_time = GetCropsSpendTime(uid,crops.props_id);
	if(need_time == 0)
	{
		need_time = 1;
	}
	unsigned endts = Time::GetGlobalTime() + need_time;

	//作物消耗
	CommonGiftConfig::CommonModifyItem cfg;
	CommonGiftConfig::PropsItem* cropscfg = cfg.add_props();
	cropscfg->set_id(crops.props_id);
	cropscfg->set_count(-realnum);

	LogicUserManager::Instance()->CommonProcess(uid, cfg, "CropPlant", resp->mutable_commons());

	//种植
//	for(int i = 0; i < realnum; ++i)
	int i = 0;
	set<unsigned>::iterator iter = validlands.begin();
	for(; iter != validlands.end() && i < realnum; ++iter)
	{
		++i;
		unsigned ud = *iter;
		DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid, ud);

		cropland.harvest_time = endts;
		cropland.plant = crops.props_id;
		cropland.status = status_growup;

		DataCroplandManager::Instance()->UpdateItem(cropland);

		cropland.SetMessage(resp->add_cropland());

		//将种植农作物
		LogicTaskManager::Instance()->AddTaskData(uid,task_of_plant_crops,1,crops.props_id);
		LogicMissionManager::Instance()->AddMission(uid,mission_of_plant_product,1,crops.props_id);
		LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_plant_crops, 1, crops.props_id);

		//debug_log("plant now. uid=%u,propud=%u,cropland=%u", uid, propud, lands[0]);
	}
	if(iter != validlands.end())
	{
		validlands.erase(iter, validlands.end());
	}

	//将批量的种植任务加入到常规的任务中
	LogicQueueManager::Instance()->JoinRoutine<DataCropProduceRoutine>(uid, endts, routine_type_build, validlands);

	return 0;
}

int LogicProductLineManager::GetCropsSpendTime(unsigned uid,unsigned cropsid)
{
	ItemCfgWrap propscfgwrap;
	const ConfigItem::PropItem & propscfg = propscfgwrap.GetPropsItem(cropsid);

	unsigned spendtime = propscfg.time_gain();

	//是否有仙人加速
	if(UserManager::Instance()->IsFairySpeedUpCrop(uid))
	{
		spendtime = spendtime * UserManager::Instance()->GetFairySpeedUpCrop(uid);
	}
	else{
		spendtime  = spendtime * (1 - (LogicVIPManager::Instance()->VIPCropsSpeedUp(uid) + LogicFriendWorkerManager::Instance()->GetCropsSpeedUpPercent(uid) + LogicAccessAdManager::Instance()->ScarecrowBonus(uid)));
	}

	if(spendtime == 0)
		spendtime = 1;

	return spendtime;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::ReapCropReq* req, ProtoProduce::ReapCropResp* resp)
{
	vector<unsigned> lands;

	for(int i = 0; i < req->uds_size(); ++i)
	{
		lands.push_back(req->uds(i));
	}

	ReapCrop(uid, lands, resp);

	return 0;
}

int LogicProductLineManager::ReapCrop(unsigned uid, vector<unsigned> & lands, ProtoProduce::ReapCropResp * resp)
{
	//可以收获的地块
	vector<unsigned> gainlands;

	for(size_t i = 0; i < lands.size(); ++i)
	{
		//判断地块是否存在，如果不存在，则记录错误日志，不做功能处理
		unsigned ud = lands[i];

		if (!DataCroplandManager::Instance()->IsExistItem(uid, ud))
		{
			error_log("cropland is not exist. uid=%u,ud=%u", uid, ud);
			continue;
		}

		//判断地块是否已有作物
		DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid, ud);

		if (status_harvest != cropland.status)
		{
			error_log("cropland isn't harvest. uid=%u,ud=%u", uid, ud);
			continue;
		}

		gainlands.push_back(ud);
	}

	unsigned restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_granary);

	//一块地，默认产出2份作物
	int gainum = min(restspace, (uint32_t)gainlands.size() * 2)/2;  //之所以除以2，是为了得到可以收获的地块数目
	map<unsigned, unsigned> crops;  //id->num

	CommonGiftConfig::CommonModifyItem reward;

	uint32_t cropsId = 0;
	for(int i = 0; i < gainum; ++i)
	{
		unsigned ud = gainlands[i];
		DataCropland & cropland = DataCroplandManager::Instance()->GetData(uid, ud);
		cropsId = cropland.plant;
		//获取收割作物的经验奖励
		GetExpReward(cropland.plant, 2, reward);
		int nums = CropsActivity::Instance()->GetExtraCropNumber(uid) + 2;
		crops[cropland.plant] += nums;  //产出2份+农作物活动奖励
		cropland.Harvest();
		DataCroplandManager::Instance()->UpdateItem(cropland);
		cropland.SetMessage(resp->add_cropland());

		//debug_log("reap. uid=%u,cropland=%u", uid, cropland.id);
	}

	//收获作物
	for(map<unsigned, unsigned>::iterator iter = crops.begin(); iter != crops.end(); ++iter)
	{
		LogicPropsManager::Instance()->AddProps(uid, iter->first, iter->second, "CropHarvest", resp->mutable_commons()->mutable_props());

		//将收获的作物记录到任务表中
		LogicTaskManager::Instance()->AddTaskData(uid,task_of_harvest_crops,iter->second,iter->first);

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
			LogicXsgReportManager::Instance()->XSGGetHaverstReport(uid,1,iter->first,iter->second);
	}

	//额外奖励，暂时只处理了经验
	LogicUserManager::Instance()->CommonProcess(uid, reward, "CropHarvest", resp->mutable_commons());

	//判断是否满仓
	if (restspace < gainlands.size() * 2)
	{
		//满仓
		resp->set_isfull(true);
	}
	else
	{
		resp->set_isfull(false);
	}

	//更新收获次数
	if(!RandomPropsAfterHarvest(uid, gainum * 2))
	{
		if(LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_granary) > 0)
		{
			LogicUserManager::Instance()->OnWatchAdsReward(uid, cropsId);
		}
	}

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::ExpandQueueReq* req, ProtoProduce::ExpandQueueResp* resp)
{
	unsigned equipud = req->equipud();

	ExpandQueue(uid, equipud, resp);

	return 0;
}

int LogicProductLineManager::ExpandQueue(unsigned uid, unsigned equipud, ProtoProduce::ExpandQueueResp * resp)
{
	unsigned build_id = GetBuildId(uid, equipud);

	//根据建筑id，获取设备生产线的配置
	const ConfigProductLine::EquipLine & equipcfg = ProductlineCfgWrap().GetEquiplineCfg(build_id);
	//获取生产设备建筑的配置
	const ConfigBuilding::ProduceEquipment & producecfg = BuildCfgWrap().GetProduceCfgById(build_id);

	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid, equipud);

	if (equipcfg.maxqueue() < producecfg.init_queue())
	{
		error_log("config error. uid=%u,equipid=%u", uid, build_id);
		throw runtime_error("config_error");
	}

	//判断队列是否最大
	unsigned common_user_shelf = equipment.queuenum - LogicVIPManager::Instance()->GetVIPRewardProductShelf(uid);
	if (common_user_shelf >= equipcfg.maxqueue())
	{
		error_log("queue already max. uid=%u,equipud=%u", uid, equipud);
		throw runtime_error("queue_already_max");
	}

	//下一个队列的解锁消耗索引
	int nextindex = common_user_shelf - producecfg.init_queue();

	if (nextindex < 0)
	{
		nextindex = 0;
	}

	//消耗
	LogicUserManager::Instance()->CommonProcess(uid, equipcfg.queue_price(nextindex), "QueueExpand", resp->mutable_commons());

	//增加设备的队列数
	equipment.queuenum += 1;
	DataProduceequipManager::Instance()->UpdateItem(equipment);

	equipment.SetMessage(resp->mutable_equipment());

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::JoinQueueReq* req, ProtoProduce::JoinQueueResp* resp)
{
	unsigned equipud = req->equipud();
	unsigned product = req->propsid();

	JoinEquipQueue(uid, equipud, product, PRODUCE_TYPE_MAN, resp);

	return 0;
}

int LogicProductLineManager::JoinEquipQueue(unsigned uid, unsigned equipud, unsigned productid, uint32_t productType, ProtoProduce::JoinQueueResp * resp)
{
	//获取设备id
	unsigned build_id = GetBuildId(uid, equipud);

	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid, equipud);
	int workqueue = equipment.GetWorkQueueNum(equipment.queuedata);

	//判断是否有空余队列
	if (workqueue >= equipment.queuenum)
	{
		error_log("queue is full. uid=%u,equipud=%u", uid, equipud);
		throw runtime_error("queue_is_full");
	}

	//检查该设备是否产出该产品
	ProductlineCfgWrap productlinecfg;
	productlinecfg.GetEquipProductIndex(build_id, productid);

	//判断该产品是否解锁
	DBCUserBaseWrap userwrap(uid);
	ItemCfgWrap itemcfgwrap;
	const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(productid);

	if (propscfg.unlock_level() > userwrap.Obj().level)
	{
		error_log("product locked. uid=%u,equipud=%u,productid=%u", uid, equipud, productid);
		throw runtime_error("product_locked");
	}

	//根据产品id，进行原料的扣除
	//消耗
	LogicUserManager::Instance()->CommonProcess(uid, propscfg.material(), "EquipProduce", resp->mutable_commons());

	//向设备的生产队列插入数据
	equipment.InsertQueue(equipment.queuedata, productid);

	//判断定时任务队列中是否有该设备的任务，如果有，则不做处理，否则，就加入队列
	bool hasroutine = LogicQueueManager::Instance()->IsExistBuildRoutine(uid, equipud);

	if (!hasroutine)
	{
		//不存在正在生产的任务，则开始生产
		ProduceEquipNextMove(equipment);
		equipment.produce_type = productType;
	}

	DataProduceequipManager::Instance()->UpdateItem(equipment);
	equipment.SetMessage(resp->mutable_equipment());

	//生产产品加入到任务系统中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_build_product,1,productid);
	LogicMissionManager::Instance()->AddMission(uid,mission_of_product,1,productid);

	return 0;
}
//立即完成工作
int LogicProductLineManager::OnFinishJobInstant(uint32_t uid, uint32_t equipud, uint32_t productId)
{
	DataProduceequip & equip = DataProduceequipManager::Instance()->GetData(uid, equipud);
	HandleProductRotinueOver(equip, productId);
	DataProduceequipManager::Instance()->UpdateItem(equip);
	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::FetchProductReq* req, ProtoProduce::FetchProductResp* resp)
{
	unsigned equipud = req->equipud();
	unsigned pos = req->pos();

	FetchBackStorage(uid, equipud, pos, resp);

	return 0;
}

int LogicProductLineManager::FetchBackStorage(unsigned uid, unsigned equipud, unsigned pos, ProtoProduce::FetchProductResp * resp)
{
	//获取设备id
	unsigned equip_id = GetBuildId(uid, equipud);

	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid, equipud);

	//参数判断
	if (pos > equipment.queuenum)
	{
		error_log("param error. uid=%u,pos=%u", uid, pos);
		throw runtime_error("param_error");
	}

	//获取收获的产品id
	int productid = equipment.GetPosValue(equipment.shelfdata, pos);

	if (0 == productid)
	{
		error_log("position has no product. uid=%u,equipud=%u,pos=%u", uid, equipud, pos);
		throw runtime_error("position_no_product");
	}

	ProductlineCfgWrap productlinecfg;

	//产品在产品列表中的位置
	unsigned index = productlinecfg.GetEquipProductIndex(equip_id, productid);
	const ConfigProductLine::EquipLine & equipcfg = productlinecfg.GetEquiplineCfg(equip_id);

	int count = 0;
	unsigned id = 0, num = 0;
	CommonGiftConfig::CommonModifyItem reward;

	for(int i = 0; i < equipcfg.product_list(index).props_size(); ++i)
	{
		id = equipcfg.product_list(index).props(i).id();
		num = equipcfg.product_list(index).props(i).count();

		//获取经验奖励
		GetExpReward(id, num, reward);
		count += num;
	}

	//获取货仓的已用空间
	int restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse);

	if (restspace < count)
	{
		resp->set_isfull(true);
		equipment.SetMessage(resp->mutable_equipment());

		return 0;
	}

	//货仓有足够的空间
	LogicUserManager::Instance()->CommonProcess(uid, equipcfg.product_list(index), "EquipFetch", resp->mutable_commons());
	//额外奖励，暂时只处理了经验
	LogicUserManager::Instance()->CommonProcess(uid, reward, "EquipFetch", resp->mutable_commons());

	//将收获的产品添加至任务列表中
	for(int i = 0; i < equipcfg.product_list(index).props_size(); ++i)
	{
		id = equipcfg.product_list(index).props(i).id();
		num = equipcfg.product_list(index).props(i).count();

		LogicTaskManager::Instance()->AddTaskData(uid,task_of_harvest_product,num,id);
		LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_harvest_product, num, id);

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
			LogicXsgReportManager::Instance()->XSGEquipmentReport(uid,equip_id,id,num);
	}

	//处理设备.首先，弹出展示架上取出的物品
	equipment.PopArray(equipment.shelfdata, pos);

	//处理设备的暂停生产状态
	if (status_suspend == equipment.status)
	{
		//重新处理一遍设备的生产结束方法
		equipment.FinishCurrentJob();

		//考虑设备的下一步操作
		ProduceEquipNextMove(equipment);
	}

	DataProduceequipManager::Instance()->UpdateItem(equipment);
	equipment.SetMessage(resp->mutable_equipment());

	resp->set_isfull(false);

	if(LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse) > 0)
	{
		LogicUserManager::Instance()->OnWatchAdsReward(uid, productid);
	}
	return 0;
}

//仓库添加物品
int LogicProductLineManager::AddFetchReward(unsigned uid, const CommonGiftConfig::CommonModifyItem& productCost, const string& reason, DataCommon::CommonItemsCPP * obj)
{

	unsigned id = 0, num = 0;
	CommonGiftConfig::CommonModifyItem reward;

	for(int i = 0; i < productCost.props_size(); ++i)
	{
		id = productCost.props(i).id();
		num = productCost.props(i).count();

		//获取经验奖励
		GetExpReward(id, num, reward);
	}

	//货仓有足够的空间
	LogicUserManager::Instance()->CommonProcess(uid, productCost, reason, obj);
	//额外奖励，暂时只处理了经验
	LogicUserManager::Instance()->CommonProcess(uid, reward, reason, obj);

	return 0;
}
//检查仓库容量是否足够
bool LogicProductLineManager::CheckStorage(uint32_t uid, uint32_t equipud, uint32_t productId)
{
	//获取设备id
	unsigned equip_id = GetBuildId(uid, equipud);
	DataProduceequip & equipment = DataProduceequipManager::Instance()->GetData(uid, equipud);

	ProductlineCfgWrap productlinecfg;
	const ConfigProductLine::EquipLine & equipcfg = productlinecfg.GetEquiplineCfg(equip_id);


	//产品在产品列表中的位置
	unsigned index = productlinecfg.GetEquipProductIndex(equip_id, productId);
	int count = 0;
	for(int i = 0; i < equipcfg.product_list(index).props_size(); ++i)
	{
		unsigned num = equipcfg.product_list(index).props(i).count();
		count += num;
	}

	//获取货仓的已用空间
	int restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse);
	return restspace >= count;
}
//检查动物生产材料是否足够
bool LogicProductLineManager::CheckAnimalMaterial(uint32_t uid, uint32_t animalId, uint32_t& materialUd)
{
	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::AnimalLine &animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animalId);
	//获取动物产品的等待时间
	unsigned productId = animalcfg.product().props(0u).id();

	uint32_t mCount = 0;
	DBCUserBaseWrap userwrap(uid);
	userwrap.GetMaterialLeft(uid, productId, materialUd, mCount);
	return mCount > 0;
}
void LogicProductLineManager::ProduceEquipNextMove(DataProduceequip & equipment)
{
	if (status_suspend == equipment.status)
	{
		//暂停生产状态，不做任何处理
		return ;
	}
	else if (status_free == equipment.status)
	{
		//空闲状态，则判断生产队列是否有数据，如果有，则开启常规队列
		int productid = equipment.GetQueueFront();

		if (0 == productid)
		{
			//生产队列空闲
			return ;
		}
		/*
		//生产队列不空闲
		//根据产品id，获取生产时间
		ItemCfgWrap itemcfgwrap;
		const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(productid);

		//获取设备的星级属性，判断是否有减速的星级
		unsigned build_id = GetBuildId(equipment.uid, equipment.id);
		const ConfigBuilding::ProduceEquipment & equipcfg = BuildCfgWrap().GetProduceCfgById(build_id);

		DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(equipment.uid, build_id);

		int percent = GetTimePercent(datastar, equipcfg);

		unsigned endts = Time::GetGlobalTime() + (100 - percent)/static_cast<double>(100) * propscfg.time_gain();
		*/
		uint32_t spendTime = LogicProductLineManager::Instance()->GetProduceSpendTime(equipment.uid, equipment.id, productid);
		unsigned endts = Time::GetGlobalTime() + spendTime;


		//将设备生产任务加入到常规的任务中
		LogicQueueManager::Instance()->JoinRoutine<DataProduceEquipRoutine>(equipment.uid, endts, routine_type_build, equipment.id);

		//设置设备状态
		equipment.status = status_procing;
		equipment.finish_time = endts;
	}
}

int LogicProductLineManager::GetTimePercent(DataEquipmentStar & datastar, const ConfigBuilding::ProduceEquipment & equipcfg)
{
	int percent = 0;

	//计算星级属性中，生产时间缩短的百分比
	for(int i = 0; i < datastar.star; ++i)
	{
		if (equipcfg.upgrade_star(i).ptype() == property_type_time)
		{
			percent += equipcfg.upgrade_star(i).value();  //扣除时间的百分比
		}
	}

	return percent;
}
bool LogicProductLineManager::CheckAutoProduce(uint32_t bId)
{
	return false;
}
//获取空闲的生产线
bool LogicProductLineManager::GetFreeProductLine(uint32_t uid, uint32_t bId, uint32_t& bUd)
{
	vector<uint32_t> vIdx;
	DataProduceequipManager::Instance()->GetIndexs(uid, vIdx);
	for(vector<uint32_t>::iterator iter = vIdx.begin(); iter != vIdx.end(); ++iter)
	{
		DataProduceequip & equipment = DataProduceequipManager::Instance()->GetDataByIndex(*iter);
		if(GetBuildId(uid, equipment.id) == bId && equipment.status == status_free)
		{
			bUd = equipment.id;
			return true;
		}
	}
	return false;
}
unsigned LogicProductLineManager::GetBuildId(unsigned uid, unsigned equipud)
{
	if (!DataBuildingMgr::Instance()->IsExistItem(uid, equipud) || !DataProduceequipManager::Instance()->IsExistItem(uid, equipud))
	{
		error_log("equip not exist. uid=%u,buildud=%u", uid, equipud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑消息
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, equipud);

	//判断建筑是否可用
	if (databuild.level == 0)
	{
		error_log("build is unavailable. uid=%u,equipud=%u", uid, equipud);
		throw runtime_error("build_unavailable");
	}

	return databuild.build_id;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::AdoptAnimalReq* req, ProtoProduce::AdoptAnimalResp* resp)
{
	unsigned buildud = req->buildud();

	Adopt(uid, buildud, resp);

	LogicKeeperManager::Instance()->OnAddAnimalHungry(uid);
	return 0;
}

int LogicProductLineManager::Adopt(unsigned uid, unsigned buildud, ProtoProduce::AdoptAnimalResp * resp)
{
	//判断该动物住所是否存在
	if (!DataBuildingMgr::Instance()->IsExistItem(uid, buildud))
	{
		error_log("build not exist. uid=%u,buildud=%u", uid, buildud);
		throw runtime_error("build_not_exist");
	}

	DataBuildings & build = DataBuildingMgr::Instance()->GetData(uid, buildud);

	//判断建筑是否已揭幕
	if (0 == build.level)
	{
		error_log("build not exist. uid=%u,buildud=%u", uid, buildud);
		throw runtime_error("build_not_ready");
	}

	//读取该动物住所的配置
	BuildCfgWrap buildcfgwrap;
	const ConfigBuilding::AnimalResidence & residencecfg = buildcfgwrap.GetAnimalResidenceCfgById(build.build_id);

	unsigned animalid = residencecfg.animal_id();  //住所豢养的动物id
	unsigned singlemax = residencecfg.capacity();  //动物住所容纳的动物数量

	//首先，判断该建筑能够容纳的动物是否到上限
	unsigned build_adopted = DataAnimalManager::Instance()->GetBuildAdoptedNum(uid, buildud);

	if (build_adopted >= singlemax)
	{
		error_log("build is full. uid=%u,buildud=%u", uid, buildud);
		throw runtime_error("build_full");
	}

	//获取动物配置
	const ConfigBuilding::Animal &  animalcfg = buildcfgwrap.GetAnimalCfgById(animalid);

	//获取本次领养动物需要的价格
	DBCUserBaseWrap userwrap(uid);
	int levelindex = buildcfgwrap.GetLevelIndex(animalcfg.need_level(), userwrap.Obj().level);

	//获取当前动物的领养数目
	unsigned num = DataAnimalManager::Instance()->GetAdoptedNum(uid, animalid);

	//判断当前数目是否超过要求
	if (num >= animalcfg.gain_num(levelindex))
	{
		error_log("animal num already max. uid=%u,animalid=%u", uid, animalid);
		throw runtime_error("animal_num_max");
	}

	//获取消耗
	//动物相关的索引，而当前这个索引，是用于判断当前领养的动物该用哪个索引的配置，比如消耗
	int num_index = buildcfgwrap.GetGainNumIndex(animalcfg.gain_num(), num + 1);

	//处理消耗，获取当前建造的数目的下一个，对应的正确消耗
	LogicUserManager::Instance()->CommonProcess(uid, animalcfg.need_cost(num_index), "AnimalAdopt", resp->mutable_commons());

	//添加动物
	DataAnimal & dataanimal = DataAnimalManager::Instance()->AddNewAnimal(uid, animalid, buildud);
	DataAnimalManager::Instance()->UpdateItem(dataanimal);

	//领养动物，获取动物建造奖励
	LogicUserManager::Instance()->CommonProcess(uid, animalcfg.build_reward(), "AnimalAdopt", resp->mutable_commons());

	dataanimal.SetMessage(resp->mutable_animal());

	//领养动物加入到任务系统
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_adopt_animal,1,animalid);
	LogicMissionManager::Instance()->AddMission(uid,mission_of_have_animal,1,animalid);
	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::FeedAnimalReq* req, ProtoProduce::FeedAnimalResp* resp)
{
	unsigned animalud = req->animalud();
	unsigned fodderud = req->fodderud();

	Feed(uid, animalud, fodderud, PRODUCE_TYPE_MAN, resp);

	return 0;
}

int LogicProductLineManager::Feed(unsigned uid, unsigned animalud, unsigned fodderud, unsigned productType, ProtoProduce::FeedAnimalResp * resp)
{
	//判断该动物是否存在
	if (!DataAnimalManager::Instance()->IsExistItem(uid, animalud))
	{
		error_log("animal not exist. uid=%u,animalud=%u", uid, animalud);
		throw runtime_error("animal_not_exist");
	}

	DataAnimal & animal = DataAnimalManager::Instance()->GetData(uid, animalud);

	//判断动物是否处于饥饿状态
	if (animal.status != status_hungry)
	{
		error_log("animal not hungry. uid=%u,animalud=%u", uid, animalud);
		throw runtime_error("animal_not_hungry");
	}

	//获取饲料数据
	if (!DataItemManager::Instance()->IsExistItem(uid, fodderud))
	{
		error_log("fodder not exist. uid=%u,fodderud=%u", uid, fodderud);
		throw runtime_error("fodder_not_exist");
	}

	DataItem & props = DataItemManager::Instance()->GetData(uid, fodderud);

	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::AnimalLine & animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animal.animal_id);

	if (props.props_id != animalcfg.fodder())
	{
		error_log("fodder not match. uid=%u,animalud=%u,fodderud=%u", uid, animalud, fodderud);
		throw runtime_error("fodder_not_match");
	}

	//扣除饲料消耗
	CommonGiftConfig::CommonModifyItem costcfg;
	CommonGiftConfig::PropsItem* propscfgmsg = costcfg.add_props();
	propscfgmsg->set_id(props.props_id);
	propscfgmsg->set_count(-1);

	//处理消耗
	LogicUserManager::Instance()->CommonProcess(uid, costcfg, "AnimalFeed", resp->mutable_commons());

	//修改动物状态
	uint32_t spendTime = LogicProductLineManager::GetFeedAnimalSpendTime(uid, animal.animal_id);

	unsigned endts = Time::GetGlobalTime() + spendTime;
	animal.status = status_growup;
	animal.full_time = endts;
	animal.produce_type = productType;

	DataAnimalManager::Instance()->UpdateItem(animal);

	//加入到动物饱腹的队列
	LogicQueueManager::Instance()->JoinRoutine<DataAnimalRoutine>(uid, endts, routine_type_build, animalud);

	animal.SetMessage(resp->mutable_animal());
	if(productType != PRODUCE_TYPE_KEEPER)
	{
		LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_feed_animal, 1);
	}

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
		LogicXsgReportManager::Instance()->XSGFeedAnimalReport(uid,animal.animal_id,props.props_id,1);
	return 0;
}
//立即完成喂养
int LogicProductLineManager::OnFinishFeedInstant(unsigned uid, unsigned animalud, unsigned fodderud)
{
	//判断该动物是否存在
	if (!DataAnimalManager::Instance()->IsExistItem(uid, animalud))
	{
		error_log("animal not exist. uid=%u,animalud=%u", uid, animalud);
		throw runtime_error("animal_not_exist");
	}

	DataAnimal & animal = DataAnimalManager::Instance()->GetData(uid, animalud);

	//判断动物是否处于饥饿状态
	if (animal.status != status_hungry)
	{
		error_log("animal not hungry. uid=%u,animalud=%u", uid, animalud);
		throw runtime_error("animal_not_hungry");
	}

	//获取饲料数据
	if (!DataItemManager::Instance()->IsExistItem(uid, fodderud))
	{
		error_log("fodder not exist. uid=%u,fodderud=%u", uid, fodderud);
		throw runtime_error("fodder_not_exist");
	}

	DataItem & props = DataItemManager::Instance()->GetData(uid, fodderud);

	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::AnimalLine & animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animal.animal_id);

	if (props.props_id != animalcfg.fodder())
	{
		error_log("fodder not match. uid=%u,animalud=%u,fodderud=%u", uid, animalud, fodderud);
		throw runtime_error("fodder_not_match");
	}

	//扣除饲料消耗
	CommonGiftConfig::CommonModifyItem costcfg;
	CommonGiftConfig::PropsItem* propscfgmsg = costcfg.add_props();
	propscfgmsg->set_id(props.props_id);
	propscfgmsg->set_count(-1);

	DataCommon::CommonItemsCPP cost;
	//处理消耗
	LogicUserManager::Instance()->CommonProcess(uid, costcfg, "AutoAnimalFeed", &cost);

	//获取动物产品的等待时间
	unsigned productid = animalcfg.product().props(0u).id();
	//根据作物配置，获取等待时长
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productid);

	animal.status = status_growup;

	DataAnimalManager::Instance()->UpdateItem(animal);

	return 0;
}
int LogicProductLineManager::Process(unsigned uid, ProtoProduce::ObtainProductReq* req, ProtoProduce::ObtainProductResp* resp)
{
	unsigned animalud = req->animalud();

	Obtain(uid, animalud, resp);

	return 0;
}

int LogicProductLineManager::Obtain(unsigned uid, unsigned animalud, ProtoProduce::ObtainProductResp * resp)
{
	//判断该动物是否存在
	if (!DataAnimalManager::Instance()->IsExistItem(uid, animalud))
	{
		error_log("animal not exist. uid=%u,animalud=%u", uid, animalud);
		throw runtime_error("animal_not_exist");
	}

	DataAnimal & animal = DataAnimalManager::Instance()->GetData(uid, animalud);

	//判断动物是否处于饱腹
	if (animal.status != status_full)
	{
		error_log("animal not full. uid=%u,animalud=%u", uid, animalud);
		throw runtime_error("animal_not_full");
	}

	//获取货仓的已用空间
	int restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse);

	//获取动物生产线的配置，判断传递的饲料是否是该动物需要的饲料
	const ConfigProductLine::AnimalLine & animalcfg = ProductlineCfgWrap().GetAnimallineCfg(animal.animal_id);
	int count = 0;
	unsigned id = 0, num = 0;
	CommonGiftConfig::CommonModifyItem reward;

	uint32_t propsId = 0;
	for(int i = 0; i < animalcfg.product().props_size(); ++i)
	{
		id = animalcfg.product().props(i).id();
		num = animalcfg.product().props(i).count();

		propsId = id;

		//获取经验奖励
		GetExpReward(id, num, reward);
		count += num;
	}

	if (restspace < count)
	{
		//满仓
		resp->set_isfull(true);
		return 0;
	}

	resp->set_isfull(false);

	//收获动物的产品
	LogicUserManager::Instance()->CommonProcess(uid, animalcfg.product(), "AnimalObtain", resp->mutable_commons());

	//额外奖励，暂时只处理了经验
	LogicUserManager::Instance()->CommonProcess(uid, reward, "AnimalObtain", resp->mutable_commons());

	//将收获的农产品添加到任务中
	for(int i = 0; i < animalcfg.product().props_size(); ++i)
	{
		id = animalcfg.product().props(i).id();
		num = animalcfg.product().props(i).count();

		LogicTaskManager::Instance()->AddTaskData(uid,task_of_harvest_cropsproduct,num,id);
		LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_cropsproduct, num, id);

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
			LogicXsgReportManager::Instance()->XSGGetHaverstReport(uid,3,id,num);
	}

	//产品已收获
	animal.Obtain();

	DataAnimalManager::Instance()->UpdateItem(animal);

	ProtoProduce::AnimalCPP* pAnimal = resp->mutable_animal();
	animal.SetMessage(pAnimal);
	if(animal.produce_type == PRODUCE_TYPE_KEEPER)
	{
		pAnimal->set_keeper(animal.produce_type);
	}

	//处理收获后的随机材料奖励
	if(!RandomPropsAfterHarvest(uid, count))
	{
		if(LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_warehouse) > 0)
		{
			LogicUserManager::Instance()->OnWatchAdsReward(uid, propsId);
		}
	}
	LogicKeeperManager::Instance()->OnAddAnimalHungry(uid);
	return 0;
}
int LogicProductLineManager::OnKeeperSuspendEquip(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	ItemCfgWrap itemcfgwrap;
	const string reason = "AutoProductLineFinish";
	vector<unsigned> equips;
	DataProduceequipManager::Instance()->GetIndexs(uid, equips);
	for(int i = 0; i < equips.size(); ++i)
	{
		DataProduceequip & equip = DataProduceequipManager::Instance()->GetDataByIndex(equips[i]);
		int productid = equip.GetPosValue(equip.queuedata, 0);
		if(equip.status == status_suspend && !equip.IsShelfQueueFull())
		{
			if(productid == 0)
			{
				equip.status == status_free;
			}
			else if(LogicProductLineManager::Instance()->CheckStorage(equip.uid, equip.id, productid))
			{
				ProtoProduce::FetchProductResp * resp = new ProtoProduce::FetchProductResp;
				ProtoProduce::ProduceEquipCPP* pEquip = resp->mutable_equipment();
				equip.PopArray(equip.queuedata, 0);

				ProductlineCfgWrap productlinecfg;
				//产品在产品列表中的位置

				uint32_t buildId = LogicProductLineManager::Instance()->GetBuildId(equip.uid, equip.id);
				unsigned index = productlinecfg.GetEquipProductIndex(buildId, productid);
				const ConfigProductLine::EquipLine & equipcfg = productlinecfg.GetEquiplineCfg(buildId);
				const CommonGiftConfig::CommonModifyItem& product = equipcfg.product_list(index);
				LogicProductLineManager::Instance()->AddFetchReward(equip.uid, product, reason, resp->mutable_commons());

				resp->set_isfull(false);
				pEquip->set_keeper(equip.produce_type);
				//状态设置为空闲,结束时间重置为0
				equip.status = status_free;
				equip.finish_time = 0;
				equip.produce_type = PRODUCE_TYPE_MAN;
				LogicProductLineManager::Instance()->ProduceEquipNextMove(equip);
				equip.SetMessage(pEquip);
				LogicKeeperManager::Instance()->OnAddProductLine(uid, equip.id);
				LMI->Instance()->sendMsg(uid, resp);
				/*
				// 加入生产
				const ConfigItem::PropItem & propscfg = itemcfgwrap.GetPropsItem(productid);
				uint32_t taskId = 0;
				if(KeeperCfgWrap().GetProductTaskId(productid, taskId)
						&& LogicKeeperManager::Instance()->CheckKeeperTaskNeed(uid, taskId)
						&& userwrap.CheckResBeforeCost(uid, propscfg.material()))
				{
					ProtoPush::PushBuildingsCPP * pResp = new ProtoPush::PushBuildingsCPP;
					ProtoProduce::ProduceEquipCPP* ppEquip = pResp->add_equipments();
					equip.SetMessage(ppEquip);
					LogicUserManager::Instance()->CommonProcess(equip.uid, propscfg.material(), reason, pResp->mutable_commons());
					ppEquip->set_keeper(equip.produce_type);

					LMI->Instance()->sendMsg(uid, pResp);
				}
				*/
			}
			DataProduceequipManager::Instance()->UpdateItem(equip);
		}
	}
	return 0;
}
int LogicProductLineManager::Process(unsigned uid, ProtoProduce::ReapFruitReq* req, ProtoProduce::ReapFruitResp* resp)
{
	unsigned treeud = req->treeud();

	ReapFruit(uid, treeud, resp);

	return 0;
}

int LogicProductLineManager::ReapFruit(unsigned uid, unsigned treeud, ProtoProduce::ReapFruitResp * resp)
{
	//判断果树是否存在
	if (!DataFruitManager::Instance()->IsExistItem(uid, treeud))
	{
		error_log("fruit tree not exist. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruit_not_exist");
	}

	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid, treeud);

	//判断果树状态是否可收获
	if (fruit.status != status_harvest)
	{
		error_log("fruit tree can not harvest. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruittree_cannot_harvest");
	}

	//是否还有剩余果子
	if (fruit.fruit_left_num == 0)
	{
		error_log("fruit tree is empty. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruittree_empty");
	}

	//有剩余果子，且可收获，则判断粮仓是否能放下
	//获取货仓的已用空间
	int restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_granary);

	if (0 == restspace)
	{
		error_log("storage is full. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("storage_is_full");
	}

	//仓库可容纳，则存放数据
	const ConfigProductLine::FruitLine & fruitcfg = ProductlineCfgWrap().GetFruitlineCfg(fruit.treeid);
	unsigned propsid = fruitcfg.stage_product(fruit.stage - 1).props(0u).id();

	CommonGiftConfig::CommonModifyItem  addfruitproduct;
	CommonGiftConfig::PropsItem* itemcfg = addfruitproduct.add_props();

	itemcfg->set_id(propsid);

	//一次收获一个
	unsigned num = 1;
	itemcfg->set_count(num);

	CommonGiftConfig::CommonModifyItem reward;

	//获取经验奖励
	GetExpReward(propsid, num, reward);

	//收获动物的产品
	LogicUserManager::Instance()->CommonProcess(uid, addfruitproduct, "FruitHarvest", resp->mutable_commons());

	//额外奖励，暂时只处理了经验
	LogicUserManager::Instance()->CommonProcess(uid, reward, "FruitHarvest", resp->mutable_commons());

	//将收获到的果实添加到任务列表中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_harvest_fuit, num, propsid);

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
		LogicXsgReportManager::Instance()->XSGGetHaverstReport(uid,2,propsid,num);

	//更新果树剩余果子树
	fruit.fruit_left_num -= 1;

	if (0 == fruit.fruit_left_num)
	{
		//果子被摘完了，判断接下来果树应该处的状态，这个取决于果树的阶段
		if (fruit.stage == fruitcfg.stage_product_size() || fruit.stage == fruitcfg.stage_product_size() - 1)
		{
			//最后一个阶段，或者倒数第二个阶段，果树都应该处于枯萎状态，阶段不变
			//fruit.status = status_withered;  //枯萎
			LogicProductLineManager::Instance()->SetFruitStatus(fruit, status_withered);
			fruit.harvest_time = 0;
		}
		else
		{
			//其余阶段，果树重新进入生长状态
			FruitGrowUP(fruit);
		}
	}

	//更新果树
	DataFruitManager::Instance()->UpdateItem(fruit);
	fruit.SetMessage(resp->mutable_fruit());

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::SeekHelpReq* req, ProtoProduce::SeekHelpResp* resp)
{
	unsigned treeud = req->treeud();

	SeekHelp(uid, treeud, resp);

	return 0;
}

int LogicProductLineManager::SeekHelp(unsigned uid, unsigned treeud, ProtoProduce::SeekHelpResp * resp)
{
	//只能对处于枯萎状态，且是倒数第二阶段的果树进行求助操作
	//判断果树是否存在
	if (!DataFruitManager::Instance()->IsExistItem(uid, treeud))
	{
		error_log("fruit tree not exist. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruit_not_exist");
	}

	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid, treeud);

	//判断果树状态是否处于枯萎状态
	if (fruit.status != status_withered)
	{
		error_log("fruit tree not withered. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruittree_not_withered");
	}

	//获取果树生产线的配置
	const ConfigProductLine::FruitLine & fruitcfg = ProductlineCfgWrap().GetFruitlineCfg(fruit.treeid);

	if (fruit.stage != fruitcfg.stage_product_size() - 1)
	{
		error_log("fruit tree stage error. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruittree_stage_error");
	}

	//设置求助状态
	//fruit.status = status_seek_help;
	LogicProductLineManager::Instance()->SetFruitStatus(fruit, status_seek_help);
	DataFruitManager::Instance()->UpdateItem(fruit);

	fruit.SetMessage(resp->mutable_fruit());

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::CutFruitTreeReq* req, ProtoProduce::CutFruitTreeResp* resp)
{
	unsigned treeud = req->treeud();

	CutTree(uid, treeud, resp);

	return 0;
}

int LogicProductLineManager::CutTree(unsigned uid, unsigned treeud, ProtoProduce::CutFruitTreeResp * resp)
{
	//只能对处于枯萎状态，且是倒数第二阶段的果树进行求助操作
	//判断果树是否存在
	if (!DataFruitManager::Instance()->IsExistItem(uid, treeud))
	{
		error_log("fruit tree not exist. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruit_not_exist");
	}

	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid, treeud);

	//果树状态处于枯萎的，或者援助中的树可以被砍
	if (fruit.status != status_withered && fruit.status != status_seek_help)
	{
		error_log("fruit tree not withered. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruittree_not_withered");
	}

	//获取果树建筑配置
	const ConfigBuilding::FruitTree & treecfg = BuildCfgWrap().GetFruitTreeCfgById(fruit.treeid);

	//扣除资源
	LogicUserManager::Instance()->CommonProcess(uid, treecfg.cut_tree_props(), "CutTree", resp->mutable_commons());

	//删除果树建筑
	DataBuildingMgr::Instance()->DelBuild(uid, treeud);

	//删除果树
	DataFruitManager::Instance()->DelFruitTree(fruit);

	resp->set_destroyud(treeud);

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::OfferHelpReq* req)
{
	unsigned othuid = req->othuid();
	unsigned othtreeud = req->treeud();
	if(CMI->IsNeedConnectByUID(othuid))
	{
		DBCUserBaseWrap userwrap(uid);
		unsigned aid = userwrap.Obj().alliance_id;

		ProtoProduce::CSOfferHelpReq* m = new ProtoProduce::CSOfferHelpReq;
		m->set_othuid(othuid);
		m->set_myuid(uid);
		m->set_treeud(othtreeud);
		m->set_allianceid(aid);
		int ret = BMI->BattleConnectNoReplyByUID(othuid, m);
		AddHelpTreeDyInfoOverServer(uid,othuid);	//跨服救树增加动态消息
		return ret;
	}

	ProtoProduce::OfferHelpResp* resp = new ProtoProduce::OfferHelpResp;
	try{
		OfferHelp(uid, othuid, othtreeud, resp);
		AddHelpTreeDyInfo(uid,othuid);			//同服救树增加动态消息
	}catch(const std::exception &e)
	{
		delete resp;
		error_log("failed:%s",e.what());
		return R_ERROR;
	}
	return LMI->sendMsg(uid,resp)?0:R_ERROR;
}

int LogicProductLineManager::Process(ProtoProduce::CSOfferHelpReq* req)
{
	unsigned myuid = req->othuid();
	unsigned treeud = req->treeud();
	unsigned othuid = req->myuid();

	//提供帮助，先加载对方的数据
	OffUserSaveControl offuserctl(myuid);

	//判断果树是否存在
	if (!DataFruitManager::Instance()->IsExistItem(myuid, treeud))
	{
		error_log("fruit tree not exist. uid=%u,treeud=%u", myuid, treeud);
		throw runtime_error("fruit_not_exist");
	}

	DataFruit & fruit = DataFruitManager::Instance()->GetData(myuid, treeud);

	//判断果树状态是否请求帮助
	if (fruit.status != status_seek_help)
	{
		error_log("fruit tree not in seek help. uid=%u,treeud=%u", myuid, treeud);
		throw runtime_error("fruittree_not_seeking_help");
	}

	//更新果树状态
	//fruit.status = status_already_help;
	LogicProductLineManager::Instance()->SetFruitStatus(fruit, status_already_help);
	fruit.aid_uid = othuid;

	DataFruitManager::Instance()->UpdateItem(fruit);

	//添加援助记录
	DataAidRecordManager::Instance()->AddAidRecord(myuid, othuid, Time::GetGlobalTime());

	//判断用户是否在商会上，更新商会上的帮助数目
	unsigned aid = req->allianceid();
	LogicUserManager::Instance()->UpdateAllianceHelpTimes(othuid,aid);

	ProtoProduce::CSOfferHelpResp* resp = new ProtoProduce::CSOfferHelpResp;
	fruit.SetMessage(resp->mutable_othfruit());
	resp->set_myuid(req->myuid());
	resp->set_othuid(req->othuid());
	return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);

}

int LogicProductLineManager::Process(ProtoProduce::CSOfferHelpResp* req)
{
	unsigned myuid = req->myuid();

	ProtoProduce::OfferHelpResp *resp = new ProtoProduce::OfferHelpResp;
	try{
		resp->mutable_othfruit()->MergeFrom(*(req->mutable_othfruit()));
		resp->mutable_othfruit()->set_helpuid(myuid);

		resp->set_othuid(req->othuid());

		//发送果树援助奖励
		const ConfigProductLine::ProductLine & productcfg = ProductlineCfgWrap().GetProductLineCfg();
		LogicUserManager::Instance()->CommonProcess(myuid, productcfg.aid_reward(), "AidFruitReward", resp->mutable_commons());

		//更新用户的援助次数
		unsigned aid = 0;
		LogicUserManager::Instance()->UpdateUserHelpTimes(myuid,aid);

		//推送果树信息
		ProtoProduce::PushFruitInfo *msg = new ProtoProduce::PushFruitInfo;
		msg->mutable_fruit()->MergeFrom(*(req->mutable_othfruit()));
		msg->mutable_fruit()->set_helpuid(myuid);
		msg->set_uid(req->othuid());
		LMI->sendMsg(req->othuid(),msg);

		//帮助救活果树添加至任务链表中
		LogicTaskManager::Instance()->AddTaskData(myuid,task_of_alive_fuit,1);
		LogicAllianceManager::Instance()->AddRaceOrderProgress(myuid, alliance_race_task_of_help, 1);

	}catch(const std::exception &e)
	{
		delete resp;
		error_log("failed:%s",e.what());
		return R_ERROR;
	}
	return LMI->sendMsg(myuid,resp,false)?0:R_ERROR;

}


int LogicProductLineManager::OfferHelp(unsigned uid, unsigned othuid, unsigned treeud, ProtoProduce::OfferHelpResp * resp)
{
	//提供帮助，先加载对方的数据
	OffUserSaveControl offuserctl(othuid);

	//判断果树是否存在
	if (!DataFruitManager::Instance()->IsExistItem(othuid, treeud))
	{
		error_log("fruit tree not exist. uid=%u,treeud=%u", othuid, treeud);
		throw runtime_error("fruit_not_exist");
	}

	DataFruit & fruit = DataFruitManager::Instance()->GetData(othuid, treeud);

	//判断果树状态是否请求帮助
	if (fruit.status != status_seek_help)
	{
		error_log("fruit tree not in seek help. uid=%u,treeud=%u", othuid, treeud);
		throw runtime_error("fruittree_not_seeking_help");
	}

	//更新果树状态
//	fruit.status = status_already_help;
	LogicProductLineManager::Instance()->SetFruitStatus(fruit, status_already_help);
	fruit.aid_uid = uid;

	DataFruitManager::Instance()->UpdateItem(fruit);

	fruit.SetMessage(resp->mutable_othfruit());

	resp->mutable_othfruit()->set_helpuid(uid);

	//发送果树援助奖励
	const ConfigProductLine::ProductLine & productcfg = ProductlineCfgWrap().GetProductLineCfg();
	LogicUserManager::Instance()->CommonProcess(uid, productcfg.aid_reward(), "AidFruitReward", resp->mutable_commons());

	resp->set_othuid(othuid);


	//添加援助记录
	DataAidRecordManager::Instance()->AddAidRecord(othuid, uid, Time::GetGlobalTime());

	//更新用户的援助次数
	LogicUserManager::Instance()->UpdateHelpTimes(uid);

	//推送果树信息
	ProtoProduce::PushFruitInfo *msg = new ProtoProduce::PushFruitInfo;
	fruit.SetMessage(msg->mutable_fruit());
	msg->mutable_fruit()->set_helpuid(uid);
	msg->set_uid(othuid);
	LMI->sendMsg(othuid,msg);

	//帮助救活果树添加至任务链表中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_alive_fuit,1);
	LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_help, 1);

	return 0;
}

int LogicProductLineManager::Process(unsigned uid, ProtoProduce::ConfirmHelpReq* req, ProtoProduce::ConfirmHelpResp* resp)
{
	unsigned treeud = req->treeud();

	ConfirmHelp(uid, treeud, resp);

	return 0;
}

int LogicProductLineManager::ConfirmHelp(unsigned uid, unsigned treeud, ProtoProduce::ConfirmHelpResp * resp)
{
	//确认帮助，让果树进入成长状态
	//判断果树是否存在
	if (!DataFruitManager::Instance()->IsExistItem(uid, treeud))
	{
		error_log("fruit tree not exist. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruit_not_exist");
	}

	DataFruit & fruit = DataFruitManager::Instance()->GetData(uid, treeud);

	//判断果树状态是否请求帮助
	if (fruit.status != status_already_help)
	{
		error_log("fruit tree not in already help. uid=%u,treeud=%u", uid, treeud);
		throw runtime_error("fruittree_help_not_already");
	}

	fruit.aid_uid = 0;  //删除援助的人

	//果树进入成长状态
	FruitGrowUP(fruit);

	fruit.SetMessage(resp->mutable_fruit());

	return 0;
}

int LogicProductLineManager::FruitGrowUP(DataFruit & fruit)
{
	//获取配置
	const ConfigProductLine::FruitLine & fruitcfg = ProductlineCfgWrap().GetFruitlineCfg(fruit.treeid);

	//获取生产的产品id
	unsigned productid = fruitcfg.stage_product(0u).props(0u).id();
	//根据作物配置，获取加速价格
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(productid);
	unsigned endts = Time::GetGlobalTime() + propscfg.time_gain();

	//果树进入下一个阶段
	fruit.stage += 1;
	fruit.harvest_time = endts;
//	fruit.status = status_growup;
	LogicProductLineManager::Instance()->SetFruitStatus(fruit, status_growup);

	DataFruitManager::Instance()->UpdateItem(fruit);

	//加入生产队列
	LogicQueueManager::Instance()->JoinRoutine<DataFruitRoutine>(fruit.uid, endts, routine_type_build, fruit.id);

	return 0;
}

int LogicProductLineManager::GetExpReward(unsigned productid, unsigned count, CommonGiftConfig::CommonModifyItem &reward)
{
	ItemCfgWrap itemcfgwrap;
	//获取物品的奖励配置
	const ConfigItem::PropItem &  propscfg = itemcfgwrap.GetPropsItem(productid);

	unsigned oldexp = reward.based().exp();

	//只处理经验
	if (propscfg.extra_reward().has_based() &&
			propscfg.extra_reward().based().has_exp())
	{
		int exp = propscfg.extra_reward().based().exp();

		exp *= count;
		//经验
		reward.mutable_based()->set_exp(exp + oldexp);
	}

	return 0;
}

bool LogicProductLineManager::RandomPropsAfterHarvest(unsigned uid, unsigned count)
{
	//根据uid，获取当前已经收获的作物个数
	DBCUserBaseWrap userwrap(uid);
	userwrap.Obj().count += count;

	//根据用户等级，开启随机产生道具的机会需要的次数
	const ConfigProductLine::MaterailReward & materialcfg = ProductlineCfgWrap().GetMaterialCfg();

	int cond_count = materialcfg.init_count() + userwrap.Obj().level;  //条件数量

	//判断当前次数是否满足条件
	if (userwrap.Obj().count < cond_count)
	{
		//条件不满足，返回
		return false;
	}

	//次数满足要求，随机一个数，判断是否能产生材料
	int val = Math::GetRandomInt(100); //在0与100之间随机一个值

	if (val >= materialcfg.rate())
	{
		//不满足概率的条件，次数重置
		userwrap.Obj().count = 0;
		userwrap.Save();

		return false;
	}

	//按权重值从材料的数组中随机抽取一个产品
	std::vector<unsigned>weights;
	weights.clear();
	unsigned id = uid % 10; //取uid最后一位

	for(int i = 0; i < materialcfg.random_reward(id).id_size(); i++)
	{
		weights.push_back(materialcfg.random_reward(id).id(i).weight());
	}
	int index = 0;
	LogicCommonUtil::TurnLuckTable(weights,weights.size(),index);

	//发放奖励
	ProtoProduce::PushRandomRewardReq * randommsg = new ProtoProduce::PushRandomRewardReq;
	LogicUserManager::Instance()->CommonProcess(uid, materialcfg.random_reward(id).id(index).reward(), "HarvestRandom", randommsg->mutable_commons());

	//收获次数重置
	userwrap.Obj().count = 0;
	userwrap.Save();

	//推送奖励
	randommsg->set_propsid(materialcfg.random_reward(id).id(index).reward().props(0u).id());

	LogicManager::Instance()->sendMsg(uid, randommsg);

	return true;
}

bool LogicProductLineManager::AddHelpTreeDyInfo(unsigned uid,unsigned other_uid)
{
	//uid:援助者,other_uid:被援助者,给好友提供帮助会让被访问者增加一条动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_TREE,pattach))
	{
		return true;
	}
	return false;
}

bool LogicProductLineManager::AddHelpTreeDyInfoOverServer(unsigned uid,unsigned other_uid)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_TREE);
	msg->set_productid(0);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
