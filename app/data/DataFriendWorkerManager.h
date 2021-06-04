/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_FRIENDWORKER_MANAGER_H_
#define DATA_FRIENDWORKER_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataFriendWorker.h"

class DataFriendWorkerManager
	: public DBCMultipleTemplate<DataFriendWorker, DB_FRIENDWORKER, DB_FRIENDWORKER_FULL, CDataFriendWorker>
	, public CSingleton<DataFriendWorkerManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataFriendWorker"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};
#endif /* DATA_FRIENDWORKER_MANAGER_H_ */
