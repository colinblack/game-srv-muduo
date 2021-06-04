#include"DataNPCShopManager.h"

unsigned DataNPCShopManager::GetNewShelfUd(unsigned uid)
{
	unsigned ud_max = 0;

	std::vector<unsigned> shopInfo;
	DataNPCShopManager::Instance()->GetIndexs(uid,shopInfo);

	//遍历,找出最大的ud
	std::vector<unsigned>::iterator it = shopInfo.begin();
	for(; it != shopInfo.end(); it++)
	{
		DataNPCShop &shop = DataNPCShopManager::Instance()->GetDataByIndex(*it);
		ud_max = (ud_max < shop.id) ? shop.id : ud_max;
	}

	return ud_max + 1;
}
