#include "LogicAllianceManager.h"
#include <fstream>
LogicAllianceManager::LogicAllianceManager()
{

}

OtherAllianceSaveControl::OtherAllianceSaveControl(unsigned aid):
		aid_(aid)
{
	int ret = LogicAllianceManager::Instance()->LoadAlliance(aid_);
	if (ret)
	{
		throw runtime_error("load_other_alliance_data_error");
	}
	LogicAllianceManager::Instance()->CheckRace(aid_);
}

OtherAllianceSaveControl::~OtherAllianceSaveControl()
{
	LogicAllianceManager::Instance()->SaveAlliance(aid_);
}

int LogicAllianceManager::SaveAlliance(unsigned alliance_id)
{
	return UMI->AllianceSave(alliance_id);
}
int LogicAllianceManager::LoadAlliance(unsigned alliance_id)
{
	if(!IsAllianceId(alliance_id))
		return R_ERR_PARAM;
	if(CMI->IsNeedConnectByAID(alliance_id))
		return R_ERR_PARAM;

	DataAllianceManager::Instance()->IsExist(alliance_id);

	//获取成员商会信息
	int index = DataAllianceManager::Instance()->GetIndex(alliance_id);
	if (-1 == index)
	{
		//商会不存在
		return R_ERR_PARAM;
	}
	//加载成员信息
	DataAllianceMemberManager::Instance()->LoadBuffer(alliance_id);
	//加载其他信息
	DataAllianceApplyManager::Instance()->LoadBuffer(alliance_id);
	DataAllianceDonationManager::Instance()->LoadBuffer(alliance_id);
	DataAllianceNotifyManager::Instance()->LoadBuffer(alliance_id);

	//判断共享内存中是否有该商会信息
	if(!MemoryAllianceManager::Instance()->IsExist(alliance_id))
	{
		unsigned count = DataAllianceMemberManager::Instance()->GetMemberCount(alliance_id);
		DataAlliance & alliance = DataAllianceManager::Instance()->GetData(alliance_id);
		MemoryAllianceManager::Instance()->Add(alliance_id, count, alliance.apply_type, alliance.apply_level_limit, DataAllianceMemberManager::Instance()->GetMemberOnlineNum(alliance_id));

		debug_log("alliance_member_count aid=%u count=%u", alliance_id, count);

	}

	return 0;
}

int LogicAllianceManager::OnInit()
{
	return 0;
}

void LogicAllianceManager::OnRaceSettle()
{
	map<uint32_t, pair<uint32_t, uint32_t> > rank;
	MemoryAllianceRaceGroupManager::Instance()->Rank(rank);

	string path = Config::GetPath("tools/");
	string file = path + "racepoint.txt";
	ofstream fout(file.c_str(), ios_base::out | ios_base::trunc);

//	Json::Value globalRank(Json::arrayValue); // allianceId -> point
	for(map<uint32_t, pair<uint32_t, uint32_t> >::iterator iter = rank.begin(); iter != rank.end(); ++iter)
	{
		uint32_t aid = iter->first;
		uint32_t rankId = iter->second.first;
		uint32_t count = iter->second.second;

		uint32_t len = 0;
		try
		{
			OtherAllianceSaveControl allianceCtl(aid);
			DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);

			alliance.race_rank_id = rankId;
			alliance.race_opoint = alliance.race_point;
			alliance.race_olevel = alliance.race_level;

			if(rankId >= 1 && rankId <= 3 && alliance.race_level > 1)	// 前三名升到上一级
			{
				--alliance.race_level;
			}
			if(rankId >= 12 && rankId <= 15 && alliance.race_level < 5)	// 最后三名降到下一级
			{
				++alliance.race_level;
			}

			memset(alliance.race_order_id, 0, sizeof(alliance.race_order_id));
			memset(alliance.race_order_cd, 0, sizeof(alliance.race_order_cd));
			DataAllianceManager::Instance()->UpdateItem(alliance);
			vector<unsigned> indexs;

			DataAllianceMemberManager::Instance()->GetIndexs(aid, indexs);

			for(size_t i = 0; i < indexs.size(); ++i)
			{
				DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
				if(IsInRace(member.join_ts))	// 参与到本轮竞赛才有奖励
				{
					uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(member.userlevel);
					uint8_t race_grade_reward[DataAllianceMember_race_grade_reward_LENG]; // 等级奖励
					memset(race_grade_reward, 0, sizeof(race_grade_reward));
					if(rankId >= 1 && rankId <= 3)
					{
						AllianceRaceCfgWrap().RefreshGradeReward(levelId, race_grade_reward, 3);
					}
					for(uint32_t j = 0; j < DataAllianceMember_race_grade_reward_LENG; ++j)
					{
						member.race_grade_reward[j].rewardId = race_grade_reward[j];
					}
					if(member.race_point == 0)	// 本轮竞赛没有获得积分不会刷新阶段奖励,结算时要刷新
					{
						member.race_user_level = member.userlevel;
						const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 > fixId;
						RefreshMemberRaceStageReward(fixId, levelId, member);
					}
					member.flag = 0;
					ResetRaceMemberInfo(member);
					DataAllianceMemberManager::Instance()->UpdateItem(member);
				}
			}
			/*
			Json::Value obj;
			obj["v"] = alliance.race_ts;
			obj["aid"] = aid;
			obj["p"] = AllianceRaceCfgWrap().getRankPoint(alliance.race_level, rankId) + ((indexs.size() > 0) ? (alliance.race_point / indexs.size()) : 0);
			globalRank.append(obj);
			*/
			fout << aid << " " << alliance.race_ts << " "
					<< AllianceRaceCfgWrap().getRankPoint(alliance.race_level, rankId) + ((count > 0) ? (alliance.race_point / count) : 0) << '\n';
		}
		catch(runtime_error &e)
		{
			error_log("load alliance fail aid=%u %s", aid, e.what());
		}
	}
	fout << endl;
	/*
	string cmd = "cd " + path + " && nohup ./SaveRankPoint 1 &";
	File::Write(file, Json::ToString(globalRank));
	system(cmd.c_str());
	*/
}
int LogicAllianceManager::Online(unsigned uid)
{
	//成员登录，检查该成员的商会
	DBCUserBaseWrap userwrap(uid);
	LoadAlliance(userwrap.Obj().alliance_id);

	return UpdateMemberNow(uid);
}

int LogicAllianceManager::Offline(unsigned uid)
{
	return UpdateMemberNow(uid);
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAlliance* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceBC* m = new ProtoAlliance::RequestAllianceBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::AllianceCPP* resp = new ProtoAlliance::AllianceCPP;
			FillAlliance(aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	FixSelf(uid);
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceBC* resp = new ProtoAlliance::ReplyAllianceBC;
	resp->set_uid(uid);
	try{
		OtherAllianceSaveControl allianceCtl(aid);
		if (IsAllianceId(aid) && CheckMember(aid, uid))
			FillAlliance(aid, resp->mutable_alliance());
	}
	catch(runtime_error &e){
		delete resp;
		return R_ERROR;
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	FixSelf(req->uid());
	return 0;
}
void LogicAllianceManager::FillAlliance(uint32_t aid, ProtoAlliance::AllianceCPP* resp)
{
	CheckRace(aid);
	DataAllianceManager::Instance()->SetMessage1(aid, resp);
}
void LogicAllianceManager::FixSelf(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);

	DataCommon::CommonItemsCPP * msg = new DataCommon::CommonItemsCPP;
	SetUserAllianceMsg(userwrap.Obj().alliance_id, 0, msg);

	userwrap.Obj().alliance_id = 0;
	userwrap.Save();

	LMI->sendMsg(uid, msg);
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::GetAllianceFunctionReq* req)
{
	unsigned type = req->type();

	GetAllianceFunc(uid, type);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceFunctionBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	unsigned type = req->type();
	ProtoAlliance::ReplyAllianceFunctionBC* resp = new ProtoAlliance::ReplyAllianceFunctionBC;
	resp->set_uid(uid);
	resp->set_type(type);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		GetAllianceFuncLocal(aid, uid,  type, resp->mutable_alliance());
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceFunctionBC* req)
{
	unsigned type = req->type();
	if (functional_type_invite == type || functional_type_all == type)
	{
		int invitemax = 10;
		//邀请列表
		DataInvitedListManager::Instance()->FullMessage(req->uid(), invitemax, req->mutable_alliance()->mutable_invitelist());
	}
	return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
}
int LogicAllianceManager::GetAllianceFunc(unsigned uid, unsigned type)
{
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0 && type != functional_type_invite)
	{
		error_log("not have alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	unsigned aid = userwrap.Obj().alliance_id;
	ProtoAlliance::GetAllianceFunctionResp * resp = NULL;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceFunctionBC* m = new ProtoAlliance::RequestAllianceFunctionBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_type(type);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			resp = new ProtoAlliance::GetAllianceFunctionResp;
			GetAllianceFuncLocal(aid, uid,  type, resp);
		}
	}
	if(resp == NULL)
	{
		resp = new ProtoAlliance::GetAllianceFunctionResp;
	}
	if (functional_type_invite == type || functional_type_all == type)
	{
		int invitemax = 10;
		//邀请列表
		DataInvitedListManager::Instance()->FullMessage(uid, invitemax, resp->mutable_invitelist());
	}
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
int LogicAllianceManager::GetAllianceFuncLocal(unsigned alliance_id, unsigned uid,  unsigned type, ProtoAlliance::GetAllianceFunctionResp * resp)
{

	OtherAllianceSaveControl allianceCtl(alliance_id);
	//获取全部时，入会申请最多30条
	int maxcount = (type == functional_type_all ? 30 :99);

	if (functional_type_apply == type || functional_type_all == type)
	{
		//入会申请
		DataAllianceApplyManager::Instance()->FullMessage(alliance_id, maxcount, resp->mutable_applylist());
	}

	//获取全部时，捐收申请最多30条
	if (functional_type_donation == type || functional_type_all == type)
	{
		//捐收
		DataAllianceDonationManager::Instance()->FullMessage(alliance_id, uid, maxcount, resp->mutable_donations());
	}

	if (functional_type_help == type || functional_type_all == type)
	{
		//帮助列表
		//遍历群成员?
		vector<unsigned> indexs;
		DataAllianceMemberManager::Instance()->GetIndexs(alliance_id, indexs);
		int count = 0;

		for(int i = 0; i < indexs.size(); ++i)
		{
			DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);

			if (member.id == uid)
			{
				continue;
			}

			unsigned othuid = member.id;

				//判断是否可援助
				//bool isneedhelp = LogicUserManager::Instance()->IsUserNeedHelp(othuid);

			if (member.helpTs)
			{
				ProtoAlliance::AidInfoCPP* aidmsg = resp->add_aidlist();

				aidmsg->set_uid(othuid);
				aidmsg->set_name(member.username);

				++count;

				//超过最大条数，则不再添加数据
				if (count >= maxcount)
				{
					break;
				}
			}
		}
	}

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::GetNotifyReq* req)
{
	GetAllianceNotify(uid);

	return 0;
}

int LogicAllianceManager::GetAllianceNotify(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("not have alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceNotifyBC* m = new ProtoAlliance::RequestAllianceNotifyBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::GetNotifyResp* resp = new ProtoAlliance::GetNotifyResp;
			//商会通知
			DataAllianceNotifyManager::Instance()->FullMessage(aid, resp->mutable_notifies());
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}

	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceNotifyBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceNotifyBC* resp = new ProtoAlliance::ReplyAllianceNotifyBC;
	resp->set_uid(uid);

	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		DataAllianceNotifyManager::Instance()->FullMessage(aid, resp->mutable_alliance()->mutable_notifies());
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceNotifyBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::GetMemberReq* req)
{
	unsigned aid = req->allianceid();

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceMemberBC* m = new ProtoAlliance::RequestAllianceMemberBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else
		{
			ProtoAlliance::GetMemberResp* resp = new ProtoAlliance::GetMemberResp;
			GetMembers(uid, aid, resp);
			resp->set_allianceid(aid);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}

	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceMemberBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceMemberBC* resp = new ProtoAlliance::ReplyAllianceMemberBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid))
	{
		GetMembers(uid, aid, resp->mutable_alliance());
		resp->mutable_alliance()->set_allianceid(aid);
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceMemberBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}

int LogicAllianceManager::GetMembers(unsigned uid, unsigned alliance_id, ProtoAlliance::GetMemberResp *resp)
{
	if (0 == alliance_id)
	{
		error_log("alliance not exist. uid=%u,alliance_id=%u", uid, alliance_id);
		throw runtime_error("alliance_not_exist");
	}

	OtherAllianceSaveControl allianceCtl(alliance_id);
	//判断商会是否存在
	if (!DataAllianceManager::Instance()->IsExist(alliance_id))
	{
		error_log("alliance not exist. uid=%u,alliance_id=%u", uid, alliance_id);
		throw runtime_error("alliance_not_exist");
	}

	FullMessage(alliance_id, resp->mutable_members());

	resp->set_allianceid(alliance_id);

	return 0;
}

int LogicAllianceManager::FullMessage(unsigned alliance_id, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceMemberCPP >* msg)
{
	vector<unsigned> indexs;

	DataAllianceMemberManager::Instance()->GetIndexs(alliance_id, indexs);

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
		SetMemberMessage(member, msg->Add());
	}

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::CheckNameAvailableReq* req, ProtoAlliance::CheckNameAvailableResp* resp)
{
	string name = req->name();

	CheckName(uid, name, resp);

	return 0;
}

int LogicAllianceManager::CheckName(unsigned uid, string &name, ProtoAlliance::CheckNameAvailableResp * resp)
{
	string reason;
	bool isavail = IsNameAvailable(uid, name, reason);

	resp->set_name(name);
	resp->set_isavailable(isavail);

	if (!isavail)
	{
		//不可用，填写原因
		resp->set_reason(reason);
	}

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::CreateAllianceReq* req, ProtoAlliance::CreateAllianceResp* resp)
{
	CreateAlliance(uid, req, resp);

	return 0;
}

int LogicAllianceManager::CreateAlliance(unsigned uid, ProtoAlliance::CreateAllianceReq * req, ProtoAlliance::CreateAllianceResp * resp)
{
	string reason;
	string name = req->name();

	bool isavail = IsNameAvailable(uid, name, reason);

	if (!isavail)
	{
		throw runtime_error("name_check_error");
	}

	//判断用户是否已有商会
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id > 0)
	{
		error_log("user have alliance. uid=%u", uid);
		throw runtime_error("already_in_alliance");
	}

	//获取商会配置
	const ConfigAlliance::Alliance & alliancecfg = AllianceCfgWrap().GetAllianceCfg();

	if (userwrap.Obj().level < alliancecfg.create_level())
	{
		error_log("user level not enough. uid=%u", uid);
		throw runtime_error("level_not_enough");
	}

	//参数校验
	unsigned atype = req->applytype();

	if (join_type_anyone != atype && join_type_apply != atype && join_type_invite != atype)
	{
		error_log("applytype param error. uid=%u,applytype=%u", uid, atype);
		throw runtime_error("param_error");
	}

	unsigned levellimit = req->applylevellimit();

	if (levellimit < 10 || levellimit > 100)
	{
		error_log("applylevellimit param error. uid=%u,applylevellimit=%u", uid, levellimit);
		throw runtime_error("param_error");
	}

	string description = req->description();
	//敏感词过滤
	SensitiveFilter(description);

	//扣除建造商会的费用
	LogicUserManager::Instance()->CommonProcess(uid, alliancecfg.create_cost(), "AllianceCreate", resp->mutable_commons());

	//获取商会的id
	uint64_t allianceid;
	int serverid = Config::GetZoneByUID(uid);
	CLogicIdCtrl::Instance()->GetNextId(KEY_ALLIANCE_ID_CTRL, allianceid, serverid);

	DataAllianceMapping tempalliance;
	//建立商会,直接操作dbc
	tempalliance.Clear();
	snprintf(tempalliance.alliance_name, sizeof(tempalliance.alliance_name), "%s", name.c_str());
	tempalliance.alliance_id = allianceid;

	allianceMapping.Add(tempalliance);

	//创建商会
	DataAlliance alliance;
	alliance.alliance_id = allianceid;
	alliance.create_uid = uid;
	snprintf(alliance.name, sizeof(alliance.name), "%s", name.c_str());
	alliance.apply_level_limit = levellimit;
	alliance.flag = req->flag();
	alliance.apply_type = atype;
	alliance.race_level = 5;

	alliance.active_ts = Time::GetGlobalTime();
	alliance.create_time = Time::GetGlobalTime();
	snprintf(alliance.description, sizeof(alliance.description), "%s", description.c_str());
	snprintf(alliance.create_username, sizeof(alliance.create_username), "%s", userwrap.Obj().name);

	DataAllianceMember member;
	FillMember(uid, allianceid, pos_type_chief, userwrap, member);


	/*
	if(true)	// 测试代码
	{
		alliance.race_point = 2100;
		alliance.race_ts = MemoryAllianceRaceGroupManager::Instance()->GetTs();
		alliance.race_rank_id = Math::GetRandomInt(3) + 1;
		for(uint32_t i = 0; i < DataAlliance_race_order_id_LENG; ++i)
		{
			if((alliance.race_order_id[i] = AllianceRaceCfgWrap().GetRandTaskId(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID)) > 0)
			{
				alliance.race_order_cd[i] = 0; //now + AllianceRaceCfgWrap().GetCfg().task().cdtime();
			}
		}
		uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(member.userlevel);
		uint8_t race_grade_reward[DataAllianceMember_race_grade_reward_LENG]; // 阶段奖励
		memset(race_grade_reward, 0, sizeof(race_grade_reward));
		AllianceRaceCfgWrap().RefreshGradeReward(levelId, race_grade_reward, 3);
		for(uint32_t j = 0; j < DataAllianceMember_race_grade_reward_LENG; ++j)
		{
			member.race_grade_reward[j].rewardId = race_grade_reward[j];
			member.race_user_level = member.userlevel;
			member.race_point = 0;
		}
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 > fixId;
		RefreshMemberRaceStageReward(fixId, levelId, member);
	}
	*/


	debug_log("alliance_member_count aid=%u count=%u", allianceid, 0);
	MemoryAllianceManager::Instance()->Add(allianceid, 0, alliance.apply_type, alliance.apply_level_limit, 1);
	//创建商会
	DataAllianceManager::Instance()->AddNewItem(alliance);
	DataAllianceManager::Instance()->SetMessage(allianceid, resp);
	//商会添加会长
	AddNewMember(member, resp->mutable_member());


	//更新用户档中的商会id
	userwrap.Obj().alliance_id = allianceid;
	userwrap.Save();

	SetUserAllianceMsg(0, allianceid, resp->mutable_commons());

	LogicManager::Instance()->DoDataManagerSave(uid);
	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RecommendllianceReq* req, ProtoAlliance::RecommendllianceResp* resp)
{
	RecommendAlliance(uid, resp);

	return 0;
}

