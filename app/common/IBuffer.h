/*
 * IBuffer.h
 *
 *  Created on: 2012-2-8
 *      Author: dada
 */

#ifndef IBUFFER_H_
#define IBUFFER_H_


class IBuffer : public IBase
{
public:

	/// 获得原始Buffer
	virtual byte *GetNativeBuffer() = 0;

	/// 获得常量Buffer
	virtual const byte *GetConstBuffer() const = 0;

	/// 获得数据大小
	virtual uint32_t GetSize() const = 0;

	/// 设置数据大小
	virtual bool SetSize(uint32_t size) = 0;

	/// 获得缓冲区容量
	virtual uint32_t GetCapacity() const = 0;

	/// 获取剩余缓冲区容量
	virtual uint32_t GetFreeCapacity() const;
//	{
//		return GetCapacity() - GetSize();
//	}
	/// Buffer是否为空
	virtual bool IsEmpty() const = 0;

	/// 清空当前buffer
	virtual bool Clear() = 0;

	/// 获得uIndex位的数据
	virtual byte GetAt(uint32_t uIndex) const = 0;

	/// 设置uIndex位的数据
	virtual bool SetAt(uint32_t uIndex, byte cValue) = 0;

	/// 清空原值，并复制pcBuffer的大小为uSize数据
	virtual bool CopyFrom(const byte *pcBuffer, uint32_t uSize) = 0;

	/// 清空原值，复制pBuffer的数据
	virtual bool CopyFromBuffer(const IBuffer *pBuffer)
	{
		if(pBuffer == NULL)
		{
			return false;
		}
		return CopyFrom(pBuffer->GetConstBuffer(), pBuffer->GetSize());
	}

	/// 添加pcBuffer，大小为uSize数据
	virtual bool Append(const byte *pcBuffer, uint32_t uSize) = 0;

	/// 添加pBuffer数据
	virtual bool AppendBuffer(const IBuffer *pBuffer)
	{
		if(pBuffer == NULL)
		{
			return false;
		}
		return Append(pBuffer->GetConstBuffer(), pBuffer->GetSize());
	}

	///移除从uStart开始，大小为uSize数据
	virtual bool Remove(uint32_t uStart, uint32_t uSize) = 0;

	/// 获取数据
	virtual bool GetData(byte *pBuffer, uint32_t uSize, uint32_t uIndex) const = 0;

	virtual string ToString() const
	{
		return String::b2s((const char *)GetConstBuffer(),GetSize());
//		static char s_itox[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
//		string result;
//		const byte *buffer = GetConstBuffer();
//		uint32_t size = GetSize();
//		String::Format(result, "Size: %u", size);
//		for(uint32_t i = 0; i < size; i++)
//		{
//			if(i % 16 == 0)
//			{
//				string head;
//				String::Format(head, "\r\n%08X", i);
//				result.append(head);
//			}
//			result.append(1, ' ');
//			result.append(1, s_itox[buffer[i] >> 4]);
//			result.append(1, s_itox[buffer[i] & 0x0F]);
//		}
//		result.append("\r\n");
//		return result;

		string result;
		string buffer;
		buffer.append((const char *)GetConstBuffer(), GetSize());
		Crypt::Base64EncodeTrim(result, buffer);
		return result;
	}

	/// 赋值运算符重载
	IBuffer &operator=(const IBuffer &buffer)
	{
		CopyFrom(buffer.GetConstBuffer(), buffer.GetSize());
		return *this;
	}
};

#endif /* IBUFFER_H_ */
