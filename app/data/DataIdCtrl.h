/*
 * DataIdCtrl.h
 *
 *  Created on: 2011-7-14
 *      Author: dada
 */

#ifndef DATAIDCTRL_H_
#define DATAIDCTRL_H_

#include "Kernel.h"

class CDataIdCtrl : public CDataBaseDBC
{
public:
	CDataIdCtrl(int table = DB_ID_CTRL) : CDataBaseDBC(table) {}

public:
	int SetId(int key, uint64_t curr_id, unsigned serverid);
	int GetId(int key, uint64_t &curr_id, unsigned serverid);

};

#endif /* DATAIDCTRL_H_ */
