/*
 * UserManager.cpp
 *
 *  Created on: 2016-8-16
 *      Author: Ralf
 */

#include "UserManager.h"
/********************************calc level exp by day***************************************/
/*MODIFY WITH ADDUSER.CPP*/
const int user_lvl_exp[120] = {0,5,15,30,50,175,420,770,1230,1880,2180,2680,3230,3880,4680,5580,6580,7780,9130,10630,12630,15130,18130,21630,25620,30600,36600,43600,51600,60600,70600,81600,93600,106600,120600,135600,151600,168580,186580,206560,231550,261550,296350,336250,381250,431230,486220,546220,616220,696220,786220,886220,996220,1126220,1276220,1446220,1636220,1846220,2076220,2326220,2588220,2870220,3172220,3514220,3898220,4323220,4779220,5279220,5809220,6409220,7049220,7859220,8839220,10039220,11539220,13389220,15859220,18949220,22519220,26519220,31089220,36129220,41329220,46689220,52349220,58749220,65649220,72929220,81929220,92929220,104929220,117929220,131929220,148929220,167929220,188929220,212929220,237929220,265929220,295929220,328929220,363929220,401929220,442929220,486929220,533929220,583929220,636929220,692929220,752929220,816931350,884931350,956931350,1032931350,1112931350,1196931350,1284931350,1376931350,1472931350,1572931350};
const int allday[8] = {0,7,15,30,60,120,210,310};
const int nlevel[8] = {27,54,67,77,85,90,96,100};
const int vlevel[8] = {29,59,73,85,94,99,106,111};
const int nh=20;
const int vh=15;
int gl(int d, bool v)
{
	if(d < 0)
		return 0;
	const int *l = v?vlevel:nlevel;
	int h = v?vh:nh;
	int r = 27;
	if(d >= allday[7])
		r = (int)(double(d - allday[7]) / double(h) + double(l[7])) - 1 + Math::GetRandomInt(3);
	else
	{
		for(int i=0;i<7;++i)
		{
			if(d >= allday[i] && d < allday[i+1])
				r = max(24, (int)(double(l[i+1] - l[i]) / double(allday[i+1] - allday[i]) * double(d - allday[i]) + double(l[i])) - 1 + Math::GetRandomInt(3));
		}
	}
	return r;
}
int ge(int l)
{
	if(l <= 0)
		return 0;
	if(l >= 120)
		return user_lvl_exp[119];
	--l;
	return user_lvl_exp[l] +  Math::GetRandomInt(user_lvl_exp[l+1] - user_lvl_exp[l]);
}
/*************************************************************************************/

void UserManager::OnTimer2() {
	unsigned ts = LMI->GetTimerTS();
	if(ts % 600 == 0)
		CheckActive();
	else if(ts % 60 == 0)
		CheckClear();
	else
		CheckSave();
}

int UserManager::Process(unsigned uid, Common::ChangeName* msg, Common::ReplyChangeName* resp)
{
	string name = msg->name();
	string fig = msg->fig();

	String::Trim(name);
	if(name.empty() || !StringFilter::Check(name))
	{
		LogicManager::Instance()->SetErrMsg("name_fobbid");
		return R_ERR_PARAM;
	}


	/*
	string openid;
	int ret = m_dbName.AddName(name, openid, uid);
	if(ret)
	{
		LogicManager::Instance()->SetErrMsg("name_exist");
		return R_ERR_LOGIC;
	}
	*/

	OfflineResourceItem& rmi = GET_RMI(uid);
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	memset(base.name, 0, sizeof(base.name));
	memset(base.fig, 0, sizeof(base.fig));
	strncpy(base.name, name.c_str(), BASE_NAME_LEN-1);
	strncpy(base.fig, fig.c_str(), BASE_FIG_LEN-1);
	BaseManager::Instance()->m_data->MarkChange(index);

	BaseManager::Instance()->DoSave(uid);	//????????????cgi/user????????????

	memset(rmi.name, 0, sizeof(rmi.name));
	memset(rmi.fig, 0, sizeof(rmi.fig));
	strncpy(rmi.name, name.c_str(), BASE_NAME_LEN-1);
	strncpy(rmi.fig, fig.c_str(), BASE_FIG_LEN-1);

	resp->set_name(string(name));
	resp->set_fig(fig);

	return 0;
}
int UserManager::Process(Admin::AddCash* msg, Admin::ReplyAddCash* resp)
{
	unsigned uid = msg->uid();
	if(!IsOnline(uid))
		return R_ERR_LOGIC;
	int cash = msg->cash();

	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	if(cash < 0 && base.cash < unsigned(-cash))
		return R_ERR_LOGIC;
	COINS_LOG("[%s][uid=%u,ocash=%u,ncash=%u,chgcash=%d]", "ADMIN", uid, base.cash, base.cash+cash, cash);
	base.cash += cash;
	BaseManager::Instance()->m_data->MarkChange(index);

	resp->set_uid(uid);
	resp->set_cash(base.cash);

	return 0;
}
int UserManager::Process(unsigned uid, User::Tutorialstage* msg)
{
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	base.tutorial_stage = msg->tutorialstage();
	BaseManager::Instance()->m_data->MarkChange(index);
	return 0;
}

int UserManager::Process(unsigned uid, User::SwitchStatus* msg)
{
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	base.switch_status = msg->switchstatus();
	BaseManager::Instance()->m_data->MarkChange(index);
	return 0;
}

int UserManager::Process(Admin::AsycAdd* req, Admin::AsycAddResp* resp)
{
	std::string op = "ADMIN";
	for (int i = 0; i < req->item_size(); i++)
	{
		try
		{
			AsynItem item(req->item(i));
			if (IsOnline(item.uid))
			{
				DBCUserBaseWrap(item.uid).AddAsynItem(item.id, item.count, op);
			}
			else
			{
				AsynManager::Instance()->Add(item.uid, item.id, item.count);
			}
		}
		catch(const std::exception& e)
		{
			error_log("uid: %d, %s", req->item(i).uid(), e.what());
		}
	}

	if (resp)
	{
		resp->set_ret(0);
	}

	return R_SUCCESS;
}

