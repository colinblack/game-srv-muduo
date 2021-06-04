#include "LogicBuildingManager.h"
#include "ServerInc.h"

void DataBuildRoutine::CheckUd(unsigned buildud)
{
	//先调用基类的方法
	DataRoutineBase::CheckUd(buildud);

	if (!DataBuildingMgr::Instance()->IsExistItem(uid_, buildud))
	{
		error_log("build is not exist. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑数据
	DataBuildings & build = DataBuildingMgr::Instance()->GetData(uid_, buildud);

	if(0 == build.done_time)
	{
		error_log("build not upgrading. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("build_not_upgrading");
	}
}

void DataBuildRoutine::GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &rtype)
{
	unsigned nowts = Time::GetGlobalTime();
	diffts = endts_ > nowts ? endts_ - nowts : 0;
	//获取建筑数据
	DataBuildings & build = DataBuildingMgr::Instance()->GetData(uid_, buildud);
	unsigned build_id = build.build_id;

	int per = 1;  //1钻减去的秒数
	BuildCfgWrap buildcfgwrap;
	int type = buildcfgwrap.GetBuildType(build_id);

	if (build_type_animal_residence == type)
	{
		//动物住所
		const ConfigBuilding::AnimalResidence & residencecfg = buildcfgwrap.GetAnimalResidenceCfgById(build_id);
		per = residencecfg.speed_price();
	}
	else if (build_type_produce_equipment == type)
	{
		//生产设备
		const ConfigBuilding::ProduceEquipment & producecfg = buildcfgwrap.GetProduceCfgById(build_id);
		per = producecfg.speed_price();
	}
	else
	{
		error_log("wrong type. uid=%u,buildud=%u", uid_, buildud);
		throw runtime_error("wrong_build_type");
	}

	if (0 == per)
	{
		error_log("speed price config error. uid=%u,buildid=%u", uid_, buildud);
		throw runtime_error("config_error");
	}

	cash = ceil(static_cast<double>(diffts)/per);
}

void DataBuildRoutine::SingleRoutineEnd(unsigned buildud, ProtoPush::PushBuildingsCPP * msg)
{
	//获取建筑数据
	DataBuildings & build = DataBuildingMgr::Instance()->GetData(uid_, buildud);

	//生产时间重置
	build.ResetTime();
	DataBuildingMgr::Instance()->UpdateItem(build);

	//非立即升级建筑的升级完成系列
	//处理建筑解锁后的相关生产线
	unsigned type = BuildCfgWrap().GetBuildType(build.build_id);

	build.SetMessage(msg->add_buildings());

	//结束队列
//	LogicQueueManager::Instance()->FinishQueue(uid_, buildud, routine_type_build);
}

LogicBuildManager::LogicBuildManager()
{
	userActId = e_Activity_UserData_1;
}

int LogicBuildManager::OnInit()
{
	bool issuccess = ParseManager::getInstance()->Init();

	if (!issuccess)
	{
		return R_ERROR;
	}

	return 0;
}

int LogicBuildManager::NewUser(unsigned uid)
{
	DataBuildingMgr::Instance()->Init(uid);

	//读取新手文档中的配置，进行新建筑的建造
	const UserCfg::User& userCfg = UserCfgWrap().User();
	unsigned build_id = 0;
	unsigned xpos = 0, ypos = 0;

	for(int i = 0; i < userCfg.builds_size(); ++i)
	{
		build_id = userCfg.builds(i).buildid();
		xpos = userCfg.builds(i).pos(0u);
		ypos = userCfg.builds(i).pos(1u);

		try
		{
			DataBuildings & databuild = DataBuildingMgr::Instance()->AddNewBuilding(uid, build_id);
			databuild.position = xpos*grid_ypos_len + ypos;
			databuild.direct = direct_right;
			int type = BuildCfgWrap().GetBuildType(build_id);
			if (build_type_produce_equipment == type)
			{
				if (!DataEquipmentStarManager::Instance()->IsExistItem(uid, build_id))
					DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, build_id);
			}
			databuild.level = 1;
			DataBuildingMgr::Instance()->UpdateItem(databuild);
			ProtoProduce::ProduceProductCPP msg;
			LogicProductLineManager::Instance()->ProduceAfterBuild(uid, databuild.id, type, &msg);
			if (build_type_corpland == type)
			{
				DataCropland & crop = DataCroplandManager::Instance()->GetData(uid, databuild.id);
				crop.plant = 60001;
				crop.status = status_harvest;
				DataCroplandManager::Instance()->UpdateItem(crop);
			}
			LogicTaskManager::Instance()->AddTaskData(uid, task_of_build_building, 1, build_id);
			if(build_type_produce_equipment == type)
				LogicTaskManager::Instance()->AddTaskData(uid, task_of_product_device_cnt, 1);
		}
		catch(runtime_error &e)
		{
			;
		}
	}

	return 0;
}

int LogicBuildManager::CheckLogin(unsigned uid)
{
	DataBuildingMgr::Instance()->Init(uid);

	vector<unsigned> builds;
	//将正在建筑的建筑加入定时任务队列
	DataBuildingMgr::Instance()->GetIndexs(uid, builds);
	map<unsigned, set<unsigned> > tobuild;  //endts -> uds

	for(int i = 0; i < builds.size(); ++i)
	{
		DataBuildings & build = DataBuildingMgr::Instance()->GetDataByIndex(builds[i]);

		if (build.done_time > 0)
		{
			tobuild[build.done_time].insert(build.id);
		}
	}

	//遍历map
	for(map<unsigned, set<unsigned> >::iterator viter = tobuild.begin(); viter != tobuild.end(); ++viter)
	{
		LogicQueueManager::Instance()->JoinRoutine<DataBuildRoutine>(uid, viter->first, routine_type_build, viter->second);
	}

	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::BuildReq* req, ProtoBuilding::BuildResp* resp)
{
	unsigned build_id = req->buildid();
	unsigned xpos = req->pos(0u);
	unsigned ypos = req->pos(1u);
	unsigned direct = req->direct();

	debug_log("uid=%u,build_id=%u",uid,build_id);

	Build(uid, build_id, xpos, ypos, direct,resp);

	return 0;
}

int LogicBuildManager::Build(unsigned uid, unsigned build_id, unsigned xpos, unsigned ypos, unsigned direct,ProtoBuilding::BuildResp *resp)
{
	//判断总的建筑数目是否超出上限
	int allnum = DataBuildingMgr::Instance()->GetAllBuildNum(uid);

	if (allnum >= DB_BUILD_FULL)
	{
		error_log("all build num is max. uid=%u", uid);
		throw runtime_error("all_build_num_max");
	}

	//判断位置是否被占用
	int buildstatus[GRID_LENGTH] = {0};  //全地图各格子的位置
	SeteBuildGrid(uid, xpos, ypos, direct, build_id, buildstatus,true);

	//是否相交
	bool isintersect = IsIntersect(uid, 0, buildstatus);

	if (isintersect)
	{
		error_log("wrong position. uid=%u,build_id=%u,xpos=%u,ypos=%u", uid, build_id, xpos, ypos);
		throw runtime_error("wrong_position");
	}

	//不相交，则继续处理建造的相关事项
	unsigned wait_time = 0;

	//建筑前的检查以及扣除资源的消耗
	CheckAndCostBeforeBuild(uid, build_id, wait_time, resp);

	DataBuildings & databuild = DataBuildingMgr::Instance()->AddNewBuilding(uid, build_id);

	databuild.position = xpos*grid_ypos_len + ypos;
	databuild.direct = direct;

	int type = BuildCfgWrap().GetBuildType(build_id);

	//判断是否是生产设备
	if (build_type_produce_equipment == type)
	{
		//生产设备，判断是否存在该设备的星级，如果不存在，则新增
		if (!DataEquipmentStarManager::Instance()->IsExistItem(uid, build_id))
		{
			//不存在，新增
			DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, build_id);

			datastar.SetMessage(resp->mutable_equipmentstar());
		}
	}

	if (0 == wait_time)
	{
		//立即完成类型
		databuild.level = 1;

		DataBuildingMgr::Instance()->UpdateItem(databuild);
		databuild.SetMessage(resp->mutable_building());

		//建筑已建造好，处理建造的经验奖励
		BuildExpReward(uid, databuild.build_id, resp->mutable_commons());

		//处理建筑升级后的相关生产线
		//处理建筑建造完毕之后的操作
		LogicProductLineManager::Instance()->ProduceAfterBuild(uid, databuild.id, type, resp->mutable_product());

		//将建筑建造添加到玩家任务系统中
		LogicTaskManager::Instance()->AddTaskData(uid, task_of_build_building, 1, build_id);
		LogicMissionManager::Instance()->AddMission(uid,mission_of_have_build,1,build_id);

		if(build_type_produce_equipment == type)
			LogicTaskManager::Instance()->AddTaskData(uid, task_of_product_device_cnt, 1);

		return 0;
	}

	unsigned endts = Time::GetGlobalTime() + wait_time;
	databuild.done_time = endts;

	//非立即完成系列，则加入到定时任务列表
	LogicQueueManager::Instance()->JoinRoutine<DataBuildRoutine>(uid, endts, routine_type_build, databuild.id);

	//将建筑建造添加到玩家任务系统中
	LogicTaskManager::Instance()->AddTaskData(uid, task_of_build_building, 1, build_id);
	LogicMissionManager::Instance()->AddMission(uid,mission_of_have_build,1,build_id);

	if(build_type_produce_equipment == type)
		LogicTaskManager::Instance()->AddTaskData(uid, task_of_product_device_cnt, 1);

	DataBuildingMgr::Instance()->UpdateItem(databuild);
	databuild.SetMessage(resp->mutable_building());

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
		LogicXsgReportManager::Instance()->XSGBuildReport(uid,type,databuild.id,1);

	return 0;
}

