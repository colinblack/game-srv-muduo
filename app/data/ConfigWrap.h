/*
 * ConfigWrap.h
 *
 *  Created on: 2016-9-12
 *      Author: dawx62fac
 */

#ifndef USERCFGWRAP_H_
#define USERCFGWRAP_H_

#include "DataInc.h"


/*
 * 使用接口必须先重载DataType的 operator<() 函数,sData需要去重
 * DataType需要成员变量rate
 * randCount:随机物品数
 * sData中的数据不重复
 */
template<typename DataType>
bool RouletteWheel(uint32_t randCount, const vector<DataType>& vData, set<DataType>& sData)
{
	if(randCount == 0 || vData.empty() || randCount > vData.size())
	{
		return false;
	}
	int32_t maxRate = 0;
	typename vector<DataType>::const_iterator iter = vData.begin();
	for(; iter != vData.end(); ++iter)
	{
		maxRate += iter->rate;
	}
	if(maxRate == 0)
	{
		return false;
	}
	for(uint32_t i = 0; i < randCount && maxRate > 0; ++i)	// 随机randCount个物品
	{
		uint32_t randRate = Math::GetRandomInt(maxRate);
		typename vector<DataType>::const_iterator iter = vData.begin();
		for(; iter != vData.end() && randRate >= 0; ++iter)
		{
			const DataType& item = *iter;
			if(sData.find(item) != sData.end())	// 跳过已选物品
			{
				continue;
			}
			if(randRate < item.rate)
			{
				maxRate -= item.rate;
				sData.insert(item);
				break;
			}
			randRate -= item.rate;
		}
	}
	return sData.size() == randCount;
}


class UserCfgWrap
{
public:
	UserCfgWrap();

	const UserCfg::User& User() const;
	const UserCfg::UserBase& UserBase() const;

	//获取金币购买的配置
	const UserCfg::CoinPurchase & GetCoinPurchaseCfg(unsigned id) const;
private:
	const UserCfg::User& cfg_;
};

class ActivityCfgWrap
{
public:
	ActivityCfgWrap();

	const User::ActivityItem& GetItem(unsigned id) const;
private:
	const User::Activity& cfg_;
};

class BuildCfgWrap
{
public:
	enum
	{
		build_type_len = 10000, //建筑id类型映射关系
	};

	BuildCfgWrap();

	const ConfigBuilding::Buildings & GetBuildingCfg() const {return cfg_;}

	//获取设备升星的解锁等级
	int GetUnlockUpgradeStarLevel() const;

	int GetBuildType(unsigned build_id) const;
	//根据用户等级，获取对应的下标
	int GetLevelIndex(const google::protobuf::RepeatedField< ::google::protobuf::uint32 >& needlevel, unsigned lv);

	//获取数目对应的消耗
	int GetGainNumIndex(const google::protobuf::RepeatedField< ::google::protobuf::uint32 >& gain_num, unsigned num);

	//获取房子的配置
	const ConfigBuilding::House & GetHouseCfg() const;

	//判断是否是仓库
	bool IsStorage(unsigned build_id);

	//获取仓库id
	const ConfigBuilding::StorageHouse & GetStorageCfg(unsigned index) const;

	//获取建筑占据的格子数
	int GetFootPrint(unsigned build_id, vector<unsigned> & foots);

	//根据建筑id，获取仓库配置
	const ConfigBuilding::StorageHouse & GetStorageCfgById(unsigned id) const;

	//根据建筑id，获取农地配置
	const ConfigBuilding::CropLand & GetCropLandCfg() const;

	//获取动物住所配置
	const ConfigBuilding::AnimalResidence & GetAnimalResidenceCfgById(unsigned id) const;

	//获取动物配置
	const ConfigBuilding::Animal & GetAnimalCfgById(unsigned id) const;

	//根据建筑id，获取生产设备配置
	const ConfigBuilding::ProduceEquipment & GetProduceCfgById(unsigned id) const;

	//根据建筑id,获取果树配置
	const ConfigBuilding::FruitTree & GetFruitTreeCfgById(unsigned id) const;

	//根据建筑id,获取装饰配置
	const ConfigBuilding::Decoration & GetDecorationCfgById(unsigned id) const;

private:
	const ConfigBuilding::Buildings & cfg_;
};

class BarrierCfgWrap
{
public:
	BarrierCfgWrap();

	const ConfigBuilding::Barriers & GetBarrierCfg() const {return cfg_;}

	//获取建筑占据的格子数
	int GetFootPrint(unsigned barrier_id, vector<unsigned> & foots);

	const ConfigBuilding::Barrier & GetBarrierCfg(unsigned barrier_id) const;

	const ConfigBuilding::DestoryBarrier & GetDestoryBarrierCfg(unsigned barrier_id) const;

private:
	const ConfigBuilding::Barriers & cfg_;
};

class ItemCfgWrap
{
public:
	ItemCfgWrap();

	//获取道具配置
	const ConfigItem::PropItem & GetPropsItem(unsigned propsid) const;

