#ifndef MEMORY_ACTIVITY_MANAGER_H_
#define MEMORY_ACTIVITY_MANAGER_H_

#include "Kernel.h"



/**********************MemoryActivityManager*****************************/
struct DataMemoryActivityItem
{
	uint8_t isOn;	// 活动是否开启(0:没有,1:有)
	uint8_t isSettle;	// 是否结算(0:没有,1:有)
};
struct DataMemoryActivity
{
	DataMemoryActivityItem item[MEMORY_ACTIVITY_FULL];
};

class MemoryActivityManager : public MemorySingleton<DataMemoryActivity, MEMORY_ACTIVITY>, public CSingleton<MemoryActivityManager>
{
private:
	friend class CSingleton<MemoryActivityManager>;

	MemoryActivityManager()
	{
	};

	virtual ~MemoryActivityManager(){}

public:
	virtual int OnInit();
	virtual void CallDestroy() {Destroy();}
	bool IsOn(uint32_t actId);
	void SetOn(uint32_t actId, uint8_t isOn);
	bool Settle(uint32_t actId);
};


#endif /* MEMORY_ACTIVITY_MANAGER_H_ */
