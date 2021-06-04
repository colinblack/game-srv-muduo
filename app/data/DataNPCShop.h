#ifndef DATA_NPCSHOP_H_
#define DATA_NPCSHOP_H_
#include "Kernel.h"

struct DataNPCShop
{
    uint32_t uid;
    uint32_t id; //货架ud
    uint32_t props_id;//物品数量
    uint32_t props_cnt;//物品数量
    uint32_t sell_flag;   //出售标志. 0-未出售 1-已出售

    DataNPCShop():
    	uid(0),
    	id(0),
    	props_id(0),
    	props_cnt(0),
    	sell_flag(0)
	{

	}

    DataNPCShop(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(0),
    	props_id(0),
		props_cnt(0),
		sell_flag(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_ud(id);
		msg->set_propsid(props_id);
		msg->set_propscnt(props_cnt);
		msg->set_sellflag(sell_flag);
	}

	void ResetNPCShopShelf()
	{
		this->props_id = 0;
		this->props_cnt   = 0;
		this->sell_flag   = 0;
	}
};

class CDataNPCShop :public DBCBase<DataNPCShop, DB_NPCSHOP>
{
public:
	virtual int Get(DataNPCShop &data);
	virtual int Get(vector<DataNPCShop> &data);
	virtual int Add(DataNPCShop &data);
	virtual int Set(DataNPCShop &data);
	virtual int Del(DataNPCShop &data);
};

#endif //DATA_NPCSHOP_H_