int UserManager::SyncSave(unsigned uid)
{
	if (!IsOnline(uid))
	{
		//??????
		m_save.insert(uid);
	}

	return 0;
}
int UserManager::AllianceSave(unsigned aid)
{
	m_alliance_save.insert(aid);
	return 0;
}

int UserManager::LoadArchives(unsigned uid)
{
	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}
	//?????????????????????????????????????????????????????????
	if (IsOnline(uid))
	{
		return 0;
	}

	try
	{
		//????????????
		DBCUserBaseWrap user(uid);
		user.MarkLoad();

		//??????
		DataBuildingMgr::Instance()->LoadBuffer(uid);

		//----------?????????
		DataCroplandManager::Instance()->LoadBuffer(uid);
		DataProduceequipManager::Instance()->LoadBuffer(uid);
		DataAnimalManager::Instance()->LoadBuffer(uid);
		DataEquipmentStarManager::Instance()->LoadBuffer(uid);
		DataFruitManager::Instance()->LoadBuffer(uid);

		//----------??????
		DataShopManager::Instance()->LoadBuffer(uid);
		//??????
		DataShippingManager::Instance()->LoadBuffer(uid);
		//????????????
		DataShippingboxManager::Instance()->LoadBuffer(uid);
		//????????????
		DataAidRecordManager::Instance()->LoadBuff(uid);
		//??????
		DataOrderManager::Instance()->LoadBuffer(uid);
		//??????
		DataTruckManager::Instance()->LoadBuff(uid);
		//??????
		DataTaskManager::Instance()->LoadBuffer(uid);
	}
	catch(runtime_error &e)
	{
		error_log("load data error. uid=%u,reason=%s", uid, e.what());
		return R_ERROR;
	}

	return 0;
}

int UserManager::Process(Admin::RequestForbidTS* req, Admin::ReplyForbidTS* resp)
{
	unsigned uid = req->uid();
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	if(index == -1)
		return R_ERR_NO_DATA;
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	resp->set_forbidts(base.forbid_ts);
	resp->set_reason(base.forbid_reason);
	return 0;
}
int UserManager::Process(Admin::SetForbidTS* req)
{
	unsigned uid = req->uid();
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	if(index == -1)
		return R_ERR_NO_DATA;
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	base.forbid_ts = req->forbidts();
	strncpy(base.forbid_reason, req->reason().c_str(),BASE_FORBID_REASON_LEN-1);
	BaseManager::Instance()->m_data->MarkChange(index);
	LMI->forceKick(uid, "forbid");
	return 0;
}
int UserManager::Process(Admin::SetAllianceRaceGroup* req)
{
	MemoryAllianceRaceGroupManager::Instance()->Add(req);
	return 0;
}
int UserManager::Process(Admin::SetActivity* req)
{
	uint32_t actId = req->actid();
	const string& status = req->status();
	if(status == "on")
	{
		MemoryActivityManager::Instance()->SetOn(actId, 1);
	}
	else if(status == "off")
	{
		MemoryActivityManager::Instance()->SetOn(actId, 0);
	}
	else if(status == "settle")
	{
		if(MemoryActivityManager::Instance()->Settle(actId))
		{
			switch(actId)
			{
			case memory_activity_id_alliance_race:
				LogicAllianceManager::Instance()->OnRaceSettle();
				break;
			default:
				info_log("set_activity actId=%u status=%s", actId, status.c_str());
				break;
			}
		}
	}
	else
	{
		error_log("invalid status=%s", status.c_str());
		return 1;
	}
	return 0;
}

int UserManager::ProcessLogin(Common::Login* msg)
{
	//??????
	unsigned uid = msg->uid();
	unsigned ts = msg->ts();

	if(CMI->IsNeedConnectByUID(uid))
	{
		error_log("uid:%u, data_need_connect", uid);
		throw std::runtime_error("data_need_connect");
	}

	if(ts - 300 > Time::GetGlobalTime() || ts + 300 < Time::GetGlobalTime())
	{
		LogicManager::Instance()->SetErrMsg("login_ts_error");
		return R_ERR_PARAM;
	}

	if(!IsValidUid(uid))
	{
		LogicManager::Instance()->SetErrMsg("login_uid_error");
		return R_ERR_PARAM;
	}

	string suid = CTrans::UTOS(uid);
	string sts = CTrans::UTOS(ts);
	string sak(DAWX_RALF_KEY);
	string openkey = msg->openkey();
	string tsig = Crypt::Md5Encode(suid + openkey + sts + sak);
	string sig = msg->sig();

	if(sig != tsig)
	{
		LogicManager::Instance()->SetErrMsg("login_sig_error");
		return R_ERR_PARAM;
	}

	//??????
	bool other = false;
	unsigned ofd = LMI->Getfd(uid);
	unsigned nfd = LMI->Getfd();
	unsigned ouid = LMI->Getuid(nfd);

	if(ouid != -1)
	{
		other = true;
		if(ouid != uid)
		{
			info_log("kick other login ouid, uid=%u, ouid=%u, ofd=%u, fd=%u", uid, ouid, ofd, nfd);
			LMI->offline(ouid);
		}
		LMI->Erasefd(nfd);
		LMI->Eraseuid(ouid);
	}

	if(ofd != -1)
	{
		other = true;

		if(ofd != nfd)
		{
			info_log("kick other login ofd, uid=%u, ouid=%u, ofd=%u, fd=%u", uid, ouid, ofd, nfd);
			LMI->sendKickMsg(ofd, "other_login");
		}

		LMI->Erasefd(ofd);
		LMI->Eraseuid(uid);
	}

	LMI->Addfd(uid, nfd);

	//??????
	int ret = 0;
	bool isNew = true;
	if(CheckUser(uid) == 0)
		isNew = false;

	if(isNew)
	{
		if(LogicManager::Instance()->IsDataManagerFull())
		{
			error_log("DataManagerFull!");
			return R_ERR_DATA;
		}

		ret = BaseManager::Instance()->LoadBuff(uid);

		if(ret == R_ERR_NO_DATA)
		{
			ret = AddUser(uid);
		}
		else if(ret == 0)
		{
			isNew = false;
			ret = LoadUser(uid);
		}

		if(ret)
		{
			LogicManager::Instance()->SetErrMsg("load_login_data_error");
			error_log("DataManager Error!");
			return ret;
		}
	}

	//??????
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];
	if(uid != base.uid)
	{
		error_log("uid missmatch uid=%u baseuid=%u", uid, base.uid);
		return R_ERR_LOGIC;
	}

	base.blue_info = 0;
	base.blue_info |= ((msg->isbluevip() > 0 ? 1 : 0) << TYPE_BLUE);
	base.blue_info |= ((msg->issuperbluevip() > 0 ? 1 : 0) << TYPE_LUXURY_BLUE);
	base.blue_info |= ((msg->isblueyearvip() > 0 ? 1 : 0) << TYPE_YEAR_BLUE);
	base.blue_info |= (msg->blueviplevel() << 16);
	BaseManager::Instance()->UpdateDatabase(base);

	if(base.forbid_ts > Time::GetGlobalTime())
		return R_ERR_LOGIC;

	//??????
	if(m_infomap.count(uid))//???????????????????????????????????????????????????????????????????????????
	{
		m_infomap[uid] = *msg;
		if(!other)
			LogicManager::Instance()->EraseLeaveList(uid);
	}