	//NPC商店:通过玩家等级、获取类型为type的配置
	int GetUnlockNPCShopItem(unsigned type,unsigned level,vector<unsigned> &items) const;

	//NPC顾客:通过npcid获取对应的物品列表
	int GetPropsListByNPCId(unsigned npcid,vector<unsigned> &items) const;

	//NPC顾客:通过itemid获取npcid
	int GetNPCIdByItemId(unsigned itemid) const;

	//获取已解锁的航运物品
	int GetUnlockShippingItem(unsigned level, vector<unsigned> & items) const;

	//获取指定仓库已解锁的物品
	int GetUnlockItem(unsigned level,vector<unsigned> & items,unsigned location) const;
private:
	const ConfigItem::Items & cfg_;
};

//------------------生产线配置-------------
class ProductlineCfgWrap
{
public:
	ProductlineCfgWrap();

	const ConfigProductLine::ProductLine & GetProductLineCfg() const {return cfg_;}

	//获取设备的生产线配置
	const ConfigProductLine::EquipLine & GetEquiplineCfg(unsigned equipid) const;

	int GetEquipProductIndex(unsigned equipid, unsigned productid) const;

	//获取动物的生产线配置
	const ConfigProductLine::AnimalLine & GetAnimallineCfg(unsigned animalid) const;

	//获取果树生产线配置
	const ConfigProductLine::FruitLine & GetFruitlineCfg(unsigned treeid) const;

	//获取随机材料配置
	const ConfigProductLine::MaterailReward & GetMaterialCfg() const;

	//产品生产线
	void GetProductProduce(uint32_t buildId, set<uint32_t>& productId);

	// 动物生产线
	void GetAnimalProduce(uint32_t animalId, set<uint32_t>& productId);

	// 是否存在改动物
	bool ExistAnimal(uint32_t animalId);

private:
	const ConfigProductLine::ProductLine & cfg_;
};

class OrderCfgWrap
{
public:
	OrderCfgWrap();

	const ConfigOrder::Base & GetOrderCfg() const {return cfg_;}
private:
	const ConfigOrder::Base & cfg_;
};

//商店配置
class ShopCfgWrap
{
public:
	ShopCfgWrap();

	const ConfigShop::ShopCPP & GetShopInfoCfg() const;

	const ConfigShop::NPCShopCPP & GetNPCShopInfoCfg() const;

private:
	const ConfigShop::ShopInfo & cfg_;
};

//广告配置
class AdCfgWrap
{
public:
	AdCfgWrap();

	const ConfigAd::AdCPP & GetAdInfoCfg() const;

private:
	const ConfigAd::AdInfo & cfg_;
};

//任务配置
class TaskCfgWrap
{
public:
	TaskCfgWrap();

	const ConfigTask::TaskInfo & GetAllTaskCfg() const;

	//根据索引获取特定任务配置
	const ConfigTask::TaskCPP & GetTaskInfoCfg(unsigned taskid) const;
	//根据任务类型获取任务索引列表配置
	void GetTaskIndexsCfg(unsigned tasktype, set<unsigned> & ilist) const;
	//根据索引获取特定任务配置
	const ConfigTask::MissionCPP & GetMissionInfoCfg(unsigned missionid) const;

private:
	const ConfigTask::TaskInfo & cfg_;
};

//vip配置
class VIPCfgWrap
{
public:
	VIPCfgWrap();

	const ConfigVIP::VIPCfgInfo & GetVIPInfoCfg() const;

private:
	const ConfigVIP::VIPCfgInfo & cfg_;
};

//好友配置
class FriendCfgWrap
{
public:
	FriendCfgWrap();

	const ConfigFriend::Friend & GetFriendCfg() const{return cfg_;}

	const ConfigFriend::LevelNums & GetLevelCfgByLevel(unsigned level) const;
private:
	const ConfigFriend::Friend & cfg_;
};

//航运配置
class ShippingCfgWrap
{
public:
	ShippingCfgWrap();

	const ConfigShipping::ShippingItem & GetShippingCfg() const{return cfg_.shipping();}

private:
	const ConfigShipping::Shipping & cfg_;
};

//商会配置
class AllianceCfgWrap
{
public:
	AllianceCfgWrap();

	const ConfigAlliance::Alliance & GetAllianceCfg() const{return cfg_;}

	const ConfigAlliance::Donation & GetDonationCfg() const {return cfg_.donation();}

	//根据物品id，获取捐收物品类型的下标
	int GetDonationPropsTypeIndex(unsigned propsid) const;

private:
	const ConfigAlliance::Alliance & cfg_;
};

//NPC商人配置
class NPCSellerCfgWrap
{
public:
	NPCSellerCfgWrap();

	const ConfigNPCSeller::NPCSellerCfg & GetNPCSellerCfg() const;

private:
	const ConfigNPCSeller::NPCSellerCfg & cfg_;
};

//cdkey配置
class CdKeyCfgWrap{
public:
	CdKeyCfgWrap();
	const CdKey::CdKeyCfg & GetCfg() const;
private:
	const CdKey::CdKeyCfg & cfg_;
};

