/*
 * MemorySysMail.h
 *
 *  Created on: 2018-2-8
 *      Author: Ralf
 */

#ifndef MEMORYSYSMAIL_H_
#define MEMORYSYSMAIL_H_

#include "Kernel.h"
#include "DataSysMail.h"

struct MemorySysmailItem {
	uint32_t ts;
	uint32_t uid;
	char sys[Data_sysmail_sys_COUNT];
	char reward[Data_sysmail_reward_COUNT];
	MemorySysmailItem() {
		ts = 0;
		uid = 0;
		memset(sys, 0, sizeof(sys));
		memset(reward, 0, sizeof(reward));
	}
	void Add(DataSysmail& res)
	{
		res.stat = e_mail_stat_unread;
		memcpy(res.sys, sys, sizeof(res.sys));
		memcpy(res.reward, reward, sizeof(res.reward));
	}
};

#define  Memory_sysmail_count 1000
struct MemorySysmail
{
	MemorySysmailItem item[Memory_sysmail_count];
};

class MemorySysMailManager : public MemorySingleton<MemorySysmail, MEMORY_SYSMAIL>, public CSingleton<MemorySysMailManager>
{
public:
	virtual void CallDestroy() {Destroy();}
	void OnDay(bool f = false)
	{
		unsigned s = 0;
		for(int i=0;i<Memory_sysmail_count;++i)
		{
			if(m_data->item[i].ts == 0)
				break;
			//if(m_data->item[i].ts + 86400*30 > Time::GetGlobalTime())
			if(IsValid(m_data->item[i].ts))
			{
				s = i;
				break;
			}
		}
		if(s)
		{
			f = true;
			for(int i=0;i<Memory_sysmail_count;++i)
			{
				if(m_data->item[i].ts == 0)
					break;
				if(i + s < Memory_sysmail_count)
					memcpy(m_data->item + i, m_data->item + i + s, sizeof(MemorySysmailItem));
				else
					memset(m_data->item+i, 0, sizeof(MemorySysmailItem));
			}
		}
		if(f)
		{
			m_end = 0;
			m_map.clear();
			for(int i=0;i<Memory_sysmail_count;++i)
			{
				if(m_data->item[i].ts == 0)
				{
					m_end = i;
					break;
				}
				m_map[m_data->item[i].ts][m_data->item[i].uid] = i;
			}
		}
	}
	virtual int OnInit()
	{
		OnDay(true);
		return 0;
	}
	bool IsValid(uint32_t ts)
	{
		return Time::GetGlobalTime() <= (ts + 2592000);	// 邮件有效期30天
	}
	int Add(MemorySysmailItem& obj)
	{
		if(m_end < Memory_sysmail_count)
		{
			memcpy(&m_data->item[m_end], &obj, sizeof(MemorySysmailItem));
			m_map[m_data->item[m_end].ts][m_data->item[m_end].uid] = m_end;
			++m_end;
			return 0;
		}
		error_log("full");
		return R_ERR_DATA_LIMIT;
	}
	void Get(unsigned uid, unsigned ts, vector<MemorySysmailItem*>& res)
	{
		map<unsigned, map<unsigned, unsigned> >::iterator it = m_map.lower_bound(ts);
		for(;it!=m_map.end();++it)
		{
			for(map<unsigned, unsigned>::iterator iter=it->second.begin();iter!=it->second.end();++iter)
			{
				if((iter->first == 0 || iter->first == uid) && IsValid(m_data->item[iter->second].ts))
					res.push_back(&m_data->item[iter->second]);
			}
		}
	}
private:
	map<unsigned, map<unsigned, unsigned> > m_map;
	unsigned m_end;
};

#endif /* MEMORYSYSMAIL_H_ */