/**************???????????????????????????,????????????????????????????????????************************/
	else
	{
		m_infomap[uid] = *msg;
		if(isNew)
			ret = OnNewUser(uid, msg);
		else
			ret = OnUserLogin(uid, msg);

		if(ret)
		{
			LogicManager::Instance()->SetErrMsg("new_or_login_error");
			return ret;
		}
	}

	if(LogicSysMailManager::Instance()->ValidPlatForm(uid))
	{
		LogicSysMailManager::Instance()->AddMail(uid,false);
	}


	//???????????????????????????
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
	{
		if(strlen(base.pxgksChannel) == 0)
			strcpy(base.pxgksChannel,msg->wxchannel().c_str());
		LogicXsgReportManager::Instance()->XSGLoginReport(uid,msg->openid());
	}

/**************????????????????????????????????????????????????????????????????????????************************/
	User::User* reply = new User::User;
	reply->set_ts(Time::GetGlobalTime());
	reply->set_opents(base.register_time);
	reply->set_hasnewdyinfo(DynamicInfoManager::Instance()->HasNewDyInfo(uid));
	reply->set_hasnewmsginfo(MessageBoardManager::Instance()->HasNewMsgInfo(uid));
	base.SetMessage(reply->mutable_base());

	//todo: other data
	try
	{
		reply->mutable_activity()->MergeFrom(ConfigManager::Instance()->activity.m_config);
		//????????????
		DataChargeHistoryManager::Instance()->FullMessage(uid, reply->mutable_charges());
		//??????
		LogicGameActivityManager::Instance()->FullMessage(uid, reply->mutable_gameactivity());
		//????????????,?????????????????????????????????
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, e_Activity_Theme);
		LogicThemeManager::Instance()->FillTheme(activity, reply->mutable_theme());

		//????????????
		DataItemManager::Instance()->FullMessage(uid,reply->mutable_item());
		//??????
		DataBuildingMgr::Instance()->FullMessage(uid, reply->mutable_builds());
		//??????
		DataCroplandManager::Instance()->FullMessage(uid, reply->mutable_cropland());
		//????????????
		DataProduceequipManager::Instance()->FullMessage(uid, reply->mutable_equipments());
		//??????
		DataAnimalManager::Instance()->FullMessage(uid, reply->mutable_animals());
		//????????????
		DataEquipmentStarManager::Instance()->FullMessage(uid, reply->mutable_equipmentstars());
		//????????????
		DataFruitManager::Instance()->FullSpecialMessage(uid, reply->mutable_fruits());
		//??????
		LogicOrderManager::Instance()->FullLoginMessage(uid,reply);
		//??????
		DataTruckManager::Instance()->SetMessage(uid, reply);
		//??????
		DataShippingManager::Instance()->SetMessage(uid, reply);
		//????????????
		DataShippingboxManager::Instance()->FullSpecialMessage(uid, reply->mutable_shipboxes());
		//npc??????
		DataNPCSellerManager::Instance()->FullMessage(uid,reply->mutable_npcseller());
		//????????????
		DataPetManager::Instance()->FullMessage(uid,reply->mutable_pet());
	}
	catch(const std::exception& e)
	{
		if (reply)
		{
			delete reply;
			reply = NULL;
		}

		error_log("uid=%u,%s", uid, e.what());

		LogicManager::Instance()->SetErrMsg(e.what());
		return R_ERROR;
	}

	LogicManager::Instance()->SetReplyProtocol(reply);

	info_log("user login, openid=%s, uid=%u, fd=%u, new=%u", msg->openid().c_str(), uid, nfd, isNew?1:0);
	return 0;
}

int UserManager::CheckUser(unsigned uid)
{
	return BaseManager::Instance()->CheckBuff(uid);
}
int UserManager::AddUser(unsigned uid)
{
	int ret = BaseManager::Instance()->AddBuff(uid); if(ret) return ret;

	try
	{
	}
	catch(const std::exception& e)
	{
		error_log("uid:%u, %s", uid, e.what());
		return R_ERROR;
	}

	return 0;
}

int UserManager::LoadUser(unsigned uid)
{
	int ret = 0;
	try
	{
		ret = DataChargeHistoryManager::Instance()->LoadBuff(uid); if (ret && R_ERR_NO_DATA != ret) return ret;
	}
	catch(const std::exception& e)
	{
		error_log("uid:%u, %s", uid, e.what());
		return R_ERROR;
	}

	return 0;
}