int LogicAllianceManager::RecommendAlliance(unsigned uid, ProtoAlliance::RecommendllianceResp * resp)
{
	DBCUserBaseWrap userwrap(uid);
	//使用推荐的100个公会
	vector<unsigned> alliances;

	const ConfigAlliance::Alliance & alliancecfg = AllianceCfgWrap().GetAllianceCfg();
	MemoryAllianceManager::Instance()->GetRecommendAlliances(userwrap.Obj().alliance_id, userwrap.Obj().level, alliancecfg.member_num_limit(), LogicAllianceManager::join_type_anyone, alliances);

	for(int i = 0; i < alliances.size(); ++i)
	{
		//获取商会id
		unsigned alliance_id = alliances[i];

		//判断商会是否真的存在
		if (!DataAllianceManager::Instance()->IsExist(alliance_id))
		{
			//商会不存在，则通过调用GetIndex判断是否真的不存在
			if (unsigned(-1) == DataAllianceManager::Instance()->GetIndex(alliance_id))
			{
				MemoryAllianceManager::Instance()->DelAlliance(alliance_id);
				error_log("alliance not exist. uid=%u,alliance_id=%u", uid, alliance_id);
				continue;
//				throw runtime_error("alliance_not_exist");
			}
		}

		//商会存在，则获取商会信息
		DataAlliance & alliance = DataAllianceManager::Instance()->GetData(alliance_id);

		ProtoAlliance::PartAllianceCPP* partmsg = resp->add_alliancebrief();
		alliance.SetMessage(partmsg);
		partmsg->set_onlinenum(MemoryAllianceManager::Instance()->GetOnlineNum(alliance_id));

		unsigned index = MemoryAllianceManager::Instance()->GetIndex(alliance_id);
		OfflineAllianceItem & item = MemoryAllianceManager::Instance()->GetAllianceItemByIndex(index);

		partmsg->set_membercount(item.memcount);
	}

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::GetPartAllianceInfoReq* req, ProtoAlliance::GetPartAllianceInfoResp* resp)
{
	vector<unsigned> allianceids;

	for(int i = 0; i < req->allianceid_size(); ++i)
	{
		if(!CMI->IsNeedConnectByAID(req->allianceid(i)))
		{
			allianceids.push_back(req->allianceid(i));
		}
	}

	GetBatchAllianceInfo(uid, allianceids, resp);

	return 0;
}

int LogicAllianceManager::GetBatchAllianceInfo(unsigned uid, vector<unsigned> & allianceids, ProtoAlliance::GetPartAllianceInfoResp * resp)
{
	for(int i = 0; i < allianceids.size(); ++i)
	{
		//获取商会id
		unsigned alliance_id = allianceids[i];

		//判断商会是否真的存在
		if (!DataAllianceManager::Instance()->IsExist(alliance_id))
		{
			continue;
			/*
			//商会不存在，则通过调用GetIndex判断是否真的不存在
			if (unsigned(-1) == DataAllianceManager::Instance()->GetIndex(alliance_id))
			{
				//有可能取到别的区的商会，所以不做抛异常处理
				error_log("alliance not exist. uid=%u,alliance_id=%u", uid, alliance_id);
				throw runtime_error("alliance_not_exist");
			}

			//存在，则加载成员信息，因为要用到
			DataAllianceMemberManager::Instance()->LoadBuffer(alliance_id);
			*/
		}

		//商会存在，则获取商会信息
		DataAlliance & alliance = DataAllianceManager::Instance()->GetData(alliance_id);

		ProtoAlliance::PartAllianceCPP* partmsg = resp->add_alliancebrief();
		alliance.SetMessage(partmsg);

		int count = DataAllianceMemberManager::Instance()->GetMemberCount(alliance_id);
		partmsg->set_membercount(count);
	}

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::ApplyJoinReq* req)
{
	unsigned alliance_id = req->allianceid();
	string reason = req->reason();

	ApplyJoin(uid, alliance_id, reason);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestApplyJoinBC* req)
{
	unsigned aid = req->allianceid();
	unsigned uid = req->member().memberuid();

	ProtoAlliance::ReplyApplyJoinBC* resp = new ProtoAlliance::ReplyApplyJoinBC;
	resp->set_uid(uid);
	resp->set_allianceid(aid);

	int ret = 0;
	if (IsAllianceId(aid))
	{
		/*
		DBCUserBaseWrap userwrap(uid);

		if (userwrap.Obj().alliance_id > 0)
		{
			error_log("user have alliance. uid=%u", uid);
			throw runtime_error("already_in_alliance");
		}
		userwrap.Obj().alliance_id = aid;
		userwrap.Save();
		*/
		DataAllianceMember member;
		member.SetData(aid, req->mutable_member());
		ret = ApplyJoinLocal(member, req->reason(), resp->mutable_alliance());
	}
	resp->set_ret(ret);
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyApplyJoinBC* req)
{
	if(req->has_alliance())
	{
		int ret = req->ret();
		unsigned uid = req->uid();
		unsigned aid = req->allianceid();
		DBCUserBaseWrap userwrap(uid);
		if(ret != apply_join_allow && aid == userwrap.Obj().alliance_id)	// 不能直接加入需要重置工会ID
		{
			userwrap.Obj().alliance_id = 0;
			userwrap.Save();
		}
		req->mutable_alliance()->set_ret(ret);
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	}
	return 0;
}
int LogicAllianceManager::ApplyJoin(unsigned uid, unsigned aid, string & reason)
{
	//敏感词过滤
	SensitiveFilter(reason);

	//判断当前用户是否已有商会
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id > 0)
	{
		error_log("user have alliance. uid=%u", uid);
		throw runtime_error("already_in_alliance");
	}

	//判断当前用户是否处于禁止申请期间
	if (userwrap.Obj().allian_allow_ts > Time::GetGlobalTime())
	{
		error_log("user is in forbidden apply time. uid=%u", uid);
		throw runtime_error("forbidden_apply_time");
	}

	userwrap.Obj().alliance_id = aid;

	//既然时间已经达到了，则置0
	userwrap.Obj().allian_allow_ts = 0;
	userwrap.Save();

	if (IsAllianceId(aid))
	{
		DataAllianceMember member;
		FillMember(uid, aid, pos_type_member, userwrap, member);
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestApplyJoinBC* m = new ProtoAlliance::RequestApplyJoinBC;
			m->set_allianceid(aid);
			m->set_reason(reason);
			member.SetMessage(m->mutable_member());
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else
		{
			ProtoAlliance::ApplyJoinResp* resp = new ProtoAlliance::ApplyJoinResp;
			int ret = ApplyJoinLocal(member, reason, resp);
			if(ret != apply_join_allow)
			{
				userwrap.Obj().alliance_id = 0;
			}
			resp->set_ret(ret);
			userwrap.Save();
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}

	return 0;
}

int LogicAllianceManager::ApplyJoinLocal(DataAllianceMember& member, const string & reason,	ProtoAlliance::ApplyJoinResp * resp)
{
	uint32_t aid = member.alliance_id;
	uint32_t uid = member.id;
	uint32_t userLevel = member.userlevel;

	OtherAllianceSaveControl allianceCtl(aid);

	//判断商会是否存在
	if (!DataAllianceManager::Instance()->IsExist(aid))
	{
		error_log("alliance not exist. uid=%u,alliance_id=%u", uid, aid);
//		throw runtime_error("alliance_not_exist");
		return apply_join_nonexist;
	}

	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);

	//判断是否满足该商会的入会条件
	if (alliance.apply_type == join_type_invite)
	{
		error_log("alliance only support invite. uid=%u,alliance_id=%u", uid, aid);
//		throw runtime_error("invite_only");
		return apply_join_invite_only;
	}

	//判断当前玩家等级是否满足入会要求
	if (alliance.apply_level_limit > userLevel)
	{
		error_log("user level not enough. uid=%u, alliance_id=%u", uid, aid);
//		throw runtime_error("level_not_enough");
		return apply_join_level_limit;
	}

	//判断商会是否已满
	if(!CheckMemberFull(uid, aid))
	{
		error_log("CheckMemberFull fail uid=%u, alliance_id=%u", uid, aid);
		return apply_join_member_full;
	}

	//如果商会的类型是任何人，那么可以直接成为商会成员了
	if (join_type_anyone == alliance.apply_type)
	{
		//添加新成员
		AddNewMember(member, resp->mutable_member());

		SetUserAllianceMsg(0, aid, resp->mutable_commons());

		//设置商会信息
		DataAllianceManager::Instance()->SetMessage(aid, resp);

		return apply_join_allow;
	}
	else
	{
		//判断是否已申请
		if (DataAllianceApplyManager::Instance()->IsExistItem(aid, uid))
		{
			error_log("already applied. uid=%u, alliance_id=%u", uid, aid);
			return apply_join_already_apply;
		}

		//商会添加入会申请
		DataAllianceApply & apply = DataAllianceApplyManager::Instance()->GetData(aid, uid);
		apply.applyts = Time::GetGlobalTime();

		snprintf(apply.reason, sizeof(apply.reason), "%s", reason.c_str());
		snprintf(apply.username, sizeof(apply.username), "%s", member.username);

		DataAllianceApplyManager::Instance()->UpdateItem(apply);

		DataAllianceApplyManager::Instance()->SetMessage(apply, resp->mutable_apply());

		return apply_join_apply;
	}

	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::ApproveJoinReq* req)
{
	unsigned applyuid = req->applyuid();
	unsigned operate = req->operate();

	ApproveJoin(uid, applyuid, operate);

	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestApproveJoinAllianceBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		DataAllianceMember member;
		member.SetData(aid, req->mutable_member());
		ProtoAlliance::ApproveJoinResp* resp = new ProtoAlliance::ApproveJoinResp;
		ApproveJoinUpdateAlliance(uid, req->operate(), member, resp);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestApproveJoinUserBC* req)
{
	unsigned uid = req->uid();
	return ApproveJoinUpdateUser(uid, req->aid(), req->applyuid(), req->operate());
}

