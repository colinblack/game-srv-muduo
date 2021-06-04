#include "Common.h"
#include <semaphore.h>
#include <fcntl.h>

CSemaphoreLock::CSemaphoreLock()
{
	m_sem = SEM_FAILED;
}

CSemaphoreLock::~CSemaphoreLock()
{
	Close();
}

bool CSemaphoreLock::Create(const char *name)
{
	if(sem_unlink(name) != 0)
	{
		if(errno != ENOENT)
		{
			LogError("sem_unlink <%s> Fail [%d]", name, errno);
			return false;
		}
	}

	m_sem = sem_open(name, O_CREAT | O_EXCL, 0666, 1);
	if(m_sem == SEM_FAILED)
	{
		LogError("sem_open <%s> O_CREAT | O_EXC Fail [%d]", name, errno);
		return false;
	}
	return true;
}

bool CSemaphoreLock::Open(const char *name)
{
	m_sem = sem_open(name, 0);
	if(m_sem == SEM_FAILED)
	{
		LogError("sem_open <%s> Fail [%d]", name, errno);
		return false;
	}
	return true;
}

bool CSemaphoreLock::CreateOrOpen(const char *name)
{
	m_sem = sem_open(name, O_CREAT, 0666, 1);
	if(m_sem == SEM_FAILED)
	{
		LogError("sem_open <%s> O_CREAT Fail [%d]", name, errno);
		return false;
	}
	return true;
}

bool CSemaphoreLock::Close()
{
	if(m_sem != SEM_FAILED)
	{
		if(sem_close((sem_t *)m_sem) != 0)
		{
			LogError("sem_close Fail [%d]", errno);
			m_sem = SEM_FAILED;
			return false;
		}
		m_sem = SEM_FAILED;
	}
	return true;
}

bool CSemaphoreLock::Lock()
{
	if(m_sem == SEM_FAILED)
	{
		return false;
	}
	if(sem_wait((sem_t *)m_sem) != 0)
	{
		LogError("sem_wait Fail [%d]", errno);
		return false;
	}
	return true;
}

bool CSemaphoreLock::Unlock()
{
	if(m_sem == SEM_FAILED)
	{
		return false;
	}
	if(sem_post((sem_t *)m_sem) != 0)
	{
		LogError("sem_post Fail [%d]", errno);
		return false;
	}
	return true;
}
