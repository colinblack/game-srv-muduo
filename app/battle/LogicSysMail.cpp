/*
 * LogicSysMail.cpp
 *
 *  Created on: 2018-2-8
 *      Author: Ralf
 */


#include "LogicSysMail.h"

void LogicSysMailManager::OnLogin(unsigned uid, unsigned ts)
{
	vector<unsigned> vIds;
	DataSysmailManager::Instance()->GetIds(uid, vIds);
	set<unsigned> all;
	for(vector<unsigned>::iterator it=vIds.begin();it!=vIds.end();++it)
	{
		if(!MemorySysMailManager::Instance()->IsValid(DataSysmailManager::Instance()->GetData(uid, *it).id))	// 删除无效邮件
			DataSysmailManager::Instance()->DelItem(uid, *it);
		else
			all.insert(*it);
	}

	vector<MemorySysmailItem*> res;
	MemorySysMailManager::Instance()->Get(uid, ts, res);
	for(vector<MemorySysmailItem*>::iterator it=res.begin();it!=res.end();++it)
	{
		if(all.count((*it)->ts) == 0)
		{
			try
			{
				(*it)->Add(DataSysmailManager::Instance()->GetData(uid, (*it)->ts));
			}
			catch(const std::exception& e)
			{
			}
		}
	}
}
void LogicSysMailManager::SetMessage(unsigned uid, User::Base* msg)
{
	vector<unsigned> vIds;
	DataSysmailManager::Instance()->GetIds(uid, vIds);
	for(vector<unsigned>::iterator it=vIds.begin();it!=vIds.end();++it)
	{
		if(DataSysmailManager::Instance()->GetData(uid, *it).stat == e_mail_stat_unread)
		{
//			msg->set_newmail(true);
			break;
		}
	}
//	msg->set_newmail(false);
}
int LogicSysMailManager::Process(Admin::SysMail *req, Admin::ReplySysMail *resp)
{
	string sys(req->sys()), reward(req->reward());
	if(sys.length() >= Data_sysmail_sys_COUNT || reward.length() >= Data_sysmail_reward_COUNT)
	{
		error_log("too long");
		resp->set_ret(R_ERR_PARAM);
		return 0;
	}

	if(!reward.empty())
	{
		Json::Value r;
		Json::Reader().parse(reward, r);
		CommonGiftConfig::CommonModifyItem re;
		try
		{
			if(!ConfigPBBase::_j2p(&re, r))
			{
				resp->set_ret(R_ERR_PARAM);
				return 0;
			}
		}
		catch(const std::exception& e)
		{
			error_log("%s", e.what());
			resp->set_ret(R_ERR_PARAM);
			return 0;
		}
	}

	int ret = 0;
	MemorySysmailItem obj;
	obj.ts = Time::GetGlobalTime();
	strncpy(obj.sys, sys.c_str(), Data_sysmail_sys_COUNT-1);
	strncpy(obj.reward, reward.c_str(), Data_sysmail_reward_COUNT-1);
	if(req->uid_size() == 0)
	{
		ret = MemorySysMailManager::Instance()->Add(obj);
		if(ret == 0)
		{
		   	vector<unsigned> uids;
			UMI->GetOnlineUsers(uids);
			for(vector<unsigned>::iterator it = uids.begin(); it != uids.end(); ++it)
			{
				if(!DataSysmailManager::Instance()->IsExistItem(*it, obj.ts))
				{
					try
					{
						obj.Add(DataSysmailManager::Instance()->GetData(*it, obj.ts));
					}
					catch(const std::exception& e)
					{
					}
				}
			}
			UMI->SendNewMsgToAll(NEW_MSG_TYPE_SYS_MAIL);
		}
	}
	else
	{
		for(int i=0;i<req->uid_size();++i)
		{
			obj.uid = req->uid(i);
			if(UMI->IsOnline(obj.uid))
			{
				if(!DataSysmailManager::Instance()->IsExistItem(obj.uid, obj.ts))
				{
					try
					{
						obj.Add(DataSysmailManager::Instance()->GetData(obj.uid, obj.ts));
					}
					catch(const std::exception& e)
					{
						ret += R_ERR_DATA_LIMIT;
					}
				}
				UMI->SendNewMsgToUser(obj.uid, NEW_MSG_TYPE_SYS_MAIL);
			}
			else
				ret += MemorySysMailManager::Instance()->Add(obj);
		}
	}
	resp->set_ret(ret);
	return 0;
}
int LogicSysMailManager::Process(unsigned uid, User::RequestSysMail *req, User::ReplySysMail *resp)
{
	vector<unsigned> indexs;
	DataSysmailManager::Instance()->GetIndexs(uid, indexs);

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataSysmail &m = DataSysmailManager::Instance()->GetDataByIndex(indexs[i]);
		if(m.stat == e_mail_stat_del)
		{
			continue;
		}
		if(m.stat == e_mail_stat_unread)
		{
			if(string(m.reward).empty())
				m.stat = e_mail_stat_over;
			else
				m.stat = e_mail_stat_unget;
			 DataSysmailManager::Instance()->UpdateItem(m);
		}
		m.SetMessage(resp->mutable_mail()->Add());
	}
	return 0;
}
int LogicSysMailManager::Process(unsigned uid, User::RequestMailRead *req, User::ReplyMailRead *resp)
{
	if(!DataSysmailManager::Instance()->IsExistItem(uid, req->ts()))
		return R_ERR_PARAM;
	DataSysmail& m = DataSysmailManager::Instance()->GetData(uid, req->ts());
	if(m.stat == e_mail_stat_del)
	{
		error_log("email already del uid=%u ts=%u", uid, req->ts());
		throw runtime_error("email_already_del");
	}
	if(m.stat == e_mail_stat_unread)
	{
		if(string(m.reward).empty())
			m.stat = e_mail_stat_over;
		else
			m.stat = e_mail_stat_unget;
		 DataSysmailManager::Instance()->UpdateItem(m);
	}
	resp->set_ts(m.id);
	resp->set_stat(m.stat);
	return 0;
}
void LogicSysMailManager::Get(unsigned uid, unsigned ts, ProtoReward::RewardInfo* msg)
{
	DataSysmail& m = DataSysmailManager::Instance()->GetData(uid, ts);
	if(m.stat == e_mail_stat_del)
		return;
	if(m.stat == e_mail_stat_over)
		return;
	if(!MemorySysMailManager::Instance()->IsValid(m.id))
	{
		return;
	}

	m.stat = e_mail_stat_over;
	DataSysmailManager::Instance()->UpdateItem(m);

	string reward(m.reward);
	if(reward.empty())
		return;

	try
	{
		Json::Value r;
		Json::Reader().parse(reward, r);
		CommonGiftConfig::CommonModifyItem re;
		if(!ConfigPBBase::_j2p(&re, r))
		{
			return;
		}
		LogicUserManager::Instance()->CommonProcess(uid, re, "get_sysmail", msg->mutable_common());
	}
	catch(const std::exception& e)
	{
		error_log("%s", e.what());
		return;
	}
}
int LogicSysMailManager::Process(unsigned uid, User::RequestMailGet *req, User::ReplyMailGet *resp)
{
	if(!DataSysmailManager::Instance()->IsExistItem(uid, req->ts()))
		return R_ERR_PARAM;
	Get(uid, req->ts(), resp->mutable_reward());
	DataSysmail& m = DataSysmailManager::Instance()->GetData(uid, req->ts());
	resp->set_ts(m.id);
	resp->set_stat(m.stat);
	return 0;
}
int LogicSysMailManager::Process(unsigned uid, User::RequestMailDel *req)
{
	DataSysmail &mail = DataSysmailManager::Instance()->GetData(uid, req->ts());
	if(mail.stat == e_mail_stat_over)
	{
		mail.stat = e_mail_stat_del;
		DataSysmailManager::Instance()->UpdateItem(mail);
	}
	return 0;
}
int LogicSysMailManager::Process(unsigned uid, User::RequestMailAllGet *req, User::ReplyMailAllGet *resp)
{
	vector<unsigned> vIds;
	DataSysmailManager::Instance()->GetIds(uid, vIds);
	for(vector<unsigned>::iterator it=vIds.begin();it!=vIds.end();++it)
		Get(uid, *it, resp->mutable_reward());
	return 0;
}
int LogicSysMailManager::Process(unsigned uid, User::RequestMailAllDel *req)
{
	vector<unsigned> vIds;
	DataSysmailManager::Instance()->GetIds(uid, vIds);
	for(vector<unsigned>::iterator it=vIds.begin();it!=vIds.end();++it)
	{
		DataSysmail &mail = DataSysmailManager::Instance()->GetData(uid, *it);
		if(mail.stat == e_mail_stat_over)
		{
			mail.stat = e_mail_stat_del;
			DataSysmailManager::Instance()->UpdateItem(mail);
		}
	}
	return 0;
}
int LogicSysMailManager::Process(User::ReqSendMailBC *req)
{
	MemorySysmailItem ms;
	ms.uid = req->uid();
	ms.ts = req->ts();
	strncpy(ms.sys, req->sys().c_str(), Data_sysmail_sys_COUNT-1);
	strncpy(ms.reward, req->reward().c_str(), Data_sysmail_reward_COUNT-1);

	DoRewardLocal(ms);

	return 0;
}
bool LogicSysMailManager::ExistNewMsg(unsigned uid)
{
	vector<unsigned> vIds;
	DataSysmailManager::Instance()->GetIds(uid, vIds);
	for(vector<unsigned>::iterator it=vIds.begin();it!=vIds.end();++it)
	{
		DataSysmail &m = DataSysmailManager::Instance()->GetData(uid, *it);
		if(MemorySysMailManager::Instance()->IsValid(m.id) && m.stat == e_mail_stat_unread)
		{
			return true;
		}
	}
	return false;
}
int LogicSysMailManager::DoReward(unsigned uid, const string& sys, const CommonGiftConfig::CommonModifyItem& reward)
{
	Json::Value obj;
	if(reward.has_based())
	{
		if(reward.based().has_coin())
		{
			obj["based"]["coin"] = reward.based().coin();
		}
		if(reward.based().has_cash())
		{
			obj["based"]["cash"] = reward.based().cash();
		}
		if(reward.based().has_exp())
		{
			obj["based"]["exp"] = reward.based().exp();
		}
		if(reward.based().has_friend_())
		{
			obj["based"]["friend"] = reward.based().friend_();
		}
	}
	for(uint32_t i = 0; i < reward.props_size(); ++i)
	{
		const CommonGiftConfig::PropsItem& item = reward.props(i);
		Json::Value propObj;
		propObj["id"] = item.id();
		propObj["count"] = item.count();
		obj["props"].append(propObj);
	}
	MemorySysmailItem ms;
	ms.uid = uid;
	ms.ts = Time::GetGlobalTime();
	strncpy(ms.sys, sys.c_str(), Data_sysmail_sys_COUNT-1);
	strncpy(ms.reward, Json::ToString(obj).c_str(), Data_sysmail_reward_COUNT-1);

	if(CMI->IsNeedConnectByUID(uid))
	{
		User::ReqSendMailBC* req = new User::ReqSendMailBC;
		req->set_uid(uid);
		req->set_ts(ms.ts);
		req->set_sys(ms.sys);
		req->set_reward(ms.reward);
		return BMI->BattleConnectNoReplyByUID(uid, req);
	}
	else
	{
		DoRewardLocal(ms);
	}

	return 0;
}
int LogicSysMailManager::DoRewardLocal(MemorySysmailItem& ms)
{
	if(!DataSysmailManager::Instance()->IsExistItem(ms.uid, ms.ts))
	{
		try
		{
			ms.Add(DataSysmailManager::Instance()->GetData(ms.uid, ms.ts));
			UMI->SendNewMsgToUser(ms.uid, NEW_MSG_TYPE_SYS_MAIL);
		}
		catch(const std::exception& e)
		{
			error_log("add sysmail fail uid=%u ts=%u", ms.uid, ms.ts);
		}
	}
	else
	{
		error_log("already exist sysmail uid=%u ts=%u", ms.uid, ms.ts);
	}
	return 0;
}