int LogicAllianceManager::Process(ProtoAlliance::ReplyApproveJoinBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::ApproveJoin(unsigned uid, unsigned apply_uid, unsigned operate)
{
	//权限校验
	//判断是否有该用户的邀请
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (aid == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	//参数验证
	if (approve_operate_pass != operate && approve_operate_reject != operate)
	{
		error_log("operate param error. uid=%u,operate=%u", uid, operate);
		throw runtime_error("param_error");
	}

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByUID(apply_uid))
		{
			ProtoAlliance::RequestApproveJoinUserBC* m = new ProtoAlliance::RequestApproveJoinUserBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_applyuid(apply_uid);
			m->set_operate(operate);
			return BMI->BattleConnectNoReplyByAID(apply_uid, m);
		}
		else
		{
			ApproveJoinUpdateUser(uid, aid, apply_uid, operate);
		}
	}

	return 0;
}
int LogicAllianceManager::ApproveJoinUpdateAlliance(unsigned uid, unsigned operate, DataAllianceMember& member, ProtoAlliance::ApproveJoinResp * resp)
{
	unsigned aid = member.alliance_id;
	unsigned apply_uid = member.id;

	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_approve);
	if(!DataAllianceApplyManager::Instance()->IsExistItem(aid, apply_uid))
	{
		error_log("user not in apply list. uid=%u,apply_uid=%u", uid, apply_uid);
		throw runtime_error("user_not_apply");
	}
	resp->set_applyuid(apply_uid);
	resp->set_operate(operate);
	//如果是拒绝，则不处理申请用户的相关信息
	if (approve_operate_reject == operate)
	{
		//拒绝用户通过
		//删除申请记录
		DataAllianceApplyManager::Instance()->DelItem(aid, apply_uid);
		return 0;
	}
	//删除该玩家的申请记录
	DataAllianceApplyManager::Instance()->DelItem(aid, apply_uid);
	//添加新成员
	AddNewMember(member, resp->mutable_member());

	//对方的职位变化通知
	NotifyPositionChange(apply_uid, pos_type_none, pos_type_member, aid);
	return 0;
}
int LogicAllianceManager::ApproveJoinUpdateUser(unsigned uid, unsigned aid, unsigned apply_uid, unsigned operate)
{
	//获取申请玩家的数据
	OffUserSaveControl offuserctl(apply_uid);
	DBCUserBaseWrap othuserwrap(apply_uid);

	if (othuserwrap.Obj().alliance_id > 0)	// 玩家已经加入其它联盟则拒绝玩家
	{
		operate = approve_operate_reject;
	}

	if (approve_operate_reject != operate)
	{
		//更新对方的商会id
		othuserwrap.Obj().alliance_id = aid;
		othuserwrap.Save();
	}

	if (IsAllianceId(aid))
	{
		DataAllianceMember member;
		FillMember(apply_uid, aid, pos_type_member, othuserwrap, member);
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestApproveJoinAllianceBC* m = new ProtoAlliance::RequestApproveJoinAllianceBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_operate(operate);
			member.SetMessage(m->mutable_member());
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ApproveJoinResp *resp = new ProtoAlliance::ApproveJoinResp;
			ApproveJoinUpdateAlliance(uid, operate, member, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::InviteJoinReq* req)
{
	unsigned inviteuid = req->inviteduid();

	InviteJoin(uid, inviteuid);

	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestInviteJoinBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyInviteJoinBC resp;
	resp.set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		InviteJoinUpdateAlliance(uid, aid, req->inviteduid(), req->name());
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyInviteJoinBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestInviteJoinUserBC* req)
{
	unsigned uid = req->uid();
	ProtoAlliance::ReplyInviteJoinBC* resp = new ProtoAlliance::ReplyInviteJoinBC;
	resp->set_uid(uid);
	InviteJoinUpdateUser(uid, req->aid(), req->inviteduid(), req->allianceflag(), req->name(), req->alliancename(), resp->mutable_alliance());
	return BMI->BattleConnectNoReplyByAID(uid, resp);
}

int LogicAllianceManager::InviteJoin(unsigned uid, unsigned invited_uid)
{
	//权限校验
//	CheckPrivilege(uid, privilege_invite);

	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (aid == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestInviteJoinBC* m = new ProtoAlliance::RequestInviteJoinBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_inviteduid(invited_uid);
			m->set_name(userwrap.Obj().name);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			InviteJoinUpdateAlliance(uid, aid, invited_uid, userwrap.Obj().name);
		}
	}


	return 0;
}
int LogicAllianceManager::InviteJoinUpdateAlliance(unsigned uid, unsigned aid, unsigned invited_uid, const string& userName)
{

	OtherAllianceSaveControl allianceCtl(aid);


	CheckMemberPrivilege(uid, aid, privilege_invite);
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);

	if(CMI->IsNeedConnectByUID(invited_uid))
	{
		ProtoAlliance::RequestInviteJoinUserBC* m = new ProtoAlliance::RequestInviteJoinUserBC;
		m->set_aid(aid);
		m->set_uid(uid);
		m->set_inviteduid(invited_uid);
		m->set_allianceflag(alliance.flag);
		m->set_name(userName);
		m->set_alliancename(alliance.name);
		AddInviteDyInfoOverServer(uid,invited_uid,aid);		//邀请跨服 添加动态
		return BMI->BattleConnectNoReplyByUID(invited_uid, m);
	}
	else
	{
		ProtoAlliance::InviteJoinResp *resp = new ProtoAlliance::InviteJoinResp;
		InviteJoinUpdateUser(uid, aid, invited_uid, alliance.flag, userName.c_str(), alliance.name, resp);
		AddInviteDyInfo(uid,invited_uid,aid);				//添加动态
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}
	return 0;
}
int LogicAllianceManager::InviteJoinUpdateUser(unsigned uid, unsigned aid, unsigned invited_uid, uint8_t aflag, const string& inviteName, const string& aname, ProtoAlliance::InviteJoinResp * resp)
{

	//先加载被邀请人的数据
	OffUserSaveControl offuserctl(invited_uid);

	//判断是否已有该公会的邀请数据
	if (DataInvitedListManager::Instance()->IsExistItem(invited_uid, aid))
	{
		error_log("alliance already sent invitation. uid=%u,invited_uid=%u", uid, invited_uid);
		throw runtime_error("alliance_already_sent_invitation");
	}

	//未发出，则判断邀请数目是否已达最大
	vector<unsigned> aids;
	DataInvitedListManager::Instance()->GetIds(invited_uid, aids);

	if (aids.size() >= DB_INVITED_LIST_FULL)
	{
		error_log("invitation is max. uid=%u,invited_uid=%u", uid, invited_uid);
		throw runtime_error("invitation_max");
	}


	//发出邀请
	DataInvitedList & invited = DataInvitedListManager::Instance()->GetData(invited_uid, aid);
	invited.invite_uid = uid;
	invited.flag = aflag;
	invited.invitets = Time::GetGlobalTime();

	snprintf(invited.alliance_name, sizeof(invited.alliance_name), aname.c_str());
	snprintf(invited.invite_name, sizeof(invited.invite_name), inviteName.c_str());

	DataInvitedListManager::Instance()->UpdateItem(invited);

	resp->set_inviteduid(invited_uid);

	//判断被邀请的人是否在线，如果在线，则发送通知
	if (UserManager::Instance()->IsOnline(invited_uid))
	{
		ProtoAlliance::InvitedPushReq * pushmsg = new ProtoAlliance::InvitedPushReq;
		DataInvitedListManager::Instance()->SetMessage(invited_uid, aid, pushmsg->mutable_invite());

		//推送
		LogicManager::Instance()->sendMsg(invited_uid, pushmsg);
	}

	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::AcceptInviteReq* req)
{
	unsigned allianceid = req->allianceid();
	unsigned operate = req->operate();

	AcceptInvite(uid, allianceid, operate);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestAcceptInviteBC* req)
{
	unsigned aid = req->allianceid();
	unsigned uid = req->member().memberuid();
	ProtoAlliance::ReplyAcceptInviteBC* resp = new ProtoAlliance::ReplyAcceptInviteBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid))
	{
		resp->mutable_alliance()->set_operate(req->operate());
		resp->mutable_alliance()->set_allianceid(aid);
		DataAllianceMember member;
		member.SetData(aid, req->mutable_member());
		SetUserAllianceMsg(0, aid, resp->mutable_alliance()->mutable_commons());
		AcceptInviteUpdateAlliance(req->inviteuid(), member, resp->mutable_alliance());
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyAcceptInviteBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::AcceptInvite(unsigned uid, unsigned aid, unsigned operate)
{
	//判断是否存在该商会的邀请
	if (!DataInvitedListManager::Instance()->IsExistItem(uid, aid))
	{
		error_log("alliance has no invitation. uid=%u,alliance_id=%u", uid, aid);
		throw runtime_error("alliance_has_no_invitation");
	}

	//参数验证
	if (accept_operate_yes != operate && accept_operate_no != operate)
	{
		error_log("operate param error. uid=%u,operate=%u", uid, operate);
		throw runtime_error("param_error");
	}


	//如果是拒绝，则直接删除该邀请即可
	if (accept_operate_no == operate)
	{
		//拒绝，则删除该商会的邀请数据
		DataInvitedListManager::Instance()->DelItem(uid, aid);

		ProtoAlliance::AcceptInviteResp* resp = new ProtoAlliance::AcceptInviteResp;
		resp->set_operate(operate);
		resp->set_allianceid(aid);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}

	//判断自身是否已有商会
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id > 0)
	{
		error_log("user have alliance. uid=%u", uid);
		throw runtime_error("already_in_alliance");
	}
	//等级限制
	if (userwrap.Obj().level < 10)
	{
		error_log("level limit uid=%u,operate=%u,level=%u", uid, operate, userwrap.Obj().level);
		throw runtime_error("alliance_invite_level_limit_10");
	}

	DataInvitedList invited = DataInvitedListManager::Instance()->GetData(uid, aid);
	//删除该邀请记录
	DataInvitedListManager::Instance()->DelItem(uid, aid);

	//更新用户的商会id
	userwrap.Obj().alliance_id = aid;
	userwrap.Save();

	if (IsAllianceId(aid))
	{
		ProtoAlliance::AcceptInviteResp* resp = new ProtoAlliance::AcceptInviteResp;
		resp->set_operate(operate);
		resp->set_allianceid(aid);

		DataAllianceMember member;
		FillMember(uid, aid, pos_type_member, userwrap, member);
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAcceptInviteBC* m = new ProtoAlliance::RequestAcceptInviteBC;
			member.SetMessage(m->mutable_member());
			m->set_allianceid(aid);
			m->set_inviteuid(invited.invite_uid);
			m->set_operate(operate);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else
		{
			//resp->set_allianceid(aid);
			SetUserAllianceMsg(0, userwrap.Obj().alliance_id, resp->mutable_commons());
			AcceptInviteUpdateAlliance(invited.invite_uid, member, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}

int LogicAllianceManager::AcceptInviteUpdateAlliance(unsigned invite_uid, DataAllianceMember& member, ProtoAlliance::AcceptInviteResp * resp)
{
	unsigned uid = member.id;
	unsigned aid = member.alliance_id;

	OtherAllianceSaveControl allianceCtl(aid);


	//接受邀请，那么则判断以下条件
	//1.判断发出邀请的用户是否还在该商会
	if (!DataAllianceMemberManager::Instance()->IsExistItem(aid, invite_uid))
	{
		error_log("inviter not in this alliance. uid=%u,alliance_id=%u,invite_uid=%u", uid, aid, invite_uid);
		throw runtime_error("inviter_not_in_this_alliance");
	}

	//2.判断发出邀请的用户是否有权限发出邀请
	DataAllianceMember & inviteMember = DataAllianceMemberManager::Instance()->GetData(aid, invite_uid);

	if (!IsPrivilege(inviteMember.authority, privilege_invite))
	{
		error_log("inviter privilege not enough. uid=%u,invite_uid=%u", uid, invite_uid);
		throw runtime_error("inviter_privilege_not_enough");
	}

	//添加新成员
	AddNewMember(member, resp->mutable_member());

	//设置商会的信息
	DataAllianceManager::Instance()->SetMessage(aid, resp);

	return 0;
}

int LogicAllianceManager::AddNewMember(DataAllianceMember& member, ProtoAlliance::AllianceMemberCPP * msg)
{
	unsigned uid = member.id;
	unsigned alliance_id = member.alliance_id;
	//判断商会是否已满
	IsMemberFull(uid, alliance_id);

	//添加新成员
	/*
	SetMemberByPostion(alliance_id, uid, pos,
			helptimes, level, name,
			online, needHelp, fig,
			addmember);
	*/
	DataAllianceMemberManager::Instance()->AddNewItem(member);

	SetMemberMessage(member, msg);

	debug_log("alliance_member_count aid=%u count=%d", alliance_id, 1);
	//更新共享内存中商会成员个数
	MemoryAllianceManager::Instance()->UpdateMemberCount(alliance_id, 1);

	//debug_log("add new member. uid=%u, alliance_id, position=%u", uid, alliance_id, pos);

	return 0;
}

int LogicAllianceManager::FillMember(unsigned uid, unsigned aid, unsigned pos, DBCUserBaseWrap& userwrap, DataAllianceMember& member)
{
	member.alliance_id = aid;  //商会id
	member.id = uid;  //用户id
	member.position = pos;
	member.authority = GetAuthorityByPosition(member.position);  //权限
	member.helptimes = userwrap.Obj().helptimes; //帮助次数
	member.userlevel = userwrap.Obj().level;  //用户等级

	member.helpTs = LogicUserManager::Instance()->IsUserNeedHelp(uid) ? Time::GetGlobalTime() : 0;
	member.onlineTs = UserManager::Instance()->IsOnline(uid) ? Time::GetGlobalTime() : 0;
	member.join_ts = Time::GetGlobalTime();
	snprintf(member.username, sizeof(member.username), userwrap.Obj().name);
	snprintf(member.fig, sizeof(member.fig), userwrap.Obj().fig);
	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::ManipulateMemberReq* req)
{
//	unsigned memberuid = req->memberuid();
//	unsigned operate = req->operate();
//	unsigned type = req->type();
//	unsigned dest = req->destination();

	ManipulateMember(uid, req);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestManipulateMemberBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyManipulateMemberBC* resp = new ProtoAlliance::ReplyManipulateMemberBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		ManipulateMemberLocal(uid, aid, req->info().memberuid(), req->info().operate(), req->info().type(), req->info().destination(), resp->mutable_alliance());
	}

	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyManipulateMemberBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::ManipulateMember(unsigned uid, ProtoAlliance::ManipulateMemberReq* req)
{
	unsigned memberuid = req->memberuid();
	unsigned operate = req->operate();
	unsigned type = req->type();
	unsigned dest = req->destination();
	//参数校验
	if (operate != operate_promotion && operate != operate_demotion)
	{
		error_log("operator param error. uid=%u,operate=%u", uid, operate);
		throw runtime_error("param_error");
	}

	if (type != manipute_type_committee && type != manipute_type_director)
	{
		error_log("type param error. uid=%u,type=%u", uid, type);
		throw runtime_error("param_error");
	}

	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (aid == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestManipulateMemberBC* m = new ProtoAlliance::RequestManipulateMemberBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->mutable_info()->CopyFrom(*req);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ManipulateMemberResp* resp = new ProtoAlliance::ManipulateMemberResp;
			ManipulateMemberLocal(uid, aid, memberuid, operate, type, dest, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}

int LogicAllianceManager::ManipulateMemberLocal(unsigned uid, unsigned aid, unsigned member_uid, unsigned operate, unsigned type, unsigned dest
		,ProtoAlliance::ManipulateMemberResp * resp)
{
	OtherAllianceSaveControl allianceCtl(aid);
	//权限校验
	if (type == manipute_type_committee)
	{
		CheckMemberPrivilege(uid, aid, privilege_manipulate_committee);
	}
	else if (type == manipute_type_director)
	{
		CheckMemberPrivilege(uid, aid, privilege_manipulate_director);
	}
	else
	{
		error_log("type param error. uid=%u,type=%u", uid, type);
		throw runtime_error("type_param_error");
	}


	//成员校验
	CheckMember(uid, aid, member_uid);

	DataAllianceMember & selfmember = DataAllianceMemberManager::Instance()->GetData(aid, uid);

	//获取商会成员的数据
	DataAllianceMember & othmember = DataAllianceMemberManager::Instance()->GetData(aid, member_uid);
	unsigned oldpos = othmember.position;

	//判断该成员的职位是否与操作者相同
	if (othmember.position <= selfmember.position)
	{
		error_log("need greater position. uid=%u,allianceid=%u, memberuid=%u", uid, aid, member_uid);
		throw runtime_error("privilege_not_enough");
	}

	//获取商会配置
	const ConfigAlliance::Alliance & alliancecfg = AllianceCfgWrap().GetAllianceCfg();

	if (type == manipute_type_committee)
	{
		//委员的升降
		if (operate_promotion == operate)
		{
			//判断委员的数量是是否超出上限
			int count = DataAllianceMemberManager::Instance()->GetPositionCount(aid, pos_type_committee);

			if (alliancecfg.committee_num_limit() <= count)
			{
				error_log("commitee num already max. uid=%u,allianceid=%u, memberuid=%u", uid, aid, member_uid);
				throw runtime_error("commitee_num_max");
			}

			//升职
			PositionChange(othmember, pos_type_committee);
		}
		else if (dest == pos_type_member)
		{
			//降的职位只能是普通成员
			PositionChange(othmember, pos_type_member);
		}
		else
		{
			error_log("param dest error, need lower than committee. uid=%u,dest=%u", uid, dest);
			throw runtime_error("param error.");
		}
	}
	else
	{
		//理事的升降
		if (operate_promotion == operate)
		{
			//判断理事的数量是是否超出上限
			int count = DataAllianceMemberManager::Instance()->GetPositionCount(aid, pos_type_director);

			if (alliancecfg.director_num_limit() <= count)
			{
				error_log("commitee num already max. uid=%u,allianceid=%u, memberuid=%u", uid, aid, member_uid);
				throw runtime_error("director_num_max");
			}

			//升职
			PositionChange(othmember, pos_type_director);
		}
		else if (dest == pos_type_committee)
		{
			//委员，判断委员是否达到上限
			//判断委员的数量是是否超出上限
			int count = DataAllianceMemberManager::Instance()->GetPositionCount(aid, pos_type_committee);

			if (alliancecfg.committee_num_limit() <= count)
			{
				error_log("commitee num already max. uid=%u,allianceid=%u, memberuid=%u", uid, aid, member_uid);
				throw runtime_error("commitee_num_max");
			}

			//降的职位必须低于理事
			PositionChange(othmember, pos_type_committee);
		}
		else if (dest == pos_type_member)
		{
			//成员
			PositionChange(othmember, pos_type_member);
		}
		else
		{
			//如果高于理事，则参数不对
			error_log("param dest error, need lower than committee. uid=%u,dest=%u", uid, dest);
			throw runtime_error("param error.");
		}
	}

	DataAllianceMemberManager::Instance()->UpdateItem(othmember);
	SetMemberMessage(othmember, resp->mutable_member());

	//对方的职位变化通知
	NotifyPositionChange(member_uid, oldpos, othmember.position, aid);

	return 0;
}
int LogicAllianceManager::PositionChange(DataAllianceMember &member, unsigned newposition)
{
	unsigned authority = GetAuthorityByPosition(newposition);

	member.position = newposition;
	member.authority = authority;

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::KickOutReq* req)
{
	unsigned memberuid = req->memberuid();

	KickOut(uid, memberuid);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestKickOutBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	unsigned member_uid = req->memberuid();
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		KickOutUpdateAlliance(uid, aid, req->memberuid());
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyKickOutBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestKickOutMemberBC* req)
{
	unsigned uid = req->uid();
	KickOutUpdateMember(req->aid(), req->memberuid());
	ProtoAlliance::ReplyKickOutBC* resp = new ProtoAlliance::ReplyKickOutBC;
	resp->set_uid(uid);
	resp->mutable_alliance()->set_memberuid(req->memberuid());
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}

int LogicAllianceManager::KickOut(unsigned uid, unsigned member_uid)
{
	//权限校验
//	CheckPrivilege(uid, privilege_kick_out);

	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (aid == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestKickOutBC* m = new ProtoAlliance::RequestKickOutBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_memberuid(member_uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			KickOutUpdateAlliance(uid, aid, member_uid);
		}
	}

	return 0;
}
int LogicAllianceManager::KickOutUpdateAlliance(unsigned uid, unsigned aid, unsigned member_uid)
{
	OtherAllianceSaveControl allianceCtl(aid);
	CheckMemberPrivilege(uid, aid, privilege_kick_out);
	//成员校验
	CheckMember(uid, aid, member_uid);

	unsigned oldpos = DataAllianceMemberManager::Instance()->GetData(aid, member_uid).position;

	//有职务不能踢出
	if (oldpos != pos_type_member)
	{
		error_log("only support kick member. uid=%u,member_uid=%u", uid, member_uid);
		throw runtime_error("only_support_kick_member");
	}

	//踢出公会
	ClearMemberData(aid, member_uid);

	//对方的职位变化通知
	NotifyPositionChange(member_uid, oldpos, pos_type_none, 0);

	if(CMI->IsNeedConnectByUID(member_uid))
	{
		ProtoAlliance::RequestKickOutMemberBC* m = new ProtoAlliance::RequestKickOutMemberBC;
		m->set_aid(aid);
		m->set_uid(uid);
		m->set_memberuid(member_uid);
		return BMI->BattleConnectNoReplyByAID(member_uid, m);
	}
	else
	{
		ProtoAlliance::KickOutResp *resp = new ProtoAlliance::KickOutResp;
		resp->set_memberuid(member_uid);
		KickOutUpdateMember(aid, member_uid);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}

	return 0;
}
int LogicAllianceManager::KickOutUpdateMember(unsigned aid, unsigned member_uid)
{
	//因为要修改对方用户的数据，所以要先加载数据
	//获取成员玩家的数据
	OffUserSaveControl offuserctl(member_uid);
	DBCUserBaseWrap othuserwrap(member_uid);

	if(aid == othuserwrap.Obj().alliance_id)
	{
		othuserwrap.Obj().alliance_id = 0;
		othuserwrap.Obj().allian_allow_ts = Time::GetGlobalTime() + 24*3600; //24小时后才能再申请加入其它商会
		othuserwrap.Save();
	}

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::ExitAllianceReq* req)
{
	ExitAlliance(uid);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestExitAllianceBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyExitAllianceBC* resp = new ProtoAlliance::ReplyExitAllianceBC;
	resp->set_uid(uid);
	resp->set_aid(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		ExitAllianceUpdateAlliance(uid, aid);
	}
	return BMI->BattleConnectNoReplyByAID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyExitAllianceBC* req)
{
	ExitAllianceUpdateUser(req->uid(), req->aid(), req->mutable_alliance());
	return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::ExitAlliance(unsigned uid)
{
	//退出商会，有条件，如果是会长，则只有当商会只有会长一个人时，才允许退出，而这时，退出就解散商会
	//获取用户的商会id
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestExitAllianceBC* m = new ProtoAlliance::RequestExitAllianceBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ExitAllianceResp* resp = new ProtoAlliance::ExitAllianceResp;
			ExitAllianceUpdateAlliance(uid, aid);
			ExitAllianceUpdateUser(uid, aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::ExitAllianceUpdateAlliance(unsigned uid, unsigned aid)
{
	OtherAllianceSaveControl allianceCtl(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);

	if (member.position != pos_type_chief)
	{
		//不是会长，直接退出
		ClearMemberData(aid, uid);
	}
	else
	{
		//会长，判断商会成员个数
		int count = DataAllianceMemberManager::Instance()->GetMemberCount(aid);

		if (count > 1)
		{
			error_log("chief can't quit during alliance has other members.uid=%u", uid);
			throw runtime_error("chief_can_not_quit");
		}

		//删除商会
		DropAlliance(uid, aid);
	}
	return 0;
}
int LogicAllianceManager::ExitAllianceUpdateUser(unsigned uid, unsigned aid, ProtoAlliance::ExitAllianceResp * resp)
{
	DBCUserBaseWrap userwrap(uid);
	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	if(userwrap.Obj().alliance_id == aid)
	{
		//更新当前用户的商会
		userwrap.Obj().alliance_id = 0;
		userwrap.Obj().allian_allow_ts = 0;  //主动退出，下次入会申请无需等待
		userwrap.Save();
		SetUserAllianceMsg(aid, userwrap.Obj().alliance_id, resp->mutable_commons());
	}
	return 0;
}

int LogicAllianceManager::DropAlliance(unsigned uid, unsigned alliance_id)
{
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(alliance_id);

	int ret = allianceMapping.Del(alliance.name);

	if (ret)
	{
		error_log("drop alliance mapping error. uid=%u,alliance_id=%u", uid, alliance_id);
		throw runtime_error("del_alliance_mapping_error");
	}

	//删除商会名称映射
	DataAllianceManager::Instance()->DelItem(alliance_id);

	//清除商会成员数据
	DataAllianceMemberManager::Instance()->DropAlliance(alliance_id);

	//清除商会申请数据
	DataAllianceApplyManager::Instance()->DropAlliance(alliance_id);

	//删除共享内存中的商会数据
	MemoryAllianceManager::Instance()->DelAlliance(alliance_id);

	return 0;
}

int LogicAllianceManager::ClearMemberData(unsigned alliance_id, unsigned member_uid)
{
	//清除商会成员
	DataAllianceMemberManager::Instance()->DelItem(alliance_id, member_uid);

	//清除商会成员的捐收信息
	if (DataAllianceDonationManager::Instance()->IsExistItem(alliance_id, member_uid))
	{
		DataAllianceDonationManager::Instance()->DelItem(alliance_id, member_uid);
	}

	//清除成员的通知信息
	DataAllianceNotifyManager::Instance()->ResetNotifyWhenChange(member_uid);

	//更新共享内存中商会成员个数
	debug_log("alliance_member_count aid=%u count=%d", alliance_id, -1);
	MemoryAllianceManager::Instance()->UpdateMemberCount(alliance_id, -1);

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::TransferReq* req)
{
	unsigned memberuid = req->memberuid();

	TransformChief(uid, memberuid, req->othername());

	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestTransferBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyTransferBC* resp = new ProtoAlliance::ReplyTransferBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		TransformChiefLocal(uid, aid, req->memberuid(), req->othername(), resp->mutable_alliance());
	}

	return BMI->BattleConnectNoReplyByAID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyTransferBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}

int LogicAllianceManager::TransformChief(unsigned uid, unsigned member_uid, const string& otherName)
{
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	unsigned aid = userwrap.Obj().alliance_id;

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestTransferBC* m = new ProtoAlliance::RequestTransferBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_memberuid(member_uid);
			m->set_othername(otherName);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::TransferResp* resp = new ProtoAlliance::TransferResp;
			TransformChiefLocal(uid, aid, member_uid, otherName, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}

	return 0;
}

int LogicAllianceManager::TransformChiefLocal(unsigned uid, unsigned aid, unsigned member_uid, const string& otherName, ProtoAlliance::TransferResp * resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_transfer_chief);
	//成员校验
	CheckMember(uid, aid, member_uid);

	//获取自身的数据
	DataAllianceMember & selfmember = DataAllianceMemberManager::Instance()->GetData(aid, uid);

	//获取商会成员的数据
	DataAllianceMember & othmember = DataAllianceMemberManager::Instance()->GetData(aid, member_uid);

	//判断当前用户的职位是否是会长
	if (selfmember.position != pos_type_chief)
	{
		error_log("should be chief. uid=%u", uid);
		throw runtime_error("should_be_chief");
	}

	//判断对方的职位是否是理事
	if (othmember.position != pos_type_director)
	{
		error_log("member should be chief. uid=%u,member_uid=%u", uid, member_uid);
		throw runtime_error("should_be_director");
	}

	//交换双方职位
	PositionChange(selfmember, pos_type_director);
	PositionChange(othmember, pos_type_chief);
	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(selfmember);
	DataAllianceMemberManager::Instance()->UpdateItem(othmember);

	//更新商会的创建者信息
	DataAlliance & dataalliance = DataAllianceManager::Instance()->GetData(aid);
	dataalliance.create_uid = member_uid;

//	OffUserSaveControl offcontrol(member_uid);
//	DBCUserBaseWrap othuserwrap(member_uid);
	snprintf(dataalliance.create_username, sizeof(dataalliance.create_username), otherName.c_str());

	DataAllianceManager::Instance()->UpdateItem(dataalliance);

	DataAllianceManager::Instance()->SetMessage(aid, resp);

	//设置前端消息
	SetMemberMessage(selfmember, resp->mutable_myself());
	SetMemberMessage(othmember, resp->mutable_other());

	//通知对方职位变化
	NotifyPositionChange(member_uid, pos_type_director, pos_type_chief, aid);
	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::EditAllianceReq* req)
{
	EditAlliance(uid, req);

	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestEditAllianceBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyEditAllianceBC* resp = new ProtoAlliance::ReplyEditAllianceBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		EditAllianceLocal(uid, aid, req->mutable_info(), resp->mutable_alliance());
	}

	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyEditAllianceBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}

int LogicAllianceManager::EditAlliance(unsigned uid, ProtoAlliance::EditAllianceReq* req)
{
	//权限校验
//	CheckPrivilege(uid, privilege_edit_alliance);

	string decription = req->description();

	//敏感词过滤
	SensitiveFilter(decription);

	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	unsigned aid = userwrap.Obj().alliance_id;


	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestEditAllianceBC* m = new ProtoAlliance::RequestEditAllianceBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->mutable_info()->CopyFrom(*req);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::EditAllianceResp* resp = new ProtoAlliance::EditAllianceResp;
			EditAllianceLocal(uid, aid, req, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}


	return 0;
}
int LogicAllianceManager::EditAllianceLocal(unsigned uid, unsigned aid, ProtoAlliance::EditAllianceReq* req, ProtoAlliance::EditAllianceResp* resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	uint32_t applyType = req->applytype();
	if(applyType == 0 || applyType >= join_type_max)
	{
		error_log("invalid applyType uid=%u aid=%u applyType=%u", uid, aid, applyType);
		throw runtime_error("invalid_applyType");
	}

	CheckMemberPrivilege(uid, aid, privilege_edit_alliance);
	//获取商会信息
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);

	alliance.flag = req->flag();
	string decription = req->description();
	snprintf(alliance.description, sizeof(alliance.description), decription.c_str());
	alliance.apply_type = req->applytype();
	alliance.apply_level_limit = req->applylevellimit();
	MemoryAllianceManager::Instance()->UpdateMember(aid, alliance.apply_type, alliance.apply_level_limit);

	//判断qq群号是否有修改
	if (req->has_qqgroup())
	{
		alliance.qqgroup = req->qqgroup();
	}

	//更新商会
	DataAllianceManager::Instance()->UpdateItem(alliance);

	DataAllianceManager::Instance()->SetMessage(aid, resp);
//	DataAllianceManager::Instance()->DoAllianceSave(aid);
	return 0;
}
int LogicAllianceManager::NotifyPositionChange(unsigned uid, unsigned oldpos, unsigned newpos, unsigned alliance_id)
{
	//debug_log("position change. uid=%u, alliance_id=%u, oldpos=%u,position=%u", uid, alliance_id, oldpos, newpos);

	//在线，推送被踢通知
	ProtoAlliance::PostionChangePushReq * pushmsg = new ProtoAlliance::PostionChangePushReq;
	pushmsg->set_oldpos(oldpos);
	pushmsg->set_newpos(newpos);

	if(!CMI->IsNeedConnectByAID(alliance_id))
	{
		//判断是否刚加入加入公会
		if (oldpos == pos_type_none && 0 != alliance_id)
		{
			DataAllianceManager::Instance()->SetMessage(alliance_id, pushmsg);
		}
	}
	LogicManager::Instance()->sendMsg(uid, pushmsg);

	return 0;
}

int LogicAllianceManager::CheckMember(unsigned uid, unsigned alliance_id, unsigned member_uid)
{
	//判断商会是否有该成员
	if (!DataAllianceMemberManager::Instance()->IsExistItem(alliance_id, member_uid))
	{
		error_log("alliance does't have this member. uid=%u,allianceid=%u, memberuid=%u", uid, alliance_id, member_uid);
		throw runtime_error("alliance_not_have_this_member");
	}

	return 0;
}
bool LogicAllianceManager::CheckMember(unsigned alliance_id, unsigned member_uid)
{
	return DataAllianceMemberManager::Instance()->IsExistItem(alliance_id, member_uid);
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::SeekDonationReq* req)
{
	unsigned propsid = req->propsid();
	unsigned count = req->count();

	SeekDonation(uid, propsid, count);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestSeekDonationBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplySeekDonationBC* resp = new ProtoAlliance::ReplySeekDonationBC;
	resp->set_uid(uid);
	resp->set_cdtime(req->cdtime());
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		SeekDonationUpdateAlliance(uid, aid, req->level(), req->name(), req->propsid(), req->count(), resp->mutable_alliance());
		resp->mutable_alliance()->set_nextdonationts(Time::GetGlobalTime() + req->cdtime());
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplySeekDonationBC* req)
{
	if(req->has_alliance())
	{
		SeekDonationUpdateUser(req->uid(), req->cdtime(), req->mutable_alliance());
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	}
	return 0;
}
int LogicAllianceManager::SeekDonation(unsigned uid, unsigned propsid, unsigned count)
{
	//权限校验
//	CheckPrivilege(uid, privilege_donate);

	//判断捐助cd
	DBCUserBaseWrap userwrap(uid);
	unsigned nowts = Time::GetGlobalTime();

	if (userwrap.Obj().next_donation_ts > nowts)
	{
		error_log("donation in cd. uid=%u", uid);
		throw runtime_error("donation_in_cd");
	}

	unsigned aid = userwrap.Obj().alliance_id;
	if (aid == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	if (!IsAllianceId(aid))
	{
		error_log("not a valid alliance id uid=%u aid=%u", uid, aid);
		throw runtime_error("not_a_valid_alliance_id");
	}


	//判断物品是否可捐收，以及数量是否满足要求
	//获取捐收配置
	AllianceCfgWrap alliancewrap;
	const ConfigAlliance::Donation &  donationcfg = alliancewrap.GetDonationCfg();

	int typeindex = alliancewrap.GetDonationPropsTypeIndex(propsid);

	if (donationcfg.type_storge(typeindex).limitnum() < count)
	{
		error_log("donation's nums beyond max. uid=%u,propsid=%u,count=%u", uid, propsid, count);
		throw runtime_error("donation_nums_beyond_max");
	}

	if(CMI->IsNeedConnectByAID(aid))
	{
		ProtoAlliance::RequestSeekDonationBC* m = new ProtoAlliance::RequestSeekDonationBC;
		m->set_uid(uid);
		m->set_count(count);
		m->set_aid(aid);
		m->set_propsid(propsid);
		m->set_name(userwrap.Obj().name);
		m->set_cdtime(donationcfg.cdtime());
		m->set_level(userwrap.Obj().level);
		return BMI->BattleConnectNoReplyByAID(aid, m);
	}
	else if(CheckMember(aid, uid))
	{
		ProtoAlliance::SeekDonationResp* resp = new ProtoAlliance::SeekDonationResp;
		SeekDonationUpdateAlliance(uid, aid, userwrap.Obj().level, userwrap.Obj().name, propsid, count, resp);
		SeekDonationUpdateUser(uid, donationcfg.cdtime(), resp);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}

	return 0;
}

int LogicAllianceManager::SeekDonationUpdateAlliance(unsigned uid, unsigned aid, unsigned level, const string& name, unsigned propsid, unsigned count, ProtoAlliance::SeekDonationResp * resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_donate);
	//当存在已捐收数据，且物品未取完，则不允许重复发出捐收
	if (DataAllianceDonationManager::Instance()->IsExistItem(aid, uid))
	{
		DataAllianceDonation & donation = DataAllianceDonationManager::Instance()->GetData(aid, uid);

		if (donation.donate_count > donation.fetch_count)
		{
			error_log("donation item has left. uid=%u", uid);
			throw runtime_error("donation_item_has_left");
		}
	}

	//新增捐收数据
	DataAllianceDonation & donation = DataAllianceDonationManager::Instance()->GetData(aid, uid);
	//重置捐收数据
	donation.Reset();
	donation.propsid = propsid;
	donation.count = count;
	donation.applyts = Time::GetGlobalTime();
	donation.level = level;

	snprintf(donation.username, sizeof(donation.username), "%s", name.c_str());

	DataAllianceDonationManager::Instance()->UpdateItem(donation);
	DataAllianceDonationManager::Instance()->SetMessage(donation, resp->mutable_donation());

	return 0;
}
int LogicAllianceManager::SeekDonationUpdateUser(unsigned uid, unsigned cdtime, ProtoAlliance::SeekDonationResp * resp)
{
	DBCUserBaseWrap userwrap(uid);
	//更新用户的下次可捐收时间
	userwrap.Obj().next_donation_ts = Time::GetGlobalTime() + cdtime;
	userwrap.Save();
	resp->set_nextdonationts(userwrap.Obj().next_donation_ts);

	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::CutUpDonationCDReq* req, ProtoAlliance::CutUpDonationCDResp* resp)
{
	CutCD(uid, req->type(),resp);

	return 0;
}

int LogicAllianceManager::CutCD(unsigned uid, unsigned type,ProtoAlliance::CutUpDonationCDResp * resp)
{
	//权限校验
//	CheckPrivilege(uid, privilege_donate);

	DBCUserBaseWrap userwrap(uid);
	unsigned nowts = Time::GetGlobalTime();

	if (userwrap.Obj().next_donation_ts <= nowts)
	{
		error_log("cd is over. uid=%u", uid);
		throw runtime_error("cd_is_over");
	}

	//花钻或者看广告秒cd
	if(type != 1 && type != 2)
	{
		throw std::runtime_error("type_param_error");
	}

	//获取配置
	const ConfigAlliance::Donation & donationcfg = AllianceCfgWrap().GetDonationCfg();

	if(type == 1)
	{
		//花钻秒cd
		int diffts = userwrap.Obj().next_donation_ts > nowts ? userwrap.Obj().next_donation_ts - nowts : 0;

		//获取订单配置中，撕单的总价格
		int cash = ceil(static_cast<double>(diffts)/donationcfg.cdtime() * donationcfg.cdprice());

		//扣钻
		userwrap.CostCash(cash, "AllianceDonation_CutCD");

		//设置返回给前端的消息
		DataCommon::BaseItemCPP* basemsg = resp->mutable_commons()->mutable_userbase()->add_baseitem();

		basemsg->set_change(-cash);
		basemsg->set_totalvalue(userwrap.Obj().cash);
		basemsg->set_type(type_cash);
	}

	//更新cd时间
	userwrap.Obj().next_donation_ts = 0;
	userwrap.Save();

	resp->set_nextdonationts(0);

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::OfferDonationReq* req)
{
	unsigned applyuid = req->applyuid();
	unsigned propsid = req->propsid();
	OfferHelp(uid, applyuid, propsid);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestOfferDonationBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyOfferDonationBC* resp = new ProtoAlliance::ReplyOfferDonationBC;
	resp->set_uid(uid);
	resp->set_aid(aid);
	resp->set_propsid(req->propsid());
	resp->mutable_alliance()->mutable_commons()->CopyFrom(req->commons());
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		OfferHelpUpdateAlliance(uid, req->name(), aid, req->applyuid(), req->propsid(), resp->mutable_alliance());
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyOfferDonationBC* req)
{
	if(req->has_alliance())
	{
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	}
	return 0;
}

int LogicAllianceManager::OfferHelp(unsigned uid, unsigned apply_uid, unsigned propsid)
{
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	unsigned aid = userwrap.Obj().alliance_id;
	if (!IsAllianceId(aid))
	{
		error_log("not a valid alliance id uid=%u aid=%u", uid, aid);
		throw runtime_error("not_a_valid_alliance_id");
	}
	//判断是否自己捐收自己的情况
	if (uid == apply_uid)
	{
		error_log("cannot donate self. uid=%u,aid=%u,applyuid=%u", uid, aid, apply_uid);
		throw runtime_error("cannot_donate_self");
	}

	//捐收未完成，则进行捐收
	//捐收物品消耗
	CommonGiftConfig::CommonModifyItem cfg;
	CommonGiftConfig::PropsItem*  itemcfg = cfg.add_props();
	itemcfg->set_count(-1);   //一次捐收，就只能捐收1个
	itemcfg->set_id(propsid);

	ProtoAlliance::OfferDonationResp* resp = new ProtoAlliance::OfferDonationResp;
	LogicUserManager::Instance()->CommonProcess(uid, cfg, "AllianceDonation_Offer", resp->mutable_commons());
	if(CMI->IsNeedConnectByAID(aid))
	{
		ProtoAlliance::RequestOfferDonationBC* m = new ProtoAlliance::RequestOfferDonationBC;
		m->set_uid(uid);
		m->set_applyuid(apply_uid);
		m->set_aid(aid);
		m->set_propsid(propsid);
		m->set_name(userwrap.Obj().name);
		m->mutable_commons()->CopyFrom(resp->commons());
		delete resp;
		return BMI->BattleConnectNoReplyByAID(aid, m);
	}
	else
	{
		OfferHelpUpdateAlliance(uid, userwrap.Obj().name, aid, apply_uid, propsid, resp);
		OfferHelpUpdateUser(uid, aid, propsid, resp);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}


	return 0;
}
int LogicAllianceManager::OfferHelpUpdateAlliance(unsigned uid, const string& name, unsigned aid, unsigned apply_uid, unsigned propsid, ProtoAlliance::OfferDonationResp * resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_donate);

	//判断是否同一公会
	CheckMember(uid, aid, apply_uid);


	//判断是否存在该用户的捐收请求
	if (!DataAllianceDonationManager::Instance()->IsExistItem(aid, apply_uid))
	{
		error_log("user have no help request. uid=%u,alliance_id,applyuid=%u", uid, aid, apply_uid);
		throw runtime_error("user_have_no_help_request");
	}

	//获取捐收数据
	DataAllianceDonation & donation = DataAllianceDonationManager::Instance()->GetData(aid, apply_uid);

	if (donation_finish == donation.status)
	{
		error_log("donation finished. uid=%u,alliance_id,applyuid=%u", uid, aid, apply_uid);
		throw runtime_error("donation_finished");
	}
	if (propsid != donation.propsid)
	{
		error_log("donation propsid missmatch uid=%u,alliance_id=%u,applyuid=%u,realid=%u propsid=%u", uid, aid, apply_uid,donation.propsid, propsid);
		throw runtime_error("donation_finished");
	}

	//捐收成功，修改捐收数据
	donation.Donate(uid, name.c_str());

	DataAllianceDonationManager::Instance()->UpdateItem(donation);

	//增加帮助次数
	LogicUserManager::Instance()->UpdateAllianceHelpTimes(uid, aid);

	//设置返回给前端的消息
	DataAllianceDonationManager::Instance()->SetMessage(donation, resp->mutable_donation());

	return 0;
}
int LogicAllianceManager::OfferHelpUpdateUser(unsigned uid, unsigned aid, unsigned propsid, ProtoAlliance::OfferDonationResp * resp)
{
	//增加帮助次数
	LogicUserManager::Instance()->UpdateUserHelpTimes(uid, aid);
	//捐收可获得经验奖励
	const ConfigItem::PropItem & propscfg = ItemCfgWrap().GetPropsItem(propsid);
	LogicUserManager::Instance()->CommonProcess(uid, propscfg.extra_reward(), "AllianceDonation_Offer", resp->mutable_commons());

	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::FetchDonationReq* req)
{
	FetchDonation(uid);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestFetchDonationBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyFetchDonationBC* resp = new ProtoAlliance::ReplyFetchDonationBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		unsigned propsid = 0;
		FetchDonationAllianceLocal(uid, aid, propsid, resp->mutable_alliance());
		resp->set_propsid(propsid);
	}
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyFetchDonationBC* req)
{
	if(req->has_alliance())
	{
		AddDonation(req->uid(), req->propsid(), req->mutable_alliance());
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	}
	return 0;
}

int LogicAllianceManager::FetchDonation(unsigned uid)
{
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	//获取粮仓的剩余空间
	int restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, type_granary);
	if (0 == restspace)
	{
		error_log("storage is full. uid=%u", uid);
		throw runtime_error("storage_is_full");
	}
	unsigned aid = userwrap.Obj().alliance_id;

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestFetchDonationBC* m = new ProtoAlliance::RequestFetchDonationBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else
		{
			ProtoAlliance::FetchDonationResp* resp = new ProtoAlliance::FetchDonationResp;
			unsigned propsid = 0;
			FetchDonationAllianceLocal(uid, aid, propsid, resp);
			AddDonation(uid, propsid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::FetchDonationAllianceLocal(unsigned uid, unsigned aid, unsigned& propsid, ProtoAlliance::FetchDonationResp * resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_donate);

	//判断是否存在该用户的捐收请求
	if (!DataAllianceDonationManager::Instance()->IsExistItem(aid, uid))
	{
		error_log("user have no help request. uid=%u,alliance_id", uid, aid);
		throw runtime_error("user_have_no_help_request");
	}

	DataAllianceDonation & donation = DataAllianceDonationManager::Instance()->GetData(aid, uid);
//	info_log("fetch_donation uid=%u aid=%u fc=%u dc=%u", uid, aid, donation.fetch_count, donation.donate_count);

	propsid = donation.propsid;
	//判断是否有空余的物品可以领取
	if (donation.fetch_count >= donation.donate_count)
	{
		error_log("donation item has no left. uid=%u,alliance_id", uid, aid);
		throw runtime_error("donation_item_no_left");
	}

	donation.fetch_count += 1;

	//如果可捐收物品已取完，且捐收已完成，则删除捐收数据
	if (donation.fetch_count >= donation.donate_count && donation.status == donation_finish)
	{
		//删除捐收数据
		DataAllianceDonationManager::Instance()->DelItem(aid, uid);
	}
	else
	{
		if(!DataAllianceDonationManager::Instance()->UpdateItem(donation))
		{
			error_log("DataAllianceDonationManager UpdateItem fail uid=%u aid=%u", uid, aid);
		}
	}
	DataAllianceDonationManager::Instance()->SetMessage(donation, resp->mutable_donation());
	return 0;
}
int LogicAllianceManager::AddDonation(unsigned uid, unsigned propsid, ProtoAlliance::FetchDonationResp * resp)
{
	//捐收物品领取
	CommonGiftConfig::CommonModifyItem cfg;
	CommonGiftConfig::PropsItem*  itemcfg = cfg.add_props();
	itemcfg->set_count(1);   //一次捐收，就只能捐收1个
	itemcfg->set_id(propsid);
	LogicUserManager::Instance()->CommonProcess(uid, cfg, "AllianceDonation_Fetch", resp->mutable_commons());
	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::SendNotifyReq* req)
{
	string content = req->content();

	SendNotify(uid, content);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestSendNotifyBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplySendNotifyBC* resp = new ProtoAlliance::ReplySendNotifyBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		SendNotifyLocal(uid, aid, req->name(), req->content(), resp->mutable_alliance());
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplySendNotifyBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}


int LogicAllianceManager::SendNotify(unsigned uid, string & content)
{
	//敏感词过滤
	SensitiveFilter(content);

	//权限校验
//	CheckPrivilege(uid, privilege_broadcast_notice);

	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	unsigned aid = userwrap.Obj().alliance_id;

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestSendNotifyBC* m = new ProtoAlliance::RequestSendNotifyBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_content(content);
			m->set_name(userwrap.Obj().name);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else
		{
			ProtoAlliance::SendNotifyResp* resp = new ProtoAlliance::SendNotifyResp;
			SendNotifyLocal(uid, aid, userwrap.Obj().name, content, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}

	return 0;
}
int LogicAllianceManager::SendNotifyLocal(unsigned uid, unsigned aid, const string& userName, const string & content, ProtoAlliance::SendNotifyResp * resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_broadcast_notice);
	//获取用户在商会中的职位
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);

	//发送公告
	unsigned notifyid = DataAllianceNotifyManager::Instance()->GetFreeNotifyId(aid, uid, member.position);

	if (-1 == notifyid)
	{
		error_log("notify is full. uid=%u", uid);
		throw runtime_error("notify_is_full");
	}

	//公告可用，则更新公告信息
	DataAllianceNotify & notify = DataAllianceNotifyManager::Instance()->GetData(aid, notifyid);

	notify.announce_uid = uid;
	notify.create_ts = Time::GetGlobalTime();
	notify.position = member.position;

	snprintf(notify.content, sizeof(notify.content), "%s", content.c_str());
	snprintf(notify.username, sizeof(notify.username), "%s", userName.c_str());

	DataAllianceNotifyManager::Instance()->UpdateItem(notify);

	DataAllianceNotifyManager::Instance()->SetMessage(notify, resp->mutable_notify());
	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::DelNotifyReq* req)
{
	unsigned id = req->id();

	DelNofity(uid, id);

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestDelNotifyBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyDelNotifyBC* resp = new ProtoAlliance::ReplyDelNotifyBC;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		DelNofityLocal(uid, aid, req->id(), resp->mutable_alliance());
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}
int LogicAllianceManager::Process(ProtoAlliance::ReplyDelNotifyBC* req)
{
	if(req->has_alliance())
		return LMI->sendMsg(req->uid(), req->mutable_alliance(), false) ? 0 : R_ERROR;
	return 0;
}
int LogicAllianceManager::DelNofity(unsigned uid, unsigned id)
{
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}
	unsigned aid = userwrap.Obj().alliance_id;

	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestDelNotifyBC* m = new ProtoAlliance::RequestDelNotifyBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_id(id);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else
		{
			ProtoAlliance::DelNotifyResp* resp = new ProtoAlliance::DelNotifyResp;
			DelNofityLocal(uid, aid, id, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}

	return 0;
}
int LogicAllianceManager::DelNofityLocal(unsigned uid, unsigned aid, unsigned id, ProtoAlliance::DelNotifyResp* resp)
{
	OtherAllianceSaveControl allianceCtl(aid);

	CheckMemberPrivilege(uid, aid, privilege_broadcast_notice);
	if (!DataAllianceNotifyManager::Instance()->IsExistItem(aid, id))
	{
		error_log("notify not exist. uid=%u,notifyid=%u", uid, id);
		throw runtime_error("notify_not_exist");
	}

	//获取用户在商会中的职位
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);

	//获取通知数据，判断是否是自身
	DataAllianceNotify & notify = DataAllianceNotifyManager::Instance()->GetData(aid, id);

	if (notify.announce_uid != uid && member.position != pos_type_chief)
	{
		//此时，只有当自身是会长时，才有删除的权限
		error_log("privilege not enough. uid=%u,notifyid=%u", uid, id);
		throw runtime_error("privilege_not_enough");
	}

	//有权限，则重置该通知
	notify.Reset();
	DataAllianceNotifyManager::Instance()->UpdateItem(notify);

	resp->set_id(id);
	return 0;
}

int LogicAllianceManager::CheckPrivilege(unsigned uid, unsigned privilege)
{
	//先判断当前用户是否有权限做此操作
	//获取用户的商会id
	DBCUserBaseWrap userwrap(uid);

	if (userwrap.Obj().alliance_id == 0)
	{
		error_log("user not in alliance. uid=%u", uid);
		throw runtime_error("not_in_alliance");
	}

	unsigned alliance_id = userwrap.Obj().alliance_id;

	return CheckMemberPrivilege(uid, alliance_id, privilege);
}
int LogicAllianceManager::CheckMemberPrivilege(unsigned uid, unsigned alliance_id, unsigned privilege)
{
	//判断商会是否真实存在
	if (!DataAllianceManager::Instance()->IsExist(alliance_id))
	{
		error_log("alliance not exist. uid=%u,alliance_id=%u", uid, alliance_id);
		throw runtime_error("alliance_not_exist");
	}

	//获取用户在商会中的职位
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(alliance_id, uid);
	member.authority = GetAuthorityByPosition(member.position);  //权限
	//判断用户的权限是否满足要求
	if (!IsPrivilege(member.authority, privilege))
	{
		error_log("privilege not enough. uid=%u", uid);
		throw runtime_error("privilege_not_enough");
	}

	return 0;
}

int LogicAllianceManager::SensitiveFilter(string & str)
{
	//敏感词过滤
	String::Trim(str);

	if (str.empty())
	{
		error_log("words empty. name=%s", str.c_str());
		throw runtime_error("words_empty");
	}

	if(!StringFilter::Check(str))
	{
		error_log("sensitive words. name=%s", str.c_str());
		throw runtime_error("sensitive_words");
	}

	return 0;
}

bool LogicAllianceManager::IsMemberFull(unsigned uid, unsigned alliance_id)
{
	//判断商会成员是否已满
	unsigned count = DataAllianceMemberManager::Instance()->GetMemberCount(alliance_id);

	//获取商会配置
	const ConfigAlliance::Alliance & alliancecfg = AllianceCfgWrap().GetAllianceCfg();

	if (count >= alliancecfg.member_num_limit())
	{
		error_log("alliance is full. uid=%u, alliance_id=%u", uid, alliance_id);
		throw runtime_error("alliance_is_full");
	}

	return false;
}
bool LogicAllianceManager::CheckMemberFull(unsigned uid, unsigned alliance_id)
{
	//判断商会成员是否已满
	unsigned count = DataAllianceMemberManager::Instance()->GetMemberCount(alliance_id);

	//获取商会配置
	const ConfigAlliance::Alliance & alliancecfg = AllianceCfgWrap().GetAllianceCfg();

	if (count >= alliancecfg.member_num_limit())
	{
		error_log("alliance is full. uid=%u, alliance_id=%u count=%u", uid, alliance_id, count);
		return false;
	}

	return true;
}
int LogicAllianceManager::SetMemberMessage(DataAllianceMember &member, ProtoAlliance::AllianceMemberCPP * msg)
{
	member.SetMessage(msg);
	return 0;
}
int LogicAllianceManager::SetMemberMessage(DataAllianceMember &member, ProtoAlliance::ReplyAllianceRaceOrder * msg)
{
	member.SetMessage(msg);
	return 0;
}

bool LogicAllianceManager::IsNameAvailable(unsigned uid, string & name, string & reason)
{
	//敏感词汇过滤
	try
	{
		SensitiveFilter(name);
	}
	catch(runtime_error &e)
	{
		reason = "sensitive_name";
		return false;
	}

	//判断名称是否已存在
	DataAllianceMapping tempalliance;
	snprintf(tempalliance.alliance_name, sizeof(tempalliance.alliance_name), "%s", name.c_str());

	int ret = allianceMapping.Get(tempalliance);

	if(ret == 0)
	{
		error_log("duplicate_alliance_name. uid=%u,allianceId=%u,name=%s]", uid, tempalliance.alliance_id, name.c_str());
		reason = "duplicate_alliance_name";

		return false;
	}
	else if(ret != R_ERR_NO_DATA)
	{
		error_log("GetMapping fail. ret=%d,uid=%u,allianceId=%u,name=%s", ret, uid, tempalliance.alliance_id, name.c_str());
		reason = "get_namemapping_error";

		return false;
	}

	return true;
}
bool LogicAllianceManager::IsInRace(unsigned joinTs)
{
	return joinTs < MemoryAllianceRaceGroupManager::Instance()->GetTs();	// 加入时间得在竞赛开始之前
}

int LogicAllianceManager::SetMemberByPostion(unsigned allianceid, unsigned uid, unsigned position,
		unsigned helptimes, unsigned level, const string& name,
		unsigned online, unsigned needHelp, const string& fig,
		DataAllianceMember & member)
{
	member.alliance_id = allianceid;
	member.position = position;
	member.id = uid;
	member.helptimes = helptimes;
	member.userlevel = level;
	member.onlineTs = online;
	member.helpTs = needHelp;
	snprintf(member.username, sizeof(member.username), name.c_str());
	snprintf(member.fig, sizeof(member.fig), fig.c_str());

	unsigned authority = GetAuthorityByPosition(position);

	member.authority = authority;
	/*
	//判断当前用户是否在线
	if (UserManager::Instance()->IsOnline(uid))
	{
		DBCUserBaseWrap userwrap(uid);

		member.helptimes = userwrap.Obj().helptimes;

		member.userlevel = userwrap.Obj().level;
		snprintf(member.username, sizeof(member.username), userwrap.Obj().name);
	}
	else if ((unsigned)-1 != ResourceManager::Instance()->GetIndex(uid))
	{
		//应有离线数据
		unsigned index = ResourceManager::Instance()->GetIndex(uid);
		OfflineResourceItem & item = ResourceManager::Instance()->GetResourceItemByIndex(index);

		member.helptimes = item.helptimes;

		member.userlevel = item.level;
		snprintf(member.username, sizeof(member.username), item.name);
	}
	else
	{
		//离线，且离线资源内不存在离线用户数据
		//加载离线数据
		OffUserSaveControl offuserctl(uid);
		DBCUserBaseWrap userwrap(uid);

		member.helptimes = userwrap.Obj().helptimes;
		member.userlevel = userwrap.Obj().level;
		snprintf(member.username, sizeof(member.username), userwrap.Obj().name);
	}
	*/


	return 0;
}

int LogicAllianceManager::GetAuthorityByPosition(unsigned position)
{
	unsigned authority = 0;

	//根据职位，初始化成员的权限
	switch(position)
	{
		case pos_type_chief: authority |= (privilege_manipulate_director |privilege_transfer_chief| privilege_edit_alliance);
		case pos_type_director: authority |= (privilege_edit_competition|privilege_manipulate_committee |privilege_broadcast_notice|privilege_kick_out|privilege_del_race_order);
		case pos_type_committee: authority |= (privilege_invite |privilege_approve);
		case pos_type_member: authority |= (privilege_join_competition |privilege_donate);
			break;
		default:
			break;
	}

	return authority;
}

void LogicAllianceManager::SetUserAllianceMsg(unsigned old, unsigned alliance_id, DataCommon::CommonItemsCPP * msg)
{
	DataCommon::BaseItemCPP* basemsg = msg->mutable_userbase()->add_baseitem();

	basemsg->set_change(old);
	basemsg->set_totalvalue(alliance_id);

	basemsg->set_type(type_alliance_id);
}
int LogicAllianceManager::Process(ProtoAlliance::RequestUpdateMemberBC* req)
{
	UpdateMemberLocal(req->uid(), req->aid(), req->onlinets(), req->helpts(), req->level(), req->name(), req->viplevel());
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAddMemberHelpTimesBC* req)
{
	AddMemberHelpTimesLocal(req->uid(), req->aid());
	return 0;
}




//设置商会成员竞赛标志
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceSetFlag* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceSetFlagBC* m = new ProtoAlliance::RequestAllianceRaceSetFlagBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_id(req->id());
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			uint32_t id = req->id();
			ProtoAlliance::ReplyAllianceRaceInfo* resp = new ProtoAlliance::ReplyAllianceRaceInfo;
			SetAllianceRaceFlag(uid, aid, id, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceSetFlagBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		ProtoAlliance::ReplyAllianceRaceInfo* resp = new ProtoAlliance::ReplyAllianceRaceInfo;
		SetAllianceRaceFlag(uid, aid, req->id(), resp);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}
	return 0;
}
int LogicAllianceManager::SetAllianceRaceFlag(uint32_t uid, uint32_t aid, uint32_t id, ProtoAlliance::ReplyAllianceRaceInfo* resp)
{
	if(id != flag_id_race_order_confirm_grade_reward)
	{
		error_log("invalid flagId uid=%u,id=%u", uid, id);
		throw runtime_error("param_error");
	}
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	setFlag(member.flag, flag_id_race_order_confirm_grade_reward);
	FillAllianceRaceInfo(uid, aid, resp);

	DataAllianceMemberManager::Instance()->UpdateItem(member);
	return 0;
}

