#ifndef DATA_ITEM_H_
#define DATA_ITEM_H_
#include "Kernel.h"

struct DataItem
{
    uint32_t uid;
    uint32_t id;   //ud
    uint32_t props_id; //道具id
    uint32_t item_cnt;  //道具数量

    DataItem():
    	uid(0),
    	id(0),
    	props_id(0),
    	item_cnt(0)
	{

	}

    DataItem(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	props_id(0),
    	item_cnt(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_ud(id);
		msg->set_propsid(props_id);
		msg->set_itemcnt(item_cnt);
	}

    void FromMessage(const ProtoUser::PropsItemCPP * msg)
    {
    	props_id = msg->propsid();
    	item_cnt = msg->itemcnt();
    }
};

class CDataItem :public DBCBase<DataItem, DB_ITEM>
{
public:
	virtual int Get(DataItem &data);
	virtual int Get(vector<DataItem> &data);
	virtual int Add(DataItem &data);
	virtual int Set(DataItem &data);
	virtual int Del(DataItem &data);
};

#endif //DATA_ITEM_H_
