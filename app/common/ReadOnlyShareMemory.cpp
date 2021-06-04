#include "Common.h"

#include <sys/types.h> 
#include <sys/mman.h> 
#include <fcntl.h>

struct ReadOnlyShareMemoryHead
{
	int32_t Size;
	int32_t RefCount;
	int32_t Init;
	int32_t Lock;
};

#define HEAD_SIZE sizeof(ReadOnlyShareMemoryHead)

CReadOnlyShareMemory::CReadOnlyShareMemory()
{
	m_pAddress = MAP_FAILED;
}

CReadOnlyShareMemory::~CReadOnlyShareMemory()
{
	Close();
}

bool CReadOnlyShareMemory::Create(const char *name, int size)
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

	((ReadOnlyShareMemoryHead *)m_pAddress)->Size = allocSize;
	((ReadOnlyShareMemoryHead *)m_pAddress)->RefCount = 1;
	((ReadOnlyShareMemoryHead *)m_pAddress)->Init = 0;
	((ReadOnlyShareMemoryHead *)m_pAddress)->Lock = 0;

	return true;
}

bool CReadOnlyShareMemory::Open(const char *name, int size)
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

	__sync_add_and_fetch(&(((ReadOnlyShareMemoryHead *)m_pAddress)->RefCount), 1);

	return true;
}

bool CReadOnlyShareMemory::CreateOrOpen(const char *name, int size)
{
	if(!Open(name, size))
	{
		if(!Create(name, size))
		{
			return false;
		}
	}
	return true;
}

bool CReadOnlyShareMemory::Close()
{
	if(m_pAddress == MAP_FAILED)
	{
		return true;
	}

	__sync_sub_and_fetch(&(((ReadOnlyShareMemoryHead *)m_pAddress)->RefCount), 1);

	if(m_pAddress != MAP_FAILED)
	{
		if(munmap(m_pAddress, GetSize()) != 0)
		{
			LogError("munmap Fail [%d]", errno);
		}
		m_pAddress = MAP_FAILED;
	}
	return true;
}

void *CReadOnlyShareMemory::GetAddress() const
{
	if(m_pAddress == MAP_FAILED)
	{
		return NULL;
	}
	return (unsigned char *)m_pAddress + HEAD_SIZE;
}

int CReadOnlyShareMemory::GetSize() const
{
	if(m_pAddress == MAP_FAILED)
	{
		return 0;
	}
	return ((ReadOnlyShareMemoryHead *)m_pAddress)->Size - HEAD_SIZE;
}

bool CReadOnlyShareMemory::HasInit()
{
	return ((ReadOnlyShareMemoryHead *)m_pAddress)->Init != 0;
}

void CReadOnlyShareMemory::SetInitDone()
{
	((ReadOnlyShareMemoryHead *)m_pAddress)->Init = 1;
}

