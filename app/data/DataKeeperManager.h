#ifndef DATA_KEEPER_MANAGER_H_
#define DATA_KEEPER_MANAGER_H_
#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataKeeper.h"
class DataKeeperManager: public DBCMultipleTemplate<DataKeeper, DB_KEEPER, DB_KEEPER_FULL, CDataKeeper>, public CSingleton<DataKeeperManager> {
public:
	virtual void CallDestroy() {
		Destroy();
	}
	const char* name() const {
		return "DataKeeperManager";
	}
};
#endif //DATA_KEEPER_MANAGER_H_

