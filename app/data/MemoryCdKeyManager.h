#ifndef _MEMORY_CD_KEY_MANAGER__H_
#define _MEMORY_CD_KEY_MANAGER__H_

#include "Kernel.h"

struct cdKeyItem{
	uint32_t type;
	uint32_t uid;

	cdKeyItem(){
		type = 0;
		uid = 0;
	}
};

struct cdKeyData{
	cdKeyItem item[MEMORY_CD_KEY_MAX];
};

class CDKeyManager : public MemorySingleton<cdKeyData, MEMORY_CD_KEY>, public CSingleton<CDKeyManager>
{
private:
	friend class CSingleton<CDKeyManager>;
	CDKeyManager(){}
	virtual ~CDKeyManager(){}

	static const int MAX_SIZE = MEMORY_CD_KEY_MAX;
	set<unsigned> m_freeIndex;
	map<unsigned,set<unsigned> > m_type_uids;

public:

	virtual void CallDestroy() {Destroy();}
	virtual int OnInit()
	{
		for(unsigned idx = 0; idx < MAX_SIZE; idx++)
		{
			unsigned uid = m_data->item[idx].uid;
			if(IsValidUid(uid))
			{
				unsigned type = m_data->item[idx].type;
				m_type_uids[type].insert(uid);
			}
			else
			{
				m_freeIndex.insert(idx);
			}
		}
		return 0;
	}

	bool IsFull()
	{
		return m_freeIndex.size() < 1;
	}

	unsigned GetFreeIndex()
	{
		if(m_freeIndex.empty())
			return (unsigned)-1;
		unsigned ret = *(m_freeIndex.begin());		//从set中取一个index
		m_freeIndex.erase(m_freeIndex.begin());

		return ret;
	}

	bool HasUseThisType(unsigned uid,unsigned type)
	{
		if(m_type_uids.count(type) && m_type_uids[type].count(uid))
		{
			return true;
		}
		return false;
	}

	int UseCard(unsigned uid,unsigned type)
	{
		unsigned index = GetFreeIndex();
		if((unsigned) -1 == index)
		{
			return -1;
		}
		m_data->item[index].type = type;
		m_data->item[index].uid = uid;

		m_type_uids[type].insert(uid);
		return 0;
	}

};

#endif
