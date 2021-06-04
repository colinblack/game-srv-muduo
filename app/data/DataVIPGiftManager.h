/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_VIPGIFT_MANAGER_H_
#define DATA_VIPGIFT_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataVIPGift.h"

class DataVIPGiftManager
	: public DBCMultipleTemplate<DataVIPGift, DB_VIPGIFT, DB_VIPGIFT_FULL, CDataVIPGift>
	, public CSingleton<DataVIPGiftManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataVIPGift"; }
};
#endif /* DATA_VIPGIFT_MANAGER_H_ */
