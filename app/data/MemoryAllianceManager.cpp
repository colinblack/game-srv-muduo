#include "MemoryAllianceManager.h"
#include "DataAllianceAllManager.h"

int MemoryAllianceManager::OnInit()
{
	for(unsigned i=0; i < MEMORY_ALLIANCE_FULL; ++i)
	{
		if(m_data->item[i].allianceid != 0)
			m_map[m_data->item[i].allianceid] = i;
		else
			m_freeIndex.insert(i);
	}

	return 0;
}

int MemoryAllianceManager::UpdateMemberCount(unsigned aid, int change)
{
	ismodify = true;

	//判断是否有该公会的数据
	if (!m_map.count(aid))
	{
		return 0;
	}

	//已存在，则更新
	unsigned index = m_map[aid];

	if(change >= 0 || m_data->item[index].memcount >= (-change))
	{
		//增加
		m_data->item[index].memcount += change;
	}
	else
	{
		m_data->item[index].memcount = 0;
	}
	debug_log("alliance_member_count_now aid=%u change=%d count=%u", aid, change, m_data->item[index].memcount);

	return 0;
}
int MemoryAllianceManager::UpdateOnlineNum(unsigned aid, unsigned onlineNum)
{
	ismodify = true;
	//判断是否有该公会的数据
	if (!m_map.count(aid))
	{
		return 0;
	}
	m_onlineNum[aid] = onlineNum;

	return 0;
}
unsigned MemoryAllianceManager::GetOnlineNum(unsigned aid)
{
	map<unsigned, unsigned>::iterator iter = m_onlineNum.find(aid);
	return (iter != m_onlineNum.end()) ? iter->second : 0;
}
int MemoryAllianceManager::UpdateMember(uint32_t aid, int8_t apply_type, int8_t apply_level_limit)
{
	//判断是否有该公会的数据
	if (!m_map.count(aid))
	{
		return 0;
	}

	//已存在，则更新
	unsigned index = m_map[aid];
	m_data->item[index].apply_type = apply_type;
	m_data->item[index].apply_level_limit = apply_level_limit;
	return 0;
}

int MemoryAllianceManager::GetRecommendAlliances(uint32_t aid, uint32_t level, uint32_t member_num_limit, uint32_t apply_type_limit, vector<unsigned> & alliances)
{
	vector<uint32_t> o10;	// 在线人数大于20
	vector<uint32_t> a20;	// 公会人数大于20
	vector<uint32_t> a10;	// 公会人数大于10
	vector<uint32_t> aOth; // 其他

	uint32_t count = 0;
	//获取商会配置
	for(map<unsigned, unsigned>::iterator uiter = m_map.begin(); uiter != m_map.end(); ++uiter)
	{
		unsigned index = uiter->second;
		OfflineAllianceItem& adata = m_data->item[index];
		if(adata.apply_type == apply_type_limit
				&& level >= adata.apply_level_limit
				&& adata.memcount < member_num_limit
				&& adata.allianceid != aid)	// 只遍历1000个候选联盟
		{
			if(GetOnlineNum(adata.allianceid) >= 10)
			{
				if(o10.size() < recommend_alliance_max_num)
				{
					o10.push_back(adata.allianceid);
				}
			}
			else if(adata.memcount >= 20)
			{
				if(a20.size() < recommend_alliance_max_num)
				{
					a20.push_back(adata.allianceid);
				}
			}
			else if(adata.memcount >= 10)
			{
				if(a10.size() < recommend_alliance_max_num)
				{
					a10.push_back(adata.allianceid);
				}
			}
			else
			{
				if(aOth.size() < recommend_alliance_max_num)
				{
					aOth.push_back(adata.allianceid);
				}
			}

			if(o10.size() >= recommend_alliance_max_num)
			{
				break;
			}
			if(++count > 2000)
			{
				break;
			}
		}
	}
	alliances.assign(o10.begin(), o10.end());
	if(alliances.size() > recommend_alliance_max_num)
	{
		alliances.resize(recommend_alliance_max_num);
		return 0;
	}
	alliances.insert(alliances.end(), a20.begin(), a20.end());
	if(alliances.size() > recommend_alliance_max_num)
	{
		alliances.resize(recommend_alliance_max_num);
		return 0;
	}
	alliances.insert(alliances.end(), a10.begin(), a10.end());
	if(alliances.size() > recommend_alliance_max_num)
	{
		alliances.resize(recommend_alliance_max_num);
		return 0;
	}
	alliances.insert(alliances.end(), aOth.begin(), aOth.end());
	if(alliances.size() > recommend_alliance_max_num)
	{
		alliances.resize(recommend_alliance_max_num);
		return 0;
	}

	/*
	if (!ismodify)
	{
		//直接拷贝已有的
		alliances.assign(recommends.begin(), recommends.end());

		return 0;
	}

	//已有更新，则从当前的全部商会中选择推荐
	multimap<unsigned, unsigned> alliance_counts;  //商会成员数目. count->allianceid

	for(map<unsigned, unsigned>::iterator uiter = m_map.begin(); uiter != m_map.end(); ++uiter)
	{
		unsigned index = uiter->second;

		alliance_counts.insert(make_pair(m_data->item[index].memcount, uiter->first));
	}

	//遍历multimap，选出前20
	int count = 0;
	recommends.clear();

	for(multimap<unsigned, unsigned>::iterator it= alliance_counts.begin();it!= alliance_counts.end();++it)
	{
		recommends.insert(it->second);

		++count;

		if (recommend_alliance_max_num == count)
		{
			break;
		}
	}

	alliances.assign(recommends.begin(), recommends.end());
	ismodify = false;
	*/
	return 0;
}

