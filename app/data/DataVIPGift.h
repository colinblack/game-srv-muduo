#ifndef DATA_VIPGIFT_H_
#define DATA_VIPGIFT_H_
#include "Kernel.h"

struct DataVIPGift
{
    uint32_t uid;
    uint32_t id;
    uint32_t props_cnt;

    DataVIPGift():
    	uid(0),
    	id(0),
    	props_cnt(0)
	{

	}

    DataVIPGift(unsigned uid_,unsigned id_)
    	:uid(uid_),
    	id(id_),
		props_cnt(0)
    {

    }

	void SetMessage(ProtoVIP::VIPGiftCPP *msg)
	{
		msg->set_propsid(id);
		msg->set_propscnt(props_cnt);
	}
};

class CDataVIPGift :public DBCBase<DataVIPGift, DB_VIPGIFT>
{
public:
	virtual int Get(DataVIPGift &data);
	virtual int Get(vector<DataVIPGift> &data);
	virtual int Add(DataVIPGift &data);
	virtual int Set(DataVIPGift &data);
	virtual int Del(DataVIPGift &data);
};

#endif //DATA_VIPGIFT_H_