bool LogicSysMailManager::ValidPlatForm(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	return userwrap.Obj().register_platform == PT_Mi2;
}

int LogicSysMailManager::AddMail(unsigned uid,string title,string content,string reward)
{
	//1.校验邮件内容
	Json::Reader reader;
	Json::Value sys_json,reward_json;
	string sys = "{\"t\":\"" + title + "\",\"c\":\"" + content + "\"}";
	if(sys.empty() || reward.empty() || !reader.parse(reward, reward_json) || !reader.parse(sys, sys_json))
	{
		error_log("AddMail_Failed");
		return 1;
	}
	//2.添加邮件信息
	DataSysmail &mail = DataSysmailManager::Instance()->GetData(uid,Time::GetGlobalTime());
	strncpy(mail.sys, sys.c_str(), sizeof(mail.sys)-1);
	strncpy(mail.reward, reward.c_str(), sizeof(mail.reward)-1);
	mail.stat = e_mail_stat_unread;

	//3.发送邮件
	UMI->SendNewMsgToUser(uid, NEW_MSG_TYPE_SYS_MAIL);
	return 0;
}

int LogicSysMailManager::FirstRechargeAddMail(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);
	int IsRecevieRechargeMail = 0; //是否已收到过充值邮件提醒
	IsRecevieRechargeMail = userwrap.Obj().flag  & (1 << base_flag_id_is_send_recharge_mail);
	if(userwrap.Obj().acccharge > 0 && !IsRecevieRechargeMail)
	{
		//证明玩家有充值且从未收到过首充提醒邮件
		//1.若从未收到游戏内发的邮件，则添加
		DataSysmail &mail = DataSysmailManager::Instance()->GetData(uid,Time::GetGlobalTime());
		string title = ConfigManager::Instance()->user.m_config.xmuserreward().mailtitle();
		string content = ConfigManager::Instance()->user.m_config.xmuserreward().firstchargemailmsg();
		string reward = ConfigManager::Instance()->user.m_config.xmuserreward().rewarddiamondcnt(0).reward();
		AddMail(uid,title,content,reward);

		//2.更改标记
		userwrap.Obj().flag |= (1 << base_flag_id_is_send_recharge_mail);
		DataBase &base = BaseManager::Instance()->Get(uid);
		BaseManager::Instance()->UpdateDatabase(base);
	}
	return 0;
}

