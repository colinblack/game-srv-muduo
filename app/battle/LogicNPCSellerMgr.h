#ifndef LOGIC_NPCSELLER_MANAGER_H_
#define LOGIC_NPCSELLER_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicNPCSellerManager :public BattleSingleton, public CSingleton<LogicNPCSellerManager>
{
private:
	friend class CSingleton<LogicNPCSellerManager>;
	LogicNPCSellerManager(){};

	enum{
		buy_npc_seller_props = 1,//购买npc商人物品
		ignore_npc_seller_props = 2,//忽略此次npc商人物品
	};
public:
	virtual void CallDestroy() { Destroy();}

	//初始NPC商人信息
	int NewUser(unsigned uid);

	//登录校验
	int CheckLogin(unsigned uid);

	//获取折扣
	int Process(unsigned uid, ProtoNPCSeller::GetPropsDiscountReq* req, ProtoNPCSeller::GetPropsDiscountResp* resp);

	//回应npc商人
	int Process(unsigned uid, ProtoNPCSeller::ResponseNPCSellerReq* req, ProtoNPCSeller::ResponseNPCSellerResp* resp);

	//改变商人状态
	int Process(unsigned uid, ProtoNPCSeller::ChangeNPCSellerStatusReq* req, ProtoNPCSeller::ChangeNPCSellerStatusResp* resp);

	//玩家达到一定等级之后，推送NPC商人信息
	int PushNPCSellerInfo(unsigned uid,unsigned old_level,unsigned new_level);

private:
	//生成随机数
	int GetRandom(int start,int end);

	//生成随机物品
	unsigned GetRandomPropsId(unsigned uid);

	//生成随机物品数量
	unsigned GetRandomPropsCnt();

	//生成随机折扣
	unsigned GetRandomDiscount();

	//获取下一次NPC访问的ts
	unsigned GetNPCSellerVisitTs(unsigned uid);

	//校验玩家等级是否已达到解锁NPC商人
	bool CheckLevelValid(unsigned uid);

};

#endif //LOGIC_NPCSELLER_MANAGER_H_
