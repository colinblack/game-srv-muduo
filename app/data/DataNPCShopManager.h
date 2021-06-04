/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_NPCSHOP_MANAGER_H_
#define DATA_NPCSHOP_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataNPCShop.h"

class DataNPCShopManager
	: public DBCMultipleTemplate<DataNPCShop, DB_NPCSHOP, DB_NPCSHOP_FULL, CDataNPCShop>
	, public CSingleton<DataNPCShopManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataNPCShop"; }

	 //获取新增货架的ud
	 unsigned GetNewShelfUd(unsigned uid);
};
#endif /* DATA_NPCSHOP_MANAGER_H_ */
