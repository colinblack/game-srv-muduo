#include "FriendOrderManager.h"
#include "../battle/LogicUserManager.h"

FriendOrderManager::FriendOrderManager()
{

}

int FriendOrderManager::OnInit()
{
	//m_dyinfolist.clear();
	for(int idx = 0; idx < MAX_SIZE; idx++)
	{
		unsigned uid = m_data->item[idx].owneruid;
		if( uid != 0)
		{
			//程序重启，从磁盘读取共享内存数据
			if(!m_foinfomap[uid].size())
			{
				for(unsigned i = 0;i < MAX_FO_BASKET_SIZE;++i)
				{
					m_foinfomap[uid].push_front(MAX_FRIEND_ORDER_NUM);	//空闲栏位foidx;
				}
			}

			if(m_data->item[idx].status <= STATUS_FO_FREEZE)
			{
				unsigned basket = m_data->item[idx].sourceId;
				m_foinfomap[uid][basket] = idx;
			}
			else
			{
				m_foinfomap[uid].push_back(idx);
			}
		}
		else
		{
			m_freeIndex.insert(idx);
		}
	}
	return 0;
}

bool FriendOrderManager::IsFull()
{
	return m_freeIndex.size() < PER_USER_MAX_FRIEND_ORDER;
}

unsigned FriendOrderManager::GetFreeIndex()
{
	if(m_freeIndex.empty())
		return (unsigned)-1;
	unsigned ret = *(m_freeIndex.begin());		//从set中取一个index
	m_freeIndex.erase(m_freeIndex.begin());

	return ret;
}

map<unsigned,deque<unsigned> > & FriendOrderManager::GetFoInfoMap()
{
	return m_foinfomap;
}

FoInfoItem & FriendOrderManager::GetFoInfoItem(unsigned foidx)
{
	return m_data->item[foidx];
}

unsigned FriendOrderManager::GetSourceFoStatus(unsigned senderuid,unsigned sourceId)
{
	if(sourceId >= MAX_FO_BASKET_SIZE)
	{
		return 0;
	}

	unsigned foidx = m_foinfomap[senderuid][sourceId];
	if(foidx >= MAX_FRIEND_ORDER_NUM)					//源订单是个空闲栏位
	{
		return STATUS_FO_FREE_BASKET;
	}
	return m_data->item[foidx].status;
}

bool FriendOrderManager::AddHelperUid(unsigned senderuid,unsigned sourceId,unsigned helperuid)
{
	if(sourceId >= MAX_FO_BASKET_SIZE)
	{
		return false;
	}

	unsigned foidx = m_foinfomap[senderuid][sourceId];
	if(foidx >= MAX_FRIEND_ORDER_NUM)					//源订单是个空闲栏位
	{
		return false;
	}

	m_data->item[foidx].helperuid = helperuid;

	return true;
}

bool FriendOrderManager::ChangeShmFoStatus(unsigned uid,unsigned index,unsigned newStatus)
{
	unsigned foidx = m_foinfomap[uid][index];
	if(foidx >= MAX_FRIEND_ORDER_NUM)
	{
		return false;
	}
	m_data->item[foidx].status = newStatus;

	return true;
}

