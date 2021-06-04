#ifndef LOGIC_KEEPER_MANAGER_H_
#define LOGIC_KEEPER_MANAGER_H_
#include "ServerInc.h"
class LogicKeeperManager: public BattleSingleton, public CSingleton<LogicKeeperManager> {
private:
	virtual void CallDestroy() {
		Destroy();
	}
	enum
	{
		keeper_id_animal = 1,	// 动物助手ID

		keeper_task_status_unknow = 0, // 未知状态
		keeper_task_status_finish = 1, // 助手任务完成
		keeper_task_status_doing = 2, // 助手任务正在生产
		keeper_task_status_ready = 3, // 助手任务原料不足

	};
public:
	//查询农场助手
	int Process(unsigned uid, ProtoKeeper::KeeperInfoReq* req, ProtoKeeper::KeeperInfoResp* resp);
	//花钻石奖励时间
	int Process(unsigned uid, ProtoKeeper::KeeperBuyTime* req, ProtoKeeper::KeeperInfoResp* resp);
	//看广告奖励
	int Process(unsigned uid, ProtoKeeper::KeeperWatchAds* req, ProtoKeeper::KeeperInfoResp* resp);
	//设置任务
	int Process(unsigned uid, ProtoKeeper::KeeperSetTask* req, ProtoKeeper::KeeperInfoResp* resp);
	//升级
	int Process(unsigned uid, ProtoKeeper::KeeperUpgrade* req, ProtoKeeper::KeeperInfoResp* resp);
	//设置自动喂养
	int Process(unsigned uid, ProtoKeeper::KeeperSetAutoFeed* req, ProtoKeeper::KeeperSetAutoFeedResp* resp);
	//材料库存变多
	int OnAddMaterial(unsigned uid, uint32_t mId);
	//生产线空闲
	int OnAddProductLine(unsigned uid, uint32_t bud);
	//动物空闲
	int OnAddAnimalFull(unsigned uid);
	int OnAddAnimalHungry(uint32_t uid);
public:
	virtual void OnTimer2();
	uint32_t GetAnimalKeeperTs(uint32_t uid);
	uint32_t GetProductLineKeeperTs(uint32_t uid);
	bool IsKeeperOn(uint32_t uid, uint32_t keeperId);
	bool AutoFeed(uint32_t uid);
	int CheckProductLineLogin(unsigned uid, map<uint32_t, uint32_t>& finishTime, map<unsigned, set<unsigned> > &tobuild);
	int CheckAnimalLogin(unsigned uid, DataAnimal & animal, map<unsigned, set<unsigned> >& tobuild);
	bool GetKeeperTaskBuildId(uint32_t uid, set<uint32_t>& taskBuild);
	bool OnKeeperProduce(uint32_t uid, uint32_t eud, uint32_t endTs, map<unsigned, set<unsigned> > &tobuild);
	bool CheckKeeperTaskNeed(uint32_t uid, uint32_t taskId);
	void AddKeeperTaskFinish(uint32_t uid, uint32_t taskId, uint32_t count);
	int OnStorageChange(uint32_t uid);
private:
	//助手信息
	int FillKeeperInfo(uint32_t uid, uint32_t keeperId, ProtoKeeper::KeeperInfoResp* resp);
	//材料数量变化
//	void OnMaterialChange(uint32_t uid, uint32_t mId);
	void ChangeKeeperTaskStatus(uint32_t uid, uint32_t taskId, uint8_t status);
	bool CheckProductLineIdle(uint32_t productId); // 检查多台生产设备
	bool CheckMaterial(uint32_t productId);
	bool CheckTaskStatus(uint32_t productId);

	void OnProduce(uint32_t uid, map<uint32_t, map<uint32_t, uint32_t> >& building);
	//动物助手任务类型
	int GetKeeperTaskType(uint32_t taskId){ return taskId / 100;}
	int GetKeeperIdByTaskId(uint32_t taskId);
	int OnProductLine(uint32_t uid, uint32_t eud);
	int OnAnimalAutoFeed(uint32_t uid);		// 自动喂养
	int OnAnimalAutoObtain(uint32_t uid);	// 自动收获
private:
	//原材料变化
	map<uint32_t, set<uint32_t> > material; // uid->itemId
	map<uint32_t, set<uint32_t> > productLine;	// uid->bud
	set<uint32_t> animalFull;	// uid->animalud
	set<uint32_t> animalHungry;	// uid
};
#endif //LOGIC_KEEPER_MANAGER_H_
