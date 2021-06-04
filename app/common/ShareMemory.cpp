#include "Common.h"

#include <sys/types.h> 
#include <sys/mman.h> 
#include <fcntl.h>

struct ShareMemoryHead
{
	int32_t Size;
	int32_t RefCount;
	int32_t Init;
	int32_t Lock;
};

#define HEAD_SIZE sizeof(ShareMemoryHead)

CShareMemory::CShareMemory()
{
	m_pAddress = MAP_FAILED;
}

CShareMemory::~CShareMemory()
{
	Close();
}

bool CShareMemory::Create(const char *name, int size, int semid)
{
	if(name == NULL)
	{
		return false;
	}

	int fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
	if(fd < 0)
	{
		LogError("open <%s> O_RDWR | O_CREAT | O_EXCL Fail [%d]", name, errno);
		return false;
	}

	int allocSize = size + HEAD_SIZE;
	if(ftruncate(fd, allocSize) != 0)
	{
		LogError("ftruncate Fail [%d]", errno);
		close(fd);
		return false;
	}

	m_pAddress = mmap(NULL, allocSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(m_pAddress == MAP_FAILED)
	{
		LogError("mmap Fail [%d]", errno);
		close(fd);
		if(close(fd) != 0)
		{
			LogError("close Fail [%d]", errno);
		}
		return false;
	}
	close(fd);

	((ShareMemoryHead *)m_pAddress)->Size = allocSize;
	((ShareMemoryHead *)m_pAddress)->RefCount = 1;
	((ShareMemoryHead *)m_pAddress)->Init = 0;
	((ShareMemoryHead *)m_pAddress)->Lock = 0;
	string lockName = name;
	for(string::iterator itr = lockName.begin(); itr != lockName.end(); itr++)
	{
		if(*itr == '/')
		{
			*itr = '_';
		}
	}
	if(!m_lock.Create(lockName.c_str(),semid))
	{
		Close();
		return false;
	}
	return true;
}

bool CShareMemory::Open(const char *name, int size, int semid)
{
	if(name == NULL)
	{
		return false;
	}

	int fd = open(name, O_RDWR, 0666);
	if(fd < 0)
	{
		LogError("open <%s> O_RDWR Fail [%d]", name, errno);
		return false;
	}

	int allocSize = size + HEAD_SIZE;

	m_pAddress = mmap(NULL, allocSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(m_pAddress == MAP_FAILED)
	{
		LogError("mmap Fail [%d]", errno);
		close(fd);
		return false;
	}
	close(fd);

	string lockName = name;
	for(string::iterator itr = lockName.begin(); itr != lockName.end(); itr++)
	{
		if(*itr == '/')
		{
			*itr = '_';
		}
	}
	if(!m_lock.CreateOrOpen(lockName.c_str(),semid))
	{
		if(munmap(m_pAddress, allocSize) != 0)
		{
			LogError("munmap Fail [%d]", errno);
		}
		m_pAddress = MAP_FAILED;
		return false;
	}
	Lock();
	((ShareMemoryHead *)m_pAddress)->RefCount += 1;
	Unlock();
	return true;
}

bool CShareMemory::CreateOrOpen(const char *name, int size, int semid)
{
	if(!Open(name, size,semid))
	{
		if(!Create(name, size,semid))
		{
			return false;
		}
	}
	return true;
}

bool CShareMemory::Close()
{
	if(m_pAddress == MAP_FAILED)
	{
		return true;
	}

	Lock();
	int32_t refCount = ((ShareMemoryHead *)m_pAddress)->RefCount - 1;
	((ShareMemoryHead *)m_pAddress)->RefCount = refCount;
	Unlock();

	if(m_pAddress != MAP_FAILED)
	{
		if(munmap(m_pAddress, ((ShareMemoryHead *)m_pAddress)->Size) != 0)
		{
			LogError("munmap Fail [%d]", errno);
		}
		m_pAddress = MAP_FAILED;
	}

	m_lock.Close();

	return true;
}

bool CShareMemory::Flush()
{
	if(m_pAddress == MAP_FAILED)
	{
		return true;
	}

	Lock();
	if(msync(m_pAddress, ((ShareMemoryHead *)m_pAddress)->Size, MS_SYNC) != 0)
	{
		Unlock();
		LogError("msync Fail [%d]", errno);
		return false;
	}
	Unlock();
	return true;
}

void *CShareMemory::GetAddress() const
{
	if(m_pAddress == MAP_FAILED)
	{
		return NULL;
	}
	return (unsigned char *)m_pAddress + HEAD_SIZE;
}

int CShareMemory::GetSize() const
{
	if(m_pAddress == MAP_FAILED)
	{
		return 0;
	}
	return ((ShareMemoryHead *)m_pAddress)->Size - HEAD_SIZE;
}

bool CShareMemory::HasInit()
{
	return ((ShareMemoryHead *)m_pAddress)->Init != 0;
}

void CShareMemory::SetInitDone()
{
	((ShareMemoryHead *)m_pAddress)->Init = 1;
}

bool CShareMemory::Lock()
{
	if(m_lock.Lock())
	{
		((ShareMemoryHead *)m_pAddress)->Lock = 1;
		return true;
	}
	return false;
}

bool CShareMemory::Unlock()
{
	if(m_lock.Unlock())
	{
		((ShareMemoryHead *)m_pAddress)->Lock = 0;
		return true;
	}
	return false;
}
