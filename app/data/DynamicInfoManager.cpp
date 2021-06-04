#include "DynamicInfoManager.h"
#include "../battle/UserManager.h"

DynamicInfoManager::DynamicInfoManager()
{

}

int DynamicInfoManager::OnInit()
{

	for(int idx = 0; idx < MAX_SIZE; idx++)
	{
		unsigned uid = m_data->item[idx].uid;
		if( uid != 0)
		{
			//todo 程序重启，从磁盘读取共享内存数据
			m_map_uid_uidex[uid] = idx;
			for(int jdx = 0; jdx < PER_USER_MAX_DYNAMIC_INFO; ++jdx)
			{
				unsigned type = m_data->item[idx].dyInfo[jdx].type_id;
				if(IS_UP_TOP_TYPE(type))
				{
					m_dyinfomap[uid].push_front(jdx);
				}
				else if(type != 0)
				{
					m_dyinfomap[uid].push_back(jdx);
				}
			}
		}
		else
		{
			m_freeIndex.insert(idx);
		}
	}
	return 0;
}

bool DynamicInfoManager::IsFull()
{
	return m_freeIndex.size() < PER_USER_MAX_DYNAMIC_INFO;
}

bool DynamicInfoManager::AddDyInfo(unsigned uid,DyInfoItem & dyItem)
{
	unsigned dyidx = 0;
	int uidx = GetUserDyIndex(uid);
	if(-1 == uidx)
	{
		//玩家的第一条动态
		unsigned index = GetFreeIndex();
		if((unsigned)-1 == index)					//没有可用的index 人数已满
		{
			return false;
		}

		m_map_uid_uidex[uid] = (unsigned)index;

		m_data->item[index].uid = uid;
		m_data->item[index].last_off_time = Time::GetGlobalTime();	//第一条动态,记录下时间

		dyItem.dyidx = 0;
		m_data->item[index].dyInfo[0].Set(dyItem);	//放在第0位置
		m_dyinfomap[uid].push_front(0);

	}
	else
	{
		//玩家已有动态,添加一条新动态,考虑动态装满时的淘汰算法
		unsigned index = m_map_uid_uidex[uid];
		unsigned type = dyItem.type_id;
		if(m_dyinfomap[uid].size() == PER_USER_MAX_DYNAMIC_INFO)	//该玩家动态已装满
		{
			if(IS_UP_TOP_TYPE(type))	//置顶消息
			{
				dyidx = m_dyinfomap[uid].back();				//取deque最后一条记录的索引
				m_data->item[index].dyInfo[dyidx].Clear();		//先将原有记录清零
				m_dyinfomap[uid].pop_back();					//将该记录从deque中删除

				dyItem.dyidx = dyidx;							//将新纪录放在刚刚清零的位置
				m_data->item[index].dyInfo[dyidx].Set(dyItem);	//赋新值
				m_dyinfomap[uid].push_front(dyidx);				//将该索引放到最前面
			}
			else						//普通消息
			{
				deque<unsigned>::iterator it = SearchPlaceForNormal(uid,m_dyinfomap[uid]);	//为普通动态找一个deque位置
				if(it == m_dyinfomap[uid].end())
				{
					m_map_has_new[uid] = false;
					return false;							 	//动态已被置顶消息占满，无法插入普通消息
				}
				bool lastPlace = false;
				if((it + 1) == m_dyinfomap[uid].end())			//只有最后一个位置不是置顶消息
				{
					lastPlace = true;
				}

				dyidx = m_dyinfomap[uid].back();				//取deque最后一条记录的索引
				m_data->item[index].dyInfo[dyidx].Clear();		//先将原有记录清零
				m_dyinfomap[uid].pop_back();					//将最后一条记录从deque中删除

				dyItem.dyidx = dyidx;							//将新纪录放在刚刚清零的位置
				m_data->item[index].dyInfo[dyidx].Set(dyItem);	//赋新值
				if(!lastPlace)
				{
					m_dyinfomap[uid].insert(it,1,dyidx);		//在it前插入一条动态dyidx
				}
				else
				{
					m_dyinfomap[uid].push_back(dyidx);		    //在deque的末尾插入一条动态dyidx
				}
			}
		}
		else	//动态未满
		{
			dyidx = FindFreePlace(m_dyinfomap[uid]);		//从deque中找一个未使用的索引
			dyItem.dyidx = dyidx;							//将新纪录放在该索引位置
			m_data->item[index].dyInfo[dyidx].Set(dyItem);	//赋新值

			if(IS_UP_TOP_TYPE(type))	//置顶消息
			{
				m_dyinfomap[uid].push_front(dyidx);			//将该索引放到最前面
			}
			else						//普通消息
			{
				deque<unsigned>::iterator it = SearchPlaceForNormal(uid,m_dyinfomap[uid]);	//为普通动态找一个deque位置
				m_dyinfomap[uid].insert(it,1,dyidx);		//在it前插入一条动态dyidx
			}
		}
	}

	m_map_has_new[uid] = true;

	return true;
}

