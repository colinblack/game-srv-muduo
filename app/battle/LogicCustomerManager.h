#ifndef LOGIC_CUSTOMER_MANAGER_H
#define LOGIC_CUSTOMER_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicCustomerManager :public BattleSingleton, public CSingleton<LogicCustomerManager>
{
private:
	friend class CSingleton<LogicCustomerManager>;
	LogicCustomerManager(){}

public:

	virtual void CallDestroy() { Destroy();}

	//获取NPC顾客信息
	int Process(unsigned uid,ProtoNPCCustomer::GetNPCCustomerReq *req,ProtoNPCCustomer::GetNPCCustomerResp *resp);

	//出售物品给NPC顾客
	int Process(unsigned uid,ProtoNPCCustomer::SellPropsReq *req,ProtoNPCCustomer::SellPropsResp *resp);

	//拒绝出售给NPC顾客
	int Process(unsigned uid,ProtoNPCCustomer::RefuseSellPropsReq *req,ProtoNPCCustomer::RefuseSellPropsResp *resp);

private:

	//根据等级获取NPC间隔时间
	int GetWaittime(unsigned uid);

	//获取npc顾客信息
	int GetNPCCustomerInfo(unsigned uid,unsigned & propsid,unsigned & propscnt,unsigned & nextts,ProtoNPCCustomer::GetNPCCustomerResp *resp);

	//重置NPC顾客数据
	int ResetNPCCustomerItem(unsigned uid,unsigned propsid);

	bool NPCCustomerIsEmpty(unsigned propsid,unsigned propscnt);

	//随机npc物品信息
	int RandomNPCItemInfo(unsigned uid,unsigned & propsid,unsigned & propscnt);

	//生成随机数[start-end]
	int GetRandom(int start,int end);


	int GetNPCCustomerIdListByItemId(unsigned itemid,unsigned itemcnt , set<unsigned> & npcidList);

	void FromMessage(unsigned propsid,unsigned propscnt,unsigned ts,ProtoNPCCustomer::NPCCustomerCPP *msg);

	//通过物品id获取npcid
	int GetNPCIdByPropsId(unsigned propsid);
};


#endif //LOGIC_CUSTOMER_MANAGER_H
