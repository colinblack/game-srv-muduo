/*
 * DBCBase.h
 *
 *  Created on: 2016-8-16
 *      Author: Ralf
 */

#ifndef DBCBASE_H_
#define DBCBASE_H_

#include "Common.h"
#include "DataBaseDBC.h"

/*
 * dbc连接类的模版基类
 * T1是对应数据库的结构体
 * T2是dbc的table id
 */
template<typename T1, int T2>
class DBCBase : public CDataBaseDBC
{
public:
	DBCBase(int table = T2) : CDataBaseDBC(table) {}
	virtual ~DBCBase(){};
	virtual int Get(T1 &data) {return 0;}
	virtual int Get(vector<T1> &data) {return 0;}
	virtual int Add(T1 &data) {return 0;}
	virtual int Set(T1 &data) {return 0;}
	virtual int Del(T1 &data) {return 0;}
};


#endif /* DBCBASE_H_ */
