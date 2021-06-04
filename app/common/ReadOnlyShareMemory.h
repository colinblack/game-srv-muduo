#ifndef __READONLYSHAREMEMORY_H__
#define __READONLYSHAREMEMORY_H__

#include "ILock.h"
#include "IShareMemory.h"
#include "Semaphore.h"

class CReadOnlyShareMemory : public IShareMemory
{
public:
	CReadOnlyShareMemory();
	~CReadOnlyShareMemory();

	bool Create(const char *name, int size);
	bool Open(const char *name, int size);
	bool CreateOrOpen(const char *name, int size);
	bool Close();
	void *GetAddress() const;
	int GetSize() const;

	bool HasInit();
	void SetInitDone();

private:
	void *m_pAddress;
};

#endif //__READONLYSHAREMEMORY_H__