bool FriendOrderManager::AddFoInfo(uint32_t uid,FoInfoItem &foItem,int basket)
{
	unsigned shm_index = GetFreeIndex();
	if((unsigned)-1 == shm_index)					//没有可用的index 总订单数已满
	{
		return false;
	}
	if(basket >= MAX_FO_BASKET_SIZE)			//没有这个订单栏位
	{
		return false;
	}

	if(m_foinfomap.end() == m_foinfomap.find(uid))	//map中没有uid的记录
	{
		for(unsigned i = 0;i < MAX_FO_BASKET_SIZE;++i)
		{
			m_foinfomap[uid].push_front(MAX_FRIEND_ORDER_NUM);	//空闲栏位foidx;
		}
	}

	if(foItem.status <= STATUS_FO_FREEZE && basket >= 0)	//这是一条源订单
	{
		unsigned source_size = GetSourceFoSize(m_foinfomap[uid]);
		if(MAX_FO_BASKET_SIZE == source_size)				//没有空闲的订单栏位
		{
			return false;
		}
		if(m_foinfomap[uid][basket] != MAX_FRIEND_ORDER_NUM)
		{
			return false;									//这不是一个空闲栏位
		}
		m_foinfomap[uid][basket] = shm_index;
	}
	else													//这是好友订单
	{
		if(m_foinfomap[uid].size() < MAX_FO_BASKET_SIZE)
		{
			for(unsigned i = 0;i < MAX_FO_BASKET_SIZE;++i)
			{
				m_foinfomap[uid].push_front(MAX_FRIEND_ORDER_NUM);	//空闲栏位foidx;
			}
		}

		deque<unsigned>::iterator it = SearchPlaceForNormalOrder(m_foinfomap[uid]);
		unsigned normal_size = m_foinfomap[uid].end() - it;
		if(normal_size == PER_USER_MAX_FRIEND_ORDER)		//好友订单数已装满
		{
			unsigned last_foidx = m_foinfomap[uid].back();
			m_foinfomap[uid].pop_back();
			m_data->item[last_foidx].Clear();
			m_freeIndex.insert(last_foidx);
		}
		m_foinfomap[uid].insert(it,1,shm_index);				//放在源订单的后面,普通好友订单的最前面
	}

	m_data->item[shm_index].Set(foItem);

	return true;
}

unsigned FriendOrderManager::GetSourceFoSize(deque<unsigned> &deq)
{
	unsigned ret = 0;
	deque<unsigned>::iterator it = deq.begin();
	for(unsigned num = MAX_FO_BASKET_SIZE;it != deq.end() && num;++it,--num)
	{
		if(*it >= MAX_FRIEND_ORDER_NUM)			//空闲栏位foidx
		{
			continue;
		}
		if(m_data->item[*it].status <= STATUS_FO_FREEZE && m_data->item[*it].status > STATUS_FO_FREE_BASKET)
		{
			++ret;
		}
	}

	return ret;
}

deque<unsigned>::iterator FriendOrderManager::SearchPlaceForNormalOrder(deque<unsigned> &deq)
{
	deque<unsigned>::iterator it = deq.begin();
	for(;it != deq.end();++it)
	{
		if(*it >= MAX_FRIEND_ORDER_NUM)			//空闲栏位foidx
		{
			continue;
		}
		if(m_data->item[*it].status > STATUS_FO_FREEZE)
		{
			break;
		}
	}

	return it;
}

bool FriendOrderManager::DeleteOneSourceOrder(unsigned uid,unsigned basket)
{

	if(basket >= MAX_FO_BASKET_SIZE || basket < 0)	//没有这个订单栏位
	{
		return false;
	}

	if(!GetSourceFoSize(m_foinfomap[uid]))			//玩家没有源订单
	{
		return false;
	}

	unsigned foidx = m_foinfomap[uid][basket];

	m_foinfomap[uid][basket] = MAX_FRIEND_ORDER_NUM;//设置为空闲栏位foidx
	m_data->item[foidx].Clear();
	m_freeIndex.insert(foidx);

	return true;
}

