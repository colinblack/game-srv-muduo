#ifndef DATA_KEEPER_TASK_MANAGER_H_
#define DATA_KEEPER_TASK_MANAGER_H_
#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataKeeperTask.h"
class DataKeeperTaskManager: public DBCMultipleTemplate<DataKeeperTask,DB_KEEPER_TASK, DB_KEEPER_TASK_FULL, CDataKeeperTask>,public CSingleton<DataKeeperTaskManager> {
public:
	virtual void CallDestroy() {
		Destroy();
	}
	const char* name() const {
		return "DataKeeperTaskManager";
	}
};
#endif //DATA_KEEPER_TASK_MANAGER_H_
