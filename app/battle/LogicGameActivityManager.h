#ifndef LOGIC_GAME_ACTIVITY_H
#define LOGIC_GAME_ACTIVITY_H

#include "ServerInc.h"

#define ACTIVE_MAX_NUM  5

class LogicGameActivityManager : public CSingleton<LogicGameActivityManager>
{
private:
	friend class CSingleton<LogicGameActivityManager>;
	LogicGameActivityManager() {};

public:
	void FullMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);
	unsigned FullStatusMarks(unsigned uid);
	int Process(unsigned uid, ProtoActivity::GameAcitivityStatusReq * req, ProtoActivity::GameAcitivityStatusResp* resp);
	void CheckVersion();
	static unsigned index_array[ACTIVE_MAX_NUM];
private:
	//unsigned active_marks;
};

#endif  //LOGIC_GAME_ACTIVITY_H
