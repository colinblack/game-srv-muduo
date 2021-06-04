/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_MISSION_MANAGER_H_
#define DATA_MISSION_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataMission.h"

class DataMissionManager
	: public DBCMultipleTemplate<DataMission, DB_MISSION, DB_MISSION_FULL, CDataMission>
	, public CSingleton<DataMissionManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataMission"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};
#endif /* DATA_MISSION_MANAGER_H_ */
