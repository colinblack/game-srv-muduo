/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_PET_MANAGER_H_
#define DATA_PET_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataPet.h"

class DataPetManager
	: public DBCMultipleTemplate<DataPet, DB_PET, DB_PET_FULL, CDataPet>
	, public CSingleton<DataPetManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataPet"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};
#endif /* DATA_PET_MANAGER_H_ */