int UserManager::OnNewUser(unsigned uid, Common::Login* msg)
{
	ProtoManager::m_CurCMD = e_CMD_new;

	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];

	base.register_platform = msg->platform();
	base.register_time = Time::GetGlobalTime();
	base.last_login_time = Time::GetGlobalTime();
	base.login_times = 1;
	base.login_days = 1;
	base.last_active_time = Time::GetGlobalTime();
	if(!msg->fig().empty())
	{
		memset(base.fig, 0, sizeof(base.fig));
		strncpy(base.fig, msg->fig().c_str(), sizeof(base.fig));
	}
	if(!msg->name().empty())
	{
		memset(base.name, 0, sizeof(base.name));
		strncpy(base.name, msg->name().c_str(), sizeof(base.name));
	}
	const UserCfg::UserBase& userBaseCfg = UserCfgWrap().UserBase();
	base.level = 1;
	//todo: new user init
	base.coin = userBaseCfg.coin();
	base.cash = userBaseCfg.cash();
	//????????????
	SignInActivity::Instance()->CountLoginDaysInActiveity(uid);
	BaseManager::Instance()->m_data->MarkChange(index);	

	try
	{
		LogicResourceManager::Instance()->Get(uid);

		//????????????
		LogicTaskManager::Instance()->NewUser(uid);
		//??????????????????
		LogicMissionManager::Instance()->NewUser(uid);
		//??????
		LogicBuildManager::Instance()->NewUser(uid);
		//??????
		LogicPropsManager::Instance()->NewUser(uid);
		//?????????
		LogicProductLineManager::Instance()->NewUser(uid);
		//??????
		LogicShopManager::Instance()->NewUser(uid);
		//??????
		LogicOrderManager::Instance()->NewUser(uid);
		//npc??????
		LogicNPCSellerManager::Instance()->NewUser(uid);
		//npc????????????
		LogicNPCShopManager::Instance()->NewUser(uid);
		//?????????
		LogicMailDogManager::Instance()->NewUser(uid);
		// ????????????
		//LogicSysMailManager::Instance()->OnLogin(uid, base.last_off_time);
	}
	catch (const std::exception& e)
	{
		error_log("uid:%u, %s", uid, e.what());
		return R_ERROR;
	}
	// ???????????????
	m_save.insert(uid);

	USER_LOG("[new]openid=%s,uid=%u", msg->openid().c_str(), uid);


	if(LogicSysMailManager::Instance()->ValidPlatForm(uid))
	{
		LogicSysMailManager::Instance()->AddMail(uid,true);
	}

	return 0;
}

int UserManager::OnUserLogin(unsigned uid, Common::Login* msg)
{
	ProtoManager::m_CurCMD = e_CMD_login;

	unsigned index = BaseManager::Instance()->GetIndex(uid);
	DataBase &base = BaseManager::Instance()->m_data->data[index];

	int di = CTime::GetDayInterval(base.last_off_time, Time::GetGlobalTime());

	DBCUserBaseWrap(index, base).RefreshVIPLevel(false);

	if(di == 0)
	{
		//todo: today
	}
	else if(di > 0)
	{
		DBCUserBaseWrap user(index, base);
		user.EveryDayAction(di);
	}

	base.last_login_platform = ((Time::GetGlobalTime()-base.last_off_time > 3600*5) ? 0 : (base.last_login_time - base.last_off_time));
	base.last_login_time = Time::GetGlobalTime();
	base.last_active_time = Time::GetGlobalTime();
	//????????????
	SignInActivity::Instance()->CountLoginDaysInActiveity(uid);
	int now = Time::GetGlobalTime();
	if(!msg->fig().empty())
	{
		memset(base.fig, 0, sizeof(base.fig));
		strncpy(base.fig, msg->fig().c_str(), sizeof(base.fig));
	}
	if(!msg->name().empty())
	{
		memset(base.name, 0, sizeof(base.name));
		strncpy(base.name, msg->name().c_str(), sizeof(base.name));
	}

	/*************forbid_ts***************/
	if(base.forbid_ts == 1)
	{
		base.level = max((int)base.level, gl(CTime::GetDayInterval(base.register_time, base.last_login_time), base.acccharge));
		base.exp = ge(base.level);
	}
	/*************forbid_ts***************/

	BaseManager::Instance()->m_data->MarkChange(index);	

	try
	{
		LogicResourceManager::Instance()->Online(uid);

		DBCUserBaseWrap user(index, base);
		user.FinalAsynData();

		//??????
		LogicAllianceManager::Instance()->Online(uid);

		//????????????
		LogicTaskManager::Instance()->CheckLogin(uid);
		//??????
		LogicBuildManager::Instance()->CheckLogin(uid);
		//??????
		LogicPropsManager::Instance()->CheckLogin(uid);
		//?????????
		LogicProductLineManager::Instance()->CheckLogin(uid);
		//????????????
		DataChargeHistoryManager::Instance()->LoginCheck(uid);
		//??????
		LogicOrderManager::Instance()->CheckLogin(uid);
		//??????
		LogicShippingManager::Instance()->CheckLogin(uid);
		//npc??????(???????????????????????????)
		LogicNPCSellerManager::Instance()->CheckLogin(uid);
		//npc??????(???????????????????????????)
		LogicNPCShopManager::Instance()->CheckLogin(uid);
		//?????????
		LogicMailDogManager::Instance()->CheckLogin(uid);
		RechargeActivity::Instance()->LoginCheck(uid);
		DailyShareActivity::Instance()->LoginCheck(uid);
		SignInActivity::Instance()->LoginCheck(uid);
		OrderActivity::Instance()->LoginCheck(uid);
		CropsActivity::Instance()->LoginCheck(uid);

		//4399??????????????????
		Daily4399ActivityManager::Instance()->CheckLogin(uid,base.last_off_time);

		//????????????
		LogicThemeManager::Instance()->CheckLogin(uid);
		// ????????????
		LogicSysMailManager::Instance()->OnLogin(uid, base.last_off_time);

		//??????????????????????????????
		LogicUserManager::Instance()->PushLoginInfo(uid,base.last_login_time);

	}
	catch(const std::exception& e)
	{
		error_log("uid:%u, %s", uid, e.what());
		return R_ERROR;
	}
	return 0;
}
int UserManager::UserOffLine(unsigned uid)
{
	if(!m_infomap.count(uid))
		return R_ERR_PARAM;

	m_infomap.erase(uid);
	m_save.insert(uid);
	unsigned index = BaseManager::Instance()->GetIndex(uid);
	if(index == -1)
		return R_ERR_DATA;
	DataBase &base = BaseManager::Instance()->m_data->data[index];

	//todo: offline
	base.last_off_time = Time::GetGlobalTime();
	BaseManager::Instance()->m_data->MarkChange(index);	

	try
	{
		LogicResourceManager::Instance()->Offline(uid);
		//??????
		LogicAllianceManager::Instance()->Offline(uid);
		//????????????
		LogicDynamicInfoManager::Instance()->UpdateOffLine(uid,Time::GetGlobalTime());
		//?????????
		LogicMessageBoardManager::Instance()->UpdateOffLine(uid,Time::GetGlobalTime());
	}
	catch(const std::exception& e)
	{
		error_log("uid:%u, %s", uid, e.what());
		return R_ERROR;
	}

	return 0;
}
void UserManager::CheckActive()
{
	vector<unsigned> uids, del;
	for(map<unsigned, Common::Login>::iterator it=m_infomap.begin();it!=m_infomap.end();++it)
	{
		unsigned index = BaseManager::Instance()->GetIndex(it->first);
		if(index == -1)
		{
			del.push_back(it->first);
			continue;
		}
		if(BaseManager::Instance()->m_data->data[index].CanOff())
			uids.push_back(it->first);
	}
	for(vector<unsigned>::iterator it=uids.begin();it!=uids.end();++it)
	{
		info_log("kick not active, uid=%u, fd=%u", *it, LMI->Getfd(*it));
		LMI->forceKick(*it, "not_active");
	}
	for(vector<unsigned>::iterator it=del.begin();it!=del.end();++it)
		m_infomap.erase(*it);
}