//竞赛订单完成进度
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceMemberProgress* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceMemberProgressBC* m = new ProtoAlliance::RequestAllianceRaceMemberProgressBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			return SyncRaceOrderProgress(uid, aid);
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceMemberProgressBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		return SyncRaceOrderProgress(uid, aid);

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceInfo* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceInfoBC* m = new ProtoAlliance::RequestAllianceRaceInfoBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceInfo* resp = new ProtoAlliance::ReplyAllianceRaceInfo;
			FillAllianceRaceInfo(uid, aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceInfoBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceInfo* resp = new ProtoAlliance::ReplyAllianceRaceInfo;
	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		FillAllianceRaceInfo(uid, aid, resp);
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
int LogicAllianceManager::FillAllianceRaceInfo(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceInfo* resp)
{
	CheckRace(aid);
	uint32_t now = Time::GetGlobalTime();
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);

	resp->set_point(alliance.race_point);
	resp->set_racelevel(alliance.race_level);
	resp->set_overts(GetRaceOverTs());
	resp->set_flag(member.flag);
	resp->set_inrace(IsInRace(member.join_ts) ? 1 : 0);
	resp->set_memberpoint(member.race_point);
	resp->set_openrace(IsRaceOpen() ? 1 : 0);
	return 0;
}


