/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_SHOP_MANAGER_H_
#define DATA_SHOP_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataShop.h"

class DataShopManager
	: public DBCMultipleTemplate<DataShop, DB_SHOP, DB_SHOP_FULL, CDataShop>
	, public CSingleton<DataShopManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataShop"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	 //获取新增货架的ud
	 unsigned GetNewShelfUd(unsigned uid);
};
#endif /* DATA_SHOP_MANAGER_H_ */