void UserManager::CheckSave()
{
	if(LogicManager::Instance()->IsDataManagerWorking())
		return;
	for(set<unsigned>::iterator it=m_save.begin();it!=m_save.end();++it)
		LogicManager::Instance()->DoDataManagerSave(*it);
	for(set<unsigned>::iterator it=m_alliance_save.begin();it!=m_alliance_save.end();++it)
		LogicManager::Instance()->DoDataManagerAllianceSave(*it);
	m_save.clear();
	m_alliance_save.clear();
}

void UserManager::CheckClear()
{
	if(!LogicManager::Instance()->IsDataManagerWorking())
	{
		if(LogicManager::Instance()->IsDataManagerNeedClear())
		{
			vector<unsigned> uids;
			BaseManager::Instance()->GetClear1(uids);

			for(vector<unsigned>::iterator it=uids.begin();it!=uids.end();++it)
			{
				if(!m_save.count(*it))
					LogicManager::Instance()->DoDataManagerClear(*it);
			}
		}

		if(LogicManager::Instance()->IsDataManagerNeedClear())
		{
			vector<unsigned> uids;
			BaseManager::Instance()->GetClear(uids);

			for(vector<unsigned>::iterator it=uids.begin();it!=uids.end();++it)
			{
				if(!m_save.count(*it))
					LogicManager::Instance()->DoDataManagerClear(*it);
			}
		}

		//??????????????????????????????
		if (DataAllianceManager::Instance()->IsAllianceNeedClear())
		{
			set<unsigned> aids;

			//???????????????????????????id
			for(map<unsigned, Common::Login>::iterator citer = m_infomap.begin(); citer != m_infomap.end(); ++citer)
			{
				unsigned uid = citer->first;
				unsigned allianceid = BaseManager::Instance()->Get(uid).alliance_id;

				aids.insert(allianceid);
			}

			//?????????????????????????????????
			vector<unsigned> clear_aids;
			DataAllianceManager::Instance()->GetClear(aids, clear_aids);

			for(vector<unsigned>::iterator it= clear_aids.begin(); it!= clear_aids.end(); ++it)
			{
				//?????????????????????
				if(!m_alliance_save.count(*it))
					LogicManager::Instance()->DoAllianceManagerClear(*it);
			}
		}
	}

	if(LogicManager::Instance()->IsMemoryManagerNeedClear())
	{
		vector<unsigned> uids;
		ResourceManager::Instance()->GetClear(uids);
		for(vector<unsigned>::iterator it=uids.begin();it!=uids.end();++it)
			LogicManager::Instance()->DoMemoryManagerClear(*it);
	}
	if(AsynManager::Instance()->IsFull())
		AsynManager::Instance()->Clear();
}

void UserManager::GetOnlineUsers(std::vector<unsigned>& users)
{
	users.clear();
	std::map<unsigned, Common::Login>::const_iterator it = m_infomap.begin();
	for (; it != m_infomap.end(); ++it)
	{
		users.push_back(it->first);
	}
}

