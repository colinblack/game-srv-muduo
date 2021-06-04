#ifndef LOGIC_VIP_MANAGER_H_
#define LOGIC_VIP_MANAGER_H_

class LogicVIPManager :public BattleSingleton, public CSingleton<LogicVIPManager>
{
	enum{
			e_Activity_UserData_1_index_1 = 1,//VIP每日订单加成的使用的次数
			e_Activity_UserData_1_index_2 = 2,//VIP每日航运加成的使用的次数
		};
private:
	friend class CSingleton<LogicVIPManager>;
	LogicVIPManager(){userActId = e_Activity_UserData_1;}
	int userActId;
public:
	virtual void CallDestroy() { Destroy();}

	//判定是否为VIP用户
	bool isVIP(unsigned uid);

	//获取VIP等级
	int GetVIPLevel(unsigned uid);

	//根据充值金额获取VIP等级
	unsigned GetVIPLevelByCharge(unsigned uid);

	//航运等待时间缩短     waittime:原本需要的等待时间  return:返回减少的时间
	unsigned ReduceShipWaitTime(unsigned uid,const unsigned waittime);

	//货仓粮仓容量增加
	int AddStorageSpace(unsigned uid);

	//随机生成VIP礼包
	int Process(unsigned uid, ProtoVIP::RandomVIPGiftReq* req, ProtoVIP::RandomVIPGiftResp* resp);

	//VIP贵族礼包领取
	int Process(unsigned uid, ProtoVIP::GetVIPGiftReq* req, ProtoVIP::GetVIPGiftResp* resp);

	//vip生产设备生产产品使用免费次数加速
	int Process(unsigned uid, ProtoVIP::VIPProductSpeedUpReq* req, ProtoVIP::VIPProductSpeedUpResp* resp);

	//vip撕单免等待
	int Process(unsigned uid, ProtoVIP::VIPRemoveOrderCDReq* req, ProtoVIP::VIPRemoveOrderCDResp* resp);

	//VIP货架解锁
	int Process(unsigned uid, ProtoVIP::VIPShelfUnLockReq* req, ProtoVIP::VIPShelfUnLockResp* resp);

	//VIP增加上产队列
	int Process(unsigned uid, ProtoVIP::VIPAddProductQueueReq* req, ProtoVIP::VIPAddProductQueueResp* resp);

	//获取VIP奖励到的格子数
	int GetVIPRewardProductShelf(unsigned uid);

	//VIP农作物加速
	float VIPCropsSpeedUp(unsigned uid);

	//VIP生产设备加速
	float VIPProduceSpeedUp(unsigned uid);

	//VIP订单加成
	float VIPOrderBonus(unsigned uid);

	//VIP航运奖励加成
	float VIPShipRewardBonus(unsigned uid);

	//VIP商会竞赛任务次数添加
	int VIPAllianceCompetition(unsigned viplevel);

	//VIP商会竞赛积分倍数
	int VIPCompetitionIntegral(unsigned viplevel);

	//动物加速
	float VIPAnimalSpeedUp(unsigned uid);

	//每日清空VIP礼包
	void ResetVIPGift(unsigned uid,map<unsigned ,unsigned> items);
};

#endif //LOGIC_VIP_MANAGER_H_
