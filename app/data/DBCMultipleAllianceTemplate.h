#ifndef DBC_MULTIPLE_ALLIANCE_TEMPLATE_H_
#define DBC_MULTIPLE_ALLIANCE_TEMPLATE_H_

/**
 * 适用于每个商会对应多条记录，且以alliance_id为主键的表
 * */

#include "Kernel.h"
#include "BaseManager.h"
#include "ResourceManager.h"

template<class _DBC, int _ID, int _FACOTR, class _HANDLE>
class DBCMultipleAllianceTemplate
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

	void NewItem(unsigned alliance_id, unsigned id, int index)
	{
		if(CMI->IsNeedConnectByAID(alliance_id))
		{
			error_log("Name: %s, alliance_id:%u, data_need_connect", name(), alliance_id);
			throw std::runtime_error("data_need_connect");
		}

		_DBC data;
		data.alliance_id = alliance_id;
		data.id = id;

		if(this->Add(index, data))
		{
			m_map[alliance_id].insert(std::make_pair(id, index));
		}
		else
		{
			error_log("Name: %s, add_error.alliance_id=%u,", name(), alliance_id);
			throw std::runtime_error("add_item_error");
		}
	}

	int NewItem(unsigned alliance_id, unsigned id)
	{
		if(CMI->IsNeedConnectByAID(alliance_id))
		{
			error_log("Name: %s, alliance_id:%u, data_need_connect", name(), alliance_id);
			throw std::runtime_error("data_need_connect");
		}

		int index = FreeIndex();

		_DBC data;
		data.alliance_id = alliance_id;
		data.id = id;

		if(this->Add(index, data))
		{
			m_map[alliance_id].insert(std::make_pair(id, index));
		}
		else
		{
			error_log("Name: %s, add_error.alliance_id=%u,", name(), alliance_id);
			throw std::runtime_error("add_item_error");
		}

		return index;
	}
