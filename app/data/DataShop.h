#ifndef DATA_SHOP_H_
#define DATA_SHOP_H_
#include "Kernel.h"

struct DataShop
{
    uint32_t uid;
    uint32_t id;   //商品id
    uint32_t buyer_uid;//购买者uid
    uint32_t props_id; //物品id
    uint32_t props_cnt;//物品数量
    uint32_t props_price;//物品价格
    uint32_t ad_flag;   //有无广告标志
    uint32_t sell_flag;   //出售标志. 0-未出售 1-已出售
    uint32_t vip_shelf_flag;   //VIP专属货架.0-非VIP专属货架,1-VIP专属货架
    char name[BASE_NAME_LEN];//购买者名字
    char fig[BASE_FIG_LEN];//购买者头像url

    DataShop():
    	uid(0),
    	id(0),
    	buyer_uid(0),
    	props_id(0),
    	props_cnt(0),
    	props_price(0),
    	ad_flag(0),
    	sell_flag(0),
    	vip_shelf_flag(0)
	{
    	memset(name, 0, sizeof(name));
    	memset(fig, 0, sizeof(fig));
	}

    DataShop(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	buyer_uid(0),
    	props_id(0),
		props_cnt(0),
		props_price(0),
		ad_flag(0),
		sell_flag(0),
		vip_shelf_flag(0)
    {
    	memset(name, 0, sizeof(name));
    	memset(fig, 0, sizeof(fig));
    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_ud(id);
		msg->set_propsid(props_id);
		msg->set_propscnt(props_cnt);
		msg->set_propsprice(props_price);
		msg->set_adflag(ad_flag);
		msg->set_sellflag(sell_flag);
		msg->set_vipshelfflag(vip_shelf_flag);
	}

    void FromMessage(const ProtoShop::ShopCPP *msg)
    {
    	id = msg->ud();
    	props_id = msg->propsid();
		props_cnt = msg->propscnt();
		props_price = msg->propsprice();
		ad_flag = msg->adflag();
		sell_flag = msg->sellflag();
    }

    void FromMessage(const ProtoUser::ShopCPP * msg)
    {
    	props_id = msg->propsid();
    	props_cnt = msg->propscnt();
    	props_price = msg->propsprice();
    	ad_flag = msg->adflag();
    	sell_flag = msg->sellflag();
    }

	void ResetShopShelf()
	{
		this->buyer_uid   = 0;
		this->props_id    = 0;
		this->props_cnt   = 0;
		this->props_price = 0;
		this->ad_flag     = 0;
		this->sell_flag   = 0;
		memset(name, 0, sizeof(name));
		memset(fig, 0, sizeof(fig));
	}
};

class CDataShop :public DBCBase<DataShop, DB_SHOP>
{
public:
	virtual int Get(DataShop &data);
	virtual int Get(vector<DataShop> &data);
	virtual int Add(DataShop &data);
	virtual int Set(DataShop &data);
	virtual int Del(DataShop &data);
};

#endif //DATA_SHOP_H_
