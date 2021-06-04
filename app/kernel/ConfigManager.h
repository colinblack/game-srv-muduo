/*
 * ConfigManager.h
 *
 *  Created on: 2016-8-22
 *      Author: Ralf
 */

#ifndef CONFIGMANAGER_H_
#define CONFIGMANAGER_H_

#include "Common.h"
#include "ConfigPB.h"
#include "ConfigInc.h"

struct RateItem
{
	unsigned id;
	unsigned rate;
	bool operator< (const RateItem& other)const
	{
		return id < other.id;
	}
};
struct KeeperMaterial	// 助手生产所需原料
{
	unsigned need;	// 产品所需原料数量
	unsigned taskId;	// 助手任务ID
	unsigned buildId;	// 生产任务建筑ID
	unsigned unlockLevel;	// 解锁等级
	KeeperMaterial(): need(0), taskId(0), buildId(0), unlockLevel(0)
	{
	}
};
/*
 *  配置文件管理器，增加新配置时，把新的pb文件加入ConfigInc.h，定义新的const成员，构造函数中写入配置文件名
 */
class ConfigManager : public CSingleton<ConfigManager>
{
private:
	friend class CSingleton<ConfigManager>;
	virtual ~ConfigManager(){}
	ConfigManager()
		:demo("demo.json")
		,user("UserConfig.json")
		,activity("ActivityTime.json", false)
		,npcuser("npcUser.json")
		,builds("building.json")
		,barriers("barrier.json")
		,propsitem("item.json")
		,productline("productline.json")
		,order("order.json")
		,shop("shop.json")
	    ,advertise("farmerad.json")
		,task("task.json")
		,vip("vip.json")
		,shipping("shipping.json")
		,alliance("alliance.json")
		,npcseller("npcSeller.json")
		,reward("reward.json")
		,friendcfg("friend.json")
		,levelupUnlock("levelupUnlock.json")
		,assistor("assistor.json")
		,mapexpand("mapExpand.json")
		,allianceRace("allianceRace.json")
		,npccustomer("npcCustomer.json")
		,randombox("randombox.json")
		,maildog("mailDog.json")
		,rotarytable("rotarytable.json")
		,friendlytree("friendlytree.json")
		,activityTencent("activityTencent.json")
		,accessad("accessad.json")
		,theme("theme.json")
		,activitydata("activitydata.json")
		,keeper("keeper.json")
		,card("card.json")
		,friendworker("friendworker.json")
		,language("language.json")
		,pet("pet.json")
		,cdkey("cdKey.json")
	{
		Init();
	}

	void Init();
	static bool m_init;

public:
	static bool Reload();
	static void Fail(){m_init = false;}
	bool Inited(){return m_init;}