int LogicSysMailManager::AddMail(unsigned uid,bool NewUser)
{
	DBCUserBaseWrap userwrap(uid);

	if(NewUser)
	{
		int IsRecevieLoginMail = 0; //是否已收到过登录邮件
		IsRecevieLoginMail = userwrap.Obj().flag  & (1 << base_flag_id_is_send_promot_mail);

		if(!IsRecevieLoginMail)
		{
			//1.若从未收到游戏内发的邮件，则添加
			DataSysmail &mail = DataSysmailManager::Instance()->GetData(uid,Time::GetGlobalTime());
			string title = ConfigManager::Instance()->user.m_config.xmuserreward().mailtitle();
			string content = ConfigManager::Instance()->user.m_config.xmuserreward().firstmailmsg();
			string reward = ConfigManager::Instance()->user.m_config.xmuserreward().rewarddiamondcnt(0).reward();
			AddMail(uid,title,content,reward);

			//2.更改标记
			userwrap.Obj().flag |= (1 << base_flag_id_is_send_promot_mail);
			DataBase &base = BaseManager::Instance()->Get(uid);
			BaseManager::Instance()->UpdateDatabase(base);

			//3.更新首发奖励邮件的ts
			DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
			activity.actdata[e_Activity_UserData_1_index_12] = Time::GetGlobalTime();
			DataGameActivityManager::Instance()->UpdateActivity(activity);
		}
	}
	else
	{
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		unsigned lastSendRewardTs = activity.actdata[e_Activity_UserData_1_index_12];//上一次发送奖励邮件的ts
		bool isToday =false;
		if(Time::IsToday(lastSendRewardTs))
			isToday = true;
		bool IsRecevieRechargeMail = userwrap.Obj().flag  & (1 << base_flag_id_is_send_recharge_mail);//是否收到过首充提醒邮件
		bool IsRecevieRewardMailOver = userwrap.Obj().flag  & (1 << base_flag_id_is_send_reward_mail_over);//奖励邮件是否已发送完毕
		if(!IsRecevieRewardMailOver)
		{
			if(IsRecevieRechargeMail)//收到过首充邮件
			{
				if(!isToday)//今日第一次登录
				{
					int rewardsize =  ConfigManager::Instance()->user.m_config.xmuserreward().rewarddiamondcnt_size() - 1;
					for(int i = 1; i <= rewardsize; i++)
					{
						//查询对应的领取邮件是否已发送过
						bool IsSendRewardMail = userwrap.Obj().flag  & (1 << base_flag_id_is_send_reward_mail_over + i);
						if(!IsSendRewardMail)
						{
							//1.发送邮件
							string title = ConfigManager::Instance()->user.m_config.xmuserreward().mailtitle();
							string content = ConfigManager::Instance()->user.m_config.xmuserreward().rewarddiamondcnt(i).content();
							string reward = ConfigManager::Instance()->user.m_config.xmuserreward().rewarddiamondcnt(i).reward();
							AddMail(uid,title,content,reward);

							//2.更新标记
							userwrap.Obj().flag |= (1 << base_flag_id_is_send_reward_mail_over + i);
							activity.actdata[e_Activity_UserData_1_index_12] = Time::GetGlobalTime();
							DataGameActivityManager::Instance()->UpdateActivity(activity);

							//3.若为最后一封邮件、则更新标记
							if(i == rewardsize)
							{
								//最后一封奖励邮件、更新标记
								userwrap.Obj().flag |= (1 << base_flag_id_is_send_reward_mail_over);
							}
							DataBase &base = BaseManager::Instance()->Get(uid);
							BaseManager::Instance()->UpdateDatabase(base);
							break;
						}
					}
				}
			}
		}
	}
	return 0;
}
