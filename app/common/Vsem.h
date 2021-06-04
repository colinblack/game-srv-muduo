#ifndef __VSEM_H__
#define __VSEM_H__

#include "ILock.h"

#define SEM_START 2000000000
#define SEM_ID(semdat,semgroup,semserver) (SEM_START+semserver%10000*10000+semdat%100*100+(semgroup++)%100)

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

class CVsem: public ILock {
public:
	CVsem();
	~CVsem();

	bool CreateOrOpen(const char *name, int sem = 0);
	bool Close() { return true; }
	bool Create(const char *name, int sem = 0);
	bool Open(const char *name, int sem = 0);

	bool Lock();
	bool Unlock();

private:

	int m_sem;
	int _sem;
};

#endif //__VSEM_H__
