/*
 * UserCfgWrap.cpp
 *
 *  Created on: 2016-9-12
 *      Author: dawx62fac
 */

#include "ConfigWrap.h"

UserCfgWrap::UserCfgWrap()
	: cfg_(ConfigManager::Instance()->user.m_config)
{
}

const UserCfg::User& UserCfgWrap::User() const
{
	return cfg_;
}

const UserCfg::UserBase& UserCfgWrap::UserBase() const
{
	return cfg_.user_init();
}

const UserCfg::CoinPurchase & UserCfgWrap::GetCoinPurchaseCfg(unsigned id) const
{
	if (id < 1 || id > cfg_.coinpurchase_size())
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	return cfg_.coinpurchase(id - 1);
}

///////////////////////////////////////////////////////////////////////////
ActivityCfgWrap::ActivityCfgWrap()
	: cfg_(ConfigManager::Instance()->activity.m_config)
{
}

const User::ActivityItem& ActivityCfgWrap::GetItem(unsigned id) const
{
	for(int i = 0; i < cfg_.act_size(); ++i)
	{
		if (cfg_.act(i).id() == id)
		{
			return cfg_.act(i);
		}
	}

	error_log("unknown_activity_cfg. id: %u", id);
	throw std::runtime_error("unknown_activity_cfg");
}
//////////////////////////////////////////////////////////////////////////

BuildCfgWrap::BuildCfgWrap()
	: cfg_(ConfigManager::Instance()->builds.m_config)
{

}

int BuildCfgWrap::GetUnlockUpgradeStarLevel() const
{
	return cfg_.unlock_upgardestar_level();
}

int BuildCfgWrap::GetBuildType(unsigned build_id) const
{
	int type = build_id/build_type_len;

	if (build_type_corpland == type)
	{
		//可能是仓库或者房子
		//获取房子配置
		const ConfigBuilding::House & housecfg = cfg_.house();
		const ConfigBuilding::CropLand & corpcfg = cfg_.cropland(0u);

		if (housecfg.id() == build_id)
		{
			//房子
			return build_type_house;
		}
		else if (corpcfg.id() == build_id)
		{
			//农地
			return build_type_corpland;
		}
		else
		{
			//仓库，暂时没有消耗.不允许多个
			return build_type_storage;
		}
	}

	//其他类型，值就是该类型
	return type;
}

int BuildCfgWrap::GetLevelIndex(const google::protobuf::RepeatedField< ::google::protobuf::uint32 >& needlevel, unsigned lv)
{
	int max = needlevel.size();

	if (0 == max)
	{
		return 0;
	}

	//找出lv第一个大于等于的下标
	for(int i = max - 1; i >= 0; --i)
	{
		if (lv >= needlevel.Get(i))
		{
			return i;
		}
	}

	error_log("level not enough. level=%u,min=%u", lv, needlevel.Get(0u));

	throw runtime_error("level_not_enough");
}

int BuildCfgWrap::GetGainNumIndex(const google::protobuf::RepeatedField< ::google::protobuf::uint32 >& gain_num, unsigned num)
{
	int max = gain_num.size();

	if (0 == max)
	{
		return 0;
	}

	for(int i = 0; i < max; i++)
	{
		if(num <= gain_num.Get(i))
			return i;
	}
	/*
	//找出lv第一个大于等于的下标
	for(int i = max - 1; i >= 0; --i)
	{
		if (num >= gain_num.Get(i))
		{
			return i;
		}
	}
	*/
	return 0;
}

const ConfigBuilding::House & BuildCfgWrap::GetHouseCfg() const
{
	return cfg_.house();
}

bool BuildCfgWrap::IsStorage(unsigned build_id)
{
	for(int i = 0; i < cfg_.storagehouse_size(); ++i)
	{
		if (build_id == cfg_.storagehouse(i).id())
		{
			return true;
		}
	}

	return false;
}

const ConfigBuilding::StorageHouse & BuildCfgWrap::GetStorageCfg(unsigned index) const
{
	if (index > cfg_.storagehouse_size())
	{
		error_log("param error. index=%u", index);
		throw runtime_error("param_error");
	}

	return cfg_.storagehouse(index - 1);
}

int BuildCfgWrap::GetFootPrint(unsigned build_id, vector<unsigned> & foots)
{
	debug_log("build_id=%u",build_id);
	if (!ConfigManager::Instance()->buildsFootIndex.count(build_id))
	{
		error_log("param error. build_id=%u", build_id);
		throw runtime_error("param_error");
	}
	debug_log("build_id=%u",build_id);

	foots.resize(2);
	unsigned index_ = ConfigManager::Instance()->buildsFootIndex[build_id];

	foots[0u] = cfg_.build_print(index_).footprint(0u);
	foots[1u] = cfg_.build_print(index_).footprint(1u);

	return 0;
}

const ConfigBuilding::StorageHouse & BuildCfgWrap::GetStorageCfgById(unsigned id) const
{
	if (!ConfigManager::Instance()->buildsIndex.count(id))
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	if (build_type_storage != ConfigManager::Instance()->buildsIndex[id].first)
	{
		error_log("type error. id=%u", id);
		throw runtime_error("type_error");
	}

	unsigned index_ = ConfigManager::Instance()->buildsIndex[id].second;

	return cfg_.storagehouse(index_);
}