int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceOrder* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceOrderBC* m = new ProtoAlliance::RequestAllianceRaceOrderBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceOrder* resp = new ProtoAlliance::ReplyAllianceRaceOrder;
			FillAllianceRaceOrder(uid, aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceOrderBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceOrder* resp = new ProtoAlliance::ReplyAllianceRaceOrder;
	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		FillAllianceRaceOrder(uid, aid, resp);
	}
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
int LogicAllianceManager::FillAllianceRaceOrder(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceOrder* resp)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u aid=%u", uid, aid);
		return -1;
	}
	CheckRace(aid);
	uint32_t now = Time::GetGlobalTime();
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);

	bool needSave = false;
	for(uint32_t i = 0; i < DataAlliance_race_order_id_LENG; ++i)
	{
		if(alliance.race_order_cd[i] > 0
		&& alliance.race_order_cd[i] < now	// cd时间已到
		&& (alliance.race_order_id[i] = AllianceRaceCfgWrap().GetRandTaskId(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID)) > 0)
		{
			alliance.race_order_cd[i] = 0; //now + AllianceRaceCfgWrap().GetCfg().task().cdtime();
			needSave = true;
		}
	}
	if(needSave)
	{
		DataAllianceManager::Instance()->UpdateItem(alliance);
	}

	DataAllianceManager::Instance()->SetMessage1(aid, resp);
	vector<unsigned> indexs;

	DataAllianceMemberManager::Instance()->GetIndexs(aid, indexs);
	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
		if(member.race_order_id > 0 && member.race_order_ts > now && !isRaceOrderFinish(member))
		{
			SetMemberMessage(member, resp);
		}
		if(member.id == uid)
		{
			resp->set_point(member.race_point);
			resp->set_orderrecv(member.race_order_recv);
			resp->set_flag(member.flag);
			resp->set_maxorderrecv(getRaceOrderMaxChance(uid, alliance.race_level, member.flag, member.vipLevel));
		}
	}
	return 0;
}
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceOperateOrder* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(req->operate() == race_order_operate_cut_cd)	// 秒cd时间需要先检查玩家资源是否足够
		{
			uint32_t cost = GetRaceOrderCutCdCost(uid, req->cdts());
			CommonGiftConfig::BaseItem base;
			base.set_cash(-1 * cost);
			userwrap.CheckBaseBeforeCost(uid, "alliance_race_order_cut_cd", base);
		}

		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceOperateOrderBC* m = new ProtoAlliance::RequestAllianceRaceOperateOrderBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_slot(req->slot());
			m->set_operate(req->operate());
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceOperateOrder* resp = new ProtoAlliance::ReplyAllianceRaceOperateOrder;
			resp->set_uid(uid);
			resp->set_operate(req->operate());
			unsigned ret = OperateAllianceRaceOrder(uid, aid, req->slot(), req->operate(), resp);
			resp->set_ret(ret);
			if(ret == race_order_operate_success && req->operate() == race_order_operate_cut_cd)
			{
				uint32_t cost = GetRaceOrderCutCdCost(uid, resp->cdts());
				CommonGiftConfig::CommonModifyItem cfg;
				cfg.mutable_based()->set_cash(-1 * cost);
				LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_order_cut_cd", resp->mutable_commons());
			}
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceOperateOrderBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceOperateOrder* resp = new ProtoAlliance::ReplyAllianceRaceOperateOrder;
	resp->set_uid(uid);
	resp->set_operate(req->operate());
	unsigned ret = 0;
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		ret = OperateAllianceRaceOrder(uid, aid, req->slot(), req->operate(), resp);
	}
	resp->set_ret(ret);
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}

