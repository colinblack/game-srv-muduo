/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_MAILDOG_MANAGER_H_
#define DATA_MAILDOG_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataMailDog.h"

class DataMailDogManager
	: public DBCMultipleTemplate<DataMailDog, DB_MAILDOG, DB_MAILDOG_FULL, CDataMailDog>
	, public CSingleton<DataMailDogManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataMailDog"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};
#endif /* DATA_MAILDOG_MANAGER_H_ */