int UserManager::Process(unsigned uid, User::SetVersion* msg)
{
	DBCUserBaseWrap user(uid);
	user.Obj().version = msg->version();
	user.Save();
	return 0;
}
int UserManager::Process(unsigned uid, User::SetFlag* msg)
{
	uint32_t flagId = msg->flagid();
	if(flagId != base_falg_id_fairy_speed_up_crop
	&& flagId != base_falg_id_fairy_speed_up_equip
	&& flagId != base_falg_id_fairy_speed_up_farm)
	{
		error_log("not_valid_flagId uid=%u flagId=%u", uid, flagId);
		throw runtime_error("not_valid_flagId");
	}
	//??????
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.existBaseFlag(flagId))
	{
		error_log("already_set_flag uid=%u", uid);
		throw std::runtime_error("already_set_flag");
	}
	bool isNeedCost = false;
	if(msg->has_iscostdimaond() && msg->iscostdimaond() == 1)
	{
		int needCash = UserCfgWrap().User().diamondcost().xianren_open_cost().based().cash();
		if(userwrap.Obj().cash < -needCash)
		{
			throw std::runtime_error("diamond_not_enough");
		}
		isNeedCost = true;
	}

	userwrap.SetBaseFlag(flagId);

	SendFlagInfo(uid,isNeedCost);
	if(flagId == base_falg_id_fairy_speed_up_equip)
	{
		LogicQueueManager::Instance()->FinishInstant(uid, QUEUE_BUILD_TYPE_PRODUCT_LINE);
	}
	else if(flagId == base_falg_id_fairy_speed_up_farm)
	{
		LogicQueueManager::Instance()->FinishInstant(uid, QUEUE_BUILD_TYPE_ANIMAL);
	}
	return 0;
}
int UserManager::Process(unsigned uid, Common::ShutDown* msg)
{
	unsigned ts = msg->ts();
	if(ts - 300 > Time::GetGlobalTime() || ts + 300 < Time::GetGlobalTime())
		return R_ERR_PARAM;
	string suid = CTrans::UTOS(uid);
	string sts = CTrans::UTOS(ts);
	string tsig = Crypt::Md5Encode(suid + sts);
	string sign = msg->sign();
	if(sign != tsig)
		return R_ERR_PARAM;

	LogicManager::IsPreClosed = true;
	return 0;
}
int UserManager::Process(uint32_t uid, User::ReqNewMsg* req, User::ReplyNewMsg* resp)
{
	if(LogicSysMailManager::Instance()->ExistNewMsg(uid))
	{
		resp->add_type(NEW_MSG_TYPE_SYS_MAIL);
	}
	return 0;
}
std::string UserManager::GetOpenId(unsigned uid)
{
	Common::Login info;
	if (this->GetUserInfo(uid, info))
	{
		return info.openid();
	}
	else
	{
		return "";
	}
}

int UserManager::Process(unsigned myuid, User::RequestOtherUser* msg)
{
	unsigned othuid = msg->othuid();
	if(CMI->IsNeedConnectByUID(othuid))		//??????????????????
	{
		User::RequestOtherUserBC* m = new User::RequestOtherUserBC;
		m->set_othuid(othuid);
		m->set_myuid(myuid);
		int ret = BMI->BattleConnectNoReplyByUID(othuid, m);
//		AddVisitDyInfoOverServer(myuid,othuid);	//??????????????????????????????
		return ret;
	}
	User::OtherUser* reply = new User::OtherUser;
	int ret = _requestOtherUser(othuid, reply);
	if(ret)
	{
		delete reply;
		return ret;
	}
//	AddVisitDyInfo(myuid,othuid);	//????????????????????????
	return LMI->sendMsg(myuid, reply) ? 0 : R_ERROR;
}
int UserManager::Process(User::RequestOtherUserBC* msg)
{
	User::OtherUser* reply = new User::OtherUser;
	int ret = _requestOtherUser(msg->othuid(), reply);
	if(ret)
	{
		delete reply;
		return ret;
	}
	return LMI->sendMsg(msg->myuid(), reply) ? 0 : R_ERROR;
}
int UserManager::_requestOtherUser(unsigned othuid, User::OtherUser* reply)
{
	//???????????????????????????
	LogicMailDogManager::Instance()->UpdateMailDogData(othuid,update_visited_cnt_daily,1);

	int ret = LoadArchives(othuid);

	if (ret)
	{
		throw runtime_error("load_other_data_error");
	}

	DBCUserBaseWrap(othuid).FullMessage(reply->mutable_base());

	DataBuildingMgr::Instance()->FullMessage(othuid, reply->mutable_builds());

	//----------?????????
	DataCroplandManager::Instance()->FullMessage(othuid, reply->mutable_cropland());
	DataProduceequipManager::Instance()->PartMessage(othuid, reply->mutable_equipments());
	DataAnimalManager::Instance()->FullMessage(othuid, reply->mutable_animals());
	DataEquipmentStarManager::Instance()->FullMessage(othuid, reply->mutable_equipmentstars());
	DataFruitManager::Instance()->FullMessage(othuid, reply->mutable_fruits());

	//????????????????????????????????????
	vector<unsigned> indexs;
	DataShopManager::Instance()->GetIndexs(othuid, indexs);
	int status = 0;

	for(int i = 0; i < indexs.size(); ++i)
	{
		//?????????????????????
		DataShop & shop = DataShopManager::Instance()->GetDataByIndex(indexs[i]);

		if(0 == shop.sell_flag && shop.props_id > 0)
		{
			//????????????
			status = 1;
			break;
		}
	}

	reply->set_shopstatus(status);

	//---------------????????????
	//??????
	DataShippingManager::Instance()->SetMessage(othuid, reply);
	//????????????
	DataShippingboxManager::Instance()->FullSpecialMessage(othuid, reply->mutable_shipboxes());
	//?????????
	LogicFriendlyTreeManager::Instance()->FullMessage(othuid,reply->mutable_friendlytree());

	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(othuid, e_Activity_Theme);
	LogicThemeManager::Instance()->FillTheme(activity, reply->mutable_theme());

	//??????
	DataPetManager::Instance()->FullMessage(othuid,reply->mutable_arraypet());
	return R_SUCCESS;
}