public:
	virtual const char* name() const = 0;

	virtual ~DBCMultipleAllianceTemplate() {}

	virtual int OnInit()
	{
		for(unsigned i=0; i < base::MAX_BUFF; ++i)
		{
			if(! base::m_data->Empty(i))
			{
				const _DBC& item = base::m_data->data[i];
				m_map[item.alliance_id].insert(std::pair<unsigned, unsigned>(item.id, i));
			}
		}

		return 0;
	}

	void DoClear(unsigned uid)
	{

	}

	void DoAllianceClear(unsigned alliance_id)
	{
		//商会的删除
		std::map<unsigned, std::map<unsigned, unsigned> >::iterator it = m_map.find(alliance_id);

		if (it != m_map.end())
		{
			std::map<unsigned, unsigned>::iterator  itor = it->second.begin();

			for (; itor != it->second.end(); ++itor)
			{
				base::Clear(itor->second);
			}

			m_map.erase(alliance_id);
		}
	}

	bool IsSingleFull(unsigned alliance_id)
	{
		//判断该商会中数据的数量是否超过单个所拥有的极限
		std::map<unsigned, std::map<unsigned, unsigned> >::iterator it = m_map.find(alliance_id);

		if (it != m_map.end())
		{
			if (it->second.size() >= _FACOTR)
			{
				return true;
			}
		}

		return false;
	}

	void AddNewItem(_DBC & item)
	{
		unsigned alliance_id = item.alliance_id;

		if (alliance_id == 0)
		{
			error_log("Name: %s, alliance_id:%u, alliance value error.", name(), alliance_id);
			throw std::runtime_error("item_value_error");
		}

		if(CMI->IsNeedConnectByAID(alliance_id))
		{
			error_log("Name: %s, alliance_id:%u, data_need_connect", name(), alliance_id);
			throw std::runtime_error("data_need_connect");
		}

		bool isfull = IsSingleFull(alliance_id);

		if (isfull)
		{
			error_log("Name: %s, alliance_id:%u, alliance is full", name(), alliance_id);
			throw std::runtime_error("item_is_full");
		}

		int index = FreeIndex();

		if(this->Add(index, item))
		{
			m_map[alliance_id].insert(std::make_pair(item.id, index));
		}
		else
		{
			error_log("Add to dbc failed. alliance_id=%u", alliance_id);
			throw std::runtime_error("add_item_error");
		}
	}

	void DoSave(unsigned uid)
	{
		//从user里面获取联盟id
		unsigned alliance_id = BaseManager::Instance()->Get(uid).alliance_id;

		std::map<unsigned, std::map<unsigned, unsigned> >::iterator it = m_map.find(alliance_id);

		if (it != m_map.end())
		{
			std::map<unsigned, unsigned>::iterator  itor = it->second.begin();
			for (; itor != it->second.end(); ++itor)
			{
				base::AddSave(itor->second);
			}
		}
	}

	void DoAllianceSave(unsigned aid)
	{
		std::map<unsigned, std::map<unsigned, unsigned> >::iterator it = m_map.find(aid);

		if (it != m_map.end())
		{
			std::map<unsigned, unsigned>::iterator  itor = it->second.begin();
			for (; itor != it->second.end(); ++itor)
			{
				base::AddSave(itor->second);
			}
		}
	}

	void LoadBuffer(unsigned alliance_id)
	{
		if (IsExist(alliance_id))
		{
			return ;
		}

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

		std::vector<_DBC> vResult;
		_DBC t_data;
		t_data.alliance_id = alliance_id;
		vResult.push_back(t_data);

		int ret = base::Load(vResult);
		if (ret == R_SUCCESS)
		{
			m_map[alliance_id];

			for (unsigned i = 0; i < vResult.size(); ++i)
			{
				int index = FreeIndex();
				if(! base::m_data->Empty(index))
				{
					error_log("Name: %s, alliance_id:%u, data_empty_error", name(), alliance_id);
					throw std::runtime_error("data_empty_error");
				}

				base::m_data->data[index] = vResult[i];

				if(base::m_data->MardLoad(index))
				{
					base::m_freeIndex.erase(index);
					m_map[alliance_id].insert(std::pair<unsigned, unsigned>(vResult[i].id, index));
				}
				else
				{
					error_log("Name: %s, alliance_id:%u, load_data_error", name(), alliance_id);
					throw std::runtime_error("data_mark_load_error");
				}
			}
		}
		else if (R_ERR_NO_DATA == ret)
		{
			m_map[alliance_id];
		}
		else
		{
			error_log("Name: %s, alliance_id:%u, ret: %d, load_data_error", name(), alliance_id, ret);
			throw std::runtime_error("data_load_error");
		}
	}

	bool IsExist(unsigned alliance_id)
	{
		return (m_map.count(alliance_id) > 0);
	}

	void Online(unsigned alliance_id)
	{
		LoadBuffer(alliance_id);
	}

	void Offline(unsigned alliance_id)
	{

	}

	void GetIndexsFromMem(unsigned alliance_id, std::vector<unsigned>& vResult)
	{
		vResult.clear();

		const std::map<unsigned, unsigned>& items = m_map[alliance_id];
		std::map<unsigned, unsigned>::const_iterator it = items.begin();

		for (; it != items.end(); ++it)
		{
			vResult.push_back(it->second);
		}
	}
	void GetIndexs(unsigned alliance_id, std::vector<unsigned>& vResult)
	{
		vResult.clear();

		LoadBuffer(alliance_id);

		const std::map<unsigned, unsigned>& items = m_map[alliance_id];
		std::map<unsigned, unsigned>::const_iterator it = items.begin();

		for (; it != items.end(); ++it)
		{
			vResult.push_back(it->second);
		}
	}

	int GetIndex(unsigned alliance_id, unsigned id)
	{
		LoadBuffer(alliance_id);

		const std::map<unsigned, unsigned>& items = m_map[alliance_id];
		std::map<unsigned, unsigned>::const_iterator it = items.find(id);
		if (it != items.end())
		{
			return it->second;
		}

		return NewItem(alliance_id, id);
	}

	_DBC & GetDataByIndex(unsigned index)
	{
		return base::m_data->data[index];
	}

	const _DBC & GetDataByIndex(unsigned index) const
	{
		return base::m_data->data[index];
	}

	_DBC & GetData(unsigned alliance_id, unsigned id)
	{
		unsigned index = GetIndex(alliance_id, id);

		return base::m_data->data[index];
	}

	bool UpdateItem(unsigned alliance_id, unsigned id)
	{
		unsigned index = GetIndex(alliance_id, id);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		return base::m_data->MarkChange(index);
	}

	bool UpdateItem(_DBC & data)
	{
		unsigned index = GetIndex(data.alliance_id, data.id);

		if ((unsigned)-1 == index)
		{
			return false;
		}

		return base::m_data->MarkChange(index);
	}

	bool IsExistItem(unsigned alliance_id, unsigned id)
	{
		LoadBuffer(alliance_id);

		const std::map<unsigned, unsigned>& items = m_map[alliance_id];
		std::map<unsigned, unsigned>::const_iterator it = items.find(id);
		return (it != items.end());
	}

	template<class _PROTO>
	int FullMessage(unsigned alliance_id, google::protobuf::RepeatedPtrField<_PROTO >* msg)
	{
		vector<unsigned> indexs;

		GetIndexs(alliance_id, indexs);

		for(size_t i = 0; i < indexs.size(); ++i)
		{
			SetMessage(GetDataByIndex(indexs[i]), msg->Add());
		}

		return 0;
	}

	template<class _PROTO>
	void SetMessage(_DBC & data, _PROTO * msg)
	{
		data.SetMessage(msg);
	}

	void GetIds(unsigned alliance_id, std::vector<unsigned>& vResult)
	{
		vResult.clear();

		LoadBuffer(alliance_id);

		const std::map<unsigned, unsigned>& items = m_map[alliance_id];
		std::map<unsigned, unsigned>::const_iterator it = items.begin();

		for (; it != items.end(); ++it)
		{
			vResult.push_back(it->first);
		}
	}

	void DelItem(unsigned alliance_id, unsigned id)
	{
		if(!IsExistItem(alliance_id, id))
			return;

		base::m_data->MarkDel(m_map[alliance_id][id]);
		base::AddSave(m_map[alliance_id][id]);

		m_map[alliance_id].erase(id);

		if (m_map[alliance_id].empty())
		{
			m_map.erase(alliance_id);
		}
	}

	void DropAlliance(unsigned alliance_id)
	{
		//删除商会
		if (m_map.count(alliance_id))
		{
			//遍历，进行统计
			map<unsigned, unsigned>::const_iterator itor = m_map.at(alliance_id).begin();

			vector<unsigned> ids;

			for (; itor != m_map.at(alliance_id).end(); ++itor)
			{
				unsigned id = itor->first;
				ids.push_back(id);
			}

			//遍历id集合，迭代调用删除
			for(int i = 0; i < ids.size(); ++i)
			{
				DelItem(alliance_id, ids[i]);
			}
		}
	}

	const std::map<unsigned, std::map<unsigned, unsigned> >& GetAllMap() const { return m_map; }
protected:
	std::map<unsigned, std::map<unsigned, unsigned> > m_map;
};

#endif /* DBCMULTIPLETEMPLATE_H_ */