bool FriendOrderManager::CheckClearFo()
{
	map<unsigned,deque<unsigned> >::iterator it_map = m_foinfomap.begin();
	for(;it_map != m_foinfomap.end();)
	{
		if(!it_map->second.size())						//玩家在map中没有订单了,从map中剔除,释放内存
		{
			m_foinfomap.erase(it_map++);
		}
		else
		{
			//玩家在map中有订单
			deque<unsigned>::iterator it_deq = it_map->second.begin();
			for(;it_deq != it_map->second.end();)
			{
				if(!GetSourceFoSize(it_map->second) && it_map->second.size() == MAX_FO_BASKET_SIZE)
				{
					it_map->second.clear();					//玩家没有源订单了,也没有好友订单了
					break;
				}

				unsigned foidx = *it_deq;
				if(foidx >= MAX_FRIEND_ORDER_NUM)			//跳过空闲栏位
				{
					++it_deq;
					continue;
				}
				unsigned now = Time::GetGlobalTime();
				if(m_data->item[foidx].status > STATUS_FO_FREEZE && now > m_data->item[foidx].deadtime)	//好友订单
				{
					it_deq = it_map->second.erase(it_deq);	//将foidx从deque中删除
					m_data->item[foidx].Clear();
					m_freeIndex.insert(foidx);
				}
				else if(m_data->item[foidx].status == STATUS_FO_OTHER_BOUGHT || m_data->item[foidx].status == STATUS_FO_BUY_SUCCESS)
				{
					it_deq = it_map->second.erase(it_deq);	//将foidx从deque中删除
					m_data->item[foidx].Clear();
					m_freeIndex.insert(foidx);
				}
				else if(m_data->item[foidx].status == STATUS_FO_FREEZE && now > m_data->item[foidx].deadtime)
				{
					*it_deq = MAX_FRIEND_ORDER_NUM;			//设置为空闲栏位foidx
					m_data->item[foidx].Clear();
					m_freeIndex.insert(foidx);
				}
				else if(m_data->item[foidx].status == STATUS_FO_WAIT_BUY && now > m_data->item[foidx].deadtime)
				{
					m_data->item[foidx].status = STATUS_FO_OVER_DATE;	//设置成超时
					++it_deq;
				}
				else
				{
					++it_deq;
				}
			}
			++it_map;
		}
	}

	return true;
}

bool FriendOrderManager::CheckRecyleSourceFo()
{

	map<unsigned,deque<unsigned> >::iterator it_map = m_foinfomap.begin();
	for(;it_map != m_foinfomap.end();++it_map)
	{
		unsigned deq_size = it_map->second.size();
		if(deq_size < MAX_FO_BASKET_SIZE)				//跳过大小不足的deque
		{
			continue;
		}

		unsigned now = Time::GetGlobalTime();
		for(unsigned idx = 0;idx < MAX_FO_BASKET_SIZE;++idx)
		{
			unsigned foidx = it_map->second[idx];
			if(foidx >= MAX_FRIEND_ORDER_NUM)			//跳过空闲栏位
			{
				continue;
			}
			if(m_data->item[foidx].status == STATUS_FO_OVER_DATE && now > m_data->item[foidx].deadtime + SOURCE_FO_DELETE_TIME)
			{
				//系统帮助玩家自动回收 超时很久的源订单
				unsigned coin = m_data->item[foidx].coin;
				unsigned uid = it_map->first;

				DeleteOneSourceOrder(uid,idx);			//删除源订单

				DBCUserBaseWrap user(uid);
				user.AddCoin(coin,"friend_order_time_out_recycle");		//回收金币
			}
			else if(m_data->item[foidx].status == STATUS_FO_SELL_OUT && now > m_data->item[foidx].deadtime + SOURCE_FO_DELETE_TIME)
			{
				//系统帮助玩家自动回收 超时很久的源订单
				unsigned productId = m_data->item[foidx].productId;
				unsigned count = m_data->item[foidx].count;
				unsigned uid = it_map->first;

				DeleteOneSourceOrder(uid,idx);			//删除源订单

				DataCommon::PropsAllChangeCPP * msg = new DataCommon::PropsAllChangeCPP;
				LogicPropsManager::Instance()->AddProps(uid,productId,count,"friend_order_time_out_recycle",msg);
			}
		}

	}

	return true;
}

unsigned FriendOrderManager::GetFoIndex(unsigned senderuid,unsigned myuid)
{
	if(m_foinfomap.end() == m_foinfomap.find(myuid))
	{
		return 0;
	}

	deque<unsigned>::iterator it = m_foinfomap[myuid].begin();
	for(;it != m_foinfomap[myuid].end();++it)
	{
		if(*it == MAX_FRIEND_ORDER_NUM)
		{
			continue;
		}
		if(senderuid == m_data->item[*it].senderUid)
		{
			return it - m_foinfomap[myuid].begin();
		}
	}

	return 0;

}