//等级解锁配置
class LevelupUnlockCfgWrap{
public:
	LevelupUnlockCfgWrap();
	const LevelupUnlock::LevelupUnlockCfg& GetLevelupUnlockCfg() const;
private:
	const LevelupUnlock::LevelupUnlockCfg& cfg_;
};
//商会竞赛配置
class AllianceRaceCfgWrap
{
public:
	AllianceRaceCfgWrap();

	const ConfigAllianceRace::RaceInfo & GetCfg() const{return cfg_;}
	//根据物品id，获取捐收物品类型的下标
	unsigned GetRandTaskId(unsigned storageId) const;
	unsigned GetTaskChance(unsigned raceLevel) const;
	void GetTaskInfo(unsigned storageId, unsigned id, unsigned& t, unsigned& level) const;
	const ConfigAllianceRace::RaceTaskStorageItem& GetTaskItem(unsigned storageId, unsigned id) const;
	void GetGradeReward(uint32_t uid, uint32_t levelId, uint32_t rankId, set<uint32_t>& id, CommonGiftConfig::CommonModifyItem * msg);
	void GetStageReward(uint32_t uid, uint32_t levelId, vector<uint32_t>& stageId, CommonGiftConfig::CommonModifyItem * msg);
	void RefreshGradeReward(uint32_t levelId, uint8_t rewardArr[], uint32_t rewardArrSize);
	void RefreshStageReward(uint32_t levelId, uint8_t stageId[], uint32_t stageIdSize);
	void AddReward(float rate, set<uint32_t>& id,
			const ::google::protobuf::RepeatedPtrField< ::CommonGiftConfig::CommonModifyItemRate >& items,
			CommonGiftConfig::CommonModifyItem * msg);
	const vector<RateItem>& GetRaceStageReward(uint32_t levelId, uint32_t stageId);
	const vector<RateItem>& GetRaceGradeReward(uint32_t levelId);
	uint32_t GetRaceRewardLevelId(uint32_t level);
	uint32_t getRankPoint(uint32_t raceLevel, uint32_t rankId);
	uint32_t GetRaceRewardStageId(uint32_t userLevel, uint32_t raceLevel, uint32_t point);
	void GetWatchAdReward(unsigned& diamond,unsigned& point,unsigned& count);

private:
	const ConfigAllianceRace::RaceInfo & cfg_;
};

//物品助手配置
class AssistorCfgWrap{
public:
	AssistorCfgWrap();
	const ConfigAssistor::AssistCfg& GetAssistorCfg() const;
private:
	const ConfigAssistor::AssistCfg& cfg_;
};
//腾讯活动配置
class ActivityTencentCfgWrap{
public:
	ActivityTencentCfgWrap();
	const ConfigActivityTencent::ActivityInfo& GetCfg() const;
	void GetNormalBlueDailyReward(uint32_t id, CommonGiftConfig::CommonModifyItem * msg);
	void GetSuperBlueDailyReward(CommonGiftConfig::CommonModifyItem * msg);
	void GetYearBlueDailyReward(CommonGiftConfig::CommonModifyItem * msg);
	void GetBlueGrowReward(uint32_t id, uint32_t level, CommonGiftConfig::CommonModifyItem * msg);
	void GetQQgamePrivilegeDailyReward(CommonGiftConfig::CommonModifyItem * msg);
	void GetQQgamePrivilegeGrowReward(uint32_t id, uint32_t level, CommonGiftConfig::CommonModifyItem * msg);

private:
	const ConfigActivityTencent::ActivityInfo& cfg_;
};

//付费主题
class ThemeCfgWrap{
public:
	ThemeCfgWrap();
	const ConfigTheme::Conf& GetCfg() const;
	void GetThemeCost(uint32_t themeId, uint32_t type, CommonGiftConfig::CommonModifyItem * msg);
private:
	const ConfigTheme::Conf& cfg_;
};

//农场助手
class KeeperCfgWrap{
public:
	KeeperCfgWrap();
	const ConfigKeeper::KeeperCfg& GetCfg() const;
	void GetUpgradeCost(uint32_t level, CommonGiftConfig::CommonModifyItem * msg);
	void GetExp(uint32_t level, uint32_t& exp);
	void GetMax(uint32_t level, uint32_t& max);
	void GetCap(uint32_t level, uint32_t& cap);
	unsigned ExistTarget(set<uint32_t>& id);
	bool GetProductBuild(uint32_t taskId, uint32_t& buildId);
	const map<unsigned, KeeperMaterial>& GetDependProduct(uint32_t materialId)const;
	bool ExistDependProduct(uint32_t materialId);
	unsigned GetProductKeeperId(uint32_t productId);
	bool GetProductTaskId(uint32_t productId, uint32_t& taskId);
private:
	const ConfigKeeper::KeeperCfg& cfg_;
};

//宠物配置
class PetCfgWrap{
public:
	PetCfgWrap();

	//通过宠物id获取配置
	const ConfigPet::PetCPP & GetPetCfgByPetId(unsigned petid);
private:
	const ConfigPet::PetGardenCfg & cfg_;
};
#endif /* USERCFGWRAP_H_ */