int LogicBuildManager::CheckAndCostBeforeBuild(unsigned uid, unsigned build_id, unsigned & wait_time, ProtoBuilding::BuildResp *resp)
{
	//先判断是否能被建造
	//包括数量，消耗的限制
	BuildCfgWrap buildcfgwrap;

	int num = DataBuildingMgr::Instance()->GetBuildNum(uid, build_id);
	int type = buildcfgwrap.GetBuildType(build_id);

	char build[20] = {0};
	sprintf(build, "%u", build_id);
	string reason("Build_");
	reason += string(build);
	DBCUserBaseWrap userwrap(uid);
	wait_time = 0;

	switch(type)
	{
		case build_type_corpland:
			//消耗
			{
				const ConfigBuilding::CropLand & cropcfg = buildcfgwrap.GetCropLandCfg();
				CostBeforeBuild<LogicUserManager>(uid, cropcfg, reason, num, userwrap.Obj().level, wait_time, resp);
			}
			break;
		case build_type_animal_residence :
		{
			//动物住所
			const ConfigBuilding::AnimalResidence & residencecfg = buildcfgwrap.GetAnimalResidenceCfgById(build_id);
			//消耗
			CostBeforeBuild<LogicUserManager>(uid, residencecfg, reason, num, userwrap.Obj().level, wait_time, resp);
		}
			break;
		case build_type_produce_equipment :
		{
			//生产设备
			const ConfigBuilding::ProduceEquipment & producecfg = buildcfgwrap.GetProduceCfgById(build_id);
			//消耗
			CostBeforeBuild<LogicUserManager>(uid, producecfg, reason, num, userwrap.Obj().level, wait_time, resp);
		}
			break;
		case build_type_fruit_tree :
		{
			//果树
			const ConfigBuilding::FruitTree & fruitcfg = buildcfgwrap.GetFruitTreeCfgById(build_id);

			//判断是否解锁
			if (userwrap.Obj().level < fruitcfg.need_level(0u))
			{
				error_log("level not enough. level=%u,need=%u", userwrap.Obj().level, fruitcfg.need_level(0u));
				throw runtime_error("level_not_enough");
			}

			//不检查数量，直接进行消耗
			LogicUserManager::Instance()->CommonProcess(uid, fruitcfg.need_cost(0u), reason, resp->mutable_commons());
		}
			break;
		case build_type_decorate:
		{
			//装饰物消耗
			const ConfigBuilding::Decoration & decoratecfg = buildcfgwrap.GetDecorationCfgById(build_id);

			//判断是否解锁
			if (userwrap.Obj().level < decoratecfg.need_level(0u))
			{
				error_log("level not enough. level=%u,need=%u", userwrap.Obj().level, decoratecfg.need_level(0u));
				throw runtime_error("level_not_enough");
			}
			//判断当前数目是否超过要求
			if (num >= decoratecfg.gain_num(0u))
			{
				error_log("build num already max. uid=%u,build_id=%u", uid, decoratecfg.id());
				throw runtime_error("build_num_max");
			}

			const CommonGiftConfig::CommonModifyItem& need_cost = decoratecfg.need_cost(0u);
			CommonGiftConfig::CommonModifyItem cfg;

			if(need_cost.has_based())
			{
				const CommonGiftConfig::BaseItem& based = need_cost.based();
				if(based.has_cash())
				{
					int basecost = -num* decoratecfg.factor();
					cfg.mutable_based()->set_cash(based.cash() + basecost);
				}
				if(need_cost.based().has_coin())
				{
					int basecost = -num* decoratecfg.factor();
					cfg.mutable_based()->set_coin(based.coin() + basecost);
				}
			}

			if(need_cost.props_size() > 0)
			{
				for(int i = 0; i < need_cost.props_size(); i++)
				{
					CommonGiftConfig::PropsItem * propsitem = cfg.add_props();
					propsitem->set_id(need_cost.props(i).id());
					propsitem->set_count(need_cost.props(i).count());
				}
			}

			//消耗公式：based + num*factor
			LogicUserManager::Instance()->CommonProcess(uid, cfg, reason, resp->mutable_commons());
		}
			break;
		case build_type_house:
		{
			if (num > 0)
			{
				error_log("build already exist. uid=%u,build_id=%u", uid, build_id);
				throw runtime_error("build_already_exist");
			}

			resp->mutable_commons();
		}
			break;
		case build_type_storage:
			//仓库，暂时没有消耗.不允许多个
			if (num > 0)
			{
				error_log("build already exist. uid=%u,build_id=%u", uid, build_id);
				throw runtime_error("build_already_exist");
			}

			resp->mutable_commons();

			break;
		default:
			error_log("wrong build type. uid=%u,build_id=%u", uid, build_id);
			throw runtime_error("wrong_build_type");
	}

	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::UnveilBuildReq* req, ProtoBuilding::UnveilBuildResp* resp)
{
	unsigned ud = req->ud();

	UnVeil(uid, ud, resp);

	return 0;
}