const ConfigBuilding::CropLand & BuildCfgWrap::GetCropLandCfg() const
{
	if (cfg_.cropland_size() == 0)
	{
		error_log("corpland config error.");
		throw runtime_error("config_error");
	}

	return cfg_.cropland(0u);
}

const ConfigBuilding::AnimalResidence & BuildCfgWrap::GetAnimalResidenceCfgById(unsigned id) const
{
	if (!ConfigManager::Instance()->buildsIndex.count(id))
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	if (build_type_animal_residence != ConfigManager::Instance()->buildsIndex[id].first)
	{
		error_log("type error. id=%u", id);
		throw runtime_error("type_error");
	}

	unsigned index_ = ConfigManager::Instance()->buildsIndex[id].second;

	return cfg_.animal_residence(index_);
}

const ConfigBuilding::Animal & BuildCfgWrap::GetAnimalCfgById(unsigned id) const
{
	if (!ConfigManager::Instance()->buildsIndex.count(id))
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	if (build_type_animal != ConfigManager::Instance()->buildsIndex[id].first)
	{
		error_log("type error. id=%u", id);
		throw runtime_error("type_error");
	}

	unsigned index_ = ConfigManager::Instance()->buildsIndex[id].second;

	return cfg_.animal(index_);
}

const ConfigBuilding::ProduceEquipment & BuildCfgWrap::GetProduceCfgById(unsigned id) const
{
	if (!ConfigManager::Instance()->buildsIndex.count(id))
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	if (build_type_produce_equipment != ConfigManager::Instance()->buildsIndex[id].first)
	{
		error_log("type error. id=%u", id);
		throw runtime_error("type_error");
	}

	unsigned index_ = ConfigManager::Instance()->buildsIndex[id].second;

	return cfg_.produce_equipment(index_);
}

const ConfigBuilding::FruitTree & BuildCfgWrap::GetFruitTreeCfgById(unsigned id) const
{
	if (!ConfigManager::Instance()->buildsIndex.count(id))
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	if (build_type_fruit_tree != ConfigManager::Instance()->buildsIndex[id].first)
	{
		error_log("type error. id=%u", id);
		throw runtime_error("type_error");
	}

	unsigned index_ = ConfigManager::Instance()->buildsIndex[id].second;

	return cfg_.fruit_tree(index_);
}

const ConfigBuilding::Decoration & BuildCfgWrap::GetDecorationCfgById(unsigned id) const
{
	if (!ConfigManager::Instance()->buildsIndex.count(id))
	{
		error_log("param error. id=%u", id);
		throw runtime_error("param_error");
	}

	if (build_type_decorate != ConfigManager::Instance()->buildsIndex[id].first)
	{
		error_log("type error. id=%u", id);
		throw runtime_error("type_error");
	}

	unsigned index_ = ConfigManager::Instance()->buildsIndex[id].second;

	return cfg_.decorate(index_);
}

BarrierCfgWrap::BarrierCfgWrap()
	: cfg_(ConfigManager::Instance()->barriers.m_config)
{

}

int BarrierCfgWrap::GetFootPrint(unsigned barrier_id, vector<unsigned> & foots)
{
	if (!ConfigManager::Instance()->BarrierIndex.count(barrier_id))
	{
		error_log("param error. barrier_id=%u", barrier_id);
		throw runtime_error("param_error");
	}

	foots.resize(2);
	unsigned index_ = ConfigManager::Instance()->BarrierIndex[barrier_id];

	foots[0u] = cfg_.barriers(index_).footprint(0u);
	foots[1u] = cfg_.barriers(index_).footprint(1u);

	return 0;
}

const ConfigBuilding::Barrier & BarrierCfgWrap::GetBarrierCfg(unsigned barrier_id) const
{
	if (!ConfigManager::Instance()->BarrierIndex.count(barrier_id))
	{
		error_log("param error. barrier_id=%u", barrier_id);
		throw runtime_error("param_error");
	}

	unsigned index_ = ConfigManager::Instance()->BarrierIndex[barrier_id];

	return cfg_.barriers(index_);
}

const ConfigBuilding::DestoryBarrier & BarrierCfgWrap::GetDestoryBarrierCfg(unsigned barrier_id) const
{
	const ConfigBuilding::Barrier & barriercfg = GetBarrierCfg(barrier_id);

	for(int i = 0; i < cfg_.barriers_destory_size(); ++i)
	{
		if (cfg_.barriers_destory(i).type() == barriercfg.type())
		{
			return cfg_.barriers_destory(i);
		}
	}

	error_log("type error. barrier_id=%u,type=%u", barrier_id, barriercfg.type());
	throw runtime_error("barrier_type_error");
}

ItemCfgWrap::ItemCfgWrap()
	: cfg_(ConfigManager::Instance()->propsitem.m_config)
{

}

const ConfigItem::PropItem & ItemCfgWrap::GetPropsItem(unsigned propsid) const
{
	if (!ConfigManager::Instance()->ItemIndex.count(propsid))
	{
		error_log("param error. propsid=%u", propsid);
		throw runtime_error("param_error");
	}

	unsigned index = ConfigManager::Instance()->ItemIndex[propsid];

	return cfg_.itemes(index);
}

