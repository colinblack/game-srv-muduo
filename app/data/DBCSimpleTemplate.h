/*
 * DBCSimpleTemplate.h
 *
 *  Created on: 2016-11-22
 *      Author: dawx62fac
 */

#ifndef DBCSIMPLETEMPLATE_H_
#define DBCSIMPLETEMPLATE_H_

/**
 * 适用于每个用户对应一条记录，且以uid为主键的表
 */

#include "Kernel.h"

template<class _DBC, int _ID, class _HANDLE>
class DBCSimpleTemplate
	 : public DataSingleton<_DBC, _ID , DB_BASE_FULL, _HANDLE, DB_BASE_FULL>
{
	 typedef DataSingleton<_DBC, _ID , DB_BASE_FULL, _HANDLE, DB_BASE_FULL> base;
protected:
	 int FreeIndex()
	 {
		int index = base::GetFreeIndex();
		if (index < 0)
		{
			error_log("Name: %s, index: %d", name(), index);
			throw std::runtime_error("Get_free_index_error");
		}

		return index;
	 }

	void NewItem(unsigned uid, int index)
	{
		if(CMI->IsNeedConnectByUID(uid))
		{
			error_log("Name: %s, uid:%u, data_need_connect", name(), uid);
			throw std::runtime_error("data_need_connect");
		}

		_DBC data;
		data.uid = uid;
		if(this->Add(index, data))
		{
			m_map[uid] = index;
		}
		else
		{
			error_log("Name: %s, add_error.uid=%u,", name(), uid);
			throw std::runtime_error("add_item_error");
		}
	}

public:
	virtual const char* name() const = 0;

	virtual ~DBCSimpleTemplate() {}

	int OnInit()
	{
		for(unsigned i=0; i < base::MAX_BUFF; ++i)
		{
			if(! base::m_data->Empty(i))
			{
				const _DBC& item = base::m_data->data[i];
				m_map[item.uid] = i;
			}
		}
		return 0;
	}

	void DoClear(unsigned uid)
	{
		std::map<unsigned, unsigned>::iterator it = m_map.find(uid);
		if (it != m_map.end())
		{
			unsigned idx = it->second;
			base::Clear(idx);

			m_map.erase(uid);
		}
	}

	void DoSave(unsigned uid)
	{
		std::map<unsigned, unsigned>::iterator it = m_map.find(uid);
		if (it != m_map.end())
		{
			unsigned idx = it->second;
			base::AddSave(idx);
		}
	}

	int GetIndex(unsigned uid)
	{
		std::map<unsigned, unsigned>::iterator it = m_map.find(uid);
		if (it != m_map.end())
		{
			return it->second;
		}
		else
		{
			return this->_Load(uid);
		}
	}

	void LoadBuffer(unsigned uid)
	{
		GetIndex(uid);
	}

	bool IsExist(unsigned uid)
	{
		return (m_map.count(uid) > 0);
	}

	_DBC & GetData(unsigned uid)
	{
		unsigned index = GetIndex(uid);

		return base::m_data->data[index];
	}

	bool UpdateItem(_DBC & data)
	{
		unsigned index = GetIndex(data.uid);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		return base::m_data->MarkChange(index);
	}

protected:
	int _Load(unsigned uid)
	{
		if(CMI->IsNeedConnectByUID(uid))
		{
			error_log("Name: %s, uid:%u, data_need_connect", name(), uid);
			throw std::runtime_error("data_need_connect");
		}
		if (base::IsFull())
		{
			error_log("Name: %s, uid:%u, data_is_full", name(), uid);
			throw std::runtime_error("data_is_full");
		}

		int index = FreeIndex();
		base::m_data->data[index].uid = uid;
		int ret = base::Load(index);
		if (R_ERR_NO_DATA == ret)
		{
			NewItem(uid, index);
		}
		else
		{
			if (ret != 0)
			{
				error_log("Name: %s, uid:%u, load_data_error, ret:%d", name(), uid, ret);
				throw std::runtime_error("load_data_error");
			}

			m_map[uid] = index;
		}

		return index;
	}

protected:
	std::map<unsigned, unsigned> m_map;
};


#endif /* DBCSIMPLETEMPLATE_H_ */
