#ifndef LOGIC_PRODUCTLINE_MANAGER_H
#define LOGIC_PRODUCTLINE_MANAGER_H

#include "LogicQueueManager.h"

//地块生产
class DataCropProduceRoutine : public DataRoutineBase
{
public:
	DataCropProduceRoutine(unsigned uid, unsigned endts, unsigned ud):
		DataRoutineBase(uid, endts, ud)
	{

	}

	virtual void CheckUd(unsigned buildud);

	virtual void GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type);

	//定时任务结束
	virtual void SingleRoutineEnd(unsigned buildud, ProtoPush::PushBuildingsCPP * msg);

	virtual uint32_t GetBuildType();
};

//设备生产
class DataProduceEquipRoutine : public DataRoutineBase
{
public:
	DataProduceEquipRoutine(unsigned uid, unsigned endts, unsigned ud):
		DataRoutineBase(uid, endts, ud)
	{

	}

	virtual void CheckUd(unsigned buildud);

	virtual void GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type);

	virtual void SingleRoutineEnd(unsigned buildud, ProtoPush::PushBuildingsCPP * msg);
	virtual uint32_t GetBuildType();

	//升星
	int EquipmentStarUpgrade(DataProduceequip & equipment, ProtoPush::PushBuildingsCPP * msg);
	//完成助手任务
	int FinishKeeperJob(DataProduceequip& equip);
};

//动物生产
class DataAnimalRoutine : public DataRoutineBase
{
public:
	DataAnimalRoutine(unsigned uid, unsigned endts, unsigned ud):
		DataRoutineBase(uid, endts, ud)
	{

	}

	virtual void CheckUd(unsigned animalud);

	virtual void GetPriceAndATime(unsigned animalud, int & cash, int & diffts, int &type);

	virtual void SingleRoutineEnd(unsigned animalud, ProtoPush::PushBuildingsCPP * msg);
	virtual uint32_t GetBuildType();

};

//果树生产
class DataFruitRoutine : public DataRoutineBase
{
public:
	DataFruitRoutine(unsigned uid, unsigned endts, unsigned ud):
		DataRoutineBase(uid, endts, ud)
	{

	}

	virtual void CheckUd(unsigned fruitud);

	virtual void GetPriceAndATime(unsigned fruitud, int & cash, int & diffts, int &type);

	virtual void SingleRoutineEnd(unsigned fruitud, ProtoPush::PushBuildingsCPP * msg);
};

class LogicProductLineManager :public BattleSingleton, public CSingleton<LogicProductLineManager>
{
private:
	friend class CSingleton<LogicProductLineManager>;
	LogicProductLineManager();

public:
	enum
	{
		init_queue_num = 2,  //默认队列长度
	};

	virtual void CallDestroy() { Destroy();}

	int NewUser(unsigned uid);

	int CheckLogin(unsigned uid);

	// 设置果树状态
	int SetFruitStatus(DataFruit& fruit, int8_t tmpStatus);

	//建造相关的生产线
	int ProduceAfterBuild(unsigned uid, unsigned ud, unsigned type, ProtoProduce::ProduceProductCPP * msg);

	//设备的下一步动作
	void ProduceEquipNextMove(DataProduceequip & equipment);

	//获取设备升星的时间减少百分比
	int GetTimePercent(DataEquipmentStar & datastar, const ConfigBuilding::ProduceEquipment & equipcfg);

	//判断队列是否符合自动生产条件
	bool CheckAutoProduce(uint32_t bId);

	//获取空闲的生产线
	bool GetFreeProductLine(uint32_t uid, uint32_t bId, uint32_t& bUd);

	//加入队列
	int JoinEquipQueue(unsigned uid, unsigned equipud, unsigned productid, uint32_t productType, ProtoProduce::JoinQueueResp * resp);

	//立即完成工作
	int OnFinishJobInstant(uint32_t uid, uint32_t equipud, uint32_t productId);

	//花费时间
	uint32_t GetProduceSpendTime(uint32_t uid, uint32_t equipud, uint32_t productId);
	//喂养动物花费时间
	uint32_t GetFeedAnimalSpendTime(uint32_t uid, uint32_t animalId);
	//获取饥饿动物
	int GetHurgryAnimal(uint32_t uid, uint32_t count, set<uint32_t>& mUd);

	unsigned GetBuildId(unsigned uid, unsigned equipud);

	//检查仓库容量是否足够
	bool CheckStorage(uint32_t uid, uint32_t equipud, uint32_t productId);
	//检查动物生产材料是否足够
	bool CheckAnimalMaterial(uint32_t uid, uint32_t animalId, uint32_t& materialUd);
	//仓库添加物品
	int AddFetchReward(unsigned uid, const CommonGiftConfig::CommonModifyItem& productCost, const string& reason, DataCommon::CommonItemsCPP * obj);

	//喂养动物
	int Feed(unsigned uid, unsigned animalud, unsigned fodderud, unsigned productType, ProtoProduce::FeedAnimalResp * resp);
	//立即完成喂养
	int OnFinishFeedInstant(unsigned uid, unsigned animalud, unsigned fodderud);

	//取回仓库
	int FetchBackStorage(unsigned uid, unsigned equipud, unsigned pos, ProtoProduce::FetchProductResp * resp);

