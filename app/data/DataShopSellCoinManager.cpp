#include "DataShopSellCoinManager.h"

void DataShopSellCoinManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataShopSellCoin & shopsellcoin = GetDataByIndex(indexs[i]);

		DelItem(shopsellcoin.uid, shopsellcoin.id);
	}

	//新增新数据
	for(int i = 0; i < data.shopsellcoin_size(); ++i)
	{
		unsigned id = data.shopsellcoin(i).id();

		DataShopSellCoin & shopsellcoin = GetData(uid, id);

		shopsellcoin.FromMessage(&data.shopsellcoin(i));
	}
}
