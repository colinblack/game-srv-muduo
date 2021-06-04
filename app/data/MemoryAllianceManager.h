#ifndef MEMORY_ALLIANCE_MANAGER_H_
#define MEMORY_ALLIANCE_MANAGER_H_

#include "Kernel.h"

/**********************MemoryAllianceManager*****************************/
struct OfflineAllianceItem
{
	unsigned allianceid;
	unsigned memcount;
	int8_t apply_type;	// 申请类型
	int8_t apply_level_limit;	// 限制等级

	OfflineAllianceItem(): allianceid(0), memcount(0), apply_type(0), apply_level_limit(0)
	{
	}
};

struct OfflineAlliance
{
	OfflineAllianceItem item[MEMORY_ALLIANCE_FULL];
};

class MemoryAllianceManager : public MemorySingleton<OfflineAlliance, MEMORY_ALLIANCE>, public CSingleton<MemoryAllianceManager>
{
private:
	friend class CSingleton<MemoryAllianceManager>;

	MemoryAllianceManager()
	{
		ismodify = true;
	};

	virtual ~MemoryAllianceManager(){}

	set<unsigned> m_freeIndex;
	map<unsigned, unsigned> m_map;
	map<unsigned, unsigned> m_onlineNum;

	set<unsigned> recommends; //推荐的商会数据
	bool ismodify;  //是否修改
public:
	enum
	{
		recommend_alliance_max_num = 20,  //推荐的商会数量上限
	};

	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	const map<unsigned, unsigned>& GetMap() {return m_map;}

	unsigned GetFreeCount()
	{
		return m_freeIndex.size();
	}

	unsigned GetFreeIndex()
	{
		if(m_freeIndex.empty())
			return -1;
		return *(m_freeIndex.begin());
	}

	unsigned GetIndex(unsigned allianceid)
	{
		if(m_map.count(allianceid))
			return m_map[allianceid];
		return -1;
	}

	int GetRecommendAlliances(uint32_t aid, uint32_t level, uint32_t member_num_limit, uint32_t apply_type_limit, vector<unsigned> & indexs);

	int UpdateMemberCount(unsigned allianceid, int change);
	int UpdateOnlineNum(unsigned allianceid, unsigned onlineNum);
	unsigned GetOnlineNum(unsigned allianceid);
	int UpdateMember(uint32_t aid, int8_t apply_type, int8_t apply_level_limit);

	int DelAlliance(unsigned allianceid)
	{
		if(m_map.count(allianceid))
		{
			unsigned i = m_map[allianceid];
			memset(&(m_data->item[i]), 0, sizeof(OfflineAllianceItem));
			m_freeIndex.insert(i);
			m_map.erase(allianceid);
			recommends.erase(allianceid);
			m_onlineNum.erase(allianceid);
			ismodify = true;
			debug_log("alliance_member_count_now aid=%u count=%u", allianceid, 0);
		}

		return 0;
	}

	bool IsExist(unsigned alliance_id)
	{
		if(m_map.count(alliance_id))
			return true;

		return false;
	}

	OfflineAllianceItem & GetAllianceItemByIndex(unsigned index)
	{
		if (index > MEMORY_ALLIANCE_FULL)
		{
			throw runtime_error("index_error");
		}

		return  m_data->item[index];
	}

	int Add(unsigned allianceid, unsigned count, int8_t apply_type, int8_t apply_level_limit, unsigned onlineNum)
	{
		unsigned i = GetFreeIndex();
		if(i == (unsigned)-1)
			return R_ERR_DATA;
		m_freeIndex.erase(i);
		m_map[allianceid] = i;

		m_data->item[i].allianceid = allianceid;
		m_data->item[i].memcount = count;
		m_data->item[i].apply_type = apply_type;
		m_data->item[i].apply_level_limit = apply_level_limit;
		m_onlineNum[allianceid] = onlineNum;

		return 0;
	}
};

/**********************MemoryAllianceRaceGroupManager*****************************/
struct DataAllianceRaceGroupMember
{
	unsigned allianceId;
	unsigned point;
	unsigned count;	// 成员数量

	bool CmpCount(const DataAllianceRaceGroupMember& other)const
	{
		return (count == other.count) ? (allianceId < other.allianceId) : (count < other.count);
	}
	bool operator<(const DataAllianceRaceGroupMember& other)const
	{
		return (point == other.point) ? CmpCount(other) : (point > other.point);
	}
	const DataAllianceRaceGroupMember& operator=(const DataAllianceRaceGroupMember& other)
	{
		if(&other == this)
		{
			return *this;
		}
		memcpy(this, &other, sizeof(*this));
		return *this;
	}
};
struct DataAllianceRaceGroupItem
{
	DataAllianceRaceGroupMember member[MEMORY_ALLIANCE_RACE_GROUP_SIZE];
};

struct DataAllianceRaceGroup
{
	unsigned ts;	// 活动开始时间，用于判断是否清理旧数据
	unsigned size;
	DataAllianceRaceGroupItem item[MEMORY_ALLIANCE_RACE_GROUP_FULL];
};

class MemoryAllianceRaceGroupManager : public MemorySingleton<DataAllianceRaceGroup, MEMORY_ALLIANCE_RACE_GROUP>, public CSingleton<MemoryAllianceRaceGroupManager>
{
private:
	friend class CSingleton<MemoryAllianceRaceGroupManager>;

	MemoryAllianceRaceGroupManager()
	{
	};

	virtual ~MemoryAllianceRaceGroupManager(){}

	map<unsigned, unsigned> m_map;
public:
	virtual int OnInit();
	virtual void CallDestroy() {Destroy();}
	int Add(Admin::SetAllianceRaceGroup* sGroup);
	void UpdateMemberPoint(unsigned aid, unsigned point, set<unsigned>& zoneId);
	void UpdateMemberPointLocal(unsigned aid, unsigned point);
//	const DataAllianceRaceGroupMember* GetMember(unsigned aid);
	int FillMember(unsigned aid, ProtoAlliance::ReplyAllianceRaceGroupMember* resp);
	void Reset();
	void Rank(map<uint32_t, pair<uint32_t, uint32_t> >& rank);
	uint32_t GetTs();
};


#endif /* MEMORY_ALLIANCE_MANAGER_H_ */
