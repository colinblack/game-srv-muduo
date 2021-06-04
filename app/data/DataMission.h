#ifndef DATA_MISSION_H_
#define DATA_MISSION_H_

#include "Kernel.h"

struct DataMission
{
    uint32_t uid;
    uint32_t id;   //任务id
    uint32_t cur_task_value;//当前任务数值

    DataMission():
    	uid(0),
    	id(0),
    	cur_task_value(0)
	{

	}

    DataMission(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	cur_task_value(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_id(id);
		msg->set_value(cur_task_value);
	}

    void FromMessage(const ProtoUser::MissionCPP * msg)
    {
    	cur_task_value = msg->value();
    }
};

class CDataMission :public DBCBase<DataMission, DB_MISSION>
{
public:
	virtual int Get(DataMission &data);
	virtual int Get(vector<DataMission> &data);
	virtual int Add(DataMission &data);
	virtual int Set(DataMission &data);
	virtual int Del(DataMission &data);
};

#endif //DATA_MISSION_H_
