/*
 * NotifyManager.h
 *
 *  Created on: 2016-10-9
 *      Author: Ralf
 */

#ifndef NOTIFY_MANAGER_H_
#define NOTIFY_MANAGER_H_


#include "Kernel.h"

struct NotifyItem
{
	unsigned uid, id, ts;
	char content[NOTIFY_CONTENT_LEN];

	NotifyItem():
		uid(0),
		id(0),
		ts(0)
	{
		memset(content, 0 , sizeof (content));
	}

	NotifyItem(unsigned uid_, unsigned id_)
		: uid(uid_)
		, id(id_)
		, ts(Time::GetGlobalTime())
	{
		memset(content, 0 , sizeof (content));
	}

	void Clear()
	{
		uid = id = ts = 0;
		memset(content, 0 , sizeof (content));
	}

	void SetMessage(ProtoNotify::NotifyItemCPP * msg)
	{
		msg->set_id(id);
		msg->set_ts(ts);
		msg->set_content(content);
	}
};

struct NotifyData
{
	NotifyItem item[DB_BASE_BUFF*PER_USER_MAX_NOTIFY];
};

typedef map<unsigned, map<unsigned, unsigned> > NotifyMap;

class NotifyManager : public MemorySingleton<NotifyData, MEMORY_NOTIFY>, public CSingleton<NotifyManager>
{
private:
	friend class CSingleton<NotifyManager>;
	NotifyManager(){};
	virtual ~NotifyManager(){}

	set<unsigned> m_freeIndex;
	NotifyMap m_map;

	static const int MAX_SIZE = DB_BASE_BUFF*PER_USER_MAX_NOTIFY;

public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	const NotifyMap& GetMap() {return m_map;}

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
			for(map<unsigned, unsigned>::iterator it = m_map[uid].begin(); it!=m_map[uid].end(); ++it)
			{
				m_data->item[it->second].Clear();
				m_freeIndex.insert(it->second);
			}

			m_map[uid].clear();
			m_map.erase(uid);
		}
	}

	void DelItem(unsigned uid, unsigned id)
	{
		if(m_map.count(uid) && m_map[uid].count(id))  //已存在，则更新
		{
			unsigned index = m_map[uid][id];

			m_data->item[index].Clear();
			m_freeIndex.insert(index);

			m_map[uid].erase(id);
		}
	}

	void Clear()
	{
		m_map.clear();
		m_freeIndex.clear();

		for(int i=0 ; i < MAX_SIZE; ++i)
		{
			m_data->item[i].Clear();

			m_freeIndex.insert(i);
		}
	}

	int Add(NotifyItem & notify)
	{
		unsigned uid = notify.uid;
		unsigned id = notify.id;

		unsigned index = 0;

		if(m_map.count(uid) && m_map[uid].count(id))  //已存在，则更新
		{
			index = m_map[uid][id];
		}
		else
		{
			unsigned i = GetFreeIndex();

			if(i == (unsigned)-1)
				return R_ERR_DATA;

			m_freeIndex.erase(i);

			m_map[uid][id] = i;

			index = i;
			m_data->item[i].uid = uid;
			m_data->item[i].id = id;
		}

		m_data->item[index].ts =  notify.ts;
		sprintf(m_data->item[m_map[uid][id]].content, "%s", notify.content);

		return 0;
	}

	bool Has(unsigned uid, unsigned id)
	{
		if (m_map.count(uid) && m_map[uid].count(id))
		{
			return true;
		}

		return false;
	}

	NotifyItem & Get(unsigned uid, unsigned id)
	{
		if(m_map.count(uid) && m_map[uid].count(id))
			return m_data->item[m_map[uid][id]];

		throw runtime_error("notify_not_exist");
	}

	int ClearAllItemById(unsigned id);
};

#endif /* NOTIFY_MANAGER_H_ */
