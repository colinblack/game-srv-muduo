#include "DataChargeHistoryManager.h"

int DataChargeHistoryManager::OnInit()
{
	for(unsigned i=0; i < MAX_BUFF; ++i)
	{
		if(!m_data->Empty(i))
		{
			uint32_t uid = m_data->data[i].uid;
			uint32_t ts = m_data->data[i].ts;

			m_map[uid][ts] = i;
		}
	}

	return 0;
}

int DataChargeHistoryManager::LoginCheck(unsigned uid)
{
	int ret = LoadBuff(uid);

	if (ret)
	{
		return ret;
	}

	return 0;
}

int DataChargeHistoryManager::CheckBuff(unsigned uid)
{
	if(!m_map.count(uid))
		return R_ERR_NO_DATA;

	return 0;
}

int DataChargeHistoryManager::AddBuff(DataChargeHistory & datacharge)
{
	if(CMI->IsNeedConnectByUID(datacharge.uid))
	{
		error_log("uid:%u, data_need_connect", datacharge.uid);
		throw std::runtime_error("data_need_connect");
	}

	unsigned index = GetFreeIndex();

	uint32_t uid = datacharge.uid;
	uint32_t ts = datacharge.ts ;

	if(index == (unsigned)-1)
	{
		error_log("[AddBuff] get free index error. uid=%u", uid);
		return R_ERR_DATA;
	}

	if(Add(index, datacharge))
	{
		m_map[uid][ts] = index;
	}
	else
	{
		error_log("[AddBuff] Add to dbc failed. uid=%u", uid);
		return R_ERR_DATA;
	}

	return 0;
}

int DataChargeHistoryManager::LoadBuff(unsigned uid)
{
	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}

	//为防止重复加载
	if (m_map.count(uid) > 0)
	{
		return 0;
	}

	vector<DataChargeHistory> vctcharges(1);
	vctcharges[0].uid = uid;

	int ret = Load(vctcharges);

	if (ret)
	{
		return ret;
	}

	if (0 == vctcharges.size())
	{
		return R_ERR_NO_DATA;
	}

	//单个逐个加载
	for(size_t i = 0; i < vctcharges.size(); ++i)
	{
		unsigned index = GetFreeIndex();

		if(index == (unsigned)-1)
		{
			error_log("[LoadBuff] get free index error. uid=%u", uid);
			return R_ERR_DATA;
		}

		m_data->data[index] = vctcharges[i];  //给m_data内的数据赋值
		unsigned ts = vctcharges[i].ts;

		if(m_data->MardLoad(index))
		{
			m_freeIndex.erase(index);
		}
		else
		{
			error_log("[LoadBuff] mark load failed. uid=%u,ts=%d.", uid, ts);
			return R_ERROR;
		}

		m_map[uid][ts] = index;
	}

	return 0;
}


void DataChargeHistoryManager::DoClear(unsigned uid)
{
	if(m_map.count(uid))
	{
		 map<uint32_t, uint32_t>::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); )
		 {
			 Clear(miter->second);
			 m_map[uid].erase(miter++);
		 }

		 m_map.erase(uid);
	}
}

void DataChargeHistoryManager::DoSave(unsigned uid)
{
	if(m_map.count(uid))
	{
		 map<uint32_t, uint32_t>::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); ++miter)
		 {
			 AddSave(miter->second);
		 }
	}
}

void DataChargeHistoryManager::FullMessage(unsigned uid, User::AccumulateCharge * msg)
{
	if(m_map.count(uid))
	{
		 map<uint32_t, uint32_t>::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); ++miter)
		 {
			 unsigned index = miter->second;

			 m_data->data[index].SetMessage(msg->add_accumulatecharge());
		 }
	}
}


void DataChargeHistoryManager::FullMessage(unsigned uid, google::protobuf::RepeatedPtrField<ProtoUser::ChargeItem >* msg)
{
	if(m_map.count(uid))
	{
		 map<uint32_t, uint32_t>::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); ++miter)
		 {
			 unsigned index = miter->second;

			 m_data->data[index].SetMessage(msg->Add());
		 }
	}
}

void DataChargeHistoryManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;

	GetChargeHistoryList(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataChargeHistory & chargeHistory = GetChargeHistory(indexs[i]);

		DeleteChargeHistory(chargeHistory);
	}

	//添加新数据
	for(int i = 0; i < data.charges_size(); ++i)
	{
		unsigned ts = data.charges(i).ts();

		DataChargeHistory chargeHistory;
		chargeHistory.uid = uid;

		chargeHistory.FromMessage(&data.charges(i));

		AddBuff(chargeHistory);
	}
}