int ItemCfgWrap::GetUnlockNPCShopItem(unsigned type,unsigned level,vector<unsigned> &propsid) const
{
	for(int i = 0; i < cfg_.itemes_size(); i++)
	{
		if(cfg_.itemes(i).type() == type && level >= cfg_.itemes(i).unlock_level())
			propsid.push_back(cfg_.itemes(i).id());
	}
	return 0;
}

int ItemCfgWrap::GetPropsListByNPCId(unsigned npcid,vector<unsigned> &items) const
{
	for(int i = 0; i < cfg_.itemes_size(); i++)
	{
		if(cfg_.itemes(i).npc_customer_id() == npcid)
			items.push_back(cfg_.itemes(i).id());
	}
	return 0;
}

int ItemCfgWrap::GetNPCIdByItemId(unsigned itemid) const
{
	if (!ConfigManager::Instance()->m_itemid_npcid.count(itemid))
	{
		error_log("param error. itemid=%u", itemid);
		throw runtime_error("param_error");
	}

	unsigned npcid = ConfigManager::Instance()->m_itemid_npcid[itemid];

	return npcid;
}

int ItemCfgWrap::GetUnlockShippingItem(unsigned level, vector<unsigned> & items) const
{
	//遍历item上的所有物品
	for(int i = 0; i < cfg_.itemes_size(); ++i)
	{
		if (cfg_.itemes(i).unlock_level() > level)
		{
			continue;
		}

		if (cfg_.itemes(i).type() == item_type_other)
		{
			continue;
		}

		items.push_back(cfg_.itemes(i).id());
	}

	return 0;
}

int ItemCfgWrap::GetUnlockItem(unsigned level,vector<unsigned> & items,unsigned location) const
{
	//遍历item上的所有物品
	for(int i = 0; i < cfg_.itemes_size(); ++i)
	{
		if (cfg_.itemes(i).unlock_level() <= level && cfg_.itemes(i).location() == location)
		{
			items.push_back(cfg_.itemes(i).id());
		}
	}

	return 0;
}

ProductlineCfgWrap::ProductlineCfgWrap()
	: cfg_(ConfigManager::Instance()->productline.m_config)
{

}

const ConfigProductLine::EquipLine & ProductlineCfgWrap::GetEquiplineCfg(unsigned equipid) const
{
	if (!ConfigManager::Instance()->productlineIndex.count(equipid))
	{
		error_log("param error. equipid=%u", equipid);
		throw runtime_error("param_error");
	}

	unsigned index = ConfigManager::Instance()->productlineIndex[equipid].second;

	return cfg_.equipline(index);
}

int ProductlineCfgWrap::GetEquipProductIndex(unsigned equipid, unsigned productid) const
{
	const ConfigProductLine::EquipLine & equipline = GetEquiplineCfg(equipid);

	//遍历产品列表
	for(int i = 0; i < equipline.product_list_size(); ++i)
	{
		for(int j = 0; j < equipline.product_list(i).props_size(); ++j)
		{
			if (equipline.product_list(i).props(j).id() == productid)
			{
				return i;
			}
		}
	}

	error_log("param error. equipid=%u,productid=%u", equipid, productid);
	throw runtime_error("param_error");
}

const ConfigProductLine::AnimalLine & ProductlineCfgWrap::GetAnimallineCfg(unsigned animalid) const
{
	if (!ConfigManager::Instance()->productlineIndex.count(animalid))
	{
		error_log("param error. animalid=%u", animalid);
		throw runtime_error("param_error");
	}

	unsigned index = ConfigManager::Instance()->productlineIndex[animalid].second;

	return cfg_.animal_line(index);
}
OrderCfgWrap::OrderCfgWrap()
	: cfg_(ConfigManager::Instance()->order.m_config)
{
}
const ConfigProductLine::FruitLine & ProductlineCfgWrap::GetFruitlineCfg(unsigned treeid) const
{
	if (!ConfigManager::Instance()->productlineIndex.count(treeid))
	{
		error_log("param error. treeid=%u", treeid);
		throw runtime_error("param_error");
	}

	unsigned index = ConfigManager::Instance()->productlineIndex[treeid].second;

	return cfg_.fruit_line(index);
}

const ConfigProductLine::MaterailReward & ProductlineCfgWrap::GetMaterialCfg() const
{
	return cfg_.get_storage_material();
}

//产品生产线
void ProductlineCfgWrap::GetProductProduce(uint32_t buildId, set<uint32_t>& productId)
{
	map<unsigned, set<unsigned> >::iterator iter = ConfigManager::Instance()->m_productProduce.find(buildId);
	if(iter != ConfigManager::Instance()->m_productProduce.end())
	{
		productId = iter->second;
	}
}

// 动物生产线
void ProductlineCfgWrap::GetAnimalProduce(uint32_t animalId, set<uint32_t>& productId)
{
	map<unsigned, set<unsigned> >::iterator iter = ConfigManager::Instance()->m_animalProduce.find(animalId);
	if(iter != ConfigManager::Instance()->m_animalProduce.end())
	{
		productId = iter->second;
	}
}

