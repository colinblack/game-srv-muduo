/*
 * BaseManager.cpp
 *
 *  Created on: 2016-8-16
 *      Author: Ralf
 */

#include "BaseManager.h"

int BaseManager::OnInit() {
	for(unsigned i=0;i<MAX_BUFF;++i)
	{
		if(!m_data->Empty(i))
			m_map[m_data->data[i].uid] = i;
	}
	return 0;
}
void BaseManager::OnExit() {
}

void BaseManager::OnTimer2() {
}

int BaseManager::CheckBuff(unsigned uid)
{
	if(!m_map.count(uid))
		return R_ERR_NO_DATA;
	return 0;
}
int BaseManager::AddBuff(unsigned uid)
{
	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}
	unsigned index = GetFreeIndex();
	if(index == -1)
		return R_ERR_DATA;
	DataBase b;
	b.uid = uid;
	if(Add(index, b))
	{
		m_map[uid] = index;
		return 0;
	}
	else
		return R_ERR_DATA;
}
int BaseManager::LoadBuff(unsigned uid)
{
	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}
	unsigned index = GetFreeIndex();
	if(index == -1)
		return R_ERR_DATA;
	m_data->data[index].uid = uid;
	int ret = Load(index);
	if(ret == 0)
	{
		m_map[uid] = index;
		return 0;
	}
	else
		return ret;
}
unsigned BaseManager::GetIndex(unsigned uid)
{
	if(m_map.count(uid))
		return m_map[uid];
	return -1;
}
void BaseManager::GetClear1(vector<unsigned> &uids)
{
	for(map<unsigned, unsigned>::iterator it=m_map.begin();it!=m_map.end();++it)
	{
		if(m_data->data[it->second].CanClear() && !m_data->NeedWork(it->second) && m_data->GetPlusTS(it->second) + 86400*3 < Time::GetGlobalTime())
			uids.push_back(it->first);
	}
}
void BaseManager::GetClear(vector<unsigned> &uids)
{
	for(map<unsigned, unsigned>::iterator it=m_map.begin();it!=m_map.end();++it)
	{
		if(m_data->data[it->second].CanClear() && !m_data->NeedWork(it->second))
			uids.push_back(it->first);
	}
}
void BaseManager::TryClear(vector<unsigned> &uids)
{
	for(map<unsigned, unsigned>::iterator it=m_map.begin();it!=m_map.end();++it)
	{
		/*
		debug_log("%u,%u,%u,%s,%s,%s",m_data->data[it->second].uid, m_data->data[it->second].last_login_time, m_data->data[it->second].last_off_time
				,m_data->data[it->second].CanClear()?"can clear":"not clear"
				,m_data->NeedWork(it->second)?"need word":"not work"
				,(m_data->data[it->second].CanClear() && !m_data->NeedWork(it->second))?"yes":"no");
		*/
		if(m_data->data[it->second].CanClear() && !m_data->NeedWork(it->second))
			uids.push_back(it->first);
	}
}
void BaseManager::DoClear(unsigned uid)
{
	if(m_map.count(uid))
	{
		Clear(m_map[uid]);
		m_map.erase(uid);
	}
}
void BaseManager::DoSave(unsigned uid)
{
	if(m_map.count(uid))
		AddSave(m_map[uid]);
}

DataBase& BaseManager::Get(unsigned uid)
{
	int index = GetIndex(uid);
	if (index < 0)
	{
		error_log("uid: %u, index=%u", uid, index);
		throw std::runtime_error("get_user_info_from_buff_error");
	}

	return  m_data->data[index];
}

////////////////////////////////////////////////////////////////////////////////////////////



bool BaseManager::UpdateDatabase(unsigned index)
{
	return m_data->MarkChange(index);
}

bool BaseManager::UpdateDatabase(DataBase& database)
{
	unsigned index = GetIndex(database.uid);

	if (-1 == index)
	{
		return false;
	}

	return UpdateDatabase(index);
}
