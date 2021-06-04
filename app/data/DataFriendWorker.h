#ifndef DATA_FRIENDWORKER_H_
#define DATA_FRIENDWORKER_H_
#include "Kernel.h"

struct DataFriendWorker
{
    uint32_t uid;
    uint32_t id;   //长工uid
    uint32_t endts;//长工工作结束ts(0:空闲)
    uint32_t pos; //长工被安放的槽位(0:空闲,1-5分别对应五个槽位)
    uint32_t invite_ts;//长工邀请进来的ts

    DataFriendWorker():
    	uid(0),
    	id(0),
    	endts(0),
    	pos(0),
    	invite_ts(0)
	{

	}

    DataFriendWorker(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	endts(0),
    	pos(0),
    	invite_ts(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_workeruid(id);
		msg->set_workerendts(endts);
		msg->set_workerslotpos(pos);
		msg->set_workerinvitedts(invite_ts);
	}

    void FromMessage(const ProtoFriendWorker::FriendWorkerCPP *msg)
    {
    	id = msg->workeruid();
    	endts = msg->workerendts();
		pos = msg->workerslotpos();
		invite_ts = msg->workerinvitedts();
    }

    void FromMessage(const ProtoUser::FriendWorkerCPP * msg)
    {
    	endts = msg->workerendts();
    	pos = msg->workerslotpos();
    	invite_ts = msg->workerinvitedts();
    }
};

class CDataFriendWorker :public DBCBase<DataFriendWorker, DB_FRIENDWORKER>
{
public:
	virtual int Get(DataFriendWorker &data);
	virtual int Get(vector<DataFriendWorker> &data);
	virtual int Add(DataFriendWorker &data);
	virtual int Set(DataFriendWorker &data);
	virtual int Del(DataFriendWorker &data);
};

#endif //DATA_FRIENDWORKER_H_
