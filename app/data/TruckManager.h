/*
 * TruckManager.h
 *
 *  Created on: 2018-3-5
 *      Author: Administrator
 */

#ifndef TRUCKMANAGER_H_
#define TRUCKMANAGER_H_

#include "Kernel.h"
#include "DBCSimpleTemplate.h"
#include "DBCMultipleTemplate.h"
#include "DataTruck.h"

class DataTruckManager : public DataSingleton<DataTruck, DB_TRUCK, DB_TRUCK_FULL, CDataTruck, DB_TRUCK_FULL>, public CSingleton<DataTruckManager>
{
private:
	friend class CSingleton<DataTruckManager>;
	DataTruckManager(){};
	virtual ~DataTruckManager(){}
	map<uint32_t, uint32_t> m_map;
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();
	int Init(unsigned uid);
	int CheckLogin(unsigned uid);
	DataTruck & Get(unsigned uid);
	DataTruck & GetDataTruck(unsigned uid);
	void SetMessage(uint32_t uid, User::User* reply);
	void SetMessage(uint32_t uid, ProtoArchive::UserData* msg);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	int CheckBuff(unsigned uid);
	int AddBuff(unsigned uid);
	int LoadBuff(unsigned uid);
	void DoClear(unsigned uid);
	void DoSave(unsigned uid);
	bool UpdateTruck(DataTruck & truck);
};

#endif /* TRUCKMANAGER_H_ */
