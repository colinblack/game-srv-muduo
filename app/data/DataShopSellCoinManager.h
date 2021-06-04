/*
 *
 *  Created on: 2018-07-16
 *      Author: summer
 */

#ifndef DATA_SHOPSELLCOIN_MANAGER_H_
#define DATA_SHOPSELLCOIN_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataShopSellCoin.h"

class DataShopSellCoinManager
	: public DBCMultipleTemplate<DataShopSellCoin, DB_SHOPSELLCOIN, DB_SHOPSELLCOIN_FULL, CDataShopSellCoin>
	, public CSingleton<DataShopSellCoinManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataShopSellCoin"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};
#endif /* DATA_SHOPSELLCOIN_MANAGER_H_ */
