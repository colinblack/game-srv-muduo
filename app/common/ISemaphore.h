#ifndef __ISEMAPHORE_H__
#define __ISEMAPHORE_H__

#include "IBase.h"

class ISemaphore : IBase
{
public:
	virtual bool Signal() = 0;
	virtual bool Wait() = 0;
};

#endif //__ISEMAPHORE_H__
