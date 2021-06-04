/*
 * ConfigManager.cpp
 *
 *  Created on: 2016-8-22
 *      Author: Ralf
 */


#include "ConfigManager.h"

bool ConfigManager::m_init = true;

bool ConfigManager::Reload()
{
	ConfigManager *temp = new ConfigManager;
	if(!m_init)
	{
		m_init = true;
		delete temp;
		return false;
	}
	delete temp;
	Destroy();
	temp = Instance();
	return true;
}

void ConfigManager::Init()
{
	if(!m_init)
		return;

	for(int i=0;i<demo.m_config.battle_size();++i)
	{
		if(demo.m_config.battle(i).begin() > demo.m_config.battle(i).end())
		{
			error_log("battle error begin=%u, end=%u", demo.m_config.battle(i).begin(), demo.m_config.battle(i).end());
			Fail();
			return;
		}
		for(int j=demo.m_config.battle(i).begin();j<=demo.m_config.battle(i).end();++j)
			m_server[j] = i;
	}

	for(int i=0;i<activity.m_config.act_size();++i)
		m_actmap[activity.m_config.act(i).id()] = i;

	buildsFootIndex.clear();

	for(int i = 0; i < builds.m_config.build_print_size(); ++i)
	{
		unsigned id = builds.m_config.build_print(i).id();

		buildsFootIndex[id] = i;
	}

	buildsIndex.clear();
	//仓库
	for(int i = 0; i < builds.m_config.storagehouse_size(); ++i)
	{
		unsigned id = builds.m_config.storagehouse(i).id();

		buildsIndex[id].first = build_type_storage;
		buildsIndex[id].second = i;
	}

	//农地
	for(int i = 0; i < builds.m_config.cropland_size(); ++i)
	{
		unsigned id = builds.m_config.cropland(i).id();

		buildsIndex[id].first = build_type_corpland;
		buildsIndex[id].second = i;
	}

	//动物住所
	for(int i = 0; i < builds.m_config.animal_residence_size(); ++i)
	{
		unsigned id = builds.m_config.animal_residence(i).id();

		buildsIndex[id].first = build_type_animal_residence;
		buildsIndex[id].second = i;
	}

	//动物
	for(int i = 0; i < builds.m_config.animal_size(); ++i)
	{
		unsigned id = builds.m_config.animal(i).id();

		buildsIndex[id].first = build_type_animal;
		buildsIndex[id].second = i;
	}

	//生产设备
	for(int i = 0; i < builds.m_config.produce_equipment_size(); ++i)
	{
		unsigned id = builds.m_config.produce_equipment(i).id();

		buildsIndex[id].first = build_type_produce_equipment;
		buildsIndex[id].second = i;
	}

	//果树
	for(int i = 0; i < builds.m_config.fruit_tree_size(); ++i)
	{
		unsigned id = builds.m_config.fruit_tree(i).id();

		buildsIndex[id].first = build_type_fruit_tree;
		buildsIndex[id].second = i;
	}

	//装饰
	for(int i = 0; i < builds.m_config.decorate_size(); ++i)
	{
		unsigned id = builds.m_config.decorate(i).id();

		buildsIndex[id].first = build_type_decorate;
		buildsIndex[id].second = i;
	}

	//障碍物
	BarrierIndex.clear();

	for(int i = 0; i < barriers.m_config.barriers_size(); ++i)
	{
		unsigned id = barriers.m_config.barriers(i).id();
		BarrierIndex[id] = i;
	}

	//道具
	ItemIndex.clear();
	m_itemid_npcid.clear();
	for(int i = 0; i < propsitem.m_config.itemes_size(); ++i)
	{
		unsigned id = propsitem.m_config.itemes(i).id();
		unsigned npc_customer_id = propsitem.m_config.itemes(i).npc_customer_id();
		ItemIndex[id] = i;
		m_itemid_npcid[id] = npc_customer_id;
	}

	//生产线
	productlineIndex.clear();
	m_animalProduce.clear();
	//动物
	for(int i = 0; i < productline.m_config.animal_line_size(); ++i)
	{
		unsigned id = productline.m_config.animal_line(i).id();

		productlineIndex[id].first = build_type_animal;
		productlineIndex[id].second = i;
		for(int j = 0; j < productline.m_config.animal_line(i).product().props_size(); ++j)
		{
			m_productBuild[productline.m_config.animal_line(i).product().props(j).id()] = id;
			m_animalProduce[id].insert(productline.m_config.animal_line(i).product().props(j).id());
		}
	}

	//生产设备
	m_productProduce.clear();
	for(int i = 0; i < productline.m_config.equipline_size(); ++i)
	{
		unsigned id = productline.m_config.equipline(i).id();

		productlineIndex[id].first = build_type_produce_equipment;
		productlineIndex[id].second = i;

		for(int j = 0; j < productline.m_config.equipline(i).product_list_size(); ++j)
		{
			for(int k = 0; k < productline.m_config.equipline(i).product_list(j).props_size(); ++k)
			{
				m_productBuild[productline.m_config.equipline(i).product_list(j).props(k).id()] = id;
				m_productProduce[id].insert(productline.m_config.equipline(i).product_list(j).props(k).id());
			}
		}
	}

	//订单
	m_ordermap.clear();
	for(int i = 0; i < order.m_config.storages_size(); ++i)
	{
		unsigned id = order.m_config.storages(i).storage_id();
		m_ordermap[id] = i;
	}

	//果树生产
	for(int i = 0; i < productline.m_config.fruit_line_size(); ++i)
	{
		unsigned id = productline.m_config.fruit_line(i).tree();

		productlineIndex[id].first = build_type_fruit_tree;
		productlineIndex[id].second = i;
	}

	//任务
	m_taskidmap.clear();
	m_tasktypemap.clear();
	for(int i = 0; i < task.m_config.task_size(); ++i)
	{
		unsigned id       = task.m_config.task(i).id();
		unsigned tasktype = task.m_config.task(i).tasktype();
		m_taskidmap[id] = i;
		m_tasktypemap[tasktype].insert(i);
	}
	m_missionIndex.clear();
	for(int i = 0; i < task.m_config.missions_size(); ++i)
	{
		unsigned missionid = task.m_config.missions(i).id();
		m_missionIndex[missionid] = i;
	}
	// 商会竞赛
	m_allianceRaceTask.clear();
	for(int i = 0; i < allianceRace.m_config.task().storage_size(); ++i)
	{
		const ConfigAllianceRace::RaceTaskStorage &storage = allianceRace.m_config.task().storage(i);
		uint32_t storageId = storage.storage_id();
		for(int j = 0; j < storage.items_size(); ++j)
		{
			m_allianceRaceTask[storageId].push_back(storage.items(j).id());
		}
	}

	m_allianceRaceGrade.clear();
	m_allianceRaceStage.clear();
	for(int i = 0; i < allianceRace.m_config.rewards_size(); ++i)
	{
		const ::ConfigAllianceRace::RaceReward& raceReward = allianceRace.m_config.rewards(i);
		for(int j = 0; j < raceReward.grade_reward_size(); ++j)
		{
			const ::CommonGiftConfig::CommonModifyItemRate& item = raceReward.grade_reward(j);
			RateItem ri;
			ri.id = item.id();
			ri.rate = item.rate();
			m_allianceRaceGrade[raceReward.id()].push_back(ri);
		}


		for(int j = 0; j < raceReward.stage_size(); ++j)
		{
			const ::ConfigAllianceRace::RaceRewardStage& raceStage = raceReward.stage(j);
			for(int k = 0; k < raceStage.reward_size(); ++k)
			{
				const ::CommonGiftConfig::CommonModifyItemRate& item = raceStage.reward(k);
				RateItem ri;
				ri.id = item.id();
				ri.rate = item.rate();
				m_allianceRaceStage[raceReward.id()][raceStage.id()].push_back(ri);
			}
		}
	}

	//捐收
	m_donationIndex.clear();

	for(int i = 0; i < alliance.m_config.donation().type_storge_size(); ++i)
	{
		for(int j = 0; j < alliance.m_config.donation().type_storge(i).products_size(); ++j)
		{
			unsigned propsid = alliance.m_config.donation().type_storge(i).products(j);

			m_donationIndex[propsid].first = i;
			m_donationIndex[propsid].second = j;
		}
	}

	// 农场助手任务ID
	m_keeperBuild.clear();
	m_keeperTarget.clear();
	m_rKeeperTarget.clear();
	m_targetKeeper.clear();
	for(uint32_t i = 0; i < keeper.m_config.keeper_size(); ++i)
	{
		const ConfigKeeper::KeeperInfo& ki = keeper.m_config.keeper(i);
		for(uint32_t type = 0; type < ki.item_size(); ++type)
		{
			const ConfigKeeper::KeeperTask& task = ki.item(type);
			for(uint32_t id = 0; id < task.target_size(); ++id)
			{
				uint32_t taskId = task.target(id).id();
				uint32_t productId = task.target(id).tid();
				m_keeperTarget.insert(make_pair(taskId, productId));
				m_rKeeperTarget.insert(make_pair(productId, taskId));
				m_targetKeeper.insert(make_pair(productId, ki.id()));
				if(m_productBuild.find(productId) != m_productBuild.end())
				{
					m_keeperBuild.insert(make_pair(taskId, m_productBuild[productId]));
				}
			}
		}
	}

	// 助手生产原料
	m_keeperMaterial.clear();

	for(int i = 0; i < propsitem.m_config.itemes_size(); ++i)
	{
		const ConfigItem::PropItem& item = propsitem.m_config.itemes(i);
		if(item.has_material())
		{
			const CommonGiftConfig::CommonModifyItem& material = item.material();
			for(int j = 0; j < material.props_size(); ++j)
			{
				int count = material.props(j).count();
				if(count >= 0)
				{
					continue;
				}
				unsigned productId = item.id();
				if(m_rKeeperTarget.find(productId) == m_rKeeperTarget.end())
				{
					continue;
				}
				if(m_productBuild.find(productId) == m_productBuild.end())
				{
					continue;
				}
				KeeperMaterial km;
				km.need = -1 * count;
				km.taskId = m_rKeeperTarget[productId];
				km.buildId = m_productBuild[productId];
				km.unlockLevel = item.unlock_level();
				m_keeperMaterial[material.props(j).id()][item.id()] = km;
			}
		}
	}

	//宠物
	m_petmap.clear();
	for(int i = 0; i < pet.m_config.pet_size(); i++)
	{
		unsigned petid = pet.m_config.pet(i).petid();
		m_petmap[petid] = i;
	}

	//4399首冲翻倍活动配置
	m_4399_recharge.clear();
	for(uint8_t idx = 0; idx < activitydata.m_config.charge_4399().limit_size(); idx++)
	{
		m_4399_recharge.push_back(activitydata.m_config.charge_4399().limit(idx));
	}

	//4399每日充值活动配置
	m_4399_daily.clear();
	for(uint8_t idx = 0; idx < activitydata.m_config.daily_4399().limit_size(); idx++)
	{
		m_4399_daily.push_back(activitydata.m_config.daily_4399().limit(idx));
	}
}


