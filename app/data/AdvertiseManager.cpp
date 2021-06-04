#include "AdvertiseManager.h"


AdvertiseManager::AdvertiseManager():m_adheadList(NULL),m_adtailList(NULL)
{
	InitList();
}

int AdvertiseManager::OnInit()
{
	for(int i = 0; i < MAX_SIZE; i++)
	{
		if(m_data->item[i].id != 0) {
			//初始化建立索引
			m_map[m_data->item[i].id] = i;
			m_adinfomap[m_data->item[i].uid][m_data->item[i].shelf_ud] = m_data->item[i].id;
			//将广告数据添加到链表中
			AddNode(m_data->item[i]);

		}
		else
			m_freeIndex.insert(i);
	}
	return 0;
}

//初始化带头结点与尾借点的双向链表
int AdvertiseManager::InitList()
{
	m_adheadList = (AdvertiseNode *)malloc(sizeof(AdvertiseNode));
	m_adtailList = (AdvertiseNode *)malloc(sizeof(AdvertiseNode));
	if(m_adheadList == NULL || m_adtailList == NULL)
	{
		error_log("init adlist error");
		throw std::runtime_error("init adlist error");
	}
	m_adheadList->prior = NULL;
	m_adtailList->next  = NULL;

	//构建空链表
	m_adheadList->next = m_adtailList;
	m_adtailList->prior = m_adheadList;
	return 0;
}

//头插法添加链表数据
int AdvertiseManager::AddNode(AdvertiseItem data)
{
	AdvertiseList phead = m_adheadList;

	AdvertiseNode *node = (AdvertiseNode *)malloc(sizeof(AdvertiseNode));
	if(node == NULL)
	{
		error_log("add node failed");
		return -1;
	}

	//添加节点
	node->data  = data;
	node->prior = phead;
	node->next  = phead->next;
	phead->next->prior = node;
	phead->next = node;

	return 0;
}

//删除链表指定节点
int AdvertiseManager::DelNode(unsigned id)
{
	AdvertiseList p = m_adheadList->next;
	while(p != m_adtailList)
	{
		if(p->data.id == id)
		{
			p->prior->next = p->next;
			p->next->prior = p->prior;

			delete p;
			break;
		}
		else
			p = p->next;
	}
	return 0;
}

unsigned AdvertiseManager::GetNewAdUd()
{
	std::set<unsigned>::iterator udset =  m_freeIndex.begin();
	if(udset == m_freeIndex.end())
	{
		error_log("GetNewAdUd failed");
		throw std::runtime_error("GetNewAdUd failed");
	}
	return *udset + 1;
}

unsigned AdvertiseManager::GetAdCnt()
{
	unsigned count = 0;
	AdvertiseList p = m_adheadList->next;
	while(p != m_adtailList) {
		count ++;
		p = p->next;
	}
	return count;
}

unsigned AdvertiseManager::GetAdId(unsigned uid,unsigned shelf_ud)
{
	unsigned id = 0;
	map<unsigned,map<unsigned,unsigned> >::iterator it = m_adinfomap.find(uid);
	if(it != m_adinfomap.end())
	{
		map<unsigned,unsigned>::iterator itor = it->second.find(shelf_ud);
		if(itor != it->second.end())
		{
			id = itor->second;
		}
	}
	return id;
}

unsigned AdvertiseManager::DelAdInfo(unsigned uid,unsigned shelf_ud)
{
	unsigned id  = GetAdId(uid,shelf_ud);

	if(id) {
		//删除m_adinfomap索引里的信息
		map<unsigned,map<unsigned,unsigned> >::iterator it = m_adinfomap.find(uid);
		if(it != m_adinfomap.end())
		{
			map<unsigned,unsigned>::iterator itor = it->second.find(shelf_ud);
			if(itor != it->second.end())
			{
				m_adinfomap[uid].erase(itor);
			}
		}

		//删除共享内存数据
		Del(id);

		//删除链表信息
		DelNode(id);

		//添加调试日志
		debug_log("DelAdInfo,uid=%u,shelf_ud=%u,id=%u",uid,shelf_ud,id);

	}
}

unsigned AdvertiseManager::AddAdInfo(AdvertiseItem & adItem)
{
	//添加信息
	m_adinfomap[adItem.uid][adItem.shelf_ud] = adItem.id;
	Add(adItem);
	AddNode(adItem);
	//添加调试日志
	debug_log("AddAdInfo,uid=%u,shelf_ud=%u,id=%u",adItem.uid,adItem.shelf_ud,adItem.id);
}

AdvertiseList AdvertiseManager::GetAdHeadList()
{
	return m_adheadList;
}

AdvertiseList AdvertiseManager::GetAdTailList()
{
	return m_adtailList;
}
