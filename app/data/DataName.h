/*
 * DataName.h
 *
 *  Created on: 2015-5-12
 *      Author: Cream
 */

#ifndef DATANAME_H_
#define DATANAME_H_

#include "Kernel.h"

class CDataName : public CDataBaseDBC
{
public:
	CDataName(int table = DB_NAME) : CDataBaseDBC(table) {}

public:
	int AddName(const string &name, const string &open_id, unsigned uid);
	int GetName(const string &name, string &open_id, unsigned &uid);
	int DelName(const string &name);
};

#endif /* DATANAME_H_ */
