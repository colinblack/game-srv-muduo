#ifndef DBC_SIMPLE_ALLIANCE_TEMPLATE_H_
#define DBC_SIMPLE_ALLIANCE_TEMPLATE_H_

/**
 * 适用于每个商会对应一条记录，且以alliance_id为主键的表
 */

#include "Kernel.h"
#include "BaseManager.h"

template<class _DBC, int _ID, class _HANDLE>
class DBCSimpleAllianceTemplate
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

	void NewItem(unsigned alliance_id, int index)
	{
		if(CMI->IsNeedConnectByAID(alliance_id))
		{
			error_log("Name: %s, alliance_id:%u, data_need_connect", name(), alliance_id);
			throw std::runtime_error("data_need_connect");
		}

		_DBC data;
		data.alliance_id = alliance_id;

		if(this->Add(index, data))
		{
			m_map[alliance_id] = index;
		}
		else
		{
			error_log("Name: %s, add_error.alliance_id=%u,", name(), alliance_id);
			throw std::runtime_error("add_item_error");
		}
	}

public:
	virtual const char* name() const = 0;

	virtual ~DBCSimpleAllianceTemplate() {}

	int OnInit()
	{
		for(unsigned i=0; i < base::MAX_BUFF; ++i)
		{
			if(! base::m_data->Empty(i))
			{
				const _DBC& item = base::m_data->data[i];
				m_map[item.alliance_id] = i;
				m_isLoad[item.alliance_id] = true;
			}
		}

		return 0;
	}

	void DoClear(unsigned uid)
	{

	}

	void GetClear(set<unsigned> & online_aids, vector<unsigned> & clear_aids)
	{
		for(map<unsigned, unsigned>::iterator uiter = m_map.begin(); uiter != m_map.end(); ++uiter)
		{
			if(online_aids.count(uiter->first) == 0)
			{
				clear_aids.push_back(uiter->first);
			}
		}
	}

	void DoAllianceClear(unsigned alliance_id)
	{
		//商会的删除
		std::map<unsigned, unsigned>::iterator it = m_map.find(alliance_id);

		if (it != m_map.end())
		{
			unsigned idx = it->second;
			base::Clear(idx);

			m_map.erase(alliance_id);
			m_isLoad.erase(alliance_id);
		}
	}

	void DoSave(unsigned uid)
	{
		//从user里面获取联盟id
		unsigned alliance_id = BaseManager::Instance()->Get(uid).alliance_id;

		std::map<unsigned, unsigned>::iterator it = m_map.find(alliance_id);

		if (it != m_map.end())
		{
			unsigned idx = it->second;
			base::AddSave(idx);
		}
	}

	void DoAllianceSave(unsigned aid)
	{
		std::map<unsigned, unsigned>::iterator it = m_map.find(aid);

		if (it != m_map.end())
		{
			unsigned idx = it->second;
			base::AddSave(idx);
		}
	}

	void AddNewItem(_DBC & item)
	{
		if(CMI->IsNeedConnectByAID(item.alliance_id))
		{
			error_log("Name: %s, alliance_id:%u, data_need_connect", name(), item.alliance_id);
			throw std::runtime_error("data_need_connect");
		}

		//判断id是否存在
		std::map<unsigned, unsigned>::iterator it = m_map.find(item.alliance_id);

		if (it != m_map.end())
		{
			error_log("Name: %s, add_error, alliance exist.alliance_id=%u,", name(), item.alliance_id);
			throw std::runtime_error("item_exist");
		}

		int index = FreeIndex();

		if(this->Add(index, item))
		{
			m_map[item.alliance_id] = index;
			m_isLoad[item.alliance_id] = true;
		}
		else
		{
			error_log("Add to dbc failed. alliance_id=%u", item.alliance_id);
			throw std::runtime_error("add_item_error");
		}
	}

	int GetIndex(unsigned alliance_id)
	{
		std::map<unsigned, unsigned>::iterator it = m_map.find(alliance_id);

		if (it != m_map.end())
		{
			return it->second;
		}
		else
		{
			return this->_Load(alliance_id);
		}
	}

	void LoadBuffer(unsigned alliance_id)
	{
		GetIndex(alliance_id);
	}

	bool IsExist(unsigned alliance_id)
	{
		if (m_map.count(alliance_id) > 0)
		{
			return true;
		}
		else if (m_isLoad.count(alliance_id) > 0)
		{
			//已加载，确实没有
			return false;
		}
		else
		{
			//没有加载过，加载
			LoadBuffer(alliance_id);

			return m_map.count(alliance_id) > 0;
		}
	}

	_DBC & GetData(unsigned alliance_id)
	{
		unsigned index = GetIndex(alliance_id);

		if ((unsigned)-1 == index)
		{
			error_log("alliance not exist. alliance_id=%u", alliance_id);
			throw runtime_error("get_alliance_error");
		}

		return base::m_data->data[index];
	}

	bool UpdateItem(_DBC & data)
	{
		unsigned index = GetIndex(data.alliance_id);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		return base::m_data->MarkChange(index);
	}

	bool DelItem(unsigned alliance_id)
	{
		unsigned index = GetIndex(alliance_id);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		base::m_data->MarkDel(m_map[alliance_id]);
		base::AddSave(m_map[alliance_id]);

		m_map.erase(alliance_id);
		m_isLoad.erase(alliance_id);

		return true;
	}

protected:
	int _Load(unsigned alliance_id)
	{
		if(CMI->IsNeedConnectByAID(alliance_id))
		{
			error_log("Name: %s, alliance_id:%u, data_need_connect", name(), alliance_id);
			throw std::runtime_error("data_need_connect");
		}
		if (base::IsFull())
		{
			error_log("Name: %s, alliance_id:%u, data_is_full", name(), alliance_id);
			throw std::runtime_error("data_is_full");
		}

		int index = FreeIndex();
		base::m_data->data[index].alliance_id = alliance_id;
		int ret = base::Load(index);

		if (R_ERR_NO_DATA == ret)
		{
			//删除不存在时自动添加的功能
			//NewItem(alliance_id, index);
			m_isLoad[alliance_id] = true;
			return -1;
		}
		else
		{
			if (ret != 0)
			{
				error_log("Name: %s, alliance_id:%u, load_data_error, ret:%d", name(), alliance_id, ret);
				throw std::runtime_error("load_data_error");
			}

			m_map[alliance_id] = index;
			m_isLoad[alliance_id] = true;
		}

		return index;
	}

protected:
	std::map<unsigned, unsigned> m_map;
	map<unsigned, bool> m_isLoad;  //是否已加载
};


#endif /* DBC_SIMPLE_ALLIANCE_TEMPLATE_H_ */
