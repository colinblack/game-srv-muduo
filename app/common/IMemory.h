/*
 * IMemory.h
 *
 *  Created on: 2011-9-8
 *      Author: dada
 */

#ifndef IMEMORY_H_
#define IMEMORY_H_

#include "IBase.h"

class IMemory : IBase
{
public:
	virtual void *GetAddress() const = 0;
	virtual int GetSize() const = 0;
};

#endif /* IMEMORY_H_ */