int MemoryAllianceRaceGroupManager::OnInit()
{
	for(unsigned i=0; i < m_data->size; ++i)
	{
		const DataAllianceRaceGroupItem& item = m_data->item[i];
		for(unsigned j = 0; j < MEMORY_ALLIANCE_RACE_GROUP_SIZE; ++j)
		{
			if(IsAllianceId(item.member[j].allianceId))
			{
				m_map[item.member[j].allianceId] = i;
			}
		}
	}

	return 0;
}

int MemoryAllianceRaceGroupManager::Add(Admin::SetAllianceRaceGroup* sGroup)
{
	if(sGroup->ts() != m_data->ts)	// 重置
	{
		Reset();
		m_data->ts = sGroup->ts();
	}
	for(unsigned i = 0; i < sGroup->group_size(); ++i)
	{
		const Admin::AllianceRaceGroup& group = sGroup->group(i);
		if(m_data->size >= MEMORY_ALLIANCE_RACE_GROUP_FULL)
		{
			error_log("memory_alliance_race_group_is_full");
			return -1;
		}
		DataAllianceRaceGroupItem& item = m_data->item[m_data->size];
		unsigned mSize = 0;
		for(unsigned j = 0; j < group.item_size(); ++j)
		{
			if(mSize >= MEMORY_ALLIANCE_RACE_GROUP_SIZE)
			{
				break;
			}
			const Admin::AllianceRaceGroupItem& gItem = group.item(j);
			DataAllianceRaceGroupMember& member = item.member[mSize];
			member.allianceId = gItem.aid();
			member.count = gItem.count();
			member.point = 0;
			++mSize;

			m_map[member.allianceId] = m_data->size;
		}

		m_data->size++;
	}
	info_log("alliance_race_group ts=%u size=%u", sGroup->ts(), m_data->size);
}

