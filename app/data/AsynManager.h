/*
 * AsynManager.h
 *
 *  Created on: 2016-10-9
 *      Author: Ralf
 */

#ifndef ASYNMANAGER_H_
#define ASYNMANAGER_H_


#include "Kernel.h"

struct AsynItem
{
	unsigned uid, id, count;
	AsynItem()
	{
		uid = id = count = 0;
	}

	AsynItem(const Admin::AsycItem& item)
	{
		uid = item.uid();
		id = item.id();
		count = item.count();
	}
};
struct AsynData
{
	AsynItem item[MEMORY_PROPERTY_NUM*MEMORY_ASYN_NUM];
};
typedef map<unsigned, map<unsigned, unsigned> > AsynMap;

class AsynManager : public MemorySingleton<AsynData, MEMORY_ASYN>, public CSingleton<AsynManager>
{
private:
	friend class CSingleton<AsynManager>;
	AsynManager(){};
	virtual ~AsynManager(){}

	set<unsigned> m_freeIndex;
	AsynMap m_map;
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	const AsynMap& GetMap() {return m_map;}
	unsigned GetFreeCount()
	{
		return m_freeIndex.size();
	}
	bool IsFull()
	{
		return m_freeIndex.size() < 1000;
	}
	unsigned GetFreeIndex()
	{
		if(m_freeIndex.empty())
			return -1;
		return *(m_freeIndex.begin());
	}
	void Del(unsigned uid)
	{
		if(m_map.count(uid))
		{
			for(map<unsigned, unsigned>::iterator it=m_map[uid].begin();it!=m_map[uid].end();++it)
			{
				m_data->item[it->second].uid = 0;
				m_freeIndex.insert(it->second);
			}
			m_map.erase(uid);
		}
	}
	void Clear()
	{
		m_map.clear();
		m_freeIndex.clear();
		for(unsigned i=0;i<MEMORY_PROPERTY_NUM*MEMORY_ASYN_NUM;++i)
		{
			m_data->item[i].uid = 0;
			m_freeIndex.insert(i);
		}
	}
	int Add(unsigned uid, unsigned id, unsigned count)
	{
		if(m_map.count(uid) && m_map[uid].count(id))
			m_data->item[m_map[uid][id]].count += count;
		else
		{
			unsigned i = GetFreeIndex();
			if(i == (unsigned)-1)
				return R_ERR_DATA;
			m_freeIndex.erase(i);
			m_map[uid][id] = i;
			m_data->item[i].uid = uid;
			m_data->item[i].id = id;
			m_data->item[i].count = count;
		}
		return 0;
	}
	bool Has(unsigned uid)
	{
		return m_map.count(uid);
	}
	unsigned Get(unsigned uid, unsigned id)
	{
		if(m_map.count(uid) && m_map[uid].count(id))
			return m_data->item[m_map[uid][id]].count;
		return 0;
	}
};

#endif /* ASYNMANAGER_H_ */