bool DynamicInfoManager::HasNewDyInfo(unsigned uid)
{
	map<unsigned,bool>::iterator it = m_map_has_new.find(uid);
	if(it != m_map_has_new.end())
	{
		if(it->second)
		{
			return true;
		}
	}
	return false;
}

bool DynamicInfoManager::SetHasNewDy(unsigned uid,bool has)
{
	m_map_has_new[uid] = has;
	return true;
}

unsigned DynamicInfoManager::FindFreePlace(deque<unsigned> &deq)
{
	if(!deq.size())		//空deque
	{
		return 0;
	}

	for(unsigned i = 0; i < PER_USER_MAX_DYNAMIC_INFO;++i)
	{
		if(std::find(deq.begin(),deq.end(),i) == deq.end())		//在0-30找一个未被使用的索引
		{
			return i;
		}
	}

	return 0;
}

deque<unsigned>::iterator DynamicInfoManager::SearchPlaceForNormal(unsigned uid,deque<unsigned> &deq)
{
	deque<unsigned>::iterator it = deq.begin();
	for(;it != deq.end();++it)
	{
		unsigned index = m_map_uid_uidex[uid];
		unsigned type = m_data->item[index].dyInfo[*it].type_id;
		if(!IS_UP_TOP_TYPE(type))
		{
			break;
		}
	}

	return it;
}

bool DynamicInfoManager::UpdateOffLineTime(uint32_t uid,uint32_t offtime)
{
	int uidx = GetUserDyIndex(uid);
	if(-1 == uidx)
	{
		return false;
	}
	m_data->item[uidx].last_off_time = offtime;
	return true;
}

bool DynamicInfoManager::DeleteOneDyInfo(uint32_t uid,uint32_t dyidx)
{
	if(dyidx < 0 || dyidx >= PER_USER_MAX_DYNAMIC_INFO)		//wrong dyidx
	{
		return false;
	}

	if(m_dyinfomap.find(uid) == m_dyinfomap.end())	// uid not exist
	{
		return false;
	}

	unsigned index = m_map_uid_uidex[uid];

	m_data->item[index].dyInfo[dyidx].Clear();	// 共享内存清零

	deque<unsigned>::iterator it = std::find(m_dyinfomap[uid].begin(),m_dyinfomap[uid].end(), dyidx);
	if(it != m_dyinfomap[uid].end())
	{
		m_dyinfomap[uid].erase(it);			//m_dyinfo 删除一条记录
	}

	return true;
}

bool DynamicInfoManager::DegradeDy(unsigned uid,unsigned dyidx)
{
	if(dyidx < 0 || dyidx >= PER_USER_MAX_DYNAMIC_INFO)		//wrong dyidx
	{
		return false;
	}

	if(m_dyinfomap.find(uid) == m_dyinfomap.end())	// uid not exist
	{
		return false;
	}

	unsigned index = m_map_uid_uidex[uid];
	m_data->item[index].dyInfo[dyidx].type_id += 100;	//201 -> 301

	//将留言放到置顶留言的后面
	deque<unsigned>::iterator it1 = std::find(m_dyinfomap[uid].begin(),m_dyinfomap[uid].end(),dyidx);
	m_dyinfomap[uid].erase(it1);					//先将dyidx从deque中删除

	deque<unsigned>::iterator it2 = SearchPlaceForNormal(uid,m_dyinfomap[uid]);	//为普通动态找一个deque位置
	m_dyinfomap[uid].insert(it2,1,dyidx);		//在it前插入一条动态dyidx

	return true;
}

int DynamicInfoManager::GetFriendOrderIndex(unsigned uid,unsigned dyidx)
{

	if(dyidx < 0 || dyidx >= PER_USER_MAX_DYNAMIC_INFO)		//wrong dyidx
	{
		return -1;
	}

	if(m_dyinfomap.find(uid) == m_dyinfomap.end())	// uid not exist
	{
		return -1;
	}

	unsigned index = m_map_uid_uidex[uid];

	return m_data->item[index].dyInfo[dyidx].windex;
}