// 是否存在改动物
bool ProductlineCfgWrap::ExistAnimal(uint32_t animalId)
{
	return ConfigManager::Instance()->m_animalProduce.find(animalId) != ConfigManager::Instance()->m_animalProduce.end();
}
ShopCfgWrap::ShopCfgWrap()
 	 :cfg_(ConfigManager::Instance()->shop.m_config)
{

}
const ConfigShop::ShopCPP & ShopCfgWrap::GetShopInfoCfg() const
{
	return cfg_.shop();
}

const ConfigShop::NPCShopCPP & ShopCfgWrap::GetNPCShopInfoCfg() const
{
	return cfg_.npcshop();
}


AdCfgWrap::AdCfgWrap()
 	 :cfg_(ConfigManager::Instance()->advertise.m_config)
{

}
const ConfigAd::AdCPP & AdCfgWrap::GetAdInfoCfg() const
{
	return cfg_.adinfo();
}

/*----------任务--------------*/
TaskCfgWrap::TaskCfgWrap()
 	 :cfg_(ConfigManager::Instance()->task.m_config)
{

}

const ConfigTask::TaskInfo & TaskCfgWrap::GetAllTaskCfg() const
{
	return cfg_;
}


const ConfigTask::TaskCPP & TaskCfgWrap::GetTaskInfoCfg(unsigned taskid) const
{
	if (!ConfigManager::Instance()->m_taskidmap.count(taskid))
	{
		error_log("param error. taskid=%u", taskid);
		throw runtime_error("param_error");
	}

	unsigned index = ConfigManager::Instance()->m_taskidmap[taskid];

	return cfg_.task(index);
}

void TaskCfgWrap::GetTaskIndexsCfg(unsigned tasktype, set<unsigned> & ilist) const
{
	if (!ConfigManager::Instance()->m_tasktypemap.count(tasktype))
	{
		error_log("param error. tasktype=%u", tasktype);
		throw runtime_error("param_error");
	}

	ilist = ConfigManager::Instance()->m_tasktypemap[tasktype];
}

const ConfigTask::MissionCPP & TaskCfgWrap::GetMissionInfoCfg(unsigned missionid) const
{
	if (!ConfigManager::Instance()->m_missionIndex.count(missionid))
	{
		error_log("param error. missionid=%u", missionid);
		throw runtime_error("param_error");
	}

	unsigned index = ConfigManager::Instance()->m_missionIndex[missionid];

	return cfg_.missions(index);
}


/*-----------------VIP--------------------------------*/
VIPCfgWrap::VIPCfgWrap()
 	 :cfg_(ConfigManager::Instance()->vip.m_config)
{

}

const ConfigVIP::VIPCfgInfo & VIPCfgWrap::GetVIPInfoCfg() const
{
	return cfg_;
}


FriendCfgWrap::FriendCfgWrap()
	: cfg_(ConfigManager::Instance()->friendcfg.m_config)
{

}

const ConfigFriend::LevelNums & FriendCfgWrap::GetLevelCfgByLevel(unsigned level) const
{
	//判断长度是否越界
	if (cfg_.nums_size() <= level)
	{
		error_log("param error. level=%u", level);
		throw runtime_error("param_error");
	}

	return cfg_.nums(level);
}

ShippingCfgWrap::ShippingCfgWrap()
	: cfg_(ConfigManager::Instance()->shipping.m_config)
{

}

AllianceCfgWrap::AllianceCfgWrap()
	: cfg_(ConfigManager::Instance()->alliance.m_config)
{

}

int AllianceCfgWrap::GetDonationPropsTypeIndex(unsigned propsid) const
{
	//判断是否存在该物品的捐收配置
	if (!ConfigManager::Instance()->m_donationIndex.count(propsid))
	{
		error_log("param error. propsid=%u", propsid);
		throw runtime_error("param_error");
	}

	return ConfigManager::Instance()->m_donationIndex[propsid].first;
}


/*-----------------NPCSeller--------------------------------*/
NPCSellerCfgWrap::NPCSellerCfgWrap()
 	 :cfg_(ConfigManager::Instance()->npcseller.m_config)
{

}

const ConfigNPCSeller::NPCSellerCfg & NPCSellerCfgWrap::GetNPCSellerCfg() const
{
	return cfg_;
}

/*-----------------cdkey--------------------------------*/

CdKeyCfgWrap::CdKeyCfgWrap()
	:cfg_(ConfigManager::Instance()->cdkey.m_config)
{

}

const CdKey::CdKeyCfg & CdKeyCfgWrap::GetCfg() const
{
	return cfg_;
}

/*-----------------LevelupUnlock--------------------------------*/

LevelupUnlockCfgWrap::LevelupUnlockCfgWrap()
	:cfg_(ConfigManager::Instance()->levelupUnlock.m_config)
{

}

const LevelupUnlock::LevelupUnlockCfg& LevelupUnlockCfgWrap::GetLevelupUnlockCfg() const
{
	return cfg_;
}

/*-----------------AllianceRace--------------------------------*/

AllianceRaceCfgWrap::AllianceRaceCfgWrap()
	: cfg_(ConfigManager::Instance()->allianceRace.m_config)
{

}

