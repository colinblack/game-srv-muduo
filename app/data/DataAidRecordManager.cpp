#include "DataAidRecordManager.h"

int DataAidRecordManager::OnInit()
{
	for(unsigned i=0; i < MAX_BUFF; ++i)
	{
		if(!m_data->Empty(i))
		{
			uint32_t uid = m_data->data[i].uid;
			uint32_t ts = m_data->data[i].ts;
			uint32_t aid_id = m_data->data[i].aid_id;

			m_map[uid][ts][aid_id] = i;
		}
	}

	return 0;
}

int DataAidRecordManager::CheckBuff(unsigned uid)
{
	if(!m_map.count(uid))
		return R_ERR_NO_DATA;

	return 0;
}

int DataAidRecordManager::AddBuff(DataAidRecord & datarecord)
{
	if(CMI->IsNeedConnectByUID(datarecord.uid))
	{
		error_log("uid:%u, data_need_connect", datarecord.uid);
		throw std::runtime_error("data_need_connect");
	}

	unsigned index = GetFreeIndex();

	uint32_t uid = datarecord.uid;
	uint32_t ts = datarecord.ts ;
	uint32_t aid_id = datarecord.aid_id ;

	if(index == (unsigned)-1)
	{
		error_log("get free index error. uid=%u", uid);
		return R_ERR_DATA;
	}

	if(Add(index, datarecord))
	{
		m_map[uid][ts][aid_id] = index;
	}
	else
	{
		error_log("Add to dbc failed. uid=%u", uid);
		return R_ERR_DATA;
	}

	return 0;
}

int DataAidRecordManager::LoadBuff(unsigned uid)
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

	vector<DataAidRecord> vctrecord(1);
	vctrecord[0].uid = uid;

	int ret = Load(vctrecord);

	if (ret)
	{
		return ret;
	}

	if (0 == vctrecord.size())
	{
		return R_ERR_NO_DATA;
	}

	//单个逐个加载
	for(size_t i = 0; i < vctrecord.size(); ++i)
	{
		unsigned index = GetFreeIndex();

		if(index == (unsigned)-1)
		{
			error_log("get free index error. uid=%u", uid);
			return R_ERR_DATA;
		}

		m_data->data[index] = vctrecord[i];  //给m_data内的数据赋值
		unsigned ts = vctrecord[i].ts;
		unsigned aid_id = vctrecord[i].aid_id;

		if(m_data->MardLoad(index))
		{
			m_freeIndex.erase(index);
		}
		else
		{
			error_log("mark load failed. uid=%u,ts=%d.", uid, ts);
			return R_ERROR;
		}

		m_map[uid][ts][aid_id] = index;
	}

	return 0;
}

void DataAidRecordManager::DoClear(unsigned uid)
{
	if(m_map.count(uid))
	{
		 map<uint32_t, map<uint32_t, uint32_t> >::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); ++miter)
		 {
			 map<uint32_t, uint32_t>::iterator uiter = miter->second.begin();

			 for(; uiter != miter->second.end(); ++uiter)
			 {
				 Clear(uiter->second);
			 }
		 }

		 m_map[uid].clear();
		 m_map.erase(uid);
	}
}

void DataAidRecordManager::DoSave(unsigned uid)
{
	if(m_map.count(uid))
	{
		 map<uint32_t, map<uint32_t, uint32_t> >::iterator miter = m_map[uid].begin();

		 for(; miter != m_map[uid].end(); ++miter)
		 {
			 map<uint32_t, uint32_t>::iterator uiter = miter->second.begin();

			 for(; uiter != miter->second.end(); ++uiter)
			 {
				 AddSave(uiter->second);
			 }
		 }
	}
}

int DataAidRecordManager::AddAidRecord(unsigned uid, unsigned aid, unsigned ts)
{
	//判断记录中是否有该天的数据
	if (m_map.count(uid) && m_map[uid].count(ts))
	{
		//有该天的记录，则判断是否有该援助对象的记录
		if (m_map[uid][ts].count(aid))
		{
			//有，这个好处理，直接添加次数
			unsigned index = m_map[uid][ts][aid];

			m_data->data[index].aid_times += 1;

			m_data->MarkChange(index);

			return 0;
		}
		else if (m_map[uid][ts].size() < MAX_AID_USERS)
		{
			//还有用户的空闲空间
			return AddNewRecord(uid, aid, ts);
		}

		//当天保存的用户数已满
		return R_ERROR;
	}
	else if (m_map.count(uid) && m_map[uid].size() < MAX_AID_DAYS)
	{
		//还有天数的空余空间
		return AddNewRecord(uid, aid, ts);
	}
	else if (m_map.count(uid))
	{
		//日期已满，则删除日期最旧的数据
		unsigned earlyts = m_map[uid].begin()->first;

		ClearDateRecord(uid, earlyts);

		//添加新数据
		return AddNewRecord(uid, aid, ts);
	}

	//没有该用户的数据，直接添加新数据
	return AddNewRecord(uid, aid, ts);
}

void DataAidRecordManager::ClearDateRecord(unsigned uid, unsigned ts)
{
	if (!m_map.count(uid) || !m_map[uid].count(ts))
	{
		return ;
	}

	//存在该天的数据
	 map<uint32_t, uint32_t>::iterator uiter = m_map[uid][ts].begin();

	 for(; uiter != m_map[uid][ts].end(); ++uiter)
	 {
		 Clear(uiter->second);
	 }

	 //清除该天的数据
	 m_map[uid][ts].clear();
	 m_map[uid].erase(ts);
}

int DataAidRecordManager::AddNewRecord(unsigned uid, unsigned aid, unsigned ts)
{
	DataAidRecord record;
	record.aid_id = aid;
	record.uid = uid;
	record.ts = ts;

	return AddBuff(record);
}

int DataAidRecordManager::GetRecentAidRecord(unsigned uid, vector<unsigned> & indexs)
{
	if (!m_map.count(uid))
	{
		return 0;
	}

	unsigned nowts = Time::GetGlobalTime();

	 map<uint32_t, map<uint32_t, uint32_t> >::iterator miter = m_map[uid].begin();

	 for(; miter != m_map[uid].end(); ++miter)
	 {
		 //根据时间筛选
		 if (miter->first > nowts)
		 {
			 //排除时间比当前时间大的
			 break;
		 }

		 //只要7天之内的
		 if (CTime::GetDayInterval(miter->first, nowts) > 7)
		 {
			 continue;
		 }

		 map<uint32_t, uint32_t>::iterator uiter = miter->second.begin();

		 for(; uiter != miter->second.end(); ++uiter)
		 {
			 indexs.push_back(uiter->second);
		 }
	 }

	return 0;
}

DataAidRecord & DataAidRecordManager::GetDataByIndex(unsigned index)
{
	if (index >= MAX_BUFF)
	{
		throw runtime_error("index_error");
	}

	return m_data->data[index];
}
