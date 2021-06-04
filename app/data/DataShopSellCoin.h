#ifndef DATA_ShopSellCoin_H_
#define DATA_ShopSellCoin_H_
#include "Kernel.h"

struct DataShopSellCoin
{
    uint32_t uid;
    uint32_t id;   //当天开始的时间戳
    uint32_t value;//对应当天卖出的金币
    DataShopSellCoin():
    	uid(0),
    	id(0),
    	value(0)
	{

	}

    DataShopSellCoin(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	value(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_id(id);
		msg->set_value(value);
	}

	void FromMessage(const ProtoUser::ShopSellCoinCPP * msg)
	{
		value = msg->value();
	}
};

class CDataShopSellCoin :public DBCBase<DataShopSellCoin, DB_SHOPSELLCOIN>
{
public:
	virtual int Get(DataShopSellCoin &data);
	virtual int Get(vector<DataShopSellCoin> &data);
	virtual int Add(DataShopSellCoin &data);
	virtual int Set(DataShopSellCoin &data);
	virtual int Del(DataShopSellCoin &data);
};

#endif //DATA_ShopSellCoin_H_