	const ConfigPB<Demo::Demo> demo;
	const ConfigPB<UserCfg::User> user;
	const ConfigPB<User::Activity> activity;
	const ConfigPB<ProtoNPCUser::NPCUser> npcuser;
	const ConfigPB<ConfigBuilding::Buildings> builds;
	const ConfigPB<ConfigBuilding::Barriers> barriers;
	const ConfigPB<ConfigItem::Items> 	propsitem;
	const ConfigPB<ConfigProductLine::ProductLine> 	productline;
	const ConfigPB<ConfigOrder::Base> 	order;
	const ConfigPB<ConfigShop::ShopInfo> shop;
	const ConfigPB<ConfigAd::AdInfo> advertise;
	const ConfigPB<ConfigTask::TaskInfo> task;
	const ConfigPB<ConfigVIP::VIPCfgInfo> vip;
	const ConfigPB<ConfigShipping::Shipping> shipping;
	const ConfigPB<ConfigAlliance::Alliance> alliance;
	const ConfigPB<ConfigNPCSeller::NPCSellerCfg> npcseller;
	const ConfigPB<ConfigReward::RewardList> reward;
	const ConfigPB<ConfigFriend::Friend> friendcfg;
	const ConfigPB<LevelupUnlock::LevelupUnlockCfg> levelupUnlock;
	const ConfigPB<ConfigAssistor::AssistCfg> assistor;
	const ConfigPB<ConfigMapExpand::MapExpandInfo> mapexpand;
	const ConfigPB<ConfigAllianceRace::RaceInfo> allianceRace;
	const ConfigPB<ConfigNPCCustomer::NPCCustomerInfo> npccustomer;
	const ConfigPB<ConfigRandomBox::RandomBoxInfo> randombox;
	const ConfigPB<ConfigMailDog::MailDogInfoCfg> maildog;
	const ConfigPB<ConfigRotaryTable::RotaryTableCfg>rotarytable;
	const ConfigPB<ConfigFriendlyTree::FriendlyTreeCfg>friendlytree;
	const ConfigPB<ConfigActivityTencent::ActivityInfo> activityTencent;
	const ConfigPB<ConfigAccessAd::AccessAdCfg> accessad;
	const ConfigPB<ConfigTheme::Conf> theme;
	const ConfigPB<ConfigActivity::ActivityCfg> activitydata;
	const ConfigPB<ConfigKeeper::KeeperCfg> keeper;
	const ConfigPB<ConfigCard::CardCfg> card;
	const ConfigPB<ConfigLanguage::LanguageCfg> language;
	const ConfigPB<ConfigFriendWorker::FriendWorkerCfg> friendworker;
	const ConfigPB<ConfigPet::PetGardenCfg> pet;
	const ConfigPB<CdKey::CdKeyCfg> cdkey;

	map<unsigned, unsigned> m_actmap;
	map<unsigned, unsigned > buildsFootIndex;  //id->index
	map<unsigned, pair<unsigned, unsigned> > buildsIndex;  //id->type->index  建筑
	map<unsigned, unsigned > BarrierIndex;  //id->index 障碍物
	map<unsigned, unsigned > ItemIndex;  //id->index，道具
	map<unsigned, pair<unsigned, unsigned> > productlineIndex;  //id->type->index  生产线
	map<unsigned, unsigned> m_productBuild;	// productId->buildId
	map<unsigned, unsigned> m_ordermap;
	map<unsigned, unsigned> m_taskidmap;//taskid->index
	map<unsigned, set<unsigned> > m_tasktypemap;//tasktype->index
	map<unsigned, pair<unsigned, unsigned> > m_donationIndex;  //productid->(type, index)
	map<unsigned, unsigned > m_missionIndex;  //m_missionIndex->(taskid, index)
	map<unsigned, vector<unsigned> > m_allianceRaceTask;
	map<unsigned, map<unsigned, vector<RateItem> > > m_allianceRaceStage;
	map<unsigned, vector<RateItem> > m_allianceRaceGrade;

	map<unsigned, unsigned> m_itemid_npcid;  //itemid->npcid
	map<unsigned, unsigned> m_keeperBuild;	// id->buildId
	map<unsigned, unsigned> m_keeperTarget;	// id->tid
	map<unsigned, unsigned> m_rKeeperTarget;	// tid->id
	map<unsigned, unsigned> m_targetKeeper;	// tid->keeperId
	map<unsigned, map<unsigned, KeeperMaterial> > m_keeperMaterial;	// materialId->productId->KeeperMaterial
	map<unsigned, set<unsigned> > m_productProduce;	// buildId->productId
	map<unsigned, set<unsigned> > m_animalProduce;	// buildId->produceId
	map<unsigned, unsigned > m_petmap;	// petid->index
	vector<unsigned> m_4399_recharge;//index->cash
	vector<unsigned> m_4399_daily;//index->cash


	bool GetActivity(unsigned id, User::ActivityItem& act);

	map<unsigned, unsigned> m_server;
	const Demo::Server& GetServer(unsigned server);
	bool IsServerMergeTogather(unsigned s1, unsigned s2);
	unsigned GetMainServer(unsigned server);
	bool IsNeedConnect(unsigned server);
	bool IsNeedConnectByUID(unsigned uid);
	bool IsNeedConnectByAID(unsigned aid);
};

#endif /* CONFIGMANAGER_H_ */