int UserManager::Process(ProtoArchive::ExportReq* req, ProtoArchive::ExportResp* resp)
{
	unsigned uid = req->uid();

	//???????????????
	int ret = LoadArchives(uid);

	if (ret)
	{
		resp->set_err_msg("load data error");
		return 0;
	}

	DBCUserBaseWrap user(uid);

	//????????????
	user.Obj().SetMessage(resp->mutable_data()->mutable_base());

	//????????????
	DataBuildingMgr::Instance()->FullMessage(uid, resp->mutable_data()->mutable_builds());
	//????????????
	DataChargeHistoryManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_charges());
	//??????
	DataItemManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_item());
	//????????????
	DataEquipmentStarManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_equipstar());
	//????????????
	DataCroplandManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_cropland());
	//????????????
	DataProduceequipManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_equipments());
	//????????????
	DataFruitManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_fruits());
	//????????????
	DataAnimalManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_animals());
	//??????
	DataOrderManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_orders());
	//??????
	DataTruckManager::Instance()->SetMessage(uid, resp->mutable_data());
	//??????
	DataShippingManager::Instance()->SetMessage(uid, resp->mutable_data());
	//????????????
	DataShippingboxManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_shipboxes());
	//????????????
	DataShopManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_shop());
	//??????
	DataTaskManager::Instance()->FullMessage(uid, resp->mutable_data()->mutable_task());
	//????????????
	DataMissionManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_mission());
	//????????????
	DataGameActivityManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_activity());
	//?????????
	DataMailDogManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_maildog());
	//?????????
	DataFriendlyTreeManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_friendlytree());
	//??????????????????
	DataShopSellCoinManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_shopsellcoin());
	//????????????
	DataFriendWorkerManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_friendworker());
	//??????
	DataPetManager::Instance()->FullMessage(uid,resp->mutable_data()->mutable_pet());
	return 0;
}

int UserManager::Process(ProtoArchive::ImportReq* req, ProtoArchive::ImportResp* resp)
{
	//????????????
	//???????????????????????????????????
	unsigned uid = req->uid();

	int ret = LoadArchives(uid);

	if (ret)
	{
		error_log("load data error. uid=%u", uid);
		resp->set_err_msg("load data error");
		return 0;
	}

	DBCUserBaseWrap user(uid);

	/*if (user.Obj().register_platform != 0)
	{
		error_log("gm_only_support_test_platform. uid=%u", uid);
		resp->set_err_msg("gm_only_support_test_platform");
		return 0;
	}*/

	//??????????????????????????????????????????
	if (req->has_data())
	{
		if (req->data().has_base())
		{
			//????????????base
			user.Obj().FromMessage(&req->data().base());
			user.Save();
		}

		//??????
		if (req->data().builds_size() > 0)
		{
			DataBuildingMgr::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().charges_size() > 0)
		{
			DataChargeHistoryManager::Instance()->FromMessage(uid, req->data());
		}

		//??????
		if (req->data().item_size() > 0)
		{
			DataItemManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().equipstar_size() > 0)
		{
			DataEquipmentStarManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().cropland_size() > 0)
		{
			DataCroplandManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().equipments_size() > 0)
		{
			DataProduceequipManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().fruits_size() > 0)
		{
			DataFruitManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().animals_size() > 0)
		{
			DataAnimalManager::Instance()->FromMessage(uid, req->data());
		}

		//??????
		if (req->data().orders_size() > 0)
		{
			DataOrderManager::Instance()->FromMessage(uid, req->data());
		}

		//??????
		if (req->data().has_truck() )
		{
			DataTruckManager::Instance()->FromMessage(uid, req->data());
		}

		//??????
		if (req->data().has_shipping() )
		{
			DataShippingManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().shipboxes_size() > 0)
		{
			DataShippingboxManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if (req->data().shop_size() > 0)
		{
			DataShopManager::Instance()->FromMessage(uid, req->data());
		}

		//??????
		if (req->data().task_size() > 0)
		{
			DataTaskManager::Instance()->FromMessage(uid, req->data());
		}

		//????????????
		if(req->data().mission_size() > 0)
		{
			DataMissionManager::Instance()->FromMessage(uid,req->data());
		}

		//????????????
		if(req->data().activity_size())
		{
			DataGameActivityManager::Instance()->FromMessage(uid,req->data());
		}

		//?????????
		if(req->data().activity_size())
		{
			DataMailDogManager::Instance()->FromMessage(uid,req->data());
		}

		//?????????
		if(req->data().friendlytree_size())
		{
			DataFriendlyTreeManager::Instance()->FromMessage(uid,req->data());
		}

		//??????????????????
		if(req->data().shopsellcoin_size())
		{
			DataShopSellCoinManager::Instance()->FromMessage(uid,req->data());
		}

		//????????????
		if(req->data().friendworker_size())
		{
			DataFriendWorkerManager::Instance()->FromMessage(uid,req->data());
		}

		//??????
		if(req->data().pet_size())
		{
			DataPetManager::Instance()->FromMessage(uid,req->data());
		}
		//?????????????????????????????????
		SyncSave(uid);
	}

	return 0;
}

int UserManager::Process(unsigned uid, ProtoAssistor::OpenAssistorReq* msg, ProtoAssistor::OpenAssistorResp* resp)
{
	unsigned now = Time::GetGlobalTime();
	DataBase &base = BaseManager::Instance()->Get(uid);
	unsigned &assist_end_ts = base.assist_end_ts;

	if(0 != assist_end_ts && assist_end_ts >= now)
	{
		error_log("assistor has been opened before");
		error_log("[OpenAssistor] uid: %u, start_time: %u, end_time: %u, now: %u",uid, base.assist_start_ts,  base.assist_end_ts, now);
		throw std::runtime_error("assistor_has_been_opened_before");
	}

	const ConfigAssistor::AssistCfg& assistCfg =AssistorCfgWrap().GetAssistorCfg();
	if(msg->grade() >= assistCfg.assistinfo().grades_size())
	{
		error_log("param error:grade: %d, size: %d", msg->grade(), assistCfg.assistinfo().grades_size());
		throw std::runtime_error("param_error");
	}

	const ::ConfigAssistor::Grade& grade = assistCfg.assistinfo().grades(msg->grade());

	unsigned times = grade.times();
	unsigned cash  = grade.cash();

	//?????????????????????
	if(msg->grade() < assistCfg.assistinfo().grades_size() - 1)
	{
		//????????????
		DBCUserBaseWrap user(uid);
		user.CostCash(cash, "open_assistor");
		DataCommon::CommonItemsCPP *common = resp->mutable_commons();
		DataCommon::BaseItemCPP *data = common->mutable_userbase()->add_baseitem();
		data->set_change(-cash);
		data->set_totalvalue(user.GetCash());
		data->set_type(type_cash);
	}

	//base.assist_start_ts = 0;
	assist_end_ts = now + times;
	resp->set_endts(assist_end_ts);
	//??????????????????
	BaseManager::Instance()->UpdateDatabase(base);



	//debug_log("[OpenAssistor] uid: %u, start_time: %u, end_time: %u", uid, base.assist_start_ts,  base.assist_end_ts);
	return 0;
}


int UserManager::Process(unsigned uid, ProtoAssistor::UseAssistorReq* msg, ProtoAssistor::UseAssistorResp* resp)
{
	const ConfigAssistor::AssistCfg& assistCfg =AssistorCfgWrap().GetAssistorCfg();
	unsigned cdTs = assistCfg.assistinfo().cdts();

	unsigned now = Time::GetGlobalTime();
	DataBase &base = BaseManager::Instance()->Get(uid);
	unsigned &assist_start_ts = base.assist_start_ts;
	unsigned &assist_end_ts = base.assist_end_ts;

	if(assist_end_ts == 0)
	{
		error_log("assistor not open");
		throw std::runtime_error("assistor_not_open");
	}
	else
	{
        if(now > assist_end_ts)
        {
    		assist_end_ts = 0;
    		error_log("assistor not open");
    		throw std::runtime_error("assistor_not_open");
        }
        else
        {
            if(now - assist_start_ts > cdTs || 0 == assist_start_ts)
            {
            	assist_start_ts = now;
            	resp->set_startts(assist_start_ts);
            	CommonGiftConfig::CommonModifyItem cfg;
            	CommonGiftConfig::PropsItem*  itemcfg = cfg.add_props();
            	itemcfg->set_count(msg->assistinfo().propscnt());
            	itemcfg->set_id(msg->assistinfo().propsid());
            	LogicUserManager::Instance()->CommonProcess(uid, cfg, "use_assistor", resp->mutable_commons());
            }
            else
            {
        		error_log("dog on rest");
        		throw std::runtime_error("dog_on_rest");
            }
        }
	}

	//debug_log("[UseAssistor] uid: %u, start_time: %u, end_time: %u",uid, base.assist_start_ts,  base.assist_end_ts);
	return 0;
}

bool UserManager::AddVisitDyInfo(unsigned uid,unsigned other_uid)
{
	//uid:?????????,other_uid:????????????,???????????????????????????????????????????????????????????????
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_VISIT,pattach))
	{
		return true;
	}
	return false;
}