int DataChargeHistoryManager::AddChargeHistory(unsigned uid, unsigned cash)
{
	//首先，取最近一天的充值记录
	unsigned index = 0;

	int ret = GetLatiestCharge(uid, index);

	if (ret && ret != R_ERR_NO_DATA)
	{
		//加载数据库中的数据错误，本次记录不处理
		return ret;
	}

	unsigned nowts = Time::GetGlobalTime();

	if (R_ERR_NO_DATA == ret)
	{
		//还没有充值记录，则新增
		return NewChargeHistory(uid, cash);
	}

	DataChargeHistory& charge = GetChargeHistory(index);

	if(CTime::GetDayInterval(charge.ts, nowts) == 0)
	{
		//同一天，则直接添加
		charge.cash += cash;

		UpdateChargeHistory(charge);
	}
	else if (charge.ts < nowts)
	{
		//不同天，如何处理？
		//暂时只处理时间大于最近的，即不考虑修改服务器时间后的充值
		//只保留有限时间内的数据
		if (m_map[uid].size() >= DB_CHARGE_HISTORY_FULL)
		{
			//删除最远时间的充值记录，添加当前的充值记录
			map<uint32_t, uint32_t>::iterator iter = m_map[uid].begin();

			unsigned oldindex = iter->second;

			try
			{
				DeleteChargeHistory(m_data->data[oldindex]);
			}
			catch(runtime_error &e)
			{
				error_log("delete history error. uid=%u,reason=%s", uid, e.what());
				return R_ERROR;
			}
		}

		return NewChargeHistory(uid, cash);
	}

	return 0;
}

int DataChargeHistoryManager::NewChargeHistory(unsigned uid, unsigned cash)
{
	DataChargeHistory addcharge;

	addcharge.uid = uid;
	addcharge.ts = Time::GetGlobalTime();
	addcharge.cash = cash;

	int ret = AddBuff(addcharge);

	if (ret)
	{
		return ret;
	}

	return 0;
}

int DataChargeHistoryManager::GetLatiestCharge(unsigned uid, unsigned & index)
{
	//为防止没有数据，所以先load加载一下
	int ret = LoadBuff(uid);

	if (ret)
	{
		return ret;
	}

	//运行到这里的，都是有数据的
	//使用反向迭代器，获取最近一天的充值数据
	map<uint32_t, uint32_t>::reverse_iterator riter = m_map[uid].rbegin();

	index = riter->second;

	return 0;
}

DataChargeHistory& DataChargeHistoryManager::GetChargeHistory(unsigned index)
{
	if (index >= MAX_BUFF)
	{
		error_log("index is error. index=%u", index);
		throw std::runtime_error("get_charge_history_error");
	}

	return  m_data->data[index];
}

int DataChargeHistoryManager::GetChargeHistoryList(unsigned uid, vector<unsigned> & indexs)
{
	int ret = LoadBuff(uid);

	if (ret)
	{
		return ret;
	}

	if(m_map.count(uid))
	{
		 map<uint32_t, uint32_t>::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); ++miter)
		 {
			 indexs.push_back(miter->second);
		 }
	}

	return 0;
}

bool DataChargeHistoryManager::UpdateChargeHistory(DataChargeHistory& datacharge)
{
	unsigned uid = datacharge.uid;
	unsigned ts = datacharge.ts;

	if (!m_map.count(uid) || !m_map[uid].count(ts))
	{
		return false;
	}

	unsigned index = m_map[uid][ts];

	return  m_data->MarkChange(index);
}

bool DataChargeHistoryManager::DeleteChargeHistory(DataChargeHistory & datacharge)
{
	unsigned uid = datacharge.uid;
	unsigned ts = datacharge.ts;

	if (!m_map.count(uid) || !m_map[uid].count(ts))
	{
		error_log("get friend error. uid=%u, id=%u", uid, ts);
		throw std::runtime_error("get_charge_history_error");
	}

	unsigned index =  m_map[uid][ts];

	bool isSuccess = DeleteDBC(index);

	if (!isSuccess)
	{
		return false;
	}

	m_map[uid].erase(ts);

	 return true;
}

bool DataChargeHistoryManager::DeleteDBC(unsigned index)
{
	//mark删除状态
	m_data->MarkDel(index);

	//添加至操作队列
	AddSave(index);

	return true;
}