bool ConfigManager::GetActivity(unsigned id, User::ActivityItem& act)
{
	if(m_actmap.count(id))
	{
		act = activity.m_config.act(m_actmap[id]);
		return true;
	}
	return false;
}

const Demo::Server& ConfigManager::GetServer(unsigned server)
{
	if(!m_server.count(server))
		throw std::runtime_error("no server");
	return demo.m_config.battle(m_server[server]);
}
bool ConfigManager::IsServerMergeTogather(unsigned s1, unsigned s2)
{
	if(!m_server.count(s1) || !m_server.count(s2))
	{
		s1 =  MainConfig::GetMergedDomain(s1);
		s2 =  MainConfig::GetMergedDomain(s2);
		if(s1 && s2 && s1 == s2)
			return true;
		return false;
	}
	if( m_server[s1] == m_server[s2])
		return true;
	return false;
}
unsigned ConfigManager::GetMainServer(unsigned server)
{
	if(!m_server.count(server))
	{
		server =  MainConfig::GetMergedDomain(server);
		if(server)
			return server;
		throw std::runtime_error("no server");
	}
	return demo.m_config.battle(m_server[server]).begin();
}
bool ConfigManager::IsNeedConnect(unsigned server)
{
	return !IsServerMergeTogather(server, Config::GetIntValue(CONFIG_SRVID));
}
bool ConfigManager::IsNeedConnectByUID(unsigned uid)
{
	return IsNeedConnect(Config::GetZoneByUID(uid));
}
bool ConfigManager::IsNeedConnectByAID(unsigned aid)
{
	return IsNeedConnect(Config::GetZoneByAID(aid));
}
