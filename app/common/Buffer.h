/*
 * Buffer.h
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#ifndef BUFFER_H_
#define BUFFER_H_

class CBuffer : public IBuffer
{
public:

	CBuffer() :
		m_pBuffer(NULL),
		m_uSize(0),
		m_uCapacity(0),
		m_bOwn(false)
	{
	}

	~CBuffer()
	{
		if(m_bOwn && m_pBuffer != NULL)
		{
			delete m_pBuffer;
		}
	}

	CBuffer(uint32_t uSize) :
		m_uSize(0)
	{
		m_pBuffer = new byte[uSize];
		if(m_pBuffer == NULL)
		{
			m_uCapacity = 0;
			m_bOwn = false;
		}
		else
		{
			m_uCapacity = uSize;
			m_bOwn = true;
		}
	}

	CBuffer(byte *pBuffer, uint32_t uSize) :
		m_pBuffer(pBuffer),
		m_uSize(0),
		m_uCapacity(uSize),
		m_bOwn(false)
	{
	}

	//绑定pBuffer
	virtual bool Attach(byte *pBuffer, uint32_t uSize)
	{
		if(m_bOwn)
		{
			if(m_pBuffer != NULL)
			{
				delete m_pBuffer;
			}
			m_bOwn = false;
		}
		m_pBuffer = pBuffer;
		m_uSize = 0;
		m_uCapacity = uSize;
		return true;
	}

	//解绑
	virtual bool Detach()
	{
		if(m_bOwn)
		{
			if(m_pBuffer != NULL)
			{
				delete m_pBuffer;
			}
			m_bOwn = false;
		}
		m_pBuffer = NULL;
		m_uSize = 0;
		m_uCapacity = 0;
		return true;
	}

	virtual byte *GetNativeBuffer()
	{
		return m_pBuffer;
	}

	virtual const byte *GetConstBuffer() const
	{
		return m_pBuffer;
	}

	virtual uint32_t GetSize() const
	{
		return m_uSize;
	}

	virtual bool SetSize(uint32_t size)
	{
		if(size > m_uCapacity)
		{
			return false;
		}
		m_uSize = size;
		return true;
	}

	virtual uint32_t GetCapacity() const
	{
		return m_uCapacity;
	}

	virtual bool IsEmpty() const
	{
		return m_uSize == 0;
	}

	virtual bool Clear()
	{
		m_uSize = 0;
		return true;
	}

	virtual byte GetAt(uint32_t uIndex) const
	{
		if(m_pBuffer == NULL || uIndex >= m_uSize)
		{
			return 0;
		}
		return m_pBuffer[uIndex];
	}

	virtual bool SetAt(uint32_t uIndex, byte cValue)
	{
		if(m_pBuffer == NULL || uIndex >= m_uSize)
		{
			return false;
		}
		m_pBuffer[uIndex] = cValue;
		return true;
	}

	virtual bool CopyFrom(const byte *pcBuffer, uint32_t uSize)
	{
		if(m_pBuffer == NULL || pcBuffer == NULL || uSize > m_uCapacity)
		{
			return false;
		}
		memcpy(m_pBuffer, pcBuffer, uSize);
		m_uSize = uSize;
		return true;
	}

	virtual bool Append(const byte *pcBuffer, uint32_t uSize)
	{
		if(m_pBuffer == NULL || pcBuffer == NULL || m_uSize + uSize > m_uCapacity)
		{
			return false;
		}
		memcpy(m_pBuffer + m_uSize, pcBuffer, uSize);
		m_uSize += uSize;
		return true;
	}

	virtual bool Remove(uint32_t uStart, uint32_t uSize)
	{
		if(m_pBuffer == NULL || uStart + uSize > m_uSize)
		{
			return false;
		}
		memmove(m_pBuffer + uStart, m_pBuffer + uStart + uSize, uSize);
		m_uSize -= uSize;
		return true;
	}

	virtual bool GetData(byte *pBuffer, uint32_t uSize, uint32_t uIndex) const
	{
		if(m_pBuffer == NULL || pBuffer == NULL || uIndex + uSize > m_uSize)
		{
			return false;
		}
		memcpy(pBuffer, m_pBuffer + uIndex, uSize);
		return true;
	}

private:
	byte *m_pBuffer;
	uint32_t m_uSize;
	uint32_t m_uCapacity;
	bool m_bOwn;
};

#endif /* BUFFER_H_ */
