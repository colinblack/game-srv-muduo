#ifndef __SEMAPHORELOCK_H__
#define __SEMAPHORELOCK_H__

#include "ILock.h"

class CSemaphoreLock : public ILock
{
public:
	CSemaphoreLock();
	~CSemaphoreLock();

	bool Create(const char *name);
	bool Open(const char *name);
	bool CreateOrOpen(const char *name);
	bool Close();

	bool Lock();
	bool Unlock();

private:
	void *m_sem;
};

#endif //__SEMAPHORELOCK_H__
