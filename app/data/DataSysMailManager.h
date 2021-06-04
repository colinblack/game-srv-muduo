/*
 * DataSysMailManager.h
 *
 *  Created on: 2018-2-8
 *      Author: Ralf
 */

#ifndef DATASYSMAILMANAGER_H_
#define DATASYSMAILMANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "MemorySysMail.h"
class DataSysmailManager: public DBCMultipleTemplate<DataSysmail, DB_SYSMAIL,
			DB_SYSMAIL_FULL, CDataSysmail>, public CSingleton<DataSysmailManager> {
public:
	virtual void CallDestroy() {
		Destroy();
	}
	const char* name() const {
		return "DataSysmailManager";
	}
};

#endif /* DATASYSMAILMANAGER_H_ */