bool UserManager::AddVisitDyInfoOverServer(unsigned uid,unsigned other_uid)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_VISIT);
	msg->set_productid(0);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
//????????????????????????
bool UserManager::OnFairySpeedUpOpen(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level()
		&& !userwrap.existBaseFlag(base_falg_id_fairy_speed_up_open))
	{
		userwrap.SetBaseFlag(base_falg_id_fairy_speed_up_open);
		userwrap.SetBaseFlag(base_falg_id_fairy_speed_up_crop);
		SendFlagInfo(uid);
		LogicQueueManager::Instance()->FinishInstant(uid, QUEUE_BUILD_TYPE_CROP);
	}
	return true;
}
bool UserManager::IsFairySpeedUpCrop(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	return userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level() && userwrap.existBaseFlag(base_falg_id_fairy_speed_up_crop);
}
bool UserManager::IsFairySpeedUpEquip(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	return userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level() && userwrap.existBaseFlag(base_falg_id_fairy_speed_up_equip);
}
bool UserManager::IsFairySpeedUpFarm(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	return userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level() && userwrap.existBaseFlag(base_falg_id_fairy_speed_up_farm);
}
float UserManager::GetFairySpeedUpCrop(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level() && userwrap.existBaseFlag(base_falg_id_fairy_speed_up_crop))
	{
		return UserCfgWrap().User().fairyspeedup().croprate();
	}
	return 1;
}
float UserManager::GetFairySpeedUpEquip(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level() && userwrap.existBaseFlag(base_falg_id_fairy_speed_up_equip))
	{
		return UserCfgWrap().User().fairyspeedup().equiprate();
	}
	return 1;
}
float UserManager::GetFairySpeedUpFarm(uint32_t uid)
{
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().level <= UserCfgWrap().User().fairyspeedup().level() && userwrap.existBaseFlag(base_falg_id_fairy_speed_up_farm))
	{
		return UserCfgWrap().User().fairyspeedup().farmrate();
	}
	return 1;
}
void UserManager::SendFlagInfo(uint32_t uid,bool IsNeedCost)
{
	DBCUserBaseWrap userwrap(uid);
	User::GetFlagResp* resp = new User::GetFlagResp;
	if(IsNeedCost)
	{
		LogicUserManager::Instance()->CommonProcess(uid,UserCfgWrap().User().diamondcost().xianren_open_cost(),"xianren_open_cost",resp->mutable_commons());
	}
	resp->set_flag(userwrap.Obj().flag);
	LMI->sendMsg(uid, resp);
}
bool UserManager::SendNewMsgToUser(uint32_t uid, uint32_t type)
{
	User::ReplyNewMsg* resp = new User::ReplyNewMsg;
	resp->add_type(type);
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
bool UserManager::SendNewMsgToAll(uint32_t type)
{
	User::ReplyNewMsg* msg = new User::ReplyNewMsg;
	msg->add_type(type);
	return LMI->broadcastMsg(msg);
}

int UserManager::Process(unsigned uid, User::HeartBeatReq * req, User::HeartBeatResp * resp)
{
	return 0;
}
