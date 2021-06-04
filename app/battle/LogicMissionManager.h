#ifndef LOGIC_MISSION_MANAGER_H
#define LOGIC_MISSION_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicMissionManager :public BattleSingleton, public CSingleton<LogicMissionManager>
{
private:
	friend class CSingleton<LogicMissionManager>;
	LogicMissionManager(){}

public:
	virtual void CallDestroy() { Destroy();}

	enum{
		type_of_common_reward  = 0,//普通领取
		type_of_viewad_reward  = 1,//看广告领取
		type_of_errorcode_reward
	};

	//新手任务初始化
	int NewUser(unsigned uid);

	//获取当前任务
	int Process(unsigned uid, ProtoMission::GetCurMissionReq* req, ProtoMission::GetCurMissionResp* resp);

	//领取任务奖励
	int Process(unsigned uid, ProtoMission::RewardMissionReq* req, ProtoMission::RewardMissionResp* resp);

	//添加任务
	/*
	 * tasktype:任务类型
	 * value:需要增加的数据
	 * var:condition[x,y]中的x.condtion[x,y]表示该任务中x需要y个
	 */
	int AddMission(unsigned uid,int tasktype,int value,int var = 0);
private:
	//初始化任务数据
	void InitMission(DataMission & mission,const ConfigTask::MissionCPP &cfg);

	//获取任务初始值
	unsigned GetMissionInitValue(unsigned uid,unsigned missionType,unsigned condition_x);

	//获取当前任务
	void GetCurMission(unsigned uid,ProtoMission::GetCurMissionResp* resp);

	//领取任务奖励
	void RewardMission(unsigned uid,unsigned ud,unsigned type,ProtoMission::RewardMissionResp* resp);

	//获取下一个任务
	void GetNextMission(unsigned uid,const ConfigTask::MissionCPP &curMissionCfg,ProtoMission::RewardMissionResp* resp);

	//任务完成时、推送
	void PushCompletedMission(unsigned uid,unsigned missionid,unsigned cur_value,unsigned target_value);
};


#endif //LOGIC_MISSION_MANAGER_H
