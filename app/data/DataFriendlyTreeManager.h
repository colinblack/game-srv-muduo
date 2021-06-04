/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_FRIENDLYTREE_MANAGER_H_
#define DATA_FRIENDLYTREE_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DataFriendlyTree.h"

class DataFriendlyTreeManager
	: public DBCMultipleTemplate<DataFriendlyTree, DB_FRIENDLYTREE, DB_FRIENDLYTREE_FULL, CDataFriendlyTree>
	, public CSingleton<DataFriendlyTreeManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataFriendlyTree"; }

	 void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	 unsigned GetNextWaterUd(unsigned uid);
};
#endif /* DATA_FRIENDLYTREE_MANAGER_H_ */
