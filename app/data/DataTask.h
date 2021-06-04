#ifndef DATA_TASK_H_
#define DATA_TASK_H_

#include "Kernel.h"

struct DataTask
{
    uint32_t uid;
    uint32_t id;   //任务id
    uint32_t cur_task_value;//当前任务数值
    uint32_t cur_task_star; //当前任务星级
    uint32_t reward_status;//任务领取状态(0:未领取,1:已领取)

    DataTask():
    	uid(0),
    	id(0),
    	cur_task_value(0),
    	cur_task_star(0),
    	reward_status(0)
	{

	}

    DataTask(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	cur_task_value(0),
    	cur_task_star(0),
    	reward_status(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_id(id);
		msg->set_curtaskvalue(cur_task_value);
		msg->set_curtaskstar(cur_task_star);
		msg->set_rewardstatus(reward_status);
	}

    void FromMessage(const ProtoUser::TaskCPP * msg)
    {
    	cur_task_value = msg->curtaskvalue();
    	cur_task_star = msg->curtaskstar();
    	reward_status = msg->rewardstatus();
    }
};

class CDataTask :public DBCBase<DataTask, DB_TASK>
{
public:
	virtual int Get(DataTask &data);
	virtual int Get(vector<DataTask> &data);
	virtual int Add(DataTask &data);
	virtual int Set(DataTask &data);
	virtual int Del(DataTask &data);
};

#endif //DATA_TASK_H_
