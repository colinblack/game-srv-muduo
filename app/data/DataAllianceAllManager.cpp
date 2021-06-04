#include "DataAllianceAllManager.h"
#include "ResourceManager.h"

unsigned DataAllianceMemberManager::GetMemberCount(unsigned alliance_id) const
{
	if (m_map.count(alliance_id))
	{
		return m_map.at(alliance_id).size();
	}

	return 0;
}

//获取在线商会成员数量
unsigned DataAllianceMemberManager::GetMemberOnlineNum(unsigned aid)const
{
	uint32_t onlineNum = 0;
	uint32_t now = Time::GetGlobalTime();
	vector<unsigned> indexs;
	DataAllianceMemberManager::Instance()->GetIndexs(aid, indexs);
	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataAllianceMember &member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
		if(member.onlineTs + 172800 > now)	// 两天以内有效
		{
			++onlineNum;
		}
	}
	return onlineNum;
}
unsigned DataAllianceMemberManager::GetPositionCount(unsigned alliance_id, unsigned position) const
{
	int count = 0;

	if (m_map.count(alliance_id))
	{
		//遍历，进行统计
		map<unsigned, unsigned>::const_iterator itor = m_map.at(alliance_id).begin();

		for (; itor != m_map.at(alliance_id).end(); ++itor)
		{
			unsigned index = itor->second;

			const DataAllianceMember & member = GetDataByIndex(index);

			if (member.position == position)
			{
				++count;
			}
		}
	}

	return count;
}

int DataAllianceApplyManager::FullMessage(unsigned alliance_id, unsigned maxcount, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceApplyCPP >* msg)
{
	vector<unsigned> indexs;

	GetIndexs(alliance_id, indexs);

	for(size_t i = 0; i < indexs.size() && i < maxcount; ++i)
	{
		//排除已完成的，非本人的捐收信息
		DataAllianceApply & apply = GetDataByIndex(indexs[i]);

		SetMessage(GetDataByIndex(indexs[i]), msg->Add());
	}

	return 0;
}

int DataAllianceDonationManager::FullMessage(unsigned alliance_id, unsigned uid, unsigned maxcount, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceDonationCPP >* msg)
{
	vector<unsigned> indexs;

	GetIndexsFromMem(alliance_id, indexs);
	int count = 0;

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		//排除已完成的，非本人的捐收信息
		DataAllianceDonation & donation = GetDataByIndex(indexs[i]);

		if (donation.status == donation_finish && donation.id != uid)
		{
			continue;
		}

		SetMessage(GetDataByIndex(indexs[i]), msg->Add());

		++count;

		if (count >= maxcount)
			break;
	}

	return 0;
}

int DataAllianceNotifyManager::FullMessage(unsigned alliance_id, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceNotifyCPP >* msg)
{
	vector<unsigned> indexs;

	GetIndexs(alliance_id, indexs);

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		//排除空的通知
		DataAllianceNotify & notify = GetDataByIndex(indexs[i]);

		if (notify.announce_uid == 0)
		{
			continue;
		}

		SetMessage(notify, msg->Add());
	}

	return 0;
}

int DataAllianceNotifyManager::ResetNotifyWhenChange(unsigned uid)
{
	vector<unsigned> indexs;

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataAllianceNotify & notify = GetDataByIndex(indexs[i]);

		if (notify.announce_uid == uid)
		{
			notify.Reset();
		}
	}

	return 0;
}

unsigned DataAllianceNotifyManager::GetFreeNotifyId(unsigned alliance_id, unsigned uid, unsigned position)
{
	//遍历当前已有的通知，判断是否已存在该用户的数据
	vector<unsigned> indexs;

	GetIndexs(alliance_id, indexs);

	int count = 0;
	map<unsigned, unsigned> timeids;  //ts->id
	unsigned maxid = 0;

	int limitnum = 0;

	if (position == pos_type_director)
	{
		limitnum = 1;
	}
	else if (position == pos_type_chief)
	{
		limitnum = 3;
	}
	else
	{
		error_log("privilege not enough. uid=%u", uid);
		throw runtime_error("privilege_not_enough");
	}
	int emptyId = -1;
	for(int i = 0; i < indexs.size(); ++i)
	{
		const DataAllianceNotify & notify = GetDataByIndex(indexs[i]);

		if (maxid < notify.id)
		{
			maxid = notify.id;
		}
		if(emptyId < 0 && notify.announce_uid == 0)	// 可用槽位
		{
			emptyId = notify.id;
		}

		if (notify.announce_uid == uid && position == pos_type_director)
		{
			//理事只能有一个通知，所以就返回这个通知的id了
			timeids[notify.create_ts] = notify.id;
			break;
		}
		else if (notify.announce_uid == uid)
		{
			//会长可以有三条通知
			timeids[notify.create_ts] = notify.id;
		}
	}
	if(timeids.size() < limitnum)
	{
		if(emptyId >= 0)
		{
			return emptyId;
		}
		else if(indexs.size() < DB_ALLIANCE_NOTIFY_FULL)
		{
			return maxid + 1;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		//返回时间最早的那个通知id
		//return timeids[0u];
		//提示通知条数最大
		throw runtime_error("personal_notify_nums_max");
	}
}