unsigned AllianceRaceCfgWrap::GetRandTaskId(unsigned storageId) const
{
	map<unsigned, vector<unsigned> >::iterator sIter = ConfigManager::Instance()->m_allianceRaceTask.find(storageId);

	//判断是否存在该库配置
	if (sIter == ConfigManager::Instance()->m_allianceRaceTask.end())
	{
		error_log("param error. storageId=%u", storageId);
		throw runtime_error("param_error");
	}
	vector<unsigned>& items = sIter->second;
	if(items.empty())
	{
		error_log("alliance race item is empty storageId=%u", storageId);
		throw runtime_error("alliance_race_item_empty_error");
	}
	return items[Math::GetRandomInt(items.size())];
}
unsigned AllianceRaceCfgWrap::GetTaskChance(unsigned raceLevel) const
{
	for(int i = 0; i < cfg_.task().chance_size(); ++i)
	{
		const ConfigAllianceRace::RaceTaskChance& chance = cfg_.task().chance(i);
		if(chance.race_level() == raceLevel)
		{
			return chance.value();
		}
	}
	error_log("alliance race level not exist raceLevel=%u", raceLevel);
	throw runtime_error("alliance_race_not_exist_race_level");
}
void AllianceRaceCfgWrap::GetTaskInfo(unsigned storageId, unsigned id, unsigned& t, unsigned& level) const
{
	for(int i = 0; i < cfg_.task().storage_size(); ++i)
	{
		const ConfigAllianceRace::RaceTaskStorage & storage = cfg_.task().storage(i);
		if(storage.storage_id() == storageId)
		{
			for(int j = 0; j < storage.items_size(); ++j)
			{
				const ConfigAllianceRace::RaceTaskStorageItem &item = storage.items(j);
				if(item.id() == id)
				{
					t = item.time();
					level = item.level();
					return;
				}
			}
		}
	}
	error_log("alliance race item is empty storageId=%u id=%u", storageId, id);
	throw runtime_error("alliance_race_item_empty_error");
	return;
}
const ConfigAllianceRace::RaceTaskStorageItem& AllianceRaceCfgWrap::GetTaskItem(unsigned storageId, unsigned id) const
{
	for(int i = 0; i < cfg_.task().storage_size(); ++i)
	{
		const ::ConfigAllianceRace::RaceTaskStorage & storage = cfg_.task().storage(i);
		if(storage.storage_id() == storageId)
		{
			for(int j = 0; j < storage.items_size(); ++j)
			{
				const ConfigAllianceRace::RaceTaskStorageItem &item = storage.items(j);
				if(item.id() == id)
				{
					return item;
				}
			}
		}
	}
	error_log("alliance race item is empty storageId=%u id=%u", storageId, id);
	throw runtime_error("alliance_race_item_empty_error");
}
void AllianceRaceCfgWrap::GetGradeReward(uint32_t uid, uint32_t levelId, uint32_t rankId, set<uint32_t>& id, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.rewards_size(); ++i)
	{
		const ConfigAllianceRace::RaceReward& raceReward = cfg_.rewards(i);
		if(levelId == raceReward.id())
		{
			float rate = 0;
			for(uint32_t j = 0; j < raceReward.rank_size(); ++j)
			{
				if(raceReward.rank(j).id() == rankId)
				{
					rate = raceReward.rank(j).rate();
					break;
				}
			}
//			debug_log("alliance_race_get_reward_grade uid=%u,rankId=%u,levelId=%u,idSize=%u", uid, rankId, levelId,id.size());
			AddReward(rate, id, raceReward.grade_reward(), msg);
			return;
		}
	}
	error_log("alliance race invalid levelId=%u", levelId);
	throw runtime_error("alliance_race_invalid_level");
}
void AllianceRaceCfgWrap::GetStageReward(uint32_t uid, uint32_t levelId, vector<uint32_t>& stageId, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.rewards_size(); ++i)
	{
		const ConfigAllianceRace::RaceReward& raceReward = cfg_.rewards(i);
		if(levelId == raceReward.id())
		{
			for(uint32_t j = 0; j < stageId.size() && j < raceReward.stage_size(); ++j)
			{
				set<uint32_t> sid;
				sid.insert(stageId[j]);
				AddReward(1, sid, raceReward.stage(j).reward(), msg);
			}
//			debug_log("alliance_race_get_reward_stage uid=%u,levelId=%u,idSize=%u", uid, levelId,stageId.size());
			return;
		}
	}
	error_log("alliance race invalid levelId=%u", levelId);
	throw runtime_error("alliance_race_invalid_level");
}
void AllianceRaceCfgWrap::RefreshGradeReward(uint32_t levelId, uint8_t rewardArr[], uint32_t rewardArrSize)
{
	if(rewardArr == NULL)
	{
		return;
	}
	set<RateItem> randItem;
	const vector<RateItem>& vRateItem = GetRaceGradeReward(levelId);
	if(!RouletteWheel(rewardArrSize, vRateItem, randItem))
	{
		error_log("RouletteWheel fail levelId=%u", levelId);
		throw runtime_error("alliance_race_RouletteWheel_fail");
	}
	for(uint32_t i = 0; i < rewardArrSize && !randItem.empty(); ++i)
	{
		rewardArr[i] = randItem.begin()->id;
		randItem.erase(randItem.begin());
	}
}
void AllianceRaceCfgWrap::RefreshStageReward(uint32_t levelId, uint8_t stageRewardId[], uint32_t stageIdSize)
{
	if(stageRewardId == NULL)
	{
		return;
	}
	const uint32_t groupSize = ALLIANCE_RACE_STAGE_REWARD_GROUP_SIZE;	// 9组阶段奖励，每组3个奖励
	for(uint32_t i = 0; i < stageIdSize; i += groupSize)
	{
		set<RateItem> randItem;
		const vector<RateItem>& vRateItem = GetRaceStageReward(levelId, i / groupSize + 1);
		if(!RouletteWheel(groupSize, vRateItem, randItem))
		{
			error_log("RouletteWheel fail levelId=%u i=%u", levelId, i);
			throw runtime_error("alliance_race_RouletteWheel_fail");
		}
		for(uint32_t j = i; j < i + groupSize && j < stageIdSize; ++j)	// 组内需要奖励无重复
		{
			uint32_t sid = stageRewardId[j];
			if(sid == 0)	// 需要刷新
			{
				stageRewardId[j] = randItem.begin()->id;
				randItem.erase(randItem.begin());
			}
		}
	}
}

