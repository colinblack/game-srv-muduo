/*
 * TruckManager.cpp
 *
 *  Created on: 2018-3-5
 *      Author: Administrator
 */
#include "TruckManager.h"

int DataTruckManager::OnInit()
{
	for(unsigned i = 0; i < MAX_BUFF; ++i)
	{
		if(!m_data->Empty(i))
		{
			uint32_t uid = m_data->data[i].uid;
			m_map[uid] = i;
		}
	}

	return 0;
}

int DataTruckManager::Init(unsigned uid)
{
	LoadBuff(uid);
}

int DataTruckManager::CheckLogin(unsigned uid)
{
	int ret = LoadBuff(uid);

	if (ret)
	{
		return ret;
	}

	return 0;
}

DataTruck & DataTruckManager::GetDataTruck(unsigned uid)
{
	//load内已判断是否加载到内存
	int ret = LoadBuff(uid);

	if (ret > 0 && ret != R_ERR_NO_DATA)
	{
		throw runtime_error("load truck data error");
	}

	if (!m_map.count(uid))
	{
		ret = AddBuff(uid);

		if (ret)
		{
			throw runtime_error("add truck data error");
		}
	}

	unsigned index = m_map[uid];
	return m_data->data[index];
}

void DataTruckManager::SetMessage(uint32_t uid, User::User* reply)
{
	if (!m_map.count(uid))
	{
		return;
	}

	unsigned index = m_map[uid];
	DataTruck & sTruck = m_data->data[index];

	sTruck.SetMessage(reply->mutable_truck());
}

void DataTruckManager::SetMessage(uint32_t uid, ProtoArchive::UserData* reply)
{
	if (!m_map.count(uid))
	{
		return;
	}

	unsigned index = m_map[uid];
	DataTruck & sTruck = m_data->data[index];

	sTruck.SetMessage(reply->mutable_truck());
}

void DataTruckManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	GetDataTruck(uid).FromMessage(&data.truck());
}

bool DataTruckManager::UpdateTruck(DataTruck & truck)
{
	unsigned uid = truck.uid;

	if (!m_map.count(uid))
	{
		throw runtime_error("truck_data_error");
	}

	unsigned index = m_map[uid];

	return  m_data->MarkChange(index);
}

int DataTruckManager::CheckBuff(unsigned uid)
{
	if(!m_map.count(uid))
		return R_ERR_NO_DATA;

	return 0;
}

int DataTruckManager::AddBuff(unsigned uid)
{
	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}

	unsigned index = GetFreeIndex();

	if(index == -1)
	{
		error_log("get free index error. uid=%u", uid);
		return R_ERR_DATA;
	}

	DataTruck truck;
	truck.uid = uid;

	if(Add(index, truck))
	{
		m_map[uid] = index;
	}
	else
	{
		error_log("Add to dbc failed. uid=%u", uid);
		return R_ERR_DATA;
	}

	return 0;
}

int DataTruckManager::LoadBuff(unsigned uid)
{
	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}

	if (m_map.count(uid) > 0)
	{
		return 0;
	}

	DataTruck truck;
	truck.uid = uid;
	int ret = Load(truck);

	if (ret)
	{
		return ret;
	}

	unsigned index = GetFreeIndex();

	if(index == -1)
	{
		error_log("get free index error. uid=%u", uid);
		return R_ERR_DATA;
	}

	m_data->data[index] = truck;  //给m_data内的数据赋值

	if(m_data->MardLoad(index))
	{
		m_freeIndex.erase(index);
	}
	else
	{
		error_log("truck mark load failed. uid=%u.", uid);
		return R_ERROR;
	}

	m_map[uid] = index;

	return 0;
}

DataTruck & DataTruckManager::Get(unsigned uid)
{
	if (m_map.count(uid) <= 0)
	{
		error_log("uid: %u", uid);
		throw std::runtime_error("get_truck_info_from_buff_error");
	}

	uint32_t index = m_map[uid];
	return  m_data->data[index];
}

void DataTruckManager::DoClear(unsigned uid)
{
	if(m_map.count(uid))
	{
		Clear(m_map[uid]);
		m_map.erase(uid);
	}
}

void DataTruckManager::DoSave(unsigned uid)
{
	if(m_map.count(uid))
	{
		AddSave(m_map[uid]);
	}
}
