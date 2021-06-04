#ifndef LOGIC_MailDOG_MANAGER_H
#define LOGIC_MailDOG_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicMailDogManager :public BattleSingleton, public CSingleton<LogicMailDogManager>
{
private:
	friend class CSingleton<LogicMailDogManager>;
	LogicMailDogManager(){actid = e_Activity_FriendlyTree;}

public:
	//对应此活动表存储索引
	enum{
		activiy_table_save_index_3   = 2,//用于存储领取金币的ts
	};

	virtual void CallDestroy() { Destroy();}
	//新手初始化数据
	int NewUser(unsigned uid);

	//登陆校验配置
	int CheckLogin(unsigned uid);

	//更新邮件狗相关数据
	int UpdateMailDogData(unsigned uid,int type,unsigned count);

	//每日重置邮件狗相关数据
	int ResetMailDogData(unsigned uid);

	//获取道具狗信息
	int Process(unsigned uid,ProtoMailDog::GetMailDogInfoReq *req,ProtoMailDog::GetMailDogInfoResp *resp);
private:
	//获取繁荣度
	unsigned GetProsperity(unsigned uid);

	//获取金币与经验奖励
	unsigned GetReward(unsigned uid,unsigned prosperity,unsigned & coin,unsigned & exp);

	//获取当前整点ts
	unsigned GetHourTs();

	int actid;
};


#endif //LOGIC_MailDOG_MANAGER_H