int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceRaceOperateOrder* req)
{
	uint32_t uid = req->uid();
	uint32_t ret = req->ret();
	uint32_t operate = req->operate();
	if(operate == race_order_operate_cut_cd && ret == race_order_operate_success)	// 秒cd时间需要先检查玩家资源是否足够
	{
		uint32_t cost = GetRaceOrderCutCdCost(uid, req->cdts());
		CommonGiftConfig::CommonModifyItem cfg;
		cfg.mutable_based()->set_cash(-1 * cost);
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_order_cut_cd", req->mutable_commons());
	}
	return LMI->sendMsg(req->uid(), req, false) ? 0 : R_ERROR;
}
int LogicAllianceManager::OperateAllianceRaceOrder(uint32_t uid, uint32_t aid, uint32_t slot, uint32_t operate, ProtoAlliance::ReplyAllianceRaceOperateOrder* resp)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(aid, uid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(!IsInRace(member.join_ts))
	{
		error_log("member not join alliance race uid=%u,aid=%u", uid, aid);
		throw runtime_error("member_not_join_alliance_race");
	}
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	if(slot <= 0 || slot > DataAlliance_race_order_id_LENG)
	{
		error_log("slot param error. uid=%u,slot=%u", uid, slot);
		throw runtime_error("param_error");
	}
	uint16_t orderId = alliance.race_order_id[slot - 1];
	uint32_t orderCd = alliance.race_order_cd[slot - 1];
	if(orderId == 0 && orderCd < now)	// cd时间已到
	{
		if((alliance.race_order_id[slot - 1] = AllianceRaceCfgWrap().GetRandTaskId(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID)) == 0)
		{
			error_log("GetRandTaskId fail uid=%u operate=%u", uid, operate);
			throw runtime_error("get_alliance_race_task_fail");
		}
		alliance.race_order_cd[slot - 1] = 0;
		orderId = alliance.race_order_id[slot - 1];
		orderCd = alliance.race_order_cd[slot - 1];
	}
	resp->set_cdts(0);
	if(operate == race_order_operate_del)
	{
		if(orderCd >= now)
		{
			error_log("race order still in cd time uid=%u operate=%u orderCd=%u", uid, orderCd);
			return race_order_operate_in_cd;
		}
		CheckMemberPrivilege(uid, aid, privilege_del_race_order);
		alliance.race_order_id[slot - 1] = 0;
		alliance.race_order_cd[slot - 1] = now + AllianceRaceCfgWrap().GetCfg().task().cdtime();
	}
	else if(operate == race_order_operate_accept)
	{
		if(orderCd >= now)
		{
			error_log("race order still in cd time uid=%u operate=%u orderCd=%u", uid, orderCd);
			return race_order_operate_in_cd;
		}
		//更新成员数据
		uint32_t t = 0;
		uint32_t level = 0;
		AllianceRaceCfgWrap().GetTaskInfo(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID, orderId, t, level);
		if(member.userlevel < level)
		{
			error_log("member level limit uid=%u mlevel=%u rlevel=%u", uid, member.userlevel, level);
			throw runtime_error("level_limit");
		}
		if(member.race_order_recv >= getRaceOrderMaxChance(uid, alliance.race_level, member.flag, member.vipLevel))
		{
			error_log("race order recv out of limit uid=%u", uid);
			throw runtime_error("race_order_recv_out_of_limit");
		}
		if(member.race_order_id > 0 && member.race_order_ts > now)
		{
			error_log("race order exist uid=%u", uid);
			throw runtime_error("race_order_exist");
		}

		if(orderId > 0)
		{
			alliance.race_order_id[slot - 1] = 0;
			alliance.race_order_cd[slot - 1] = now + AllianceRaceCfgWrap().GetCfg().task().cdtime();
		}
		else
		{
			error_log("race order still in cd time uid=%u operate=%u orderCd=%u", uid, orderCd);
			return race_order_operate_id_not_exist;
		//	throw runtime_error("race_order_in_cd");
		}
		member.race_order_id = orderId;
		member.race_order_recv++;
		member.race_order_ts = now + t;
		if(member.race_order_ts > GetRaceOverTs())
		{
			member.race_order_ts = GetRaceOverTs();
		}
		AddRaceMemberOrderLog(member, member.race_order_id, race_member_order_status_doing);
		//更新
		DataAllianceMemberManager::Instance()->UpdateItem(member);
	}
	else if(operate == race_order_operate_cut_cd)
	{
		resp->set_cdts(orderCd);
		if(orderId == 0)
		{
			if((alliance.race_order_id[slot - 1] = AllianceRaceCfgWrap().GetRandTaskId(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID)) == 0)
			{
				error_log("GetRandTaskId fail uid=%u operate=%u", uid, operate);
				throw runtime_error("get_alliance_race_task_fail");
			}
			alliance.race_order_cd[slot - 1] = 0;
		}
		else
		{
			return race_order_operate_id_exist;
		}
	}
	else
	{
		error_log("invalid alliance race operate uid=%u operate=%u", uid, operate);
		throw runtime_error("invalid_alliance_race_operate");
	}
	DataAllianceManager::Instance()->UpdateItem(alliance);

	FillAllianceRaceOrder(uid, aid, resp->mutable_order());
	return race_order_operate_success;
}
uint32_t LogicAllianceManager::GetRaceOrderCutCdCost(uint32_t uid, uint32_t cdTs)
{
	uint32_t now = Time::GetGlobalTime();
	if(cdTs <= now)
	{
		error_log("cdTs out of limit uid=%u", uid);
		throw runtime_error("cdTs_out_of_limit");
	}
	uint32_t leftTime = cdTs - now;
	uint32_t buyCdTime = AllianceRaceCfgWrap().GetCfg().task().buy_cd_time();
	if(buyCdTime == 0)
	{
		error_log("config error uid=%u buyCdTime=%u", uid, buyCdTime);
		throw runtime_error("config_error");
	}
	uint32_t buyCdCost = AllianceRaceCfgWrap().GetCfg().task().buy_cd_cost();
	uint32_t cost = buyCdCost * ((leftTime - 1) / buyCdTime + 1);
	if(cost == 0)
	{
		error_log("param error uid=%u cost=%u", uid, cost);
		throw runtime_error("param_error");
	}
	return cost;
}

//删除商会竞赛成员订单
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceMemberDelOrder* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	unsigned type = req->type();
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceMemberDelOrderBC* m = new ProtoAlliance::RequestAllianceRaceMemberDelOrderBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_type(type);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceOrder* resp = new ProtoAlliance::ReplyAllianceRaceOrder;
			DelAllianceRaceMemberOrder(uid, aid, type,resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceMemberDelOrderBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	unsigned type = req->type();
	ProtoAlliance::ReplyAllianceRaceOrder* resp = new ProtoAlliance::ReplyAllianceRaceOrder;
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		DelAllianceRaceMemberOrder(uid, aid, type,resp);
	}
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
int LogicAllianceManager::DelAllianceRaceMemberOrder(uint32_t uid, uint32_t aid, unsigned type,ProtoAlliance::ReplyAllianceRaceOrder* resp)
{
	OtherAllianceSaveControl allianceCtl(aid);
	//更新成员数据
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(1 == type){
		//表明为看广告删除商会订单,则需恢复“接受订单次数”
		member.race_order_recv -= 1;
	}
	member.race_order_id = 0;
	member.race_order_ts = 0;
	if(!isRaceOrderFinish(member))	// 删除已完成订单不算撕单
	{
		member.race_order_cancel++;
		UpdateRaceMemberOrderLog(member, member.race_order_id, race_member_order_status_cancel);
	}
	memset(member.race_order_progress, 0, sizeof(member.race_order_progress));
	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(member);
	FillAllianceRaceOrder(uid, aid, resp);
	return 0;
}
//更新商会竞赛成员订单
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	if (IsRaceOpen() && IsAllianceId(aid) && CheckMember(aid, uid))
	{
		AddRaceOrderProgressLocal(uid, aid, req->ordertype(), req->count(), req->target());
	}
	return 0;
}
int LogicAllianceManager::AddRaceOrderProgress(uint32_t uid, uint32_t orderType, uint32_t count, uint32_t target)
{
	if(!IsRaceOpen())
	{
		return 0;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC* m = new ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->set_ordertype(orderType);
			m->set_count(count);
			m->set_target(target);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			AddRaceOrderProgressLocal(uid, aid, orderType, count, target);
		}
	}
	return 0;
}
int LogicAllianceManager::AddRaceOrderProgressLocal(uint32_t uid, uint32_t aid, uint32_t orderType, uint32_t count, uint32_t target)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//更新成员数据
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(member.race_order_id == 0 || member.race_order_ts < now || isRaceOrderFinish(member))	// 无订单,超时,已完成
	{
		return 0;
	}
	const ConfigAllianceRace::RaceTaskStorageItem& item = AllianceRaceCfgWrap().GetTaskItem(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID, member.race_order_id);
	uint32_t finishCount = 0;
	bool needSync = false;
	for(uint32_t i = 0; i < item.cond_size() && i < DataAllianceMember_race_order_progress_LENG; ++i)
	{
		const ConfigAllianceRace::RaceTaskCond& cond = item.cond(i);
		uint32_t progress = member.race_order_progress[i];
		if(progress >= cond.count())
		{
			++finishCount;
		}
		else if(cond.type() == orderType && (target == 0 || cond.id() == target))
		{
			progress += count;
			if(progress >= cond.count())
			{
				progress = cond.count();
				++finishCount;
			}
			needSync = true;
		}
		member.race_order_progress[i] = progress;
	}

	bool refreshReward = false;