bool DynamicInfoManager::DeleteUserDyInfo(uint32_t uid)
{

//	int index = GetUserDyIndex(uid);
//	if(-1 == index)						// uid not exist
//	{
//		return false;
//	}

	unsigned index = m_map_uid_uidex[uid];

	m_data->item[index].Clear();		// delete one user all dy from shm

	m_freeIndex.insert(index);			//将index回收

	m_dyinfomap.erase(uid);			    // 释放相关内存

	m_map_has_new.erase(uid);

	m_map_uid_uidex.erase(uid);

	return true;
}

bool DynamicInfoManager::CheckClearDyInfo()
{
	unsigned now_ts = Time::GetGlobalTime();
	unsigned count = DELETE_COUNT_PER_TIME;							//每小时检查一次，单次最多淘汰人数
	map<unsigned,unsigned>::iterator it = m_map_uid_uidex.begin();
	set<unsigned> set_del;
	for(;it != m_map_uid_uidex.end();++it)
	{
		unsigned uid = it->first;
		unsigned index = it->second;
		unsigned lastofftime = m_data->item[index].last_off_time;
		if(lastofftime > 0 && lastofftime + 86400*MAX_INTERNAL_DAYS < now_ts && !UserManager::Instance()->IsOnline(uid))		//上次下线时间超过3天
		{
			set_del.insert(uid);								//淘汰多天未登陆玩家的全部动态
			--count;
		}
		if(!count)
		{
			break;
		}
	}

	for(set<unsigned>::iterator iter = set_del.begin();iter != set_del.end();++iter)
	{
		DeleteUserDyInfo(*iter);
	}

	info_log("delete %u user's dynamic info",DELETE_COUNT_PER_TIME - count);

	unsigned sum = DELETE_COUNT_PER_TIME*10;						//每小时检查一次，检查sum个玩家的超时动态
	map<unsigned,unsigned>::iterator iter_map = m_map_uid_uidex.begin();
	for(;iter_map != m_map_uid_uidex.end();++iter_map)
	{
		unsigned uid = iter_map->first;
		unsigned index = iter_map->second;
		unsigned now = Time::GetGlobalTime();
		set<unsigned> set_dyidx_del;
		deque<unsigned>::iterator it_deq = m_dyinfomap[uid].begin();
		for(;it_deq != m_dyinfomap[uid].end();++it_deq)
		{
			if(m_data->item[index].dyInfo[*it_deq].ts + 86400 < now)	//淘汰本服超时动态
			{
				set_dyidx_del.insert(*it_deq);
				--sum;
			}
		}

		for(set<unsigned>::iterator it_set = set_dyidx_del.begin();it_set != set_dyidx_del.end();++it_set)
		{
			DeleteOneDyInfo(uid,*it_set);
		}

		if(sum < PER_USER_MAX_DYNAMIC_INFO)
		{
			break;
		}
	}
	info_log("total deleted dynamic info: %u",DELETE_COUNT_PER_TIME*10 - sum);

	return true;
}

bool DynamicInfoManager::CheckClearDemo()
{
	unsigned now_ts = Time::GetGlobalTime();
	unsigned count = DELETE_COUNT_PER_TIME;							//单次最多淘汰人数
	map<unsigned,unsigned>::iterator it = m_map_uid_uidex.begin();
	for(;it != m_map_uid_uidex.end();++it)
	{
		unsigned uid = it->first;
		unsigned index = it->second;
		unsigned lastofftime = m_data->item[index].last_off_time;
		if(lastofftime > 0 && lastofftime + 30 < now_ts && !UserManager::Instance()->IsOnline(uid))	//上次下线时间超过30s
		{
			DeleteUserDyInfo(uid);
			--count;
		}
		if(!count)
		{
			break;
		}
	}

	debug_log("delete %u user's dynamic info",DELETE_COUNT_PER_TIME - count);

	return true;
}

int DynamicInfoManager::GetUserDyIndex(uint32_t uid)
{
	map<uint32_t,uint32_t>::iterator it = m_map_uid_uidex.find(uid);
	if(it != m_map_uid_uidex.end())
	{
		return m_map_uid_uidex[uid];
	}
	else
	{
		return -1;
	}
}

map<unsigned,deque<unsigned> > & DynamicInfoManager::GetDyInfoMap()
{
	return m_dyinfomap;
}

DyInfoItem & DynamicInfoManager::GetDyInfoItem(unsigned uid,unsigned dyidx)
{
	unsigned index = m_map_uid_uidex[uid];
	return m_data->item[index].dyInfo[dyidx];
}

unsigned DynamicInfoManager::GetFreeIndex()
{
	if(m_freeIndex.empty())
		return (unsigned)-1;
	unsigned ret = *(m_freeIndex.begin());		//从set中取一个index
	m_freeIndex.erase(m_freeIndex.begin());

	return ret;
}

