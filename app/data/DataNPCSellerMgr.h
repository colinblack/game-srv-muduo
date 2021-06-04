/*
 *
 *  Created on: 2018-03-08
 *      Author: summer
 */

#ifndef DATA_NPCSELLER_MANAGER_H_
#define DATA_NPCSELLER_MANAGER_H_

#include "Kernel.h"
#include "DBCSimpleTemplate.h"
#include "DataNPCSeller.h"

class DataNPCSellerManager
	: public DBCSimpleTemplate<DataNPCSeller, DB_NPCSELLER, CDataNPCSeller>
	, public CSingleton<DataNPCSellerManager>
{
public:
	 virtual void CallDestroy() { Destroy();}
	 const char* name() const  { return "DataNPCSeller"; }

	 void FullMessage(unsigned uid,ProtoNPCSeller::NPCSellerCPP *msg);
};
#endif /* DATA_NPCSELLER_MANAGER_H_ */
