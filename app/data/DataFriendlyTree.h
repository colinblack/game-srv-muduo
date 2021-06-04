#ifndef DATA_FriendlyTree_H_
#define DATA_FriendlyTree_H_
#include "Kernel.h"

struct DataFriendlyTree
{
    uint32_t uid;
    uint32_t id;
    uint32_t othuid;//浇水者uid
    uint32_t ts;//浇水ts
    char name[BASE_NAME_LEN];//浇水者名字
    char fig[BASE_FIG_LEN];//浇水者头像url
    DataFriendlyTree():
    	uid(0),
    	id(0),
    	othuid(0),
    	ts(0)
	{
    	memset(name, 0, sizeof(name));
    	memset(fig, 0, sizeof(fig));
	}

    DataFriendlyTree(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	othuid(othuid),
    	ts(0)
    {
    	memset(name, 0, sizeof(name));
    	memset(fig, 0, sizeof(fig));
    }

    template<class T>
   	void SetMessage(T *msg)
   	{
   		msg->set_id(id);
   		msg->set_othuid(othuid);
   		msg->set_ts(ts);
   		msg->set_name(name,BASE_NAME_LEN);
   		msg->set_head(fig,BASE_FIG_LEN);
   	}

   void FromMessage(const ProtoUser::FriendlyTreeBasicCPP * msg)
   {
	   ts = msg->ts();
	   othuid = msg->othuid();
	   memset(name, 0, sizeof(name));
	   strncpy(name, msg->name().c_str(), sizeof(name)-1);

	   memset(fig, 0, sizeof(fig));
	   strncpy(fig, msg->head().c_str(), sizeof(fig)-1);
   }
};

class CDataFriendlyTree :public DBCBase<DataFriendlyTree, DB_FRIENDLYTREE>
{
public:
	virtual int Get(DataFriendlyTree &data);
	virtual int Get(vector<DataFriendlyTree> &data);
	virtual int Add(DataFriendlyTree &data);
	virtual int Set(DataFriendlyTree &data);
	virtual int Del(DataFriendlyTree &data);
};

#endif //DATA_FriendlyTree_H_