void AllianceRaceCfgWrap::AddReward(float rate, set<uint32_t>& id,
		const ::google::protobuf::RepeatedPtrField< ::CommonGiftConfig::CommonModifyItemRate >& items,
		CommonGiftConfig::CommonModifyItem * msg)
{
	int32_t coin = 0;
	int32_t cash = 0;
	int32_t exp = 0;
	for(uint32_t g = 0; g < items.size(); ++g)
	{
		const CommonGiftConfig::CommonModifyItemRate& reward = items.Get(g);
		if(id.find(reward.id()) != id.end())
		{
			const CommonGiftConfig::CommonModifyItem& item = reward.item();
			if(item.has_based())
			{
				if(item.based().has_coin())
				{
					coin += item.based().coin();
				}
				if(item.based().has_cash())
				{
					cash += item.based().cash();
				}
				if(item.based().has_exp())
				{
					exp += item.based().exp();
				}
			}
			for(uint32_t p = 0; p < item.props_size(); ++p)
			{
				CommonGiftConfig::PropsItem* pItem = msg->add_props();
				pItem->set_id(item.props(p).id());
				pItem->set_count(ceil(item.props(p).count() * rate));
			}
		}
	}
	if(msg->mutable_based()->has_coin())
	{
		coin += ceil(msg->mutable_based()->coin() * rate);
	}
	if(msg->mutable_based()->has_cash())
	{
		cash += ceil(msg->mutable_based()->cash() * rate);
	}
	if(msg->mutable_based()->has_exp())
	{
		exp += ceil(msg->mutable_based()->exp() * rate);
	}

	if(coin > 0)
	{
		msg->mutable_based()->set_coin(coin);
	}
	if(cash > 0)
	{
		msg->mutable_based()->set_cash(cash);
	}
	if(exp > 0)
	{
		msg->mutable_based()->set_exp(exp);
	}
}
const vector<RateItem>& AllianceRaceCfgWrap::GetRaceStageReward(uint32_t levelId, uint32_t stageId)
{
	map<unsigned, map<unsigned, vector<RateItem> > >& raceStage = ConfigManager::Instance()->m_allianceRaceStage;
	map<unsigned, map<unsigned, vector<RateItem> > >::iterator iter1 = raceStage.find(levelId);
	if(iter1 == raceStage.end())
	{
		error_log("param error levelId=%u", levelId);
		throw runtime_error("param_error_race_levelId");
	}
	map<unsigned, vector<RateItem> >::iterator iter2 = iter1->second.find(stageId);
	if(iter2 == iter1->second.end())
	{
		error_log("param error stageId=%u", stageId);
		throw runtime_error("param_error_race_stageId");
	}
	return iter2->second;
}
const vector<RateItem>& AllianceRaceCfgWrap::GetRaceGradeReward(uint32_t levelId)
{
	map<unsigned, vector<RateItem> >& raceStage = ConfigManager::Instance()->m_allianceRaceGrade;
	map<unsigned, vector<RateItem> >::iterator iter2 = raceStage.find(levelId);
	if(iter2 == raceStage.end())
	{
		error_log("param error levelId=%u", levelId);
		throw runtime_error("param_error_race_levelId");
	}
	return iter2->second;
}
uint32_t AllianceRaceCfgWrap::GetRaceRewardLevelId(uint32_t level)
{
	for(int i = 0; i < cfg_.rewards_size(); ++i)
	{
		const ConfigAllianceRace::RaceReward& reward = cfg_.rewards(i);
		if(level >= reward.min_level() && level <= reward.max_level())
		{
			return reward.id();
		}
	}
	throw runtime_error("invalid_race_levelId");
}
uint32_t AllianceRaceCfgWrap::getRankPoint(uint32_t raceLevel, uint32_t rankId)
{
	for(int i = 0; i < cfg_.grade_size(); ++i)
	{
		const ::ConfigAllianceRace::RaceGrade& grade = cfg_.grade(i);
		if(grade.race_level() == raceLevel)
		{
			for(int j = 0; j < grade.rank_size(); ++j)
			{
				const ::ConfigAllianceRace::RaceGradeRank& rank = grade.rank(j);
				if(rank.id() == rankId)
				{
					return rank.point();
				}
			}
		}
	}
	return 0;
}
uint32_t AllianceRaceCfgWrap::GetRaceRewardStageId(uint32_t userLevel, uint32_t raceLevel, uint32_t point)
{
	for(int i = 0; i < cfg_.rewards_size(); ++i)
	{
		const ConfigAllianceRace::RaceReward& reward = cfg_.rewards(i);
		if(userLevel >= reward.min_level() && userLevel <= reward.max_level())
		{
			int j = 0;

			for(j = 0; j < reward.stage_size(); ++j)
			{
				const ConfigAllianceRace::RaceRewardStage& stage = reward.stage(j);
				if(raceLevel > stage.race_level() || point < stage.point())
				{
					break;
				}
			}
			if(j < reward.stage_size())
			{
				return reward.stage(j).id();
			}
			break;
		}
	}
	return 0;
}
void AllianceRaceCfgWrap::GetWatchAdReward(unsigned& diamond,unsigned& point,unsigned& count)
{
	const ConfigAllianceRace::RaceWatchAdReward& reward = cfg_.watch_ad_reward();
	diamond = reward.diamond();
	count = reward.count();
	point = reward.point();
}

