#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "Common.h"
#include "Vsem.h"

CVsem::CVsem() {
	m_sem = -1;
	_sem = 0;
}

CVsem::~CVsem() {
}

bool CVsem::Create(const char *name, int sem) {
	_sem = sem;

	m_sem = semget(sem, 1, IPC_CREAT | 0666);
	if (m_sem == -1) {
		LogError("semget <%s> IPC_CREAT Fail [%d]", name, errno);
		return false;
	}

	union semun semopts;
	semopts.val = 1;
	if (semctl(m_sem, 0, SETVAL, semopts) < 0) {
		LogError("semctl <%s> Fail [%d]", name, errno);
		return false;
	}

	return true;
}

bool CVsem::Open(const char *name, int sem) {
	_sem = sem;

	m_sem = semget(sem, 1, 0666);
	if (m_sem == -1) {
		//LogError("semget <%s> Fail [%d] ， try create", name, errno);
		return false;
	}

	return true;
}

bool CVsem::CreateOrOpen(const char *name, int sem) {
	_sem = sem;

	if (!Open(name, sem)) {
		if (!Create(name, sem))
			return false;
	}

	return true;
}

bool CVsem::Lock() {
	if (m_sem == -1) {
		LogError("Lock:%d not created", _sem);
		return false;
	}

	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;
	if (semop(m_sem, &sb, 1) == -1) {
		LogError("semop Fail [%d]", errno);
		return false;
	}

	return true;
}

bool CVsem::Unlock() {
	if (m_sem == -1) {
		LogError("Lock:%d not created", _sem);
		return false;
	}

	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;
	if (semop(m_sem, &sb, 1) == -1) {
		LogError("semop Fail [%d]", errno);
		return false;
	}

	return true;
}
