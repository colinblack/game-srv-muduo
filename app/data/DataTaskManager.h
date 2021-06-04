/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_TASK_MANAGER_H_
#define DATA_TASK_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataTask.h"

class DataTaskManager
	: public DBCMultipleTemplate<DataTask, DB_TASK, DB_TASK_FULL, CDataTask>
	, public CSingleton<DataTaskManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataTask"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};
#endif /* DATA_TASK_MANAGER_H_ */