/*
	if(true)	// 内网测试
	{
		finishCount = item.cond_size();
		member.race_point += 2100;
		alliance.race_point += 2100;
		refreshReward = true;
	}
*/

	if(finishCount >= item.cond_size())	// 任务完成
	{
		UpdateRaceMemberOrderLog(member, member.race_order_id, race_member_order_status_finish);
		uint32_t addPoint = item.point() * LogicVIPManager::Instance()->VIPCompetitionIntegral(member.vipLevel);
		if(member.race_point == 0 && addPoint > 0)	// 联盟积分破0后刷新所有成员阶段奖励
		{
			refreshReward = true;
		}
//		member.race_order_id = 0;
//		member.race_order_ts = 0;
		member.race_order_finish++;
		member.race_point += addPoint;
		memset(member.race_order_progress, 0xFF, sizeof(member.race_order_progress));	// 表示任务完成

		alliance.race_point += addPoint;
		DataAllianceManager::Instance()->UpdateItem(alliance);

		set<unsigned> zoneId;
		MemoryAllianceRaceGroupManager::Instance()->UpdateMemberPoint(aid, alliance.race_point, zoneId);
		for(set<unsigned>::iterator iter = zoneId.begin(); iter != zoneId.end(); ++iter)
		{
			ProtoAlliance::SetAllianceRaceGroupPointBC* m = new ProtoAlliance::SetAllianceRaceGroupPointBC;
			m->set_aid(aid);
			m->set_point(alliance.race_point);
			if(BMI->BattleConnectNoReplyByZoneID(*iter, m) != 0)
			{
				error_log("BattleConnectNoReplyByZoneID fail zoneId=%u", *iter);
				continue;
			}
		}
		debug_log("alliance_race_order_progress uid=%u,lev=%u,aid=%u,mp=%u,ap=%u,oid=%u,ots=%u,of=%u",
				uid, member.userlevel, aid, member.race_point, alliance.race_point, member.race_order_id, member.race_order_ts,member.race_order_finish);
	}
	if(needSync)
	{
		SyncRaceOrderProgress(uid, aid);
	}

	if(refreshReward)
	{
		member.race_user_level = member.userlevel;
		uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(member.race_user_level);
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 > fixId;
		RefreshMemberRaceStageReward(fixId, levelId, member);
	}
	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(member);
	return 0;
}
int LogicAllianceManager::WatchAdPlusRacePointLocal(uint32_t uid, uint32_t aid, uint32_t addPoint)
{
	//更新成员数据
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	bool refreshReward = false;
	if(member.race_point == 0 && addPoint > 0)	// 联盟积分破0后刷新所有成员阶段奖励
	{
		refreshReward = true;
	}
	member.race_point += addPoint;

	alliance.race_point += addPoint;
	DataAllianceManager::Instance()->UpdateItem(alliance);

	set<unsigned> zoneId;
	MemoryAllianceRaceGroupManager::Instance()->UpdateMemberPoint(aid, alliance.race_point, zoneId);
	for(set<unsigned>::iterator iter = zoneId.begin(); iter != zoneId.end(); ++iter)
	{
		ProtoAlliance::SetAllianceRaceGroupPointBC* m = new ProtoAlliance::SetAllianceRaceGroupPointBC;
		m->set_aid(aid);
		m->set_point(alliance.race_point);
		if(BMI->BattleConnectNoReplyByZoneID(*iter, m) != 0)
		{
			error_log("BattleConnectNoReplyByZoneID fail zoneId=%u", *iter);
			continue;
		}
	}
	debug_log("watch_ad_add_point add_point=%u,uid=%u,member_level=%u,aid=%u,member_point=%u,alliance_point=%u",
			addPoint,uid,member.userlevel,aid,member.race_point,alliance.race_point);

	if(refreshReward)
	{
		member.race_user_level = member.userlevel;
		uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(member.race_user_level);
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 > fixId;
		RefreshMemberRaceStageReward(fixId, levelId, member);
	}
	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(member);

	return 0;
}
int LogicAllianceManager::SyncRaceOrderProgress(uint32_t uid, uint32_t aid)
{
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(member.race_order_ts <= Time::GetGlobalTime())
	{
		memset(member.race_order_progress, 0, sizeof(member.race_order_progress));
		return 0;
	}
	ProtoAlliance::ReplyAllianceRaceMemberProgress* resp = new ProtoAlliance::ReplyAllianceRaceMemberProgress;
	resp->set_orderid(member.race_order_id);
	resp->set_ts(member.race_order_ts);

	if(isRaceOrderFinish(member))
	{
		resp->set_finish(1);
	}
	else
	{
		resp->set_finish(0);
	}
	for(int i = 0; i < DataAllianceMember_race_order_progress_LENG; ++i)
	{
		resp->add_progress(member.race_order_progress[i]);
	}
	const uint16_t* p = member.race_order_progress;
//	debug_log("alliance_race_order_progress uid=%u aid=%u progress=%u %u %u %u", uid, aid, p[0], p[1], p[2], p[3]);
	return LMI->sendMsg(member.id, resp) ? 0 : R_ERROR;
}


int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceBuyOrder* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		uint32_t cost = AllianceRaceCfgWrap().GetCfg().task().buy_chance_cost();
		if(cost == 0)
		{
			error_log("config error uid=%u cost=%u", uid, cost);
			throw runtime_error("config_error");
		}
		CommonGiftConfig::BaseItem base;
		base.set_cash(-1 * cost);
		userwrap.CheckBaseBeforeCost(uid, "alliance_race_buy_order", base);

		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceBuyOrderBC* m = new ProtoAlliance::RequestAllianceRaceBuyOrderBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceBuyOrder* resp = new ProtoAlliance::ReplyAllianceRaceBuyOrder;
			unsigned ret = BuyAllianceRaceOrder(uid, aid, resp);
			resp->set_ret(ret);
			resp->set_uid(uid);

			uint32_t cost = AllianceRaceCfgWrap().GetCfg().task().buy_chance_cost();
			if(cost == 0)
			{
				error_log("config error uid=%u cost=%u", uid, cost);
				throw runtime_error("config_error");
			}
			CommonGiftConfig::CommonModifyItem cfg;
			cfg.mutable_based()->set_cash(-1 * cost);
			LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_buy_order", resp->mutable_commons());

			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceBuyOrderBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	unsigned ret = 0;
	ProtoAlliance::ReplyAllianceRaceBuyOrder* resp = new ProtoAlliance::ReplyAllianceRaceBuyOrder;
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		ret = BuyAllianceRaceOrder(uid, aid, resp);
	}
	resp->set_uid(uid);
	resp->set_ret(ret);
	return BMI->BattleConnectNoReplyByAID(uid, resp);
}

int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceRaceBuyOrder* req)
{
	uint32_t ret = req->ret();
	uint32_t uid = req->uid();
	if(ret == 0)
	{
		uint32_t cost = AllianceRaceCfgWrap().GetCfg().task().buy_chance_cost();
		if(cost == 0)
		{
			error_log("config error uid=%u cost=%u", uid, cost);
			throw runtime_error("config_error");
		}
		CommonGiftConfig::CommonModifyItem cfg;
		cfg.mutable_based()->set_cash(-1 * cost);
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_buy_order", req->mutable_commons());
	}
	return LMI->sendMsg(req->uid(), req, false) ? 0 : R_ERROR;
}

int LogicAllianceManager::BuyAllianceRaceOrder(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceBuyOrder* resp)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(aid, uid);
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(member.race_order_recv < AllianceRaceCfgWrap().GetTaskChance(alliance.race_level))
	{
		error_log("alliance race order recv left uid=%u", uid);
		throw runtime_error("alliance_race_order_recv_left");
	}
	if(isFlagSet(member.flag, flag_id_race_order_buy_chance))
	{
		error_log("already buy race order uid=%u", uid);
		throw runtime_error("already_buy_race_order");
	}
	setFlag(member.flag, flag_id_race_order_buy_chance);

	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(member);

	return 0;
}



//查询竞赛奖励
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceReward* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceRewardBC* m = new ProtoAlliance::RequestAllianceRaceRewardBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceReward* resp = new ProtoAlliance::ReplyAllianceRaceReward;
			unsigned ret = FillAllianceRaceReward(uid, aid, resp);
			resp->set_ret(ret);
			resp->set_uid(uid);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceRewardBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceReward* resp = new ProtoAlliance::ReplyAllianceRaceReward;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		unsigned ret = FillAllianceRaceReward(uid, aid, resp);
		resp->set_ret(ret);
		resp->set_uid(uid);
	}
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}

//获取竞赛奖励
int LogicAllianceManager::FillAllianceRaceReward(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceReward* resp)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(aid, uid);
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	resp->set_rankid(alliance.race_rank_id);
	resp->set_userlevel(member.race_user_level);
	resp->set_olevel(alliance.race_olevel);
	resp->set_opoint(alliance.race_opoint);
	for(uint32_t i = 0; i < DataAllianceMember_race_grade_reward_LENG; ++i)
	{
		uint8_t reward = member.race_grade_reward[i].rewardId;
		if(reward == 0)
		{
			break;
		}
		resp->add_gradeid(reward);
	}
	for(uint32_t i = 0; i < DataAllianceMember_race_stage_reward_LENG; ++i)
	{
		uint8_t reward = member.race_stage_reward[i].rewardId;
		if(reward == 0)
		{
			break;
		}
		resp->add_stageid(reward);
	}

	return 0;
}






int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceTakeGradeReward* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		uint32_t cost = AllianceRaceCfgWrap().GetCfg().task().buy_chance_cost();
		if(cost == 0)
		{
			error_log("config error uid=%u cost=%u", uid, cost);
			throw runtime_error("config_error");
		}
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC* m = new ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceTakeGradeReward* resp = new ProtoAlliance::ReplyAllianceRaceTakeGradeReward;
			unsigned ret = TakeAllianceRaceGradeReward(uid, aid, resp);
			resp->set_uid(uid);
			resp->set_ret(ret);

			set<uint32_t> id;
			for(uint32_t i = 0; i < resp->id_size(); ++i)
			{
				id.insert(resp->id(i));
			}
			CommonGiftConfig::CommonModifyItem cfg;
			uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(resp->userlevel());
			AllianceRaceCfgWrap().GetGradeReward(uid, levelId, resp->rankid(), id, &cfg);
			LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_take_grade_reward", resp->mutable_commons());

			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceTakeGradeReward* resp = new ProtoAlliance::ReplyAllianceRaceTakeGradeReward;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		unsigned ret = TakeAllianceRaceGradeReward(uid, aid, resp);
		resp->set_ret(ret);
	}
	return BMI->BattleConnectNoReplyByAID(uid, resp);
}

int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceRaceTakeGradeReward* req)
{
	uint32_t ret = req->ret();
	uint32_t uid = req->uid();
	if(ret == 0)
	{
		set<uint32_t> id;
		for(uint32_t i = 0; i < req->id_size(); ++i)
		{
			id.insert(req->id(i));
		}
		CommonGiftConfig::CommonModifyItem cfg;
		uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(req->userlevel());
		AllianceRaceCfgWrap().GetGradeReward(uid, levelId, req->rankid(), id, &cfg);
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_take_grade_reward", req->mutable_commons());
	}
	return LMI->sendMsg(req->uid(), req, false) ? 0 : R_ERROR;
}
int LogicAllianceManager::TakeAllianceRaceGradeReward(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceTakeGradeReward* resp)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(aid, uid);
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(member.race_grade_reward[0].rewardId == 0)
	{
		error_log("race grade reward not exist uid=%u", uid);
		throw runtime_error("race_grade_reward_not_exist");
	}
	resp->set_userlevel(member.race_user_level);
	resp->set_rankid(alliance.race_rank_id);
	for(uint32_t i = 0; i < DataAllianceMember_race_grade_reward_LENG; ++i)
	{
		uint8_t reward = member.race_grade_reward[i].rewardId;
		if(reward == 0)
		{
			break;
		}
		resp->add_id(reward);
	}
	memset(member.race_grade_reward, 0, sizeof(member.race_grade_reward));
//	alliance.race_rank_id = 0;

	DataAllianceMemberManager::Instance()->UpdateItem(member);
	DataAllianceManager::Instance()->UpdateItem(alliance);
	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceTakeStageReward* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceTakeStageRewardBC* m = new ProtoAlliance::RequestAllianceRaceTakeStageRewardBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->mutable_id()->CopyFrom(req->id());
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceTakeStageReward* resp = new ProtoAlliance::ReplyAllianceRaceTakeStageReward;
			unsigned ret = TakeAllianceRaceStageReward(uid, aid, req->id(), resp);
			if(ret == 0)
			{
				vector<uint32_t> id;
				for(uint32_t i = 0; i < resp->id_size(); ++i)
				{
					id.push_back(resp->id(i));
				}
				CommonGiftConfig::CommonModifyItem cfg;
				uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(resp->userlevel());
				AllianceRaceCfgWrap().GetStageReward(uid, levelId, id, &cfg);
				LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_take_stage_reward", resp->mutable_commons());
			}
			resp->set_ret(ret);
			resp->set_uid(uid);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceTakeStageRewardBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceTakeStageReward* resp = new ProtoAlliance::ReplyAllianceRaceTakeStageReward;
	resp->set_uid(uid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		unsigned ret = TakeAllianceRaceStageReward(uid, aid, req->id(), resp);
		resp->set_ret(ret);
	}
	return BMI->BattleConnectNoReplyByAID(uid, resp);
}

int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceRaceTakeStageReward* req)
{
	uint32_t ret = req->ret();
	uint32_t uid = req->uid();
	if(ret == 0)
	{
		vector<uint32_t> id;
		for(uint32_t i = 0; i < req->id_size(); ++i)
		{
			id.push_back(req->id(i));
		}
		CommonGiftConfig::CommonModifyItem cfg;
		uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(req->userlevel());
		AllianceRaceCfgWrap().GetStageReward(uid, levelId, id, &cfg);
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_take_stage_reward", req->mutable_commons());
	}
	return LMI->sendMsg(req->uid(), req, false) ? 0 : R_ERROR;
}
int LogicAllianceManager::TakeAllianceRaceStageReward(uint32_t uid, uint32_t aid,
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& id, ProtoAlliance::ReplyAllianceRaceTakeStageReward* resp)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(aid, uid);
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(member.race_stage_reward[0].rewardId == 0)
	{
		error_log("race stage reward not exist uid=%u", uid);
		throw runtime_error("race_stage_reward_not_exist");
	}
	const uint32_t groupSize = ALLIANCE_RACE_STAGE_REWARD_GROUP_SIZE;
	resp->set_userlevel(member.race_user_level);

	uint32_t stageId = AllianceRaceCfgWrap().GetRaceRewardStageId(member.race_user_level, alliance.race_olevel, alliance.race_opoint);

	for(uint32_t i = 0; i < id.size() && i < stageId; ++i)
	{
		uint32_t rewardIdx = i * groupSize + id.Get(i);
		if(rewardIdx < DataAllianceMember_race_stage_reward_LENG)
		{
			uint8_t reward = member.race_stage_reward[rewardIdx].rewardId;
			if(reward == 0)
			{
				break;
			}
			resp->add_id(reward);
		}
	}
	memset(member.race_stage_reward, 0, sizeof(member.race_stage_reward));
	DataAllianceMemberManager::Instance()->UpdateItem(member);

	return 0;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceRefreshStageReward* req)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		CommonGiftConfig::BaseItem base;
		base.set_cash(-1 * AllianceRaceCfgWrap().GetCfg().stage_reward_refresh_cost());
		userwrap.CheckBaseBeforeCost(uid, "alliance_race_refresh_stage_reward", base);
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC* m = new ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC;
			m->set_aid(aid);
			m->set_uid(uid);
			m->mutable_id()->CopyFrom(req->id());
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceRefreshStageReward* resp = new ProtoAlliance::ReplyAllianceRaceRefreshStageReward;
			unsigned ret = RefreshAllianceRaceStageReward(uid, aid, req->id(), resp);
			if(ret == 0)
			{
				vector<uint32_t> id;
				for(uint32_t i = 0; i < resp->id_size(); ++i)
				{
					id.push_back(resp->id(i));
				}
				CommonGiftConfig::CommonModifyItem cfg;
				cfg.mutable_based()->set_cash(-1 * AllianceRaceCfgWrap().GetCfg().stage_reward_refresh_cost());
//				AllianceRaceCfgWrap().GetStageReward(resp->levelid(), id, &cfg);
				LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_refresh_stage_reward", resp->mutable_commons());
			}
			resp->set_uid(uid);
			resp->set_ret(ret);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceRefreshStageReward* resp = new ProtoAlliance::ReplyAllianceRaceRefreshStageReward;
	unsigned ret = 0;
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		ret = RefreshAllianceRaceStageReward(uid, aid, req->id(), resp);
	}
	resp->set_uid(uid);
	resp->set_ret(ret);
	return BMI->BattleConnectNoReplyByUID(uid, resp);
}