/*-----------------Assistor--------------------------------*/
AssistorCfgWrap::AssistorCfgWrap()
	:cfg_(ConfigManager::Instance()->assistor.m_config)
{
}

const  ConfigAssistor::AssistCfg&  AssistorCfgWrap::GetAssistorCfg() const
{
	return cfg_;
}

/*-----------------activity tencent--------------------------------*/
ActivityTencentCfgWrap::ActivityTencentCfgWrap()
	:cfg_(ConfigManager::Instance()->activityTencent.m_config)
{
}

const ConfigActivityTencent::ActivityInfo& ActivityTencentCfgWrap::GetCfg() const
{
	return cfg_;
}

void ActivityTencentCfgWrap::GetNormalBlueDailyReward(uint32_t id, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.normal_blue_daily_size(); ++i)
	{
		if(cfg_.normal_blue_daily(i).id() == id)
		{
			msg->MergeFrom(cfg_.normal_blue_daily(i).item());
			return;
		}
	}
	throw runtime_error("get_normal_blue_daily_reward_conf_fail");
}
void ActivityTencentCfgWrap::GetSuperBlueDailyReward(CommonGiftConfig::CommonModifyItem * msg)
{
	msg->MergeFrom(cfg_.super_blue_daily().item());
}
void ActivityTencentCfgWrap::GetYearBlueDailyReward(CommonGiftConfig::CommonModifyItem * msg)
{
	msg->MergeFrom(cfg_.year_blue_daily().item());
}
void ActivityTencentCfgWrap::GetBlueGrowReward(uint32_t id, uint32_t level, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.blue_grow_size(); ++i)
	{
		if(cfg_.blue_grow(i).id() == id && cfg_.blue_grow(i).level() <= level)
		{
			msg->MergeFrom(cfg_.blue_grow(i).item());
			return;
		}
	}
	throw runtime_error("get_blue_grow_reward_conf_fail");
}
void ActivityTencentCfgWrap::GetQQgamePrivilegeDailyReward(CommonGiftConfig::CommonModifyItem * msg)
{
	msg->MergeFrom(cfg_.qqgame_privilege_daily().item());
}
void ActivityTencentCfgWrap::GetQQgamePrivilegeGrowReward(uint32_t id, uint32_t level, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.qqgame_privilege_grow_size(); ++i)
	{
		if(cfg_.qqgame_privilege_grow(i).id() == id && cfg_.qqgame_privilege_grow(i).level() <= level)
		{
			msg->MergeFrom(cfg_.qqgame_privilege_grow(i).item());
			return;
		}
	}
	throw runtime_error("get_qqgame_privilege_grow_reward_conf_fail");
}

/*----------------- fee theme --------------------------------*/
ThemeCfgWrap::ThemeCfgWrap()
	:cfg_(ConfigManager::Instance()->theme.m_config)
{
}

const ConfigTheme::Conf& ThemeCfgWrap::GetCfg() const
{
	return cfg_;
}
void ThemeCfgWrap::GetThemeCost(uint32_t themeId, uint32_t type, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.theme_size(); ++i)
	{
		const ConfigTheme::ThemeInfo& ti = cfg_.theme(i);
		if(ti.themeid() == themeId)
		{
			for(uint32_t j = 0; j < ti.item_size(); ++j)
			{
				const ConfigTheme::ThemeInfoItem& item = ti.item(j);
				if(item.type() == type)
				{
					msg->CopyFrom(item.cost());
					return;
				}
			}
		}
	}
	throw runtime_error("get_theme_cost_fail");
}

/*************************宠物配置*********************************/
PetCfgWrap::PetCfgWrap()
	:cfg_(ConfigManager::Instance()->pet.m_config)
{
}

