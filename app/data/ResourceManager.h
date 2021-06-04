/*
 * ResourceManager.h
 *
 *  Created on: 2016-9-2
 *      Author: Ralf
 */

#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

#include "Kernel.h"

/***************************************************/
struct OfflineResourceItem
{
	unsigned uid;
	unsigned level;
	unsigned viplevel;
	unsigned alliance_id;
	unsigned helptimes;

	char name[BASE_NAME_LEN];//玩家名字
	char fig[BASE_FIG_LEN];//头像
	unsigned ts;//内容变化时戳

	OfflineResourceItem()
	{
		uid = level = viplevel = alliance_id = ts = 0;
		helptimes = 0;

		memset(name, 0, sizeof(name));
		memset(fig, 0, sizeof(fig));
	}

	bool CanClear()
	{
		return ts + 86400*7 < Time::GetGlobalTime();
	}
};

struct OfflineResource
{
	OfflineResourceItem item[MEMORY_PROPERTY_NUM];
};

class ResourceManager : public MemorySingleton<OfflineResource, MEMORY_RESOURCE>, public CSingleton<ResourceManager>
{
private:
	friend class CSingleton<ResourceManager>;
	ResourceManager(){};
	virtual ~ResourceManager(){}

	set<unsigned> m_freeIndex;
	map<unsigned, unsigned> m_map;
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	const map<unsigned, unsigned>& GetMap() {return m_map;}

	unsigned GetFreeCount()
	{
		return m_freeIndex.size();
	}

	unsigned GetFreeIndex()
	{
		if(m_freeIndex.empty())
			return -1;
		return *(m_freeIndex.begin());
	}

	bool IsNeedClear()
	{
		return GetFreeCount() * 10 / MEMORY_PROPERTY_NUM <= 1;
	}

	void DoClear(unsigned uid)
	{
		if(m_map.count(uid))
		{
			unsigned i = m_map[uid];
			memset(&(m_data->item[i]), 0, sizeof(OfflineResourceItem));
			m_freeIndex.insert(i);
			m_map.erase(uid);
		}
	}

	unsigned GetIndex(unsigned uid)
	{
		if(m_map.count(uid))
			return m_map[uid];
		return -1;
	}

	OfflineResourceItem & GetResourceItemByIndex(unsigned index)
	{
		if (index > MEMORY_PROPERTY_NUM)
		{
			throw runtime_error("index_error");
		}

		return  m_data->item[index];
	}

	string GetUserName(unsigned uid)
	{
		if(!m_map.count(uid))
		{
			return "";
		}

		unsigned index = m_map[uid];

		return m_data->item[index].name;
	}

	string GetHeadUrl(unsigned uid)
	{
		if(!m_map.count(uid))
		{
			return "";
		}

		unsigned index = m_map[uid];

		return m_data->item[index].fig;
	}

	int GetUserLevel(unsigned uid)
	{
		if(!m_map.count(uid))
		{
			return -1;
		}

		unsigned index = m_map[uid];

		return m_data->item[index].level;
	}

	int Add(unsigned uid)
	{
		unsigned i = GetFreeIndex();
		if(i == (unsigned)-1)
			return R_ERR_DATA;
		m_freeIndex.erase(i);
		m_map[uid] = i;
		m_data->item[i].uid = uid;
		m_data->item[i].ts = Time::GetGlobalTime();
		return 0;
	}

	void GetClear(vector<unsigned> &uids)
	{
		for(map<unsigned, unsigned>::iterator it=m_map.begin();it!=m_map.end();++it)
		{
			if(m_data->item[it->second].CanClear())
				uids.push_back(it->first);
		}
	}

	void TryClear(vector<unsigned> &uids)
	{
		for(map<unsigned, unsigned>::iterator it=m_map.begin();it!=m_map.end();++it)
		{
			debug_log("%u,%u,%s",m_data->item[it->second].uid, m_data->item[it->second].ts
					,m_data->item[it->second].CanClear()?"yes":"no");
			if(m_data->item[it->second].CanClear())
				uids.push_back(it->first);
		}
	}
};


#endif /* RESOURCEMANAGER_H_ */
