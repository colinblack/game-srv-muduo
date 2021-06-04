#include "ServerInc.h"


int LogicShopSellCoinManager::SaveSellCoin(unsigned uid,unsigned sellts,unsigned coin)
{
	//获取当天起始时间
	unsigned start_ts = Time::GetDayBeginTs(sellts);

	vector<unsigned>result;
	result.clear();
	DataShopSellCoinManager::Instance()->GetIndexs(uid,result);

	bool is_exsit = DataShopSellCoinManager::Instance()->IsExistItem(uid,start_ts);
	if(!is_exsit && result.size() >= DB_SHOPSELLCOIN_FULL)
	{
		//清掉三十天前的数据
		unsigned thirtydayts =  start_ts - DB_SHOPSELLCOIN_FULL * 24 * 3600;
		bool exsit =  DataShopSellCoinManager::Instance()->IsExistItem(uid,thirtydayts);
		if(exsit)
			DataShopSellCoinManager::Instance()->DelItem(uid,thirtydayts);
	}

	DataShopSellCoin &shopsellcoin = DataShopSellCoinManager::Instance()->GetData(uid,start_ts);
	shopsellcoin.value += coin;
	DataShopSellCoinManager::Instance()->UpdateItem(shopsellcoin);

	return 0;
}
