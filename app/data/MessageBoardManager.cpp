#include "MessageBoardManager.h"
#include "../battle/UserManager.h"

MessageBoardManager::MessageBoardManager()
{

}

int MessageBoardManager::OnInit()
{

	for(int idx = 0; idx < MAX_SIZE; idx++)
	{
		unsigned uid = m_data->item[idx].uid;
		if( uid != 0)
		{
			//todo 程序重启，从磁盘读取共享内存数据
			m_map_uid_uidex[uid] = idx;
			for(int jdx = 0; jdx < PER_USER_MAX_LEAVE_MSG; ++jdx)
			{
				unsigned type = m_data->item[idx].msgInfo[jdx].typeId;
				if(type != 0)
				{
					m_Msginfomap[uid].push_back(jdx);
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

bool MessageBoardManager::IsFull()
{
	return m_freeIndex.size() < PER_USER_MAX_LEAVE_MSG;
}

int MessageBoardManager::AddMsgInfo(unsigned uid,MsgInfoItem & MsgItem)
{
	unsigned Msgidx = 0;
	int uidx = GetUserMsgIndex(uid);
	if(-1 == uidx)
	{
		//玩家的第一条留言
		unsigned index = GetFreeIndex();
		if((unsigned)-1 == index)					//没有可用的index 人数已满
		{
			return -1;
		}

		m_map_uid_uidex[uid] = index;

		m_data->item[index].uid = uid;
		m_data->item[index].last_off_time = Time::GetGlobalTime();	//第一条留言,记录下时间

		MsgItem.msgidx = 0;
		m_data->item[index].msgInfo[0].Set(MsgItem);	//放在第0位置
		m_Msginfomap[uid].push_front(0);

	}
	else
	{
		//玩家已有留言,添加一条新留言,考虑留言装满时的淘汰算法
		unsigned index = m_map_uid_uidex[uid];
		unsigned type = MsgItem.typeId;
		if(m_Msginfomap[uid].size() == PER_USER_MAX_LEAVE_MSG)	//该玩家留言已装满
		{
			Msgidx = m_Msginfomap[uid].back();				//取deque最后一条记录的索引
			m_data->item[index].msgInfo[Msgidx].Clear();		//先将原有记录清零
			m_Msginfomap[uid].pop_back();					//将该记录从deque中删除

			MsgItem.msgidx = Msgidx;							//将新纪录放在刚刚清零的位置
			m_data->item[index].msgInfo[Msgidx].Set(MsgItem);	//赋新值

			m_Msginfomap[uid].push_front(Msgidx);				//放在最前面

		}
		else	//留言未满
		{
			Msgidx = FindFreePlace(m_Msginfomap[uid]);		//从deque中找一个未使用的索引
			MsgItem.msgidx = Msgidx;							//将新纪录放在该索引位置
			m_data->item[index].msgInfo[Msgidx].Set(MsgItem);	//赋新值

			m_Msginfomap[uid].push_front(Msgidx);			//将该索引放到最前面

		}
	}

	m_map_has_new[uid] = true;

	return Msgidx;
}

bool MessageBoardManager::HasNewMsgInfo(unsigned uid)
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

bool MessageBoardManager::SetHasNewMsg(unsigned uid,bool has)
{
	m_map_has_new[uid] = has;
	return true;
}

unsigned MessageBoardManager::FindFreePlace(deque<unsigned> &deq)
{
	if(!deq.size())		//空deque
	{
		return 0;
	}

	for(unsigned i = 0; i < PER_USER_MAX_LEAVE_MSG;++i)
	{
		if(std::find(deq.begin(),deq.end(),i) == deq.end())		//在0-30找一个未被使用的索引
		{
			return i;
		}
	}

	return 0;
}

bool MessageBoardManager::UpdateOffLineTime(uint32_t uid,uint32_t offtime)
{
	int uidx = GetUserMsgIndex(uid);
	if(-1 == uidx)
	{
		return false;
	}
	m_data->item[uidx].last_off_time = offtime;
	return true;
}

bool MessageBoardManager::DeleteOneMsgInfo(uint32_t uid,uint32_t Msgidx)
{
	if(Msgidx < 0 || Msgidx >= PER_USER_MAX_LEAVE_MSG)		//wrong Msgidx
	{
		return false;
	}

	if(m_Msginfomap.find(uid) == m_Msginfomap.end())	// uid not exist
	{
		return false;
	}

	unsigned index = m_map_uid_uidex[uid];

	m_data->item[index].msgInfo[Msgidx].Clear();	// 共享内存清零

	deque<unsigned>::iterator it = std::find(m_Msginfomap[uid].begin(),m_Msginfomap[uid].end(), Msgidx);
	if(it != m_Msginfomap[uid].end())
	{
		m_Msginfomap[uid].erase(it);			//m_Msginfo 删除一条记录
	}

	if(!m_Msginfomap[uid].size())			//删除一条记录后，剩余0
	{
		m_Msginfomap.erase(uid);
		m_map_uid_uidex.erase(uid);			//释放内存
		m_map_has_new.erase(uid);
	}

	return true;
}

bool MessageBoardManager::DeleteUserMsgInfo(uint32_t uid)
{

//	int index = GetUserMsgIndex(uid);
//	if(-1 == index)						// uid not exist
//	{
//		return false;
//	}

	unsigned index = m_map_uid_uidex[uid];

	m_data->item[index].Clear();		// delete one user all Msg from shm

	m_freeIndex.insert(index);			//将index回收

	m_Msginfomap.erase(uid);			    // 释放相关内存

	m_map_has_new.erase(uid);

	m_map_uid_uidex.erase(uid);

	return true;
}

bool MessageBoardManager::CheckClearMsgInfo()
{
	unsigned now_ts = Time::GetGlobalTime();
	unsigned count = DELETE_COUNT_PER_TIME_MSG;							//每小时检查一次，单次最多淘汰人数
	map<unsigned,unsigned>::iterator it = m_map_uid_uidex.begin();
	set<unsigned> set_del;
	for(;it != m_map_uid_uidex.end();++it)
	{
		unsigned uid = it->first;
		unsigned index = it->second;
		unsigned lastofftime = m_data->item[index].last_off_time;
		if(lastofftime > 0 && lastofftime + 86400*MAX_INTERNAL_DAYS_MSG < now_ts && !UserManager::Instance()->IsOnline(uid))		//上次下线时间超过3天
		{
			set_del.insert(uid);
			--count;
		}
		if(!count)
		{
			break;
		}
	}

	for(set<unsigned>::iterator iter = set_del.begin();iter != set_del.end();++iter)
	{
		DeleteUserMsgInfo(*iter);
	}

	info_log("delete %u user's LeaveMsg info",DELETE_COUNT_PER_TIME_MSG - count);

	return true;
}

bool MessageBoardManager::CheckClearDemo()
{
	unsigned now_ts = Time::GetGlobalTime();
	unsigned count = DELETE_COUNT_PER_TIME_MSG;							//单次最多淘汰人数
	map<unsigned,unsigned>::iterator it = m_map_uid_uidex.begin();
	for(;it != m_map_uid_uidex.end();++it)
	{
		unsigned uid = it->first;
		unsigned index = it->second;
		unsigned lastofftime = m_data->item[index].last_off_time;
		if(lastofftime > 0 && lastofftime + 30 < now_ts && !UserManager::Instance()->IsOnline(uid))	//上次下线时间超过30s
		{
			DeleteUserMsgInfo(uid);
			--count;
		}
		if(!count)
		{
			break;
		}
	}

	debug_log("delete %u user's LeaveMsg info",DELETE_COUNT_PER_TIME - count);

	return true;
}

int MessageBoardManager::GetUserMsgIndex(uint32_t uid)
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

map<unsigned,deque<unsigned> > & MessageBoardManager::GetMsgInfoMap()
{
	return m_Msginfomap;
}

MsgInfoItem & MessageBoardManager::GetMsgInfoItem(unsigned uid,unsigned Msgidx)
{
	MsgInfoItem msgitem;
	if(Msgidx >= PER_USER_MAX_LEAVE_MSG)
	{
		return msgitem;
	}
	unsigned index = m_map_uid_uidex[uid];
	if(index >= MAX_PLAYER_SIZE_MSG)
	{
		return msgitem;
	}
	return m_data->item[index].msgInfo[Msgidx];
}

unsigned MessageBoardManager::GetFreeIndex()
{
	if(m_freeIndex.empty())
		return (unsigned)-1;
	unsigned ret = *(m_freeIndex.begin());		//从set中取一个index
	m_freeIndex.erase(m_freeIndex.begin());

	return ret;
}

