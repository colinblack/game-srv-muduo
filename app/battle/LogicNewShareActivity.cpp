#include "LogicNewShareActivity.h"

void LogicNewShareActivity::SetDataFlag(unsigned & data,unsigned index)
{
	data |= (1 << index);
}

unsigned LogicNewShareActivity::GetDataFlag(unsigned data,unsigned index)
{
	return (data >> index) & 1;
}

int LogicNewShareActivity::Process(unsigned uid, ProtoActivity::GetNewShareInfoReq* req, ProtoActivity::GetNewShareInfoResp* resp)
{
	//1.获取分享好友数据
	vector<unsigned>friendList;
	friendList.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,friendList);
	for(int i = 0; i < friendList.size(); i++)
	{
		DataFriendWorker & friendWorker = DataFriendWorkerManager::Instance()->GetDataByIndex(friendList[i]);
		ProtoActivity::ShareFriendCPP * friendcpp = resp->add_friend_();
		friendcpp->set_frienduid(friendWorker.id);
		friendcpp->set_invitets(friendWorker.invite_ts);
	}

	//2.获取领取标记信息(好友信息目前支持 4 * INT_BITS个,若需扩展,只需添加正确的存储信息即可)
	unsigned data_index = 0;
	unsigned flag_index = 0;
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid,e_Activity_New_Share);
	for(int index = 0; index < friendList.size(); index++)
	{
		if(index >= 0 && index < INT_BITS)
		{
			data_index = e_Activity_New_Share_index_1;
			flag_index = index % INT_BITS;
		}
		else if(index >= INT_BITS && index < 2 * INT_BITS)
		{
			data_index = e_Activity_New_Share_index_2;
			flag_index = (index - INT_BITS) % INT_BITS;
		}
		else if(index >= 2 * INT_BITS && index < 3 * INT_BITS)
		{
			data_index = e_Activity_New_Share_index_3;
			flag_index = (index - 2 * INT_BITS) % INT_BITS;
		}
		else if(index >= 3 * INT_BITS && index < 4 * INT_BITS)
		{
			data_index = e_Activity_New_Share_index_4;
			flag_index = (index - 3 * INT_BITS) % INT_BITS;
		}
		resp->add_rewardflag(GetDataFlag(activity.actdata[data_index],flag_index));
	}

	//3.设置返回信息
	const ConfigActivity::NewShareCPP & newshare_cfg = ConfigManager::Instance()->activitydata.m_config.new_share();
	unsigned usedShareCnt =  activity.actdata[e_Activity_New_Share_index_5];
	resp->mutable_newshare()->set_nextsharets(activity.actdata[e_Activity_New_Share_index_0]);
	resp->mutable_newshare()->set_remainsharecnt(newshare_cfg.share_reward_size() - usedShareCnt);
	return 0;
}

int LogicNewShareActivity::Process(unsigned uid, ProtoActivity::RewardNewShareReq* req, ProtoActivity::RewardNewShareResp* resp)
{
	unsigned index = req->index();
	//校验参数是否合法
	vector<unsigned>friendList;
	friendList.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,friendList);
	if(index >= friendList.size())
	{
		throw std::runtime_error("index_param_error");
	}

	unsigned data_index = 0;
	unsigned flag_index = 0;
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid,e_Activity_New_Share);
	if(index >= 0 && index < INT_BITS)
	{
		data_index = e_Activity_New_Share_index_1;
		flag_index = index % INT_BITS;
	}
	else if(index >= INT_BITS && index < 2 * INT_BITS)
	{
		data_index = e_Activity_New_Share_index_2;
		flag_index = (index - INT_BITS) % INT_BITS;
	}
	else if(index >= 2 * INT_BITS && index < 3 * INT_BITS)
	{
		data_index = e_Activity_New_Share_index_3;
		flag_index = (index - 2 * INT_BITS) % INT_BITS;
	}
	else if(index >= 3 * INT_BITS && index < 4 * INT_BITS)
	{
		data_index = e_Activity_New_Share_index_4;
		flag_index = (index - 3 * INT_BITS) % INT_BITS;
	}
	else {
		throw std::runtime_error("index_param_error");
	}

	//校验是否已领取
	unsigned flag = GetDataFlag(activity.actdata[data_index],flag_index);
	if(flag == 1)
	{
		throw std::runtime_error("already_reward");
	}

	//校验通过、做相应处理
	//1.发放领取奖励
	const ConfigActivity::NewShareCPP & newshare_cfg = ConfigManager::Instance()->activitydata.m_config.new_share();
	LogicUserManager::Instance()->CommonProcess(uid,newshare_cfg.reward(index),"new_share_activity_reward",resp->mutable_common());
	//2.设置更新标记
	SetDataFlag(activity.actdata[data_index],flag_index);
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	//3.设置标记返回信息
	for(int index = 0; index < friendList.size(); index++)
	{
		if(index >= 0 && index < INT_BITS)
		{
			data_index = e_Activity_New_Share_index_1;
			flag_index = index % INT_BITS;
		}
		else if(index >= INT_BITS && index < 2 * INT_BITS)
		{
			data_index = e_Activity_New_Share_index_2;
			flag_index = (index - INT_BITS) % INT_BITS;
		}
		else if(index >= 2 * INT_BITS && index < 3 * INT_BITS)
		{
			data_index = e_Activity_New_Share_index_3;
			flag_index = (index - 2 * INT_BITS) % INT_BITS;
		}
		else if(index >= 3 * INT_BITS && index < 4 * INT_BITS)
		{
			data_index = e_Activity_New_Share_index_4;
			flag_index = (index - 3 * INT_BITS) % INT_BITS;
		}
		resp->add_rewardflag(GetDataFlag(activity.actdata[data_index],flag_index));
	}
	return 0;
}

int LogicNewShareActivity::Process(unsigned uid, ProtoActivity::NewShareReq* req, ProtoActivity::NewShareResp* resp)
{
	const ConfigActivity::NewShareCPP & newshare_cfg = ConfigManager::Instance()->activitydata.m_config.new_share();
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid,e_Activity_New_Share);

	//校验分享次数是否已最大
	if(activity.actdata[e_Activity_New_Share_index_5] >= newshare_cfg.share_reward_size())
	{
		throw std::runtime_error("daily_share_is_maxed");
	}

	//校验分享cd是否已到
	if(activity.actdata[e_Activity_New_Share_index_0] > Time::GetGlobalTime())
	{
		throw std::runtime_error("share_cd_error");
	}

	//校验通过
	//1.发放分享奖励
	unsigned cur_share_cnt = activity.actdata[e_Activity_New_Share_index_5];
	LogicUserManager::Instance()->CommonProcess(uid,newshare_cfg.share_reward(cur_share_cnt),"new_share_reward",resp->mutable_common());
	//2.更新分享数据
	activity.actdata[e_Activity_New_Share_index_0] = Time::GetGlobalTime() + newshare_cfg.share_cd_time(cur_share_cnt);
	activity.actdata[e_Activity_New_Share_index_5] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	//3.设置剩余返回信息
	resp->mutable_newshare()->set_nextsharets(activity.actdata[e_Activity_New_Share_index_0]);
	resp->mutable_newshare()->set_remainsharecnt(newshare_cfg.share_reward_size() - activity.actdata[e_Activity_New_Share_index_5]);
	return 0;
}

int LogicNewShareActivity::ResetNewShareData(unsigned uid)
{
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid,e_Activity_New_Share);
	activity.actdata[e_Activity_New_Share_index_0] = 0;
	activity.actdata[e_Activity_New_Share_index_5] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	return 0;
}
