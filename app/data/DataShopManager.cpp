#include"DataShopManager.h"

void DataShopManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataShop & shop = GetDataByIndex(indexs[i]);

		DelItem(shop.uid, shop.id);
	}

	//新增新数据
	for(int i = 0; i < data.shop_size(); ++i)
	{
		unsigned id = data.shop(i).ud();

		DataShop & shop = GetData(uid, id);
		shop.FromMessage(&data.shop(i));
	}
}

unsigned DataShopManager::GetNewShelfUd(unsigned uid)
{
	unsigned ud_max = 0;

	//获取用户的货架信息
	std::vector<unsigned> shopInfo;
	DataShopManager::Instance()->GetIndexs(uid,shopInfo);

	//遍历,找出最大的ud
	std::vector<unsigned>::iterator it = shopInfo.begin();
	for(; it != shopInfo.end(); it++)
	{
		DataShop &shop = DataShopManager::Instance()->GetDataByIndex(*it);
		ud_max = (ud_max < shop.id) ? shop.id : ud_max;
	}

	return ud_max + 1;
}
