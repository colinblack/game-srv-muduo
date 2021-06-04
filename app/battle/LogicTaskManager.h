#ifndef LOGIC_TASK_MANAGER_H
#define LOGIC_TASK_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicTaskManager :public BattleSingleton, public CSingleton<LogicTaskManager>
{
private:
	friend class CSingleton<LogicTaskManager>;
	LogicTaskManager(){actid = e_Activity_FriendlyTree;}

	//对应此活动表存储索引
	enum{
		activiy_table_save_index_5   = 4,//对应活动表的第五个字段,用于标记不能完成成就的bug

		repair_task_debug_vesion = 20180630,//对应当前修复的数据版本
	};

public:
	virtual void CallDestroy() { Destroy();}

	//新手任务初始化
	int NewUser(unsigned uid);

	//老号登陆校验
	int CheckLogin(unsigned uid);

	//获取当前已解锁的任务
	int Process(unsigned uid, ProtoTask::GetTaskReq* req, ProtoTask::GetTaskResp* resp);

	//领取任务奖励
	int Process(unsigned uid, ProtoTask::RewardTaskReq* req, ProtoTask::RewardTaskResp* resp);

	//获取指定任务中下一星级中的最大任务值
	int GetNextTaskMaxValue(unsigned taskid,unsigned star);

	//添加任务数据
	/*
	 * tasktype:任务类型
	 * value:需要增加的数据
	 * var:condition[x,y]中的x.condtion[x,y]表示该任务中x需要y个
	 */
	int AddTaskData(unsigned uid,int tasktype,int value,int var = 0);
private:
	int UpdateTaskData(unsigned uid,int taskid,int value);

	int actid;
};


#endif //LOGIC_TASK_MANAGER_H
