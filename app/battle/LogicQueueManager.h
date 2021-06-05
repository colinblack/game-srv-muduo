#ifndef LOGIC_QUEUE_MANAGER_H_
#define LOGIC_QUEUE_MANAGER_H_

#include "Common.h"
#include "Kernel.h"

#include "DataInc.h"
#include "ConfigInc.h"

//队列基类
class DataRoutineBase
{
public:
	DataRoutineBase(unsigned int uid, unsigned endts, unsigned ud) :
		uid_(uid),
		endts_(endts),
		ud_(ud),
		type(0)
	{
	}

	virtual ~DataRoutineBase() {}

	//加速。立即完成系列
	virtual void CheckUd(unsigned ud);
	virtual void GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type)
	{
		cash = 0;
		diffts = 0;
		type = routine_speed_cash;
	}
	virtual int DoEnd();
	virtual uint32_t GetBuildType();
	virtual void SingleRoutineEnd(unsigned ud, ProtoPush::PushBuildingsCPP * msg) = 0;
	void setType(uint8_t t){type = t;}
	int GetLeftTime();
public:
	unsigned int uid_;
	unsigned int endts_;  //操作的结束时间
	unsigned int ud_;
	uint8_t type;
};
class LogicQueueManager :public BattleSingleton, public CSingleton<LogicQueueManager>
{
private:
	friend class CSingleton<LogicQueueManager>;
	LogicQueueManager(){}

public:
	virtual void CallDestroy() { Destroy();}
	int OnInit();
	void OnTimer1();

	//加速
	int Process(unsigned uid, User::SpeedUpReq* req, User::SpeedUpResp* resp);

	template<class T>
	int JoinRoutine(unsigned uid, unsigned endts, uint8_t type, unsigned ud);
	template<class T>
	int JoinRoutine(unsigned uid, unsigned endts, uint8_t type, set<unsigned> & uds);

	//用户离线处理
	void Offline(unsigned uid);

	//判断指定ud是否存在定时任务队列
	bool IsExistBuildRoutine(unsigned uid, unsigned ud) const;

	virtual int FinishRoutine(DataRoutineBase* proutine, unsigned ud, int diffts);

	DataRoutineBase * GetRoutineObj(unsigned uid, unsigned ud, uint8_t type);

	//任务完成
	void TaskDone(DataRoutineBase * task);
	void FinishInstant(uint32_t uid, uint32_t buildType);

private:
	//加速队列
	int SpeedUp(unsigned uid, unsigned type, unsigned method,unsigned ud, User::SpeedUpResp * resp);

	//对某个任务进行加速
	int SpeedUpTask(DataRoutineBase *pTask, int sec, bool &taskDone);
	//添加常规任务
	void AddTask(DataRoutineBase * task);
	//删除任务映射，不释放内存
	DataRoutineBase* EraseTask(uint32_t uid, uint32_t ud, uint8_t type);

private:
	//关键是要如何才能获得唯一的rid. ud并不能保证唯一.再加个类型
	map<unsigned, list<DataRoutineBase*> > userTask; // 玩家任务
};

template<class T>
int LogicQueueManager::JoinRoutine(unsigned uid, unsigned endts, uint8_t type, unsigned ud)
{
	//加入到队列
	DataRoutineBase * pTask = new T(uid, endts, ud);
	pTask->setType(type);

	// 删除旧任务
	DataRoutineBase* pOld = EraseTask(uid, ud, type);
	if(pOld != NULL)
	{
		delete pOld;
	}

	// 添加新任务
	debug_log("JoinRoutine");
	AddTask(pTask);

	return 0;
}
template<class T>
int LogicQueueManager::JoinRoutine(unsigned uid, unsigned endts, uint8_t type, set<unsigned> & uds)
{
	for(set<unsigned>::iterator iter = uds.begin(); iter != uds.end(); ++iter)
	{
		JoinRoutine<T>(uid, endts, type, *iter);
	}

	return 0;
}

#endif //LOGIC_QUEUE_MANAGER_H_
