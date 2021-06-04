#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "ISemaphore.h"
#include "ILock.h"

class CSemaphore : public ISemaphore
{
public:

	CSemaphore();
	~CSemaphore();

	bool Create(const char *name);
	bool Open(const char *name);
	bool CreateOrOpen(const char *name);
	bool Close();

	bool Signal();
	bool Wait();

private:
	void *m_sem;
};

#endif //__SEMAPHORE_H__
