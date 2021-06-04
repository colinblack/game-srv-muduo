/*
 * IMessageQueue.h
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#ifndef IMESSAGEQUEUE_H_
#define IMESSAGEQUEUE_H_

#include "Common.h"

class IMessageQueue : IBase
{
public:
	virtual bool Enqueue(const IBuffer *pBuffer) = 0;
	virtual bool Dequeue(IBuffer *pBuffer) = 0;
	virtual bool Peek(IBuffer *pBuffer) const = 0;

	virtual uint32_t GetMaxCount() const = 0;
	virtual uint32_t GetCount() const = 0;
	virtual bool IsFull() const = 0;
	virtual bool IsEmpty() const = 0;
};

#endif /* IMESSAGEQUEUE_H_ */
