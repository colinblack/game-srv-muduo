/*
 * DBCMultipleTemplate.h
 *
 *  Created on: 2016-11-29
 *      Author: dawx62fac
 */

#ifndef DBCMULTIPLETEMPLATE_H_
#define DBCMULTIPLETEMPLATE_H_

/**
 * 适用于每个用户对应多条记录，且以uid为主键的表
 */

#include "Kernel.h"

template<class _DBC, int _ID, int _FACOTR, class _HANDLE>
class DBCMultipleTemplate
	 : public DataSingleton<_DBC, _ID , _FACOTR, _HANDLE, _FACOTR>
{
	 typedef DataSingleton<_DBC, _ID , _FACOTR, _HANDLE, _FACOTR> base;
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

	void NewItem(unsigned uid, unsigned id, int index)
	{
		if(CMI->IsNeedConnectByUID(uid))
		{
			error_log("Name: %s, uid:%u, data_need_connect", name(), uid);
			throw std::runtime_error("data_need_connect");
		}

		_DBC data;
		data.uid = uid;
		data.id = id;
		if(this->Add(index, data))
		{
			m_map[uid].insert(std::make_pair(id, index));
		}
		else
		{
			error_log("Name: %s, add_error.uid=%u,", name(), uid);
			throw std::runtime_error("add_item_error");
		}
	}

	int NewItem(unsigned uid, unsigned id)
	{
		if(CMI->IsNeedConnectByUID(uid))
		{
			error_log("Name: %s, uid:%u, data_need_connect", name(), uid);
			throw std::runtime_error("data_need_connect");
		}

		int index = FreeIndex();

		_DBC data;
		data.uid = uid;
		data.id = id;
		if(this->Add(index, data))
		{
			m_map[uid].insert(std::make_pair(id, index));
		}
		else
		{
			error_log("Name: %s, add_error.uid=%u,", name(), uid);
			throw std::runtime_error("add_item_error");
		}

		return index;
	}
public:
	virtual const char* name() const = 0;

	virtual ~DBCMultipleTemplate() {}

	int OnInit()
	{
		for(unsigned i=0; i < base::MAX_BUFF; ++i)
		{
			if(! base::m_data->Empty(i))
			{
				const _DBC& item = base::m_data->data[i];
				m_map[item.uid].insert(std::pair<unsigned, unsigned>(item.id, i));
			}
		}
		return 0;
	}

	void DoClear(unsigned uid)
	{
		std::map<unsigned, std::map<unsigned, unsigned> >::iterator it = m_map.find(uid);
		if (it != m_map.end())
		{
			std::map<unsigned, unsigned>::iterator  itor = it->second.begin();
			for (; itor != it->second.end(); ++itor)
			{
				base::Clear(itor->second);
			}

			m_map.erase(uid);

		}
	}

	void DoSave(unsigned uid)
	{
		std::map<unsigned, std::map<unsigned, unsigned> >::iterator it = m_map.find(uid);
		if (it != m_map.end())
		{
			std::map<unsigned, unsigned>::iterator  itor = it->second.begin();
			for (; itor != it->second.end(); ++itor)
			{
				base::AddSave(itor->second);
			}
		}
	}

	void LoadBuffer(unsigned uid)
	{
		if (IsExist(uid))
		{
			return ;
		}

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

		std::vector<_DBC> vResult;
		_DBC t_data;
		t_data.uid = uid;
		vResult.push_back(t_data);

		int ret = base::Load(vResult);
		if (ret == R_SUCCESS)
		{
			m_map[uid];
			for (unsigned i = 0; i < vResult.size(); ++i)
			{
				int index = FreeIndex();
				if(! base::m_data->Empty(index))
				{
					error_log("Name: %s, uid:%u, data_empty_error", name(), uid);
					throw std::runtime_error("data_empty_error");
				}

				base::m_data->data[index] = vResult[i];
				if(base::m_data->MardLoad(index))
				{
					base::m_freeIndex.erase(index);
					m_map[uid].insert(std::pair<unsigned, unsigned>(vResult[i].id, index));
				}
				else
				{
					error_log("Name: %s, uid:%u, load_data_error", name(), uid);
					throw std::runtime_error("data_mark_load_error");
				}
			}
		}
		else if (R_ERR_NO_DATA == ret)
		{
			m_map[uid];
		}
		else
		{
			error_log("Name: %s, uid:%u, ret: %d, load_data_error", name(), uid, ret);
			throw std::runtime_error("data_load_error");
		}
	}

	bool IsExist(unsigned uid)
	{
		return (m_map.count(uid) > 0);
	}

	void Online(unsigned uid)
	{
		LoadBuffer(uid);
	}

	void Offline(unsigned uid)
	{

	}

	void GetIndexsFromMem(unsigned uid, std::vector<unsigned>& vResult)
	{
		vResult.clear();

		const std::map<unsigned, unsigned>& items = m_map[uid];
		std::map<unsigned, unsigned>::const_iterator it = items.begin();
		for (; it != items.end(); ++it)
		{
			vResult.push_back(it->second);
		}
	}
	void GetIndexs(unsigned uid, std::vector<unsigned>& vResult)
	{
		vResult.clear();

		LoadBuffer(uid);

		const std::map<unsigned, unsigned>& items = m_map[uid];
		std::map<unsigned, unsigned>::const_iterator it = items.begin();
		for (; it != items.end(); ++it)
		{
			vResult.push_back(it->second);
		}
	}

	int GetIndex(unsigned uid, unsigned id)
	{
		LoadBuffer(uid);

		const std::map<unsigned, unsigned>& items = m_map[uid];
		std::map<unsigned, unsigned>::const_iterator it = items.find(id);
		if (it != items.end())
		{
			return it->second;
		}

		return NewItem(uid, id);
	}

	_DBC & GetDataByIndex(unsigned index)
	{
		return base::m_data->data[index];
	}

	_DBC & GetData(unsigned uid, unsigned id)
	{
		unsigned index = GetIndex(uid, id);

		return base::m_data->data[index];
	}

	bool UpdateItem(unsigned uid, unsigned id)
	{
		unsigned index = GetIndex(uid, id);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		return base::m_data->MarkChange(index);
	}

	bool UpdateItem(_DBC & data)
	{
		unsigned index = GetIndex(data.uid, data.id);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		return base::m_data->MarkChange(index);
	}

	bool IsExistItem(unsigned uid, unsigned id)
	{
		LoadBuffer(uid);

		const std::map<unsigned, unsigned>& items = m_map[uid];
		std::map<unsigned, unsigned>::const_iterator it = items.find(id);
		return (it != items.end());
	}

	template<class _PROTO>
	int FullMessage(unsigned uid, google::protobuf::RepeatedPtrField<_PROTO >* msg)
	{
		vector<unsigned> indexs;

		GetIndexs(uid, indexs);

		for(size_t i = 0; i < indexs.size(); ++i)
		{
			GetDataByIndex(indexs[i]).SetMessage(msg->Add());
		}

		return 0;
	}

	void GetIds(unsigned uid, std::vector<unsigned>& vResult)
	{
		vResult.clear();

		LoadBuffer(uid);

		const std::map<unsigned, unsigned>& items = m_map[uid];
		std::map<unsigned, unsigned>::const_iterator it = items.begin();
		for (; it != items.end(); ++it)
		{
			vResult.push_back(it->first);
		}
	}

	void DelItem(unsigned uid, unsigned id)
	{
		if(!IsExistItem(uid, id))
			return;
		base::m_data->MarkDel(m_map[uid][id]);
		base::AddSave(m_map[uid][id]);
		m_map[uid].erase(id);
	}

	const std::map<unsigned, std::map<unsigned, unsigned> >& GetAllMap() const { return m_map; }
protected:
	std::map<unsigned, std::map<unsigned, unsigned> > m_map;
};

#endif /* DBCMULTIPLETEMPLATE_H_ */