int LogicBuildManager::UnVeil(unsigned uid, unsigned buildud, ProtoBuilding::UnveilBuildResp * resp)
{
	//判断建筑ud是否存在
	//根据ud，判断是否在建筑表中
	bool isexist = DataBuildingMgr::Instance()->IsExistItem(uid, buildud);

	if (!isexist)
	{
		error_log("build not exist. uid=%u, ud=%u", uid, buildud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑信息
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, buildud);

	//判断建筑是否建造完毕
	if (databuild.done_time > 0)
	{
		error_log("build not finish. uid=%u, ud=%u", uid, buildud);
		throw runtime_error("build_not_finish");
	}

	unsigned type = BuildCfgWrap().GetBuildType(databuild.build_id);

	//暂时只支持这两种建造时间非0的建筑的揭幕
	if (build_type_produce_equipment != type && build_type_animal_residence != type)
	{
		error_log("build type error. uid=%u, ud=%u", uid, buildud);
		throw runtime_error("wrong_build_type");
	}

	//判断设备是否已揭幕
	if (databuild.level == 1)
	{
		error_log("build already unveiled. uid=%u, ud=%u", uid, buildud);
		throw runtime_error("build_already_unveiled");
	}

	//生产设备经验奖励
	BuildExpReward(uid, databuild.build_id, resp->mutable_commons());

	//处理建筑建造完毕之后的操作
	LogicProductLineManager::Instance()->ProduceAfterBuild(uid, databuild.id, type, resp->mutable_product());

	//更新设备等级
	databuild.level = 1;
	DataBuildingMgr::Instance()->UpdateItem(databuild);

	databuild.SetMessage(resp->mutable_building());

	return 0;
}

int LogicBuildManager::GetAllProductBuildNum(unsigned uid)
{
	int build_num = 0;
	map<uint16_t, vector<uint16_t> >builds;
	DataBuildingMgr::Instance()->GetBuildInfo(uid,builds);

	map<uint16_t, vector<uint16_t> >::iterator it = builds.begin();
	for(; it != builds.end(); it++)
	{
		int type = BuildCfgWrap().GetBuildType(it->first);
		if(build_type_produce_equipment == type)
		{
			build_num ++;
		}
	}
	return build_num;
}

int LogicBuildManager::BuildExpReward(unsigned uid, unsigned build_id, DataCommon::CommonItemsCPP * msg)
{
	//获取建筑等级，只给指定类型的建筑以经验
	BuildCfgWrap buildcfgwrap;

	unsigned type = buildcfgwrap.GetBuildType(build_id);

	if (IsNoExpReward(type))
	{
		//建造时，无经验奖励的建筑类型
		return 0;
	}

	switch(type)
	{
		case build_type_corpland:
			//消耗
			{
				const ConfigBuilding::CropLand & cropcfg = buildcfgwrap.GetCropLandCfg();
				//经验奖励
				LogicUserManager::Instance()->CommonProcess(uid, cropcfg.build_reward(), "BuildCropland", msg);
			}
			break;
		case build_type_animal_residence :
			{
				//动物住所
				const ConfigBuilding::AnimalResidence & residencecfg = buildcfgwrap.GetAnimalResidenceCfgById(build_id);
				//经验奖励
				LogicUserManager::Instance()->CommonProcess(uid, residencecfg.build_reward(), "BuildResidence", msg);
			}
			break;
		case build_type_produce_equipment :
			{
				//生产设备
				const ConfigBuilding::ProduceEquipment & producecfg = buildcfgwrap.GetProduceCfgById(build_id);
				LogicUserManager::Instance()->CommonProcess(uid, producecfg.build_reward(), "BuildProduce", msg);
			}
			break;
		default:
			break;
	}

	return 0;
}

bool LogicBuildManager::IsNoExpReward(unsigned type)
{
	switch(type)
	{
		case build_type_corpland :
		case build_type_animal_residence :
		case build_type_produce_equipment :
			return false;
		case build_type_fruit_tree :
		case build_type_decorate :
		case build_type_storage :
		case build_type_house :
			return true;
	}
	return true;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::MoveReq* req, ProtoBuilding::MoveResp* resp)
{
	unsigned build_ud = req->ud();
	unsigned xpos = req->pos(0u);
	unsigned ypos = req->pos(1u);
	unsigned direct = req->direct();

	Move(uid, build_ud, direct,xpos, ypos, resp);

	return 0;
}

int LogicBuildManager::Move(unsigned uid, unsigned build_ud, unsigned direct,unsigned xpos, unsigned ypos, ProtoBuilding::MoveResp *resp)
{
	//根据ud，判断是否在建筑表中
	bool isexist = DataBuildingMgr::Instance()->IsExistItem(uid, build_ud);

	if (!isexist)
	{
		error_log("build not exist. uid=%u, ud=%u", uid, build_ud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑信息
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, build_ud);

	int buildstatus[GRID_LENGTH] = {0};  //当前建筑的地图状态
	SeteBuildGrid(uid, xpos, ypos, direct, databuild.build_id, buildstatus,true);

	//是否相交
	bool isintersect = IsIntersect(uid, build_ud, buildstatus);

	if (isintersect)
	{
		error_log("build will have intersection. uid=%u,build_ud=%u", uid, build_ud);
		throw runtime_error("build_intersect");
	}

	//不冲突，则修改坐标位置
	databuild.position = xpos*grid_ypos_len + ypos;
	databuild.direct = direct;

	DataBuildingMgr::Instance()->UpdateItem(databuild);

	resp->set_result(true);

	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::FlipReq* req, ProtoBuilding::FlipResp* resp)
{
	unsigned build_ud = req->ud();

	Flip(uid, build_ud, resp);

	return 0;
}

int LogicBuildManager::Flip(unsigned uid, unsigned build_ud, ProtoBuilding::FlipResp *resp)
{
	//根据ud，判断是否在建筑表中
	bool isexist = DataBuildingMgr::Instance()->IsExistItem(uid, build_ud);

	if (!isexist)
	{
		error_log("build not exist. uid=%u, ud=%u", uid, build_ud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑信息
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, build_ud);
	vector<unsigned> grids;

	uint8_t changed_direct = databuild.direct == direct_right? direct_down: direct_right;

	int buildstatus[GRID_LENGTH] = {0};  //全地图各格子的位置
	unsigned xpos = databuild.position/grid_ypos_len;
	unsigned ypos = databuild.position - xpos*grid_ypos_len;

	SeteBuildGrid(uid, xpos, ypos, changed_direct, databuild.build_id, buildstatus,true);

	//是否相交
	bool isintersect = IsIntersect(uid, build_ud, buildstatus);

	if (isintersect)
	{
		error_log("build will have intersection. uid=%u,build_ud=%u", uid, build_ud);
		throw runtime_error("build_intersect");
	}

	//不冲突，则修改方向即可
	databuild.direct = changed_direct;

	DataBuildingMgr::Instance()->UpdateItem(databuild);
	resp->set_result(true);

	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::BuildingUpReq *req, ProtoBuilding::BuildingUpResp* resp)
{
	unsigned build_ud = req->ud();
	//根据ud，判断是否在建筑表中
	bool isexist = DataBuildingMgr::Instance()->IsExistItem(uid, build_ud);
	if (!isexist)
	{
		error_log("build not exist. uid=%u, ud=%u", uid, build_ud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑信息
	DataBuildings & databuild = DataBuildingMgr::Instance()->GetData(uid, build_ud);
	unsigned build_id = databuild.build_id;

	//判定其是否为仓库
	bool isstorage = BuildCfgWrap().IsStorage(build_id);
	if (!isstorage)
	{
		error_log("build is not  storage. uid=%u, build_id=%u", uid, build_id);
		throw runtime_error("build is not  storage");
	}

	//根据建筑id获取相应的配置信息
	unsigned cur_level = databuild.level;
	const ConfigBuilding::StorageHouse &storage_house_cfg = BuildCfgWrap().GetStorageCfgById(build_id);
	unsigned max_level = storage_house_cfg.level_storage_size();
	if(cur_level >= max_level)
	{
		error_log("build level is biggest. cur_level=%u", cur_level);
		throw runtime_error("build level is biggest");
	}
	const CommonGiftConfig::CommonModifyItem &item_cfg = storage_house_cfg.need_cost(cur_level);

	CommonGiftConfig::CommonModifyItem common;
	int diff_count = 0;
	for(int i = 0; i < item_cfg.props_size(); i++)
	{
		unsigned propsid = item_cfg.props(i).id();
		int need_count = -item_cfg.props(i).count();
		int count = DataItemManager::Instance()->GetItemCount(uid,propsid);

		CommonGiftConfig::PropsItem *propsitem = common.add_props();
		propsitem->set_id(propsid);
		if(count < need_count)
		{
			propsitem->set_count(-count);
			diff_count += count - need_count;
		}
		else
		{
			propsitem->set_count(-need_count);
		}
	}
	if(diff_count > 2)
	{
		//材料差两个以上，不予升级
		error_log("diff_count=%d",diff_count);
		throw std::runtime_error("props_is_not_enough");
	}
	//处理升级消耗
	string reason = "storage_house_up";
	LogicUserManager::Instance()->CommonProcess(uid, common, reason, resp->mutable_commons());

	//增加建筑等级
	databuild.level = cur_level + 1;
	DataBuildingMgr::Instance()->UpdateItem(databuild);


	databuild.SetMessage(resp->mutable_build());
	LogicKeeperManager::Instance()->OnStorageChange(uid);

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
	{
		string use_itemid = "";
		string use_num = "";
		for(int i = 0; i < common.props_size(); i++)
		{
			int propsid = common.props(i).id();
			int count = -common.props(i).count();
			use_itemid.append(CTrans::UTOS(propsid));
			use_num.append(CTrans::UTOS(count));
			if(i != common.props_size() - 1)
			{
				use_itemid.append("_");
				use_num.append("_");
			}
		}
		unsigned space = 0;
		if(BuildCfgWrap().GetStorageCfgById(build_id).level_storage_size() >= databuild.level)
			space = BuildCfgWrap().GetStorageCfgById(build_id).level_storage(databuild.level - 1);
		LogicXsgReportManager::Instance()->XSGStorageUpReport(uid,databuild.build_id,space,use_itemid,use_num);
	}

	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::UpgradeStarSpeedUpReq *req, ProtoBuilding::UpgradeStarSpeedUpResp* resp)
{
	unsigned id = req->id();

	StarSpeedUp(uid, id, resp);

	return 0;
}

int LogicBuildManager::StarSpeedUp(unsigned uid, unsigned id, ProtoBuilding::UpgradeStarSpeedUpResp * resp)
{
	//判断建筑id是否正确
	const ConfigBuilding::ProduceEquipment & producecfg = BuildCfgWrap().GetProduceCfgById(id);

	//获取该建筑的升星数据
	DataEquipmentStar & equipstar = DataEquipmentStarManager::Instance()->GetData(uid, id);

	//判断是否顶级
	if(equipstar.star == producecfg.upgrade_star_size())
	{
		error_log("equip's star already max. uid=%u,id=%u", uid, id);
		throw runtime_error("equip_star_max");
	}

	//获取下一星级需要的时间
	unsigned alltime = producecfg.upgrade_star(equipstar.star).need_time();  //统一以秒为单位
	unsigned price = producecfg.upgrade_star(equipstar.star).need_price();

	unsigned diff_time = alltime > equipstar.usedtime ? alltime -  equipstar.usedtime :0;

	if (0 == diff_time)
	{
		error_log("equip's star and usedtime's map not match. uid=%u,id=%u", uid, id);
		throw runtime_error("star_usedtime_not_match");
	}

	int cash = ceil(static_cast<double>(diff_time)/alltime * price);

	//扣钻
	CommonGiftConfig::CommonModifyItem cfg;
	cfg.mutable_based()->set_cash(-cash);

	LogicUserManager::Instance()->CommonProcess(uid, cfg, "StarSpeedUp", resp->mutable_commons());

	//建筑升星
	equipstar.star += 1;
	equipstar.usedtime = alltime;

	DataEquipmentStarManager::Instance()->UpdateItem(equipstar);

	equipstar.SetMessage(resp->mutable_equipmentstar());

	//将生产设备总星级加到任务系统中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_product_device_star,1);

	return 0;
}

//拆除障碍物
int LogicBuildManager::Process(unsigned uid, ProtoBuilding::RemoveBarrierReq *req, ProtoBuilding::RemoveBarrierResp* resp)
{
	RemoveBarrier(uid, req->id(), resp);
	return 0;
}

int LogicBuildManager::RemoveBarrier(unsigned uid, unsigned id, ProtoBuilding::RemoveBarrierResp * resp)
{
	//判断障碍物id是否存在
	const ConfigBuilding::DestoryBarrier & destorybarriercfg = BarrierCfgWrap().GetDestoryBarrierCfg(id);

	//判断障碍物是否被清除
	DBCUserBaseWrap userwrap(uid);

	//判断是否解锁
	int pos = (id - 1)/CHAR_BITS;
	int left = (id - 1) % CHAR_BITS;

	if (1 == ((userwrap.Obj().barrier[pos] >> left) & 1))
	{
		error_log("barrier already been removed. uid=%u,id=%u", uid, id);
		throw runtime_error("barrier_already_removed");
	}

	//清除
	LogicUserManager::Instance()->CommonProcess(uid, destorybarriercfg.need_props(), "RemoveBarrier", resp->mutable_commons());

	//设置清除标志位
	userwrap.Obj().barrier[pos] |= (1 << left);
	userwrap.Save();

	resp->set_barrier(userwrap.Obj().barrier, sizeof(userwrap.Obj().barrier));
	resp->set_barrierid(id);

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
	{
		int use_item_id = destorybarriercfg.need_props().props(0).id();
		int use_item_num = -destorybarriercfg.need_props().props(0).count();
		LogicXsgReportManager::Instance()->XSGDestroyBarrierReport(uid,use_item_id,use_item_num);
	}

	return 0;
}

bool LogicBuildManager::IsIntersect(unsigned uid, unsigned ud, int * build_status)
{
	//统计耗时
	//timeval tv;
	//gettimeofday(&tv,NULL);
	//debug_log("start:millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);  //毫秒
	//先判断与不可移除障碍物的相交情况
	vector<int> & cells = ParseManager::getInstance()->getParser()->getCells();
	bool isintersect = IsInteractBetween(uid, build_status, cells);

	if (isintersect)
	{
		//gettimeofday(&tv,NULL);
		//debug_log("end:millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);  //毫秒

		return isintersect;
	}

	//如果ud=0，说明是新的建筑，不考虑自己与自己重合的情况
	//先排除障碍物
	DBCUserBaseWrap userwrap(uid);

	int mapstatus[GRID_LENGTH] = {0};  //全地图各格子的位置

	//遍历当前拥有的所有建筑的格子数，排除自身
	vector<unsigned> builds;
	DataBuildingMgr::Instance()->GetIndexs(uid, builds);

	for(size_t i = 0; i < builds.size(); ++i)
	{
		DataBuildings & databuild = DataBuildingMgr::Instance()->GetDataByIndex(builds[i]);

		if (databuild.id == ud)
		{
			//排除自身
			continue;
		}

		unsigned xpos = databuild.position/grid_ypos_len;
		unsigned ypos = databuild.position - xpos*grid_ypos_len;

		SeteBuildGrid(uid, xpos, ypos, databuild.direct, databuild.build_id, mapstatus,false);
	}

	//遍历障碍物
	//获取障碍物的配置
	const ConfigBuilding::Barriers & barriercfg = BarrierCfgWrap().GetBarrierCfg();
	unsigned id = 0;
	int pos = 0, left = 0;

	for(int i = 0; i < barriercfg.barriers_size(); ++i)
	{
		id = barriercfg.barriers(i).id();

		//判断是否解锁
		pos = (id - 1)/CHAR_BITS;
		left = (id - 1) % CHAR_BITS;

		if (1 == ((userwrap.Obj().barrier[pos] >> left) & 1))
		{
			//已解锁
			continue;
		}

		//未解锁，设置占位
		SeteBuildGrid(uid, barriercfg.barriers(i).pos(0u), barriercfg.barriers(i).pos(1u), direct_right, id, mapstatus, false,true);
	}

	isintersect = IsInteractBetween(uid, build_status, mapstatus);

	//gettimeofday(&tv,NULL);
	//debug_log("end:millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);  //毫秒

	return isintersect;
}

int LogicBuildManager::SeteBuildGrid(unsigned uid, unsigned xpos, unsigned ypos, uint8_t direct, unsigned build_id,  int * status
		, bool isbuild,bool isbarrier)
{
	vector<unsigned> foots;

	if (!isbarrier)
	{
		BuildCfgWrap().GetFootPrint(build_id, foots);
	}
	else
	{
		BarrierCfgWrap().GetFootPrint(build_id, foots);
	}

	unsigned width = 0;
	unsigned height = 0;

	if (direct_right == direct)
	{
		//往右
		width = foots[0u];
		height = foots[1u];
	}
	else if (direct_down == direct)
	{
		//往下
		width = foots[1u];
		height = foots[0u];
	}
	else
	{
		error_log("wrong direction. uid=%u,build_id=%u,direction=%u", uid, build_id, direct);
		throw runtime_error("wrong_direction");
	}

	//判断是否越界整个地图
	if (ypos > MAP_LENGTH || ypos + width > MAP_LENGTH)
	{
		error_log("beyond edge. uid=%u,build_id=%u,ypos=%u,width=%u", uid, build_id, ypos, width);
		throw runtime_error("beyond_edge");
	}

	if (xpos > MAP_WIDTH || xpos + height > MAP_WIDTH)
	{
		error_log("beyond edge. uid=%u,build_id=%u,xpos=%u,height=%u", uid, build_id, xpos, height);
		throw runtime_error("beyond_edge");
	}

	//如果调用为建造,判定所经过的格子是否会越界到扩展地图,如果经过、再判定经过的扩展地块是否已解锁
	if(isbuild)
	{
		for(unsigned i = 0; i < height; ++i)
		{
			for(unsigned j = 0; j < width; ++j)
			{
				if(ypos >= MAP_NOT_EXPAND_LENGTH || ypos + j >= MAP_NOT_EXPAND_LENGTH)
				{
					bool isUnlock = CheckMapGridIsUnlock(uid,xpos,i,ypos,j);
					if(!isUnlock)
					{
						error_log("land_is_not_unlock.");
						throw runtime_error("land_is_not_unlock");
					}
				}
			}
		}
	}

	//将经过的格子计入到数组中
	unsigned gridid = 0;
	int pos = 0, left = 0;
	for(unsigned i = 0; i < height; ++i)
	{
		for(unsigned j = 0; j < width; ++j)
		{
			gridid = (xpos + i)*MAP_LENGTH + (ypos + j);

			pos = (gridid)/INT_BITS;
			left = (gridid) % INT_BITS;

			status[pos] |= 1 <<left;
		}
	}

	return 0;
}

bool LogicBuildManager::CheckMapGridIsUnlock(unsigned uid,unsigned xpos,unsigned x_offset,unsigned ypos,unsigned y_offset)
{
	bool isUnlock = false;

	DBCUserBaseWrap userwrap(uid);
	//判定地块位于第几块扩充区域
	if(ypos >= MAP_NOT_EXPAND_LENGTH && ypos < 2 * MAP_NOT_EXPAND_LENGTH)
	{
		unsigned gridid = (xpos + x_offset) * MAP_NOT_EXPAND_LENGTH + (ypos + y_offset - MAP_NOT_EXPAND_LENGTH);
		unsigned pos = (gridid) / CHAR_BITS;
		unsigned right = (gridid) % CHAR_BITS;

		if(1 == ((userwrap.Obj().expand_map_1[pos] >> right) & 1))
			isUnlock = true;
	}
	else if(ypos >= 2 * MAP_NOT_EXPAND_LENGTH && ypos < 3 * MAP_NOT_EXPAND_LENGTH)
	{
		unsigned gridid = (xpos + x_offset) * MAP_NOT_EXPAND_LENGTH + (ypos + y_offset - 2 * MAP_NOT_EXPAND_LENGTH);
		unsigned pos = (gridid) / CHAR_BITS;
		unsigned right = (gridid) % CHAR_BITS;

		if(1 == ((userwrap.Obj().expand_map_2[pos] >> right) & 1))
			isUnlock = true;
	}
	else if(ypos >= 3 * MAP_NOT_EXPAND_LENGTH && ypos < 4 * MAP_NOT_EXPAND_LENGTH)
	{
		unsigned gridid = (xpos + x_offset) * MAP_NOT_EXPAND_LENGTH + (ypos + y_offset - 3 * MAP_NOT_EXPAND_LENGTH);
		unsigned pos = (gridid) / CHAR_BITS;
		unsigned right = (gridid) % CHAR_BITS;

		if(1 == ((userwrap.Obj().expand_map_3[pos] >> right) & 1))
			isUnlock = true;
	}

	return isUnlock;
}

//获取粮仓的剩余可用空间
int LogicBuildManager::GetStorageRestSpace(unsigned uid, unsigned type)
{
	//先获取仓库配置
	const ConfigBuilding::StorageHouse & storagecfg = BuildCfgWrap().GetStorageCfg(type);  //粮仓

	unsigned build_ud = DataBuildingMgr::Instance()->GetBuildUd(uid, storagecfg.id());

	int level = 0;

	//判断仓库是否存在，如果不存在，则默认1级
	if (DataBuildingMgr::Instance()->IsExistItem(uid, build_ud))
	{
		level = DataBuildingMgr::Instance()->GetData(uid, build_ud).level;
	}

	if (level < 1)
	{
		//仓库还未解锁
		level = 1;
	}

	int used = LogicPropsManager::Instance()->GetStorageUsedSpace(uid, type);
	int capacity = storagecfg.level_storage(level - 1);

	//VIP用户增加粮仓货仓的容量
	capacity  += LogicVIPManager::Instance()->AddStorageSpace(uid);

	return capacity > used ?  capacity - used : 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::SellDecorateReq* req, ProtoBuilding::SellDecorateResq* resp)
{
	unsigned  decorate_id = DataBuildingMgr::Instance()->GetData(uid, req->ud()).build_id;
	BuildCfgWrap buildcfgwrap;

	int num = DataBuildingMgr::Instance()->GetBuildNum(uid, decorate_id);
	int type = buildcfgwrap.GetBuildType(decorate_id);
	if(build_type_decorate != type)
	{
		error_log("wrong type. uid=%u,decorate_id=%u", uid, decorate_id);
		throw runtime_error("wrong type");
	}

	char build[20] = {0};
	sprintf(build, "%u", decorate_id);
	string reason("SellDecorate_");
	reason += string(build);

	//变卖装饰物后的收入
	const ConfigBuilding::Decoration & decoratecfg = buildcfgwrap.GetDecorationCfgById(decorate_id);
	const CommonGiftConfig::CommonModifyItem& need_cost = decoratecfg.need_cost(0u);
	CommonGiftConfig::CommonModifyItem cfg;

	if(need_cost.has_based())
	{
		const CommonGiftConfig::BaseItem& based = need_cost.based();
		if(based.has_cash())
		{
			int costbase = (num -1) * decoratecfg.factor();
			float cash = -(based.cash() - costbase);
			int costcash = ceil(cash/2);
			cfg.mutable_based()->set_cash(costcash);
		}
		if(need_cost.based().has_coin())
		{
			int costbase = (num -1) * decoratecfg.factor();
			float coin = -(based.coin() - costbase);
			int costcoin = ceil(coin/2);
			cfg.mutable_based()->set_coin(costcoin);
		}
	}

	if(need_cost.props_size() > 0)
	{
		for(int i = 0; i < need_cost.props_size(); i++)
		{
			CommonGiftConfig::PropsItem * propsitem = cfg.add_props();
			unsigned count = -need_cost.props(i).count();
			propsitem->set_id(need_cost.props(i).id());
			propsitem->set_count(count);
		}
	}

	resp->set_ud(req->ud());
	LogicUserManager::Instance()->CommonProcess(uid, cfg, reason, resp->mutable_commons());

	DataBuildingMgr::Instance()->DelBuild(uid, req->ud());

	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::ViewAdReduceBuildTimeReq* req, ProtoBuilding::ViewAdReduceBuildTimeResp* resp)
{
	unsigned ud = req->ud();
	bool is_exsit = DataBuildingMgr::Instance()->IsExistItem(uid,ud);
	if(!is_exsit)
	{
		throw std::runtime_error("build_is_not_exsit");
	}

	DataBuildings & build =  DataBuildingMgr::Instance()->GetData(uid,ud);
	if(build.done_time <= Time::GetGlobalTime())
	{
		throw std::runtime_error("build_needn't_speedup");
	}

	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_11];
	const ConfigBuilding::ViewAdReduceTimeCPP & cfg_ = ConfigManager::Instance()->builds.m_config.view_ad_cfg();
	if(usedcnt >= cfg_.view_ad_max_cnt())
	{
		throw std::runtime_error("view_ad_cnt_is_maxed");
	}


	unsigned diff_time = build.done_time - Time::GetGlobalTime();
	unsigned base_time = cfg_.base_time();//基准时间
	float reduce_percent = ((float)cfg_.reduce_percent()) / 100;
	unsigned end_ts = 0;

	if(diff_time <= base_time)
	{
		build.done_time = Time::GetGlobalTime();
	}
	else
	{
		build.done_time = (unsigned)diff_time * reduce_percent + Time::GetGlobalTime();
	}

	activity.actdata[e_Activity_UserData_1_index_11] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	DataBuildingMgr::Instance()->UpdateItem(build);

	//重新加入定时队列
	set<unsigned>builds;
	builds.clear();
	builds.insert(ud);
	unsigned endts = build.done_time;
	LogicQueueManager::Instance()->JoinRoutine<DataBuildRoutine>(uid, endts, routine_type_build, builds);

	resp->set_remaincnt(cfg_.view_ad_max_cnt() - activity.actdata[e_Activity_UserData_1_index_11]);
	build.SetMessage(resp->mutable_building());
	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::GetViewAdReduceBuildTimeReq* req, ProtoBuilding::GetViewAdReduceBuildTimeResp* resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_11];
	const ConfigBuilding::ViewAdReduceTimeCPP & cfg_ = ConfigManager::Instance()->builds.m_config.view_ad_cfg();
	resp->set_remaincnt(cfg_.view_ad_max_cnt() - usedcnt);
	return 0;
}

int LogicBuildManager::ResetViewAdReduceBuildTime(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	activity.actdata[e_Activity_UserData_1_index_11] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	return 0;
}

int LogicBuildManager::Process(unsigned uid, ProtoBuilding::ExpandMapReq* req, ProtoBuilding::ExpandMapResp* resp)
{
	//验证用户等级是达到
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().level < ConfigManager::Instance()->mapexpand.m_config.landinfo().unlocklevel())
		throw std::runtime_error("level_unlock");

	LandExpand(uid,req,resp);
	return 0;
}

int LogicBuildManager::LandExpand(unsigned uid, ProtoBuilding::ExpandMapReq* req, ProtoBuilding::ExpandMapResp* resp)
{
	unsigned landid = req->id();
	const ConfigMapExpand::UnLockLandCPP & landcfg =  ConfigManager::Instance()->mapexpand.m_config.landinfo().land(landid - 1);
	unsigned type = landcfg.type();

	//存储地块左上角中的x,y坐标,之所以数组大小定义为10,是因为就目前来说,一个非矩形的地块最多只处理成2个小地块处理。所以10个容量足矣
	unsigned xpos[10] = {0};
	unsigned ypos[10] = {0};
	unsigned index = 0;
	for(int i = 0; i < landcfg.pos_size();)
	{
		xpos[index] = landcfg.pos(i);
		i++;
		ypos[index] = landcfg.pos(i);
		i++;
		index++;
	}

	//验证是否位于未解锁地块(只需判定左上角的y坐标是否位于扩展区域即可)
	for(int i = 0; i < index; i++)
	{
		if(ypos[index] < MAP_NOT_EXPAND_LENGTH)
		{
			error_log("land_need't_lock,ypos=%u",ypos[index]);
			throw std::runtime_error("land_need't_lock");
		}
	}

	//解锁消耗
	const ConfigMapExpand::UnlockLandCostCPP &costcfg = ConfigManager::Instance()->mapexpand.m_config.unlockcost(type - 1);
	LogicUserManager::Instance()->CommonProcess(uid,costcfg.cost(),"map_expand",resp->mutable_commons());

	//解锁地块
	int m = 0;
	DBCUserBaseWrap userwrap(uid);
	for(int count = 0; count < index; count++)
	{
		unsigned width = xpos[count] + landcfg.footprint(m);
		m++;
		unsigned length = ypos[count] + landcfg.footprint(m);
		m++;

		for(int i = 0; i < width; i++)
		{
			for(int j = 0; j < length; j++)
			{
				//遍历需解锁的格子位于哪个扩展区域，进而进行解锁
				if(ypos[count] >= MAP_NOT_EXPAND_LENGTH && ypos[count] < 2 * MAP_NOT_EXPAND_LENGTH)
				{
					//计算位于此扩展地块中的格子id
					unsigned gridid = (xpos[count] + i)*MAP_NOT_EXPAND_LENGTH + (ypos[count] + j -  MAP_NOT_EXPAND_LENGTH);

					unsigned pos = (gridid) / CHAR_BITS;
					unsigned left = (gridid) % CHAR_BITS;
					userwrap.Obj().expand_map_1[pos] |= (1 << left);
				}
				else if(ypos[count] >= 2 * MAP_NOT_EXPAND_LENGTH && ypos[count] < 3 * MAP_NOT_EXPAND_LENGTH)
				{
					//计算位于此扩展地块中的格子id
					unsigned gridid = (xpos[count] + i)*MAP_NOT_EXPAND_LENGTH + (ypos[count] + j - 2 * MAP_NOT_EXPAND_LENGTH);

					unsigned pos = (gridid) / CHAR_BITS;
					unsigned left = (gridid) % CHAR_BITS;
					userwrap.Obj().expand_map_2[pos] |= (1 << left);
				}
				else if(ypos[count]  >= 3 * MAP_NOT_EXPAND_LENGTH && ypos[count] < 4 * MAP_NOT_EXPAND_LENGTH)
				{
					//计算位于此扩展地块中的格子id
					unsigned gridid = (xpos[count] + i)*MAP_NOT_EXPAND_LENGTH + (ypos[count] + j - 3 * MAP_NOT_EXPAND_LENGTH);

					unsigned pos = (gridid) / CHAR_BITS;
					unsigned left = (gridid) % CHAR_BITS;
					userwrap.Obj().expand_map_3[pos] |= (1 << left);
				}
			}
		}
	}
	resp->add_mapstaus(userwrap.Obj().expand_map_1);
	resp->add_mapstaus(userwrap.Obj().expand_map_2);
	resp->add_mapstaus(userwrap.Obj().expand_map_3);
	return 0;
}