	//获取物品
	int Obtain(unsigned uid, unsigned animalud, ProtoProduce::ObtainProductResp * resp);

	//助手挂起的生产设备
	int OnKeeperSuspendEquip(uint32_t uid);


	//种植
	int Process(unsigned uid, ProtoProduce::PlantCropReq* req, ProtoProduce::PlantCropResp* resp);
	//收割
	int Process(unsigned uid, ProtoProduce::ReapCropReq* req, ProtoProduce::ReapCropResp* resp);
	//农作物成熟需要的时间
	int GetCropsSpendTime(unsigned uid,unsigned cropsid);


	//生产队列扩展
	int Process(unsigned uid, ProtoProduce::ExpandQueueReq* req, ProtoProduce::ExpandQueueResp* resp);
	//加入生产队列
	int Process(unsigned uid, ProtoProduce::JoinQueueReq* req, ProtoProduce::JoinQueueResp* resp);
	//取回物品
	int Process(unsigned uid, ProtoProduce::FetchProductReq* req, ProtoProduce::FetchProductResp* resp);

	//领养动物
	int Process(unsigned uid, ProtoProduce::AdoptAnimalReq* req, ProtoProduce::AdoptAnimalResp* resp);
	//喂养动物
	int Process(unsigned uid, ProtoProduce::FeedAnimalReq* req, ProtoProduce::FeedAnimalResp* resp);
	//获取物品
	int Process(unsigned uid, ProtoProduce::ObtainProductReq* req, ProtoProduce::ObtainProductResp* resp);

	//收割果树
	int Process(unsigned uid, ProtoProduce::ReapFruitReq* req, ProtoProduce::ReapFruitResp* resp);
	//求助
	int Process(unsigned uid, ProtoProduce::SeekHelpReq* req, ProtoProduce::SeekHelpResp* resp);
	//砍树
	int Process(unsigned uid, ProtoProduce::CutFruitTreeReq* req, ProtoProduce::CutFruitTreeResp* resp);
	//提供帮助
	int Process(unsigned uid, ProtoProduce::OfferHelpReq* req);
	//跨服提供帮助
	int Process(ProtoProduce::CSOfferHelpReq* req);
	//处理跨服帮助后的返回信息
	int Process(ProtoProduce::CSOfferHelpResp* req);
	//确认帮助
	int Process(unsigned uid, ProtoProduce::ConfirmHelpReq* req, ProtoProduce::ConfirmHelpResp* resp);

	//给好友提供帮助添加动态消息 同服
	bool AddHelpTreeDyInfo(unsigned uid,unsigned other_uid);

	//给好友提供帮助添加动态消息 跨服
	bool AddHelpTreeDyInfoOverServer(unsigned uid,unsigned other_uid);

	int OnAnimalTask(unsigned uid);
	int OnProductLineTask(unsigned uid);
private:
	int OnlineCropland(unsigned uid);

	int OnlineEquipProduce(unsigned uid);

	int OnlineAnimal(unsigned uid);

	int OnlineFruit(unsigned uid);


	//---------------------地块生产
	//地块种植
	int PlantCrop(unsigned uid, unsigned corpud, vector<unsigned> & lands, ProtoProduce::PlantCropResp *resp);

	//收割
	int ReapCrop(unsigned uid, vector<unsigned> & lands, ProtoProduce::ReapCropResp * resp);

	//---------------------设备生产
	//扩充队列
	int ExpandQueue(unsigned uid, unsigned equipud, ProtoProduce::ExpandQueueResp * resp);

	//---------------------动物生产
	//领养动物
	int Adopt(unsigned uid, unsigned buildud, ProtoProduce::AdoptAnimalResp * resp);

	//---------------------果树生产
	//收割果树
	int ReapFruit(unsigned uid, unsigned treeud, ProtoProduce::ReapFruitResp * resp);

	//求助
	int SeekHelp(unsigned uid, unsigned treeud, ProtoProduce::SeekHelpResp * resp);

	//砍树
	int CutTree(unsigned uid, unsigned treeud, ProtoProduce::CutFruitTreeResp * resp);

	//提供帮助
	int OfferHelp(unsigned uid, unsigned othuid, unsigned treeud, ProtoProduce::OfferHelpResp * resp);

	//确认帮助
	int ConfirmHelp(unsigned uid, unsigned treeud, ProtoProduce::ConfirmHelpResp * resp);

	//果树成长
	int FruitGrowUP(DataFruit & fruit);

	//获取经验奖励
	int GetExpReward(unsigned productid, unsigned count, CommonGiftConfig::CommonModifyItem &reward);

	//收获后的动作：更新收获次数，是否随机产生道具
	bool RandomPropsAfterHarvest(unsigned uid, unsigned count);


	//处理设备升星
	int HandleProductRotinueOver(DataProduceequip & equipment, uint32_t productId = 0);

	//登录校验设备生产任务是否已完成
	int RotinueTaskCheck(unsigned uid,map<unsigned, set<unsigned> > &tobuild,DataProduceequip &equip, unsigned& finish_time);

};

#endif //LOGIC_PRODUCTLINE_MANAGER_H
