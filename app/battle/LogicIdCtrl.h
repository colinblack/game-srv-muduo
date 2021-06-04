#ifndef LOGICIDCTRL_H_
#define LOGICIDCTRL_H_

#include "ServerInc.h"

class CLogicIdCtrl : public CSingleton<CLogicIdCtrl>
{
private:
	friend class CSingleton<CLogicIdCtrl>;
	CLogicIdCtrl();
	CDataIdCtrl dbIdCtrl;
public:
	int GetNextId(int key, uint64_t &nextId, unsigned serverid);
	int GetCurrentId(int key, uint64_t &currId, unsigned serverid);
};

#endif /* LOGICIDCTRL_H_ */
