#ifndef __SHAREMEMORY_H__
#define __SHAREMEMORY_H__

#include "ILock.h"
#include "IShareMemory.h"
//#include "Semaphore.h"
#include "Vsem.h"

#define SEM_TYPE 0

class CShareMemory : public IShareMemory, public ILock
{
public:
	CShareMemory();
	~CShareMemory();

	bool Create(const char *name, int size, int semid = 0);
	bool Open(const char *name, int size, int semid = 0);
	bool CreateOrOpen(const char *name, int size, int semid = 0);
	bool Close();
	bool Flush();

	void *GetAddress() const;
	int GetSize() const;

	bool HasInit();
	void SetInitDone();

	bool Lock();
	bool Unlock();

private:
	void *m_pAddress;
#if SEM_TYPE == 1
	CSemaphoreLock m_lock;
#else
	CVsem m_lock;
#endif
};

#endif //__SHAREMEMORY_H__
