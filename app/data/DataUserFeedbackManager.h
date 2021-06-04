/*
 * DataSysMailManager.h
 *
 *  Created on: 2018-07-29
 *      Author: que
 */

#ifndef DATA_USER_FEEDBACK_MANAGER_H_
#define DATA_USER_FEEDBACK_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataUserFeedback.h"

class DataUserFeedbackManager: public DBCMultipleTemplate<DataFeedback, DB_FEEDBACK,
		DB_FEEDBACK_FULL, CDataFeedback>, public CSingleton<DataUserFeedbackManager> {
public:
	virtual void CallDestroy() {
		Destroy();
	}
	const char* name() const {
		return "DataUserFeedbackManager";
	}
};

#endif