void MemoryAllianceRaceGroupManager::Reset()
{
	memset(m_data, 0, sizeof(DataAllianceRaceGroup));
	m_map.clear();
}
void MemoryAllianceRaceGroupManager::Rank(map<uint32_t, pair<uint32_t, uint32_t> >& rank)
{
	for(uint32_t i = 0; i < m_data->size; ++i)
	{
		DataAllianceRaceGroupItem& item = m_data->item[i];
		sort(item.member, item.member + MEMORY_ALLIANCE_RACE_GROUP_SIZE);
	}
	for(uint32_t i = 0; i < m_data->size; ++i)
	{
		DataAllianceRaceGroupItem& item = m_data->item[i];
		for(uint32_t j = 0; j < MEMORY_ALLIANCE_RACE_GROUP_SIZE; ++j)
		{
			DataAllianceRaceGroupMember& member = item.member[j];
			uint32_t aid = member.allianceId;
			if(IsAllianceId(aid) && !CMI->IsNeedConnectByAID(aid))
			{
				rank[aid] = make_pair(j+1, member.count);
			}
		}
	}
}
uint32_t MemoryAllianceRaceGroupManager::GetTs()
{
	return m_data->ts;
}
void MemoryAllianceRaceGroupManager::UpdateMemberPoint(unsigned aid, unsigned point, set<unsigned>& zoneId)
{
	map<unsigned, unsigned>::iterator iter = m_map.find(aid);
	if(iter != m_map.end() && iter->second < m_data->size)
	{
		DataAllianceRaceGroupItem& item = m_data->item[iter->second];
		for(unsigned i = 0; i < MEMORY_ALLIANCE_RACE_GROUP_SIZE; ++i)
		{
			DataAllianceRaceGroupMember& member = item.member[i];
			if(IsAllianceId(member.allianceId))
			{
				if(aid == member.allianceId)
				{
					member.point = point;
				}
				else if(CMI->IsNeedConnectByAID(member.allianceId))	// 需要跨服同服积分信息
				{
					zoneId.insert(Config::GetZoneByAID(member.allianceId));
				}
			}
			else
			{
				break;
			}
		}

	}
}
void MemoryAllianceRaceGroupManager::UpdateMemberPointLocal(unsigned aid, unsigned point)
{
	map<unsigned, unsigned>::iterator iter = m_map.find(aid);
	if(iter != m_map.end() && iter->second < m_data->size)
	{
		DataAllianceRaceGroupItem& item = m_data->item[iter->second];
		for(unsigned i = 0; i < MEMORY_ALLIANCE_RACE_GROUP_SIZE; ++i)
		{
			DataAllianceRaceGroupMember& member = item.member[i];
			if(IsAllianceId(member.allianceId))
			{
				if(aid == member.allianceId)
				{
					member.point = point;
				}
			}
			else
			{
				break;
			}
		}
	}
}
/*
const DataAllianceRaceGroupMember* MemoryAllianceRaceGroupManager::GetMember(unsigned aid)
{
	map<unsigned, unsigned>::iterator iter = m_map.find(aid);
	if(iter != m_map.end() && iter->second < m_data->size)
	{
		DataAllianceRaceGroupItem& item = m_data->item[iter->second];
		for(unsigned i = 0; i < MEMORY_ALLIANCE_RACE_GROUP_SIZE; ++i)
		{
			DataAllianceRaceGroupMember& member = item.member[i];
			if(IsAllianceId(member.allianceId) && aid == member.allianceId)
			{
				return &member;
			}
		}
	}
	error_log("not_exist_alliance_race_group_member_map aid=%u", aid);
	return NULL;
}
*/
int MemoryAllianceRaceGroupManager::FillMember(unsigned aid, ProtoAlliance::ReplyAllianceRaceGroupMember* resp)
{
	map<unsigned, unsigned>::iterator iter = m_map.find(aid);
	if(iter != m_map.end() && iter->second < m_data->size)
	{
		DataAllianceRaceGroupItem& item = m_data->item[iter->second];
		for(unsigned i = 0; i < MEMORY_ALLIANCE_RACE_GROUP_SIZE; ++i)
		{
			DataAllianceRaceGroupMember& member = item.member[i];
			if(IsAllianceId(member.allianceId))
			{
				ProtoAlliance::AllianceRaceGroupMember* pMember = resp->add_member();
				pMember->set_aid(member.allianceId);
				pMember->set_point(member.point);
				pMember->set_count(member.count);
			}
		}
		return 0;
	}
	error_log("not_exist_alliance_race_group_member_map aid=%u", aid);
	return -1;
}
