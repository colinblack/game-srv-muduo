#ifndef LOGIC_SHOPSELLCOIN_MANAGER_H
#define LOGIC_SHOPSELLCOIN_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicShopSellCoinManager :public BattleSingleton, public CSingleton<LogicShopSellCoinManager>
{
private:
	friend class CSingleton<LogicShopSellCoinManager>;
	LogicShopSellCoinManager(){}

public:
	virtual void CallDestroy() { Destroy();}

	//添加金币
	int SaveSellCoin(unsigned uid,unsigned sellts,unsigned coin);
private:

};


#endif //LOGIC_SHOPSELLCOIN_MANAGER_H