int LogicAllianceManager::Process(ProtoAlliance::ReplyAllianceRaceRefreshStageReward* req)
{
	uint32_t ret = req->ret();
	uint32_t uid = req->uid();
	if(ret == 0)
	{
		vector<uint32_t> id;
		for(uint32_t i = 0; i < req->id_size(); ++i)
		{
			id.push_back(req->id(i));
		}
		CommonGiftConfig::CommonModifyItem cfg;
		cfg.mutable_based()->set_cash(-1 * AllianceRaceCfgWrap().GetCfg().stage_reward_refresh_cost());
//		AllianceRaceCfgWrap().GetStageReward(req->levelid(), id, &cfg);
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "alliance_race_refresh_stage_reward", req->mutable_commons());
	}
	return LMI->sendMsg(req->uid(), req, false) ? 0 : R_ERROR;
}
int LogicAllianceManager::RefreshAllianceRaceStageReward(uint32_t uid, uint32_t aid,
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& id, ProtoAlliance::ReplyAllianceRaceRefreshStageReward* resp)
{
	uint32_t now = Time::GetGlobalTime();
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(aid, uid);
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	if(member.race_stage_reward[0].rewardId == 0)
	{
		error_log("race stage reward not exist uid=%u", uid);
		throw runtime_error("race_stage_reward_not_exist");
	}

	uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(member.race_user_level);
	RefreshMemberRaceStageReward(id, levelId, member);


	for(uint32_t i = 0; i < DataAllianceMember_race_stage_reward_LENG; ++i)
	{
		if(member.race_stage_reward[i].rewardId == 0)
		{
			break;
		}
		resp->add_id(member.race_stage_reward[i].rewardId);
	}

	return 0;
}
int LogicAllianceManager::RefreshMemberRaceStageReward(const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& id,
		uint32_t levelId, DataAllianceMember & member)
{

	uint32_t race_level = 5; // 测试代码,输入参数

	uint8_t race_stage_reward[DataAllianceMember_race_stage_reward_LENG]; // 阶段奖励
	memset(race_stage_reward, 0, sizeof(race_stage_reward));

	const uint32_t groupSize = ALLIANCE_RACE_STAGE_REWARD_GROUP_SIZE;
	for(uint32_t i = 0; i < id.size(); ++i)
	{
		uint32_t rewardIdx = 0;
		if(id.Get(i) < groupSize && (rewardIdx = i * groupSize + id.Get(i)) < DataAllianceMember_race_stage_reward_LENG)	//
		{
			race_stage_reward[rewardIdx] = member.race_stage_reward[rewardIdx].rewardId;
		}
	}
	AllianceRaceCfgWrap().RefreshStageReward(levelId, race_stage_reward, DataAllianceMember_race_stage_reward_USED);


	for(uint32_t i = 0; i < DataAllianceMember_race_stage_reward_LENG; ++i)
	{
		if(race_stage_reward[i] == 0)
		{
			break;
		}
		member.race_stage_reward[i].rewardId = race_stage_reward[i];
	}
	DataAllianceMemberManager::Instance()->UpdateItem(member);

	return 0;
}
/*
int LogicAllianceManager::RefreshAllMemberRaceStageReward(uint32_t aid)
{
	vector<unsigned> indexs;
	DataAllianceMemberManager::Instance()->GetIndexs(aid, indexs);

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
		uint32_t levelId = AllianceRaceCfgWrap().GetRaceRewardLevelId(member.userlevel);
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 > fixId;
		RefreshMemberRaceStageReward(fixId, levelId, member);
	}
	return 0;
}
*/
int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceMemberOrderLog* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceMemberOrderLogBC* m = new ProtoAlliance::RequestAllianceRaceMemberOrderLogBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceMemberOrderLog* resp = new ProtoAlliance::ReplyAllianceRaceMemberOrderLog;
			FillAllianceRaceMemberOrderLog(uid, aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceMemberOrderLogBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceMemberOrderLog* resp = new ProtoAlliance::ReplyAllianceRaceMemberOrderLog;
	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		FillAllianceRaceMemberOrderLog(uid, aid, resp);
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
int LogicAllianceManager::FillAllianceRaceMemberOrderLog(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceMemberOrderLog* resp)
{
	uint32_t now = Time::GetGlobalTime();
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	vector<unsigned> indexs;
	DataAllianceMemberManager::Instance()->GetIndexs(aid, indexs);
	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
		ProtoAlliance::AllianceRaceMemberOrderLogItem* logItem = resp->add_member();
		logItem->set_uid(member.id);
		logItem->set_finish(member.race_order_finish);
		logItem->set_max(getRaceOrderMaxChance(uid, alliance.race_level, member.flag, member.vipLevel));
		logItem->set_cancel(member.race_order_cancel);
		logItem->set_point(member.race_point);
		logItem->set_level(member.userlevel);
		logItem->set_fig(member.fig);
	}
	return 0;
}






int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRacePersonOrderLog* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRacePersonOrderLogBC* m = new ProtoAlliance::RequestAllianceRacePersonOrderLogBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRacePersonOrderLog* resp = new ProtoAlliance::ReplyAllianceRacePersonOrderLog;
			FillAllianceRacePersonOrderLog(uid, aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRacePersonOrderLogBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRacePersonOrderLog* resp = new ProtoAlliance::ReplyAllianceRacePersonOrderLog;
	OtherAllianceSaveControl allianceCtl(aid);
	if (IsAllianceId(aid) && CheckMember(aid, uid))
		FillAllianceRacePersonOrderLog(uid, aid, resp);
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}
int LogicAllianceManager::FillAllianceRacePersonOrderLog(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRacePersonOrderLog* resp)
{
	uint32_t now = Time::GetGlobalTime();
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	CheckMember(aid, uid);
	DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	DataAllianceMemberRaceOrderLog race_order_log[DataAllianceMember_race_order_log_LENG]; // 任务日志
	for(int i = 0; i < DataAllianceMember_race_order_log_LENG; ++i)
	{
		if(member.race_order_log[i].id != 0)
		{
			ProtoAlliance::AllianceRacePersonOrderLogItem* logItem = resp->add_member();
			logItem->set_id(member.race_order_log[i].id);
			logItem->set_status(member.race_order_log[i].status);
		}
		else
		{
			break;
		}
	}

	return 0;
}
uint32_t LogicAllianceManager::GetRaceOverTs()
{
	return MemoryAllianceRaceGroupManager::Instance()->GetTs() + ALLIANCE_RACE_GAME_TIME;
}
int LogicAllianceManager::ResetRaceMemberInfo(DataAllianceMember & member)
{
	member.race_point = 0;
	member.race_order_id = 0;
	member.race_order_ts = 0;
	member.race_order_recv = 0;
	member.race_order_finish = 0;
	member.race_order_cancel = 0;
	memset(member.race_order_log, 0, sizeof(member.race_order_log));
	memset(member.race_order_progress, 0, sizeof(member.race_order_progress));
}
bool LogicAllianceManager::IsRaceOpen()
{
	return MemoryActivityManager::Instance()->IsOn(memory_activity_id_alliance_race);
}
int LogicAllianceManager::CheckRace(uint32_t aid)
{
	if(!IsRaceOpen())
	{
		return 0;
	}
	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	if(!Time::IsToday(alliance.active_ts))	// 活跃时间一天一置
	{
		alliance.active_ts = Time::GetGlobalTime();
		DataAllianceManager::Instance()->UpdateItem(alliance);
	}
	if(alliance.race_ts != MemoryAllianceRaceGroupManager::Instance()->GetTs())
	{
		alliance.race_ts = MemoryAllianceRaceGroupManager::Instance()->GetTs();
		alliance.race_point = 0;
		for(uint32_t i = 0; i < DataAlliance_race_order_id_LENG; ++i)
		{
			if((alliance.race_order_id[i] = AllianceRaceCfgWrap().GetRandTaskId(ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID)) > 0)
			{
				alliance.race_order_cd[i] = 0; //now + AllianceRaceCfgWrap().GetCfg().task().cdtime();
			}
		}
		DataAllianceManager::Instance()->UpdateItem(alliance);

		vector<unsigned> indexs;
		DataAllianceMemberManager::Instance()->GetIndexs(aid, indexs);
		for(size_t i = 0; i < indexs.size(); ++i)
		{
			DataAllianceMember & member = DataAllianceMemberManager::Instance()->GetDataByIndex(indexs[i]);
			if(IsInRace(member.join_ts))
			{
				ResetRaceMemberInfo(member);
				DataAllianceMemberManager::Instance()->UpdateItem(member);
			}
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::SetAllianceRaceGroupPointBC* req)
{
	if(IsRaceOpen())
	{
		unsigned aid = req->aid();
		unsigned point = req->point();
		MemoryAllianceRaceGroupManager::Instance()->UpdateMemberPointLocal(aid, point);
	}
	return 0;
}


int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceGroupMember* req)
{
	if(!IsRaceOpen())
	{
		error_log("alliance_race_closed uid=%u", uid);
		return -1;
	}
	DBCUserBaseWrap userwrap(uid);
	unsigned aid = userwrap.Obj().alliance_id;
	if (IsAllianceId(aid))
	{
		if(CMI->IsNeedConnectByAID(aid))
		{
			ProtoAlliance::RequestAllianceRaceGroupMemberBC* m = new ProtoAlliance::RequestAllianceRaceGroupMemberBC;
			m->set_aid(aid);
			m->set_uid(uid);
			return BMI->BattleConnectNoReplyByAID(aid, m);
		}
		else if(CheckMember(aid, uid))
		{
			ProtoAlliance::ReplyAllianceRaceGroupMember* resp = new ProtoAlliance::ReplyAllianceRaceGroupMember;
			MemoryAllianceRaceGroupManager::Instance()->FillMember(aid, resp);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
	}
	return 0;
}
int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceGroupMemberBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	ProtoAlliance::ReplyAllianceRaceGroupMember* resp = new ProtoAlliance::ReplyAllianceRaceGroupMember;
	if (IsAllianceId(aid) && CheckMember(aid, uid))
	{
		MemoryAllianceRaceGroupManager::Instance()->FillMember(aid, resp);
	}
	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}

int LogicAllianceManager::Process(unsigned uid, ProtoAlliance::RequestAllianceRaceWatchAd* req)
{
	unsigned type = req->type();
	ProtoAlliance::ReplyAllianceRaceWatchAd* resp = new ProtoAlliance::ReplyAllianceRaceWatchAd;
	if(watch_ad_type_0 == type)			//观看广告
	{
		if(!IsRaceOpen())
		{
			resp->set_ret(watch_ad_result_3);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
		else if(m_set_watch_ad.count(uid))
		{
			resp->set_ret(watch_ad_result_1);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
		DBCUserBaseWrap userwrap(uid);
		unsigned aid = userwrap.Obj().alliance_id;
		if(!IsAllianceId(aid))
		{
			resp->set_ret(watch_ad_result_4);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
		else
		{
			resp->set_ret(watch_ad_result_0);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}

	}
	else if(watch_ad_type_1 == type)	//领取奖励
	{
		unsigned diamond = 0;
		unsigned point = 0;
		unsigned count = 0;
		AllianceRaceCfgWrap().GetWatchAdReward(diamond,point,count);

		if(!IsRaceOpen())
		{
			resp->set_ret(watch_ad_result_3);
			return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
		}
		m_set_watch_ad.insert(uid);
		DBCUserBaseWrap userwrap(uid);
		unsigned aid = userwrap.Obj().alliance_id;
		if (IsAllianceId(aid))
		{
			if(CMI->IsNeedConnectByAID(aid))
			{
				ProtoAlliance::RequestAllianceRaceWatchAdBC* msg = new ProtoAlliance::RequestAllianceRaceWatchAdBC;
				msg->set_aid(aid);
				msg->set_uid(uid);
				msg->set_point(point);
				BMI->BattleConnectNoReplyByAID(aid, msg);
			}
			else if(CheckMember(aid, uid))
			{
				WatchAdPlusRacePointLocal(uid, aid, point);
			}
		}
		resp->set_point(point);

		//加钻石
		DBCUserBaseWrap user(uid);
		user.AddCash(diamond,"watch_ad_in_alliance_race");
		DataCommon::CommonItemsCPP *common = resp->mutable_commons();
		DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
		base->set_change(diamond);
		base->set_totalvalue(user.GetCash());
		base->set_type(type_cash);

		resp->set_ret(watch_ad_result_2);
		return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
	}

	return 0;
}

int LogicAllianceManager::Process(ProtoAlliance::RequestAllianceRaceWatchAdBC* req)
{
	unsigned aid = req->aid();
	unsigned uid = req->uid();
	unsigned point = req->point();
	if (IsRaceOpen() && IsAllianceId(aid) && CheckMember(aid, uid))
	{
		WatchAdPlusRacePointLocal(uid, aid, point);
	}

	return 0;
}


bool LogicAllianceManager::UpdateWatchAd()
{
	m_set_watch_ad.clear();
	return true;
}

bool LogicAllianceManager::isFlagSet(uint8_t flag, uint8_t id)
{
	return ((flag >> id) & 0x1) > 0;
}

void LogicAllianceManager::setFlag(uint8_t& flag, uint8_t id)
{
	flag |= (0x1 << id);
}
bool LogicAllianceManager::isRaceOrderFinish(DataAllianceMember& member)
{
	for(int i = 0; i < DataAllianceMember_race_order_progress_LENG; ++i)
	{
		if(member.race_order_progress[i] != 0xFFFF)
		{
			return false;
		}
	}
	return true;
}
uint8_t LogicAllianceManager::getRaceOrderMaxChance(uint32_t uid, uint8_t raceLevel, uint8_t flag, uint32_t vipLevel)
{
	return (isFlagSet(flag, flag_id_race_order_buy_chance) ? 1 : 0)
			+ AllianceRaceCfgWrap().GetTaskChance(raceLevel)
			+ LogicVIPManager::Instance()->VIPAllianceCompetition(vipLevel);
}
int LogicAllianceManager::AddRaceMemberOrderLog(DataAllianceMember& member, uint16_t id, uint8_t status)
{
	for(uint32_t i = 0; i < DataAllianceMember_race_order_log_LENG; ++i)
	{
		if(member.race_order_log[i].id == 0)
		{
			member.race_order_log[i].id = id;
			member.race_order_log[i].status = status;
			if(i > 0)	// 将前一个正在进行的任务置为取消状态
			{
				uint32_t pre = i - 1;
				if(member.race_order_log[pre].id > 0 && member.race_order_log[pre].status == race_member_order_status_doing)
				{
					member.race_order_log[pre].status = race_member_order_status_cancel;
				}
			}
			break;
		}
	}
}
int LogicAllianceManager::UpdateRaceMemberOrderLog(DataAllianceMember& member, uint16_t id, uint8_t status)
{
	if(DataAllianceMember_race_order_log_LENG <= 0)
	{
		return 0;
	}
	for(int32_t i = DataAllianceMember_race_order_log_LENG - 1; i >= 0; --i)
	{
		if(member.race_order_log[i].id > 0)	// 当前订单
		{
			if(member.race_order_log[i].id == id)
			{
				member.race_order_log[i].status = status;
			}
			break;
		}
	}
}
int LogicAllianceManager::UpdateMemberNow(unsigned uid)
{
	//成员下线，检查该成员的商会
	DBCUserBaseWrap userwrap(uid);
	unsigned alliance_id = userwrap.Obj().alliance_id;
	if(!IsAllianceId(alliance_id))
		return 0;

	unsigned helpTs = LogicUserManager::Instance()->IsUserNeedHelp(uid) ? Time::GetGlobalTime() : 0;
	unsigned onlineTs = UserManager::Instance()->IsOnline(uid) ? Time::GetGlobalTime() : 0;
	try
	{
		UpdateMember(uid, alliance_id, onlineTs, helpTs, userwrap.Obj().level, userwrap.Obj().name, userwrap.Obj().viplevel);
	}
	catch(runtime_error &e)
	{
		return R_ERROR;
	}

	return 0;
}
int LogicAllianceManager::UpdateMember(unsigned uid, unsigned aid, unsigned onlineTs, unsigned helpTs, unsigned level, const string& name, unsigned vipLevel)
{
	if(CMI->IsNeedConnectByAID(aid))
	{
		ProtoAlliance::RequestUpdateMemberBC* m = new ProtoAlliance::RequestUpdateMemberBC;
		m->set_aid(aid);
		m->set_uid(uid);
		m->set_onlinets(onlineTs);
		m->set_helpts(helpTs);
		m->set_level(level);
		m->set_name(name);
		m->set_viplevel(vipLevel);
		return BMI->BattleConnectNoReplyByAID(aid, m);
	}
	else
	{
		UpdateMemberLocal(uid, aid, onlineTs, helpTs, level, name, vipLevel);
	}
	return 0;
}
int LogicAllianceManager::UpdateMemberHelpTs(unsigned uid)
{
	return UpdateMemberNow(uid);
}
int LogicAllianceManager::UpdateMemberLevel(unsigned uid)
{
	return UpdateMemberNow(uid);
}
int LogicAllianceManager::UpdateMemberLocal(unsigned uid, unsigned aid, unsigned onlineTs, unsigned helpTs, unsigned level, const string& name, unsigned vipLevel)
{
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(uid, aid, uid);

	//获取自身的数据
	DataAllianceMember & selfmember = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	selfmember.onlineTs = onlineTs;
	selfmember.helpTs = helpTs;
	selfmember.userlevel = level;
	selfmember.vipLevel = vipLevel;
	snprintf(selfmember.username, sizeof(selfmember.username), name.c_str());
	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(selfmember);

	MemoryAllianceManager::Instance()->UpdateOnlineNum(aid, DataAllianceMemberManager::Instance()->GetMemberOnlineNum(aid));

	DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
	alliance.active_ts = Time::GetGlobalTime();
	DataAllianceManager::Instance()->UpdateItem(alliance);

	return 0;
}
int LogicAllianceManager::AddMemberHelpTimesLocal(unsigned uid, unsigned aid)
{
	OtherAllianceSaveControl allianceCtl(aid);
	//成员校验
	CheckMember(uid, aid, uid);

	//获取自身的数据
	DataAllianceMember & selfmember = DataAllianceMemberManager::Instance()->GetData(aid, uid);
	++selfmember.helptimes;
	//更新
	DataAllianceMemberManager::Instance()->UpdateItem(selfmember);
	return 0;
}

bool LogicAllianceManager::AddInviteDyInfo(uint32_t uid,uint32_t other_uid,uint32_t aid)
{
	//uid:邀请者,other_uid:被邀请者
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	pattach->product_id = aid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_INVITE_ALLIANCE,pattach))
	{
		return true;
	}
	return false;
}

bool LogicAllianceManager::AddInviteDyInfoOverServer(uint32_t uid,uint32_t other_uid,uint32_t aid)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_INVITE_ALLIANCE);
	msg->set_productid(aid);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}