const ConfigPet::PetCPP & PetCfgWrap::GetPetCfgByPetId(unsigned petid)
{
	if(!ConfigManager::Instance()->m_petmap.count(petid))
	{
		throw std::runtime_error("petid_param_error");
	}

	unsigned index = ConfigManager::Instance()->m_petmap[petid];
	return cfg_.pet(index);
}

/*----------------- keeper --------------------------------*/
KeeperCfgWrap::KeeperCfgWrap()
	:cfg_(ConfigManager::Instance()->keeper.m_config)
{
}

const ConfigKeeper::KeeperCfg& KeeperCfgWrap::GetCfg() const
{
	return cfg_;
}

void KeeperCfgWrap::GetUpgradeCost(uint32_t level, CommonGiftConfig::CommonModifyItem * msg)
{
	for(uint32_t i = 0; i < cfg_.upgrade_size(); ++i)
	{
		if(level == i + 1)
		{
			const ConfigKeeper::KeeperUpgradeItem& uItem = cfg_.upgrade(i);
			msg->CopyFrom(uItem.item());
			return;
		}
	}
	throw runtime_error("get_keeper_upgrade_cost_fail");
}
void KeeperCfgWrap::GetExp(uint32_t level, uint32_t& exp)
{
	for(uint32_t i = 0; i < cfg_.upgrade_size(); ++i)
	{
		if(level == i + 1)
		{
			const ConfigKeeper::KeeperUpgradeItem& uItem = cfg_.upgrade(i);
			exp = uItem.exp();
			return;
		}
	}
	throw runtime_error("get_keeper_upgrade_exp_fail");
}
void KeeperCfgWrap::GetMax(uint32_t level, uint32_t& max)
{
	for(uint32_t i = 0; i < cfg_.upgrade_size(); ++i)
	{
		if(level == i + 1)
		{
			const ConfigKeeper::KeeperUpgradeItem& uItem = cfg_.upgrade(i);
			max = uItem.max();
			return;
		}
	}
	throw runtime_error("get_keeper_upgrade_max_fail");
}
void KeeperCfgWrap::GetCap(uint32_t level, uint32_t& cap)
{
	for(uint32_t i = 0; i < cfg_.upgrade_size(); ++i)
	{
		if(level == i + 1)
		{
			const ConfigKeeper::KeeperUpgradeItem& uItem = cfg_.upgrade(i);
			cap = uItem.cap();
			return;
		}
	}
	throw runtime_error("get_keeper_upgrade_cap_fail");
}
unsigned KeeperCfgWrap::ExistTarget(set<uint32_t>& id)
{
	map<uint32_t, uint32_t>& allTarget = ConfigManager::Instance()->m_keeperTarget;

	map<uint32_t, uint32_t>::iterator aIter = allTarget.begin();
	set<uint32_t>::iterator iter = id.begin();

	while(aIter != allTarget.end() && iter != id.end())
	{
		uint32_t allId = aIter->first;
		uint32_t taskId = *iter;
		if(allId == taskId)
		{
			++aIter;
			++iter;
		}
		else if(allId < taskId)
		{
			++aIter;
		}
		else
		{
			return taskId;
		}
	}
	return (iter == id.end()) ? 0 : (*iter);
}
bool KeeperCfgWrap::GetProductBuild(uint32_t taskId, uint32_t& buildId)
{
	map<unsigned, unsigned>& keeperBuild = ConfigManager::Instance()->m_keeperBuild;
	if(keeperBuild.find(taskId) != keeperBuild.end())
	{
		buildId = keeperBuild[taskId];
		return true;
	}
	return false;
}
const map<unsigned, KeeperMaterial>&  KeeperCfgWrap::GetDependProduct(uint32_t materialId)const
{
	map<unsigned, map<unsigned, KeeperMaterial> >::iterator iter = ConfigManager::Instance()->m_keeperMaterial.find(materialId);
	if(iter == ConfigManager::Instance()->m_keeperMaterial.end())
	{
		error_log("not_exist_material id=%u", materialId);
		throw runtime_error("not_exist_material");
	}
	return iter->second;
}
bool KeeperCfgWrap::ExistDependProduct(uint32_t materialId)
{
	return ConfigManager::Instance()->m_keeperMaterial.find(materialId) != ConfigManager::Instance()->m_keeperMaterial.end();
}
unsigned KeeperCfgWrap::GetProductKeeperId(uint32_t productId)
{
	map<unsigned, unsigned >& m_targetKeeper = ConfigManager::Instance()->m_targetKeeper;
	map<unsigned, unsigned >::iterator iter = m_targetKeeper.find(productId);
	if(iter == m_targetKeeper.end())
	{
		error_log("not_exist_product productId=%u", productId);
		throw runtime_error("not_exist_product");
	}
	return iter->second;
}
bool KeeperCfgWrap::GetProductTaskId(uint32_t productId, uint32_t& taskId)
{
	map<unsigned, unsigned >::iterator iter = ConfigManager::Instance()->m_rKeeperTarget.find(productId);
	if(iter != ConfigManager::Instance()->m_rKeeperTarget.end())
	{
		taskId = iter->second;
		return true;
	}
	return false;
}
