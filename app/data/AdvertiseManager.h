/*
 * AdvertiseManager.h
 *
 *  Created on: 2018-03-12
 *      Author: Summer
 */

#ifndef ADVERTISE_MANAGER_H_
#define ADVERTISE_MANAGER_H_


#include "Kernel.h"

struct AdvertiseItem
{
	uint32_t id;//广告id
	uint32_t uid;//发布者uid
	uint32_t shelf_ud;//货架ud
	uint32_t help_flag;//有无帮助请求信息
	uint32_t ts;//发布时间

	AdvertiseItem():
		id(0),
		uid(0),
		shelf_ud(0),
		help_flag(0),
		ts(0)
	{

	}

	AdvertiseItem(unsigned id_,unsigned uid_,unsigned shelf_ud_,unsigned help_flag_):
			id(id_),
			uid(uid_),
			shelf_ud(shelf_ud_),
			help_flag(help_flag_),
			ts(Time::GetGlobalTime())
	{

	}

	void Set(const AdvertiseItem& item)
	{
		id         = item.id;
		uid        = item.uid;
		shelf_ud   = item.shelf_ud;
		help_flag  = item.help_flag;
		ts         = item.ts;
	}

	void Clear()
	{
		uid = id = shelf_ud = help_flag = ts = 0;
	}
};


struct AdvertiseData
{
	AdvertiseItem item[DB_BASE_BUFF*PER_USER_MAX_ADVERTISE];
};

typedef struct AdvertiseNode
{
	AdvertiseItem data;
	struct AdvertiseNode *next;
	struct AdvertiseNode * prior;
}AdvertiseNode,*AdvertiseList;

typedef map<unsigned, unsigned>  AdvertiseMap;//广告id->index

class AdvertiseManager : public MemorySingleton<AdvertiseData, MEMORY_ADVERTISE>, public CSingleton<AdvertiseManager>
{
private:
	friend class CSingleton<AdvertiseManager>;
	AdvertiseManager();
	virtual ~AdvertiseManager(){}

	static const int MAX_SIZE = DB_BASE_BUFF*PER_USER_MAX_ADVERTISE;
	set<unsigned> m_freeIndex;
	AdvertiseMap m_map;
	map<unsigned, map<unsigned ,unsigned> >  m_adinfomap;//广告发布者uid->货架ud->广告id
	AdvertiseList m_adheadList;
	AdvertiseList m_adtailList;
public:
	const AdvertiseMap& GetMap() {return m_map;}

	unsigned GetFreeCount()
	{
		return m_freeIndex.size();
	}

	bool IsFull()
	{
		return m_freeIndex.size() < PER_USER_MAX_ADVERTISE;
	}

	unsigned GetFreeIndex()
	{
		if(m_freeIndex.empty())
			return -1;
		return *(m_freeIndex.begin());
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

	void Del(unsigned id)
	{
		if(m_map.count(id))
		{
			m_data->item[m_map[id]].Clear();
			m_freeIndex.insert(m_map[id]);
			m_map.erase(id);
		}
	}

	int Add(AdvertiseItem & adItem)
	{
		unsigned id = adItem.id;

		unsigned index = 0;

		if(m_map.count(id))  //已存在，则更新
		{
			index = m_map[id];
		}
		else
		{
			unsigned i = GetFreeIndex();

			if(i == (unsigned)-1)
				return R_ERR_DATA;

			m_freeIndex.erase(i);

			m_map[id] = i;

			index = i;
			m_data->item[i].id = id;
		}

		m_data->item[m_map[id]].Set(adItem);
		return 0;
	}

	AdvertiseItem & GetItem(unsigned id)
	{
		if(m_map.count(id))
			return m_data->item[m_map[id]];

		throw runtime_error("adItem_not_exist");
	}

	bool Has(unsigned id)
	{
		if (m_map.count(id))
		{
			return true;
		}

		return false;
	}
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();
	//初始化链表
	int InitList();

	//添加链表数据
	int AddNode(AdvertiseItem data);

	//删除链表中指定id的广告
	int DelNode(unsigned id);

	//获取新创建广告id
	unsigned GetNewAdUd();

	//获取广告条数
	unsigned GetAdCnt();

	//通过uid,shelf_ud获取广告id
	unsigned GetAdId(unsigned uid,unsigned shelf_ud);

	//通过uid,shelf_ud删除广告信息
	unsigned DelAdInfo(unsigned uid,unsigned shelf_ud);

	//添加广告信息
	unsigned AddAdInfo(AdvertiseItem & adItem);

	//获取广告链表头结点
	AdvertiseList GetAdHeadList();
	//获取广告链表尾节点
	AdvertiseList GetAdTailList();

};
#endif  /*ADVERTISE_MANAGER_H_ */

