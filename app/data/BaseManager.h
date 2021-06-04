/*
 * BaseManager.h
 *
 *  Created on: 2016-8-16
 *      Author: Ralf
 */

#ifndef BASEMANAGER_H_
#define BASEMANAGER_H_

#include "Kernel.h"
#include "DataBase.h"

class BaseManager : public DataSingleton<DataBase, DB_BASE, DB_BASE_FULL, CDataBase, DB_BASE_FULL>, public CSingleton<BaseManager>
{
private:
	friend class CSingleton<BaseManager>;
	BaseManager(){};
	virtual ~BaseManager(){}

	map<unsigned, unsigned> m_map;
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();
	virtual void OnExit();
	virtual void OnTimer2();

	int CheckBuff(unsigned uid);
	int AddBuff(unsigned uid);
	int LoadBuff(unsigned uid);

	unsigned GetIndex(unsigned uid);

	void GetClear(vector<unsigned> &uids);
	void GetClear1(vector<unsigned> &uids);
	void TryClear(vector<unsigned> &uids);
	void DoClear(unsigned uid);
	void DoSave(unsigned uid);

	//throw std::runtime_error
	DataBase& Get(unsigned uid);

	bool UpdateDatabase(unsigned index);

	bool UpdateDatabase(DataBase& database);
};

#endif /* BASEMANAGER_H_ */
