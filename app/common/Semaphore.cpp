#include "Common.h"
#include <semaphore.h>
#include <fcntl.h>

CSemaphore::CSemaphore()
{
	m_sem = SEM_FAILED;
}

CSemaphore::~CSemaphore()
{
	Close();
}

bool CSemaphore::Create(const char *name)
{
	if(sem_unlink(name) != 0)
	{
		if(errno != ENOENT)
		{
			LogError("sem_unlink <%s> Fail [%d]", name, errno);
			return false;
		}
	}

	m_sem = sem_open(name, O_CREAT | O_EXCL, 0666, 0);
	if(m_sem == SEM_FAILED)
	{
		LogError("sem_open <%s> O_CREAT | O_EXCL Fail [%d]", name, errno);
		return false;
	}
	return true;
}

bool CSemaphore::Open(const char *name)
{
	m_sem = sem_open(name, 0);
	if(m_sem == SEM_FAILED)
	{
		LogError("sem_open <%s> Fail [%d]", name, errno);
		return false;
	}
	return true;
}

bool CSemaphore::CreateOrOpen(const char *name)
{
	m_sem = sem_open(name, O_CREAT, 0666, 0);
	if(m_sem == SEM_FAILED)
	{
		LogError("sem_open <%s> O_CREAT Fail [%d]", name, errno);
		return false;
	}
	return true;
}

bool CSemaphore::Close()
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

bool CSemaphore::Wait()
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

bool CSemaphore::Signal()
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
