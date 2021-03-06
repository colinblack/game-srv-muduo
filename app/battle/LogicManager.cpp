#include <pthread.h>
#include <time.h>
#include <sys/select.h>
#include <stdarg.h>

#include "ServerInc.h"
#include "BattleServer.h"
#include "Util.h"

uint32_t LogicManager::ServerId = 0;
uint32_t LogicManager::SecOpenTime = 0;
uint32_t LogicManager::GlobalMilliTime = 0;
uint64_t LogicManager::StartMilliTime = 0;
bool LogicManager::IsClosed = false;
bool LogicManager::IsPreClosed = false;
bool LogicManager::NeedReloadConfig = false;
int LogicManager::m_signum = 0;

#define PER_FRAME_TIME 10

LogicManager::LogicManager():m_fd(0),channelId(-1),m_timer(0),m_last_hour_ts(0),lastLoopTime(0),pReplyProtocol(NULL),m_pConn(NULL), needDelReply(true),dispatcher(ProtoManager::DisCardMessage){}

bool LogicManager::Initialize(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	StartMilliTime = tv.tv_sec * 1000 + tv.tv_usec/1000;
	ServerId = Config::GetIntValue(CONFIG_SRVID);
	SecOpenTime = Config::GetIntValue(CONFIG_OPENTIME);
	m_last_hour_ts = time(NULL);
	m_timer = m_last_hour_ts % 60;
	unsigned base_buff = Config::GetIntValue(CONFIG_BASE);
	if(base_buff)
		DataSingletonBase::BASE_BUFF = base_buff;

	if(!ConfigManager::Instance()->Inited())
	{
		error_log("ConfigManager Init error!");
		return false;
	}

	RegProto();
	RegDataManager();
	RegMemoryManager();
	RegBattleManager();
	RegActivityManager();

	int ret = 0;
	ObjInit(m_dataManager);
	ObjInit(m_memoryManager);
	ObjInit(m_battleManager);
	ObjInit(m_activityManager);
	
	BattleServer::Instance()->SetTimerCB(std::bind(&LogicManager::onTimer2, this), 1.0);
	BattleServer::Instance()->SetTimerCB(std::bind(&LogicManager::timerProcess, this), 1.0);
	return true;
}

bool  LogicManager::sendMsgFD(unsigned fd, Message* msg, bool delmsg)
{
	CFirePacket* packet = new CFirePacket(PROTOCOL_ACCESS_SEND, delmsg);
	packet->channelId = channelId;
	packet->fd = fd;
	packet->m_msg = msg;

	if(!packet->delmsg)
	{
		packet->delmsg = true;
	}

	SendData(packet);
	return true;
}

bool LogicManager::SendData(CFirePacket* packet, TcpConnectionPtr conn){
	if(conn == nullptr)
		conn = m_pConn;
	if(packet == nullptr || conn == nullptr)
	{
		return false;
	}
	if(!packet->delmsg)
	{
		packet->delmsg = true;
		Message* m = packet->m_msg->New();
		m->MergeFrom(*packet->m_msg);
		packet->m_msg = m;
	}
	static Buffer sendBuffer;
	sendBuffer.retrieveAll();
	if(!packet->Encode(&sendBuffer)){
		error_log("[channel_closed][channelid=%d,fd=%u,time=%u,cmd=%u,bodyLen=%u]",
				packet->channelId, packet->fd, (int)packet->time, packet->cmd,packet->bodyLen);
		return false;
	}

	debug_log("readablesize=%d", sendBuffer.readableBytes());	
	conn->send(&sendBuffer);
	return true;
}


bool LogicManager::sendMsg(unsigned uid, Message* msg, bool delmsg)
{
	if(m_pConn == NULL){
		return false;
	}

	if(CMI->IsNeedConnectByUID(uid))
	{
		auto conn = BattleServer::Instance()->GetBattleClient(uid);
		if(conn == nullptr){
			return false;
		}
		CFirePacket* packet = new CFirePacket(PROTOCOL_EVENT_BATTLE_FORWARD, delmsg);
		packet->m_msg = msg;
		packet->fd = uid;
		packet->uid = Config::GetZoneByUID(uid);
		SendData(packet, conn);
		return true;
	}

	debug_log("sendMsg, channelId=%d", channelId);
	if(channelId == -1)
	{
		if(delmsg)
			delete msg;
		return false;
	}
	unsigned fd = Getfd(uid);
	if(fd == -1)
	{
		if(delmsg)
			delete msg;
		debug_log("sendMsg, fd=-1");
		return false;
	}
	string msgname = msg->GetTypeName();
	bool flag = sendMsgFD(fd, msg, delmsg);
	if(flag)
		debug_log("[MSGLOG][SEND]uid=%u,fd=%d,name=%s", uid, fd, msgname.c_str());
	else
		error_log("[send error]uid=%u,fd=%d,name=%s", uid, fd, msgname.c_str());
	return flag;
}
bool LogicManager::sendMsgGroup(set<unsigned>& uid, Message* msg, bool delmsg)
{
	if(channelId == -1)
	{
		if(delmsg)
			delete msg;
		return false;
	}

	set<unsigned> *fds = new set<unsigned>, del;
	for(set<unsigned>::iterator it=uid.begin();it!=uid.end();++it)
	{
		unsigned fd = Getfd(*it);
		if(fd == -1)
		{
			del.insert(*it);
			continue;
		}
		fds->insert(fd);
	}
	for(set<unsigned>::iterator it=del.begin();it!=del.end();++it)
		uid.erase(*it);
	if(fds->empty())
	{
		if(delmsg)
			delete msg;
		delete fds;
		return false;
	}

	CFirePacket* packet = new CFirePacket(PROTOCOL_ACCESS_GROUP_SEND, delmsg);
	packet->channelId = channelId;
	packet->m_msg = msg;
	packet->group = fds;

	string msgname = msg->GetTypeName();
	bool flag = SendData(packet);
	if(flag)
		debug_log("[MSGLOG][GROUP]name=%s", msgname.c_str());
	else
		error_log("[send error]name=%s", msgname.c_str());
	return flag;
}
bool LogicManager::broadcastMsg(Message* msg)
{
  	if(channelId == -1)
	{
		delete msg;
		return false;
	}
	CFirePacket* packet = new CFirePacket(PROTOCOL_ACCESS_BROAD_CAST);
	packet->channelId = channelId;
	packet->m_msg = msg;

	string msgname = msg->GetTypeName();
	
	bool flag = SendData(packet);
	if(flag)
		debug_log("[MSGLOG][BROADCAST]name=%s", msgname.c_str());
	else
		error_log("[send error]name=%s", msgname.c_str());
	return flag;
}
void LogicManager::process(const muduo::net::TcpConnectionPtr& conn, CFirePacket* packet)
{
	debug_log("len=%d, cmd=%d, channelid=%d, fd=%d", packet->bodyLen, packet->cmd, packet->channelId, packet->fd);
	if(packet->cmd < PROTOCOL_ACCESS_LOFFLINE){
		channelId = packet->channelId;
		m_fd = packet->fd;
	}

	switch(packet->cmd){
	case PROTOCOL_ACCESS_TRANSFER:
		clientProcess(packet);
		break;
	case PROTOCOL_DELIVER:
		deliverProcess(packet);
		break;
	case PROTOCOL_ADMIN:
		adminProcess(packet);
		break;
	case PROTOCOL_ACCESS_HEARBEAT:
		heartProcess(packet);
		break;
	case PROTOCOL_ACCESS_COFFLINE:
		preOffline(packet);
		break;
	case PROTOCOL_BOT:
		botProcess(packet);
		break;
	case PROTOCOL_EVENT_BATTLE_CONNECT:
		battleProcess(packet);
		break;
	case PROTOCOL_EVENT_BATTLE_FORWARD:
		forwardProcess(packet);
		break;
	default:
		error_log("unknown cmd:%u",packet->cmd);
		break;
	}
}

void LogicManager::clientProcess(CFirePacket* packet){
	if(IsPreClosed)
		return;

	int ret = 0;
	packet->uid = Getuid(packet->fd);
	m_errmsg.clear();
	if(!IsValidUid(packet->uid) && packet->m_msg->GetTypeName() != "Common.Login")
	{
		debug_log("[MSGLOG][RECV]kick not login, fd=%u, cmd=%s", packet->fd, packet->m_msg->GetTypeName().c_str());
		sendKickMsg(packet->fd, "not_login");
		return;
	}
	debug_log("[MSGLOG][RECV]uid=%u,fd=%d,name=%s", packet->uid, packet->fd, packet->m_msg->GetTypeName().c_str());

	ret = dispatcher.onMessage(packet->m_msg);

	if(ret != 0){
		error_log("client process failed, uid=%u, ret=%d, msg=%s, cmd=%s", packet->uid, ret, m_errmsg.c_str(), packet->m_msg->GetTypeName().c_str());
		ErrorRet::ErrorRet* reply = new ErrorRet::ErrorRet;
		reply->set_errorret(ret);
		reply->set_errormsg(m_errmsg);
		reply->set_requestmsg(packet->m_msg->GetTypeName());
		pReplyProtocol = reply;
		needDelReply = true;
	}
	else if(IsValidUid(packet->uid))
	{
		unsigned index = BaseManager::Instance()->GetIndex(packet->uid);
		DataBase &base = BaseManager::Instance()->m_data->data[index];
		base.last_active_time = Time::GetGlobalTime();
	}

	if(pReplyProtocol != NULL){
		CFirePacket* rspPacket = new CFirePacket(PROTOCOL_ACCESS_ANSWER, needDelReply);
		rspPacket->fd = packet->fd;
		rspPacket->time = packet->time;
		rspPacket->microTime = packet->microTime;
		rspPacket->channelId = packet->channelId;
		rspPacket->m_msg = pReplyProtocol;

		string msgname =  pReplyProtocol->GetTypeName();
		SendData(rspPacket);
		debug_log("[MSGLOG][SEND]uid=%u,fd=%d,name=%s", packet->uid, packet->fd, msgname.c_str());
		delete rspPacket;
		pReplyProtocol = NULL;
	}
	needDelReply = true;
}

void LogicManager::deliverProcess(CFirePacket* packet){
	if(IsPreClosed)
		return;

	int ret = 0;
	Common::Pay* msg = (Common::Pay*)packet->m_msg;
	unsigned uid = msg->uid();
	debug_log("[MSGLOG][RECV]uid=%u,fd=%d,name=%s", uid, packet->fd, packet->m_msg->GetTypeName().c_str());

	string name;
	unsigned ts = msg->ts();
	if(ts - 300 > Time::GetGlobalTime() || ts + 300 < Time::GetGlobalTime())
		ret =  R_ERR_PARAM;
	if(ret != 0)
		error_log("deliver process failed, ret=%d",ret);

	Common::ReplyPay* reply = new Common::ReplyPay;
	reply->set_ret(ret);
	reply->set_name(name);
	CFirePacket* rspPacket = new CFirePacket(PROTOCOL_DELIVER);
	rspPacket->fd = packet->fd;
	rspPacket->time = packet->time;
	rspPacket->microTime = packet->microTime;
	rspPacket->channelId = packet->channelId;
	rspPacket->m_msg = reply;

	string msgname = reply->GetTypeName();
	if(!SendData(rspPacket))
		error_log("send rsp failed:uid=%u,fd=%d,name=%s", uid, packet->fd, msgname.c_str());
	else
		debug_log("[MSGLOG][SEND]uid=%u,fd=%d,name=%s", uid, packet->fd, msgname.c_str());

	if(ret == 0)
	{
		unsigned cash = msg->cash();
		unsigned totalCash = 0;
		if(UMI->IsOnline(uid))
		{
			unsigned index = BaseManager::Instance()->GetIndex(uid);
			DataBase &base = BaseManager::Instance()->m_data->data[index];
			COINS_LOG("[%s][uid=%u,ocash=%u,ncash=%u,chgcash=%d]", "DELIVER", uid, base.cash, base.cash+cash, cash);
			base.cash += cash;
			base.acccharge += cash;
			totalCash = base.cash;

			//@add oulong 20161009 ??????
	//		if (base.acccharge >= 100 && base.first_recharge == 0)
	//		{
	//			//base.first_recharge = 1;
	//		}

			BaseManager::Instance()->m_data->MarkChange(index);

			unsigned itemid = 0;
			if(msg->has_itemid())
			{
				itemid = msg->itemid();
			}
			LogicUserManager::Instance()->NotifyRecharge(uid, cash,itemid);

			DoDataManagerSave(uid);
		}
		else
		{
			AsynManager::Instance()->Add(uid, e_asyn_cash, cash);
			AsynManager::Instance()->Add(uid, e_asyn_accrecharge, cash);
		}

		//??????????????????????????????
		if(LogicSysMailManager::Instance()->ValidPlatForm(uid))
		{
			LogicSysMailManager::Instance()->FirstRechargeAddMail(uid);
		}

		//?????????????????????
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
		{
			unsigned itemid = msg->itemid();;
			unsigned charge = msg->currency() / 100;
			string orderid = msg->tradeno();
			string channelOrderId = msg->channeltradeno();

			LogicXsgReportManager::Instance()->XSGRechargeReport(uid,CTrans::ITOS(itemid),charge,orderid,channelOrderId);
			LogicXsgReportManager::Instance()->XSGRechargeGetDiamondReport(uid,cash,totalCash);
		}
	}
}

void LogicManager::adminProcess(CFirePacket* packet){
	if(IsPreClosed)
		return;

	int ret = 0;
	m_errmsg.clear();

	ret = dispatcher.onMessage(packet->m_msg);

	if(ret != 0){
		error_log("admin process failed, ret=%d, msg=%s",ret, m_errmsg.c_str());
		ErrorRet::ErrorRet* reply = new ErrorRet::ErrorRet;
		reply->set_errorret(ret);
		reply->set_errormsg(m_errmsg);
		reply->set_requestmsg(packet->m_msg->GetTypeName());
		pReplyProtocol = reply;
		needDelReply = true;
	}

	if(pReplyProtocol != NULL){
		CFirePacket* rspPacket = new CFirePacket(PROTOCOL_ADMIN, needDelReply);
		rspPacket->fd = packet->fd;
		rspPacket->time = packet->time;
		rspPacket->microTime = packet->microTime;
		rspPacket->channelId = packet->channelId;
		rspPacket->m_msg = pReplyProtocol;

		string msgname = pReplyProtocol->GetTypeName();
		if(!SendData(rspPacket))
			error_log("send rsp failed:fd=%d,name=%s", packet->fd, msgname.c_str());

		pReplyProtocol = NULL;
	}
	needDelReply = true;
}

void LogicManager::botProcess(CFirePacket* packet){
	if(IsPreClosed)
		return;

	m_errmsg.clear();

	int ret = dispatcher.onMessage(packet->m_msg);

	/*if(packet->m_msg->GetTypeName() == "Bot.RequestLogin")
	{
		Bot::ReplyLogin* reply = new Bot::ReplyLogin;
		reply->set_ret(ret);
		pReplyProtocol = reply;
	}*/

	if(pReplyProtocol != NULL){
		CFirePacket* rspPacket = new CFirePacket(PROTOCOL_BOT, needDelReply);
		rspPacket->fd = packet->fd;
		rspPacket->time = packet->time;
		rspPacket->microTime = packet->microTime;
		rspPacket->channelId = packet->channelId;
		rspPacket->m_msg = pReplyProtocol;

		string msgname = pReplyProtocol->GetTypeName();
		if(!SendData(rspPacket))
			error_log("send rsp failed:fd=%d,name=%s", packet->fd, msgname.c_str());

		pReplyProtocol = NULL;
	}
	needDelReply = true;
}

void LogicManager::battleProcess(CFirePacket* packet){
	if(IsPreClosed)
		return;

	int ret = 0;
	m_errmsg.clear();

	ret = dispatcher.onMessage(packet->m_msg);

	if(ret != 0){
		error_log("battle process failed, cmd=%s ret=%d, msg=%s", packet->m_msg->GetTypeName().c_str(), ret, m_errmsg.c_str());
		/*??????????????????
		ErrorRet::ErrorRet* reply = new ErrorRet::ErrorRet;
		reply->set_errorret(ret);
		reply->set_errormsg(m_errmsg);
		reply->set_requestmsg(packet->m_msg->GetTypeName());
		pReplyProtocol = reply;
		needDelReply = true;
		*/
	}

	/*??????????????????
	if(pReplyProtocol != NULL){
		CFirePacket rspPacket(PROTOCOL_EVENT_BATTLE_CONNECT, needDelReply);
		rspPacket.fd = packet->fd;
		rspPacket.time = packet->time;
		rspPacket.microTime = packet->microTime;
		rspPacket.ChannelId = packet->ChannelId;
		rspPacket.m_msg = pReplyProtocol;

		if(!BattleServer::Instance()->SendData(&rspPacket))
			error_log("send rsp failed:[%u,%u,%u]",rspPacket.fd,rspPacket.time,rspPacket.microTime);

		pReplyProtocol = NULL;
	}
	*/
	pReplyProtocol = NULL;
	needDelReply = true;
}

void LogicManager::forwardProcess(CFirePacket* packet){
	if(IsPreClosed)
		return;

	unsigned uid = packet->fd;
	if(!CMI->IsNeedConnectByUID(uid))
		sendMsg(uid, packet->m_msg, false);
}

void LogicManager::heartProcess(CFirePacket* packet){
	//todo ???????????????????????????????????????
}

bool LogicManager::sendKickMsg(unsigned fd, string reason)
{
	if(channelId == -1 || fd == -1)
		return false;

	CFirePacket* packet = new CFirePacket(PROTOCOL_ACCESS_LOFFLINE);
	packet->fd = fd;
	packet->channelId = channelId;

	return SendData(packet);
}
void LogicManager::forceKick(unsigned uid, string reason){
	sendKickMsg(Getfd(uid), reason);
	offline(uid);
	Eraseuid(uid);
}

void LogicManager::offline(unsigned uid)
{
	if(IsValidUid(uid)){
		//todo ??????????????????????????????
		UserManager::Instance()->UserOffLine(uid);
		//??????????????????
		LogicQueueManager::Instance()->Offline(uid);
		//????????????????????????????????????????????????
		//LogicRoutineManager::Instance()->Offline(uid);
	}

	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
		LogicXsgReportManager::Instance()->XSGLogOutReport(uid);
}

void LogicManager::timerProcess()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	GlobalMilliTime = tv.tv_sec * 1000 + tv.tv_usec/1000 - StartMilliTime;

//	info_log("time=%d", GlobalMilliTime);
	//??????????????????????????????????????????????????????
	if(GlobalMilliTime - lastLoopTime < PER_FRAME_TIME){
//		info_log("timer process time out time=%u",GlobalMilliTime - lastLoopTime);
		return;
	}

	if(IsPreClosed){
		static int c = 0;
		++c;
		if(c == 1)
			ClearUser(true);
		else if(c == 4){
			for(vector<ActivitySingletonBase*>::iterator it=m_activityManager.begin();it!=m_activityManager.end();++it)
			{
				try
				{
					if((*it)->IsOn())
						(*it)->OnExit();
				}
				catch(const std::exception&) {}
			}
			for(vector<BattleSingleton*>::iterator it=m_battleManager.begin();it!=m_battleManager.end();++it)
			{
				try
				{
					(*it)->OnExit();
				}
				catch(const std::exception&) {}
			}
			for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
			{
				try
				{
					(*it)->OnExit();
				}
				catch(const std::exception&) {}
			}
			for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
			{
				try
				{
					(*it)->OnExit();
				}
				catch(const std::exception&) {}
			}

			for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
			{
				try
				{
					(*it)->Exit();
				}
				catch(const std::exception&) {}
			}
			for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
			{
				try
				{
					(*it)->Exit();
				}
				catch(const std::exception&) {}
			}

			IsClosed = true;
		}
	}
	else
	{
		if(m_pConn &&  m_pConn->disconnected())
			LogicManager::Instance()->ClearUser(false);

		if(NeedReloadConfig)
		{
			NeedReloadConfig = false;
			if(ConfigManager::Reload())
			{
				info_log("ConfigManager Reload sucess!");
				OnReload();
			}
			else
				info_log("ConfigManager Reload fail!");
		}

		if(m_signum)
			CheckSig();

		++m_timer;
		unsigned now = Time::GetGlobalTime();
		if(CTime::IsDiffHour(m_last_hour_ts,now))
		{
			m_last_hour_ts = now;
			try
			{
				CheckHour();
			}
			catch(const std::exception& e)
			{
				error_log("err_msg: %s", e.what());
			}
		}
		if(m_timer % 60 == 0)
		{
			try
			{
				CheckMin();
			}
			catch(const std::exception& e)
			{
				error_log("err_msg: %s", e.what());
			}
		}
		for(list<pair<unsigned, unsigned> >::iterator it=m_leaveList.begin(); it!=m_leaveList.end();){
			if(OFFLINE_DELAY + it->first <= now){
				info_log("kick offline, uid=%u", it->second);
				offline(it->second);
				it = m_leaveList.erase(it);
			}
			else
				break;
		}

	}

	gettimeofday(&tv,NULL);
	lastLoopTime = tv.tv_sec * 1000 + tv.tv_usec/1000 - StartMilliTime;
	if(lastLoopTime - GlobalMilliTime > 40)
	{
		info_log("timer run time = %u",lastLoopTime - GlobalMilliTime);
	}
	BattleServer::Instance()->SetTimerCB(std::bind(&LogicManager::timerProcess, this), 1.0);
}

void LogicManager::preOffline(CFirePacket* packet)
{
	if(IsPreClosed)
		return;

	if(Getuid(packet->fd) == -1)
		return;

	info_log("kick pre offline, uid=%u, fd=%u", Getuid(packet->fd), packet->fd);
	m_leaveList.push_back(std::make_pair(Time::GetGlobalTime(), Getuid(packet->fd)));
	Erasefd(packet->fd);
}

void LogicManager::onTimer2()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	GlobalMilliTime = tv.tv_sec * 1000 + tv.tv_usec/1000 - StartMilliTime;

	//debug_log("time=%d", GlobalMilliTime);
	for(vector<BattleSingleton*>::iterator it=m_battleManager.begin();it!=m_battleManager.end();++it)
	{
		try
		{
			(*it)->OnTimer2();
		}
		catch(const std::exception&) {}
	}
	for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
	{
		try
		{
			(*it)->OnTimer2();
		}
		catch(const std::exception&) {}
	}
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		try
		{
			(*it)->OnTimer2();
		}
		catch(const std::exception&) {}
	}

	for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
	{
		try
		{
			(*it)->Timer2();
		}
		catch(const std::exception&) {}
	}
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		try
		{
			(*it)->Timer2();
		}
		catch(const std::exception&) {}
	}

	BattleServer::Instance()->SetTimerCB(std::bind(&LogicManager::onTimer2, this), 3.0);
}

LogicManager::~LogicManager()
{
	for(vector<ActivitySingletonBase*>::iterator it=m_activityManager.begin();it!=m_activityManager.end();++it)
	{
		try
		{
			(*it)->CallDestroy();
		}
		catch(const std::exception&) {}
	}
	for(vector<BattleSingleton*>::iterator it=m_battleManager.begin();it!=m_battleManager.end();++it)
	{
		try
		{
			(*it)->CallDestroy();
		}
		catch(const std::exception&) {}
	}
	for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
	{
		try
		{
			(*it)->CallDestroy();
		}
		catch(const std::exception&) {}
	}
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		try
		{
			(*it)->CallDestroy();
		}
		catch(const std::exception&) {}
	}
	ConfigManager::Instance()->Destroy();
}

void LogicManager::RegProto()
{
	//??????
	dispatcher.registerMessageCallback<Common::Login>(ProtoManager::ProcessLogin);
	//??????????????????
	dispatcher.registerMessageCallback<User::RequestOtherUser>(ProtoManager::ProcessNoReply<User::RequestOtherUser, UserManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<User::RequestOtherUserBC>(ProtoManager::ProcessNoReplyNoUID<User::RequestOtherUserBC, UserManager>);
	//??????
	dispatcher.registerMessageCallback<User::Tutorialstage>(ProtoManager::ProcessNoReply<User::Tutorialstage, UserManager>);
	//????????????
	dispatcher.registerMessageCallback<User::SwitchStatus>(ProtoManager::ProcessNoReply<User::SwitchStatus, UserManager>);
	//??????
	dispatcher.registerMessageCallback<Common::ChangeName>(ProtoManager::Process<Common::ChangeName, Common::ReplyChangeName, UserManager>);
	/***********GM********************/
	//??????
	dispatcher.registerMessageCallback<Admin::AddCash>(ProtoManager::ProcessNoUID<Admin::AddCash, Admin::ReplyAddCash, UserManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<Admin::AsycAdd>(ProtoManager::ProcessNoUID<Admin::AsycAdd, Admin::AsycAddResp, UserManager>);
	//????????????
	dispatcher.registerMessageCallback<Admin::RequestForbidTS>(ProtoManager::ProcessNoUID<Admin::RequestForbidTS, Admin::ReplyForbidTS, UserManager>);
	//??????
	dispatcher.registerMessageCallback<Admin::SetForbidTS>(ProtoManager::ProcessNoReplyNoUID<Admin::SetForbidTS, UserManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<Admin::SetAllianceRaceGroup>(ProtoManager::ProcessNoReplyNoUID<Admin::SetAllianceRaceGroup, UserManager>);
	//??????????????????
	dispatcher.registerMessageCallback<Admin::SetActivity>(ProtoManager::ProcessNoReplyNoUID<Admin::SetActivity, UserManager>);
	//???????????????
	dispatcher.registerMessageCallback<User::SetVersion>(ProtoManager::ProcessNoReply<User::SetVersion, UserManager>);
	//???????????????
	dispatcher.registerMessageCallback<User::SetFlag>(ProtoManager::ProcessNoReply<User::SetFlag, UserManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<User::ReqNewMsg>(ProtoManager::Process<User::ReqNewMsg, User::ReplyNewMsg, UserManager>);
	//????????????
	dispatcher.registerMessageCallback<User::HeartBeatReq>(ProtoManager::Process<User::HeartBeatReq, User::HeartBeatResp, UserManager>);

	//??????????????????
	dispatcher.registerMessageCallback<User::GetThumbsUpReq>(ProtoManager::Process<User::GetThumbsUpReq, User::GetThumbsUpResp, LogicUserManager>);
	//???????????????
	dispatcher.registerMessageCallback<User::RankThumbsUpReq>(ProtoManager::ProcessNoReply<User::RankThumbsUpReq, LogicUserManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::CSRankThumbsUpReq>(ProtoManager::ProcessNoReplyNoUID<User::CSRankThumbsUpReq, LogicUserManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::CSRankThumbsUpResp>(ProtoManager::ProcessNoReplyNoUID<User::CSRankThumbsUpResp, LogicUserManager>);
	//??????????????????????????????
	dispatcher.registerMessageCallback<User::GetWorldChannelHelpCntReq>(ProtoManager::Process<User::GetWorldChannelHelpCntReq, User::GetWorldChannelHelpCntResp, LogicUserManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::WorldChannelHelpReq>(ProtoManager::Process<User::WorldChannelHelpReq, User::WorldChannelHelpResp, LogicUserManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<User::NewUserGuideShareReq>(ProtoManager::Process<User::NewUserGuideShareReq, User::NewUserGuideShareResp, LogicUserManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoPush::RewardLevelUpReq>(ProtoManager::Process<ProtoPush::RewardLevelUpReq, ProtoPush::RewardLevelUpResp, LogicUserManager>);
	//cdkey
	dispatcher.registerMessageCallback<User::UseCdKeyReq>(ProtoManager::Process<User::UseCdKeyReq, User::UseCdKeyResp, LogicUserManager>);

	//??????
	dispatcher.registerMessageCallback<ProtoArchive::ExportReq>(ProtoManager::ProcessNoUID<ProtoArchive::ExportReq, ProtoArchive::ExportResp, UserManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoArchive::ImportReq>(ProtoManager::ProcessNoUID<ProtoArchive::ImportReq, ProtoArchive::ImportResp, UserManager>);
	//????????????
	dispatcher.registerMessageCallback<User::CostCashReq>(ProtoManager::Process<User::CostCashReq, User::CostCashResp, LogicUserManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::ShareRewardsReq>(ProtoManager::Process<User::ShareRewardsReq, User::ShareRewardsResp, DailyShareActivity>);
	dispatcher.registerMessageCallback<User::ShareTotalRewardsReq>(ProtoManager::Process<User::ShareTotalRewardsReq, User::ShareTotalRewardsResp, DailyShareActivity>);
	dispatcher.registerMessageCallback<User::DaliyShareReq>(ProtoManager::Process<User::DaliyShareReq, User::DaliyShareResp, DailyShareActivity>);

	//???????????????
	dispatcher.registerMessageCallback<ProtoReward::GetFirstRechargeRewardReq>(ProtoManager::Process<ProtoReward::GetFirstRechargeRewardReq, ProtoReward::GetFirstRechargeRewardResp, LogicUserManager>);
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoReward::GetFollowPublicRewardReq>(ProtoManager::Process<ProtoReward::GetFollowPublicRewardReq, ProtoReward::GetFollowPublicRewardResp, LogicUserManager>);
	//?????????????????????????????????
	dispatcher.registerMessageCallback<ProtoReward::FetchProductWatchAdsRewardReq>(ProtoManager::Process<ProtoReward::FetchProductWatchAdsRewardReq, ProtoReward::FetchProductWatchAdsRewardResp, LogicUserManager>);

	dispatcher.registerMessageCallback<Common::ShutDown>(ProtoManager::ProcessNoReply<Common::ShutDown, UserManager>);
	//????????????
	dispatcher.registerMessageCallback<User::BuyMaterialReq>(ProtoManager::Process<User::BuyMaterialReq, User::BuyMaterialResp, LogicPropsManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<User::ViewAdGetSpeedUpCardReq>(ProtoManager::Process<User::ViewAdGetSpeedUpCardReq, User::ViewAdGetSpeedUpCardResp, LogicPropsManager>);

	//??????
	//??????
	dispatcher.registerMessageCallback<ProtoBuilding::BuildReq>(ProtoManager::Process<ProtoBuilding::BuildReq, ProtoBuilding::BuildResp, LogicBuildManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoBuilding::UnveilBuildReq>(ProtoManager::Process<ProtoBuilding::UnveilBuildReq, ProtoBuilding::UnveilBuildResp, LogicBuildManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoBuilding::MoveReq>(ProtoManager::Process<ProtoBuilding::MoveReq, ProtoBuilding::MoveResp, LogicBuildManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoBuilding::FlipReq>(ProtoManager::Process<ProtoBuilding::FlipReq, ProtoBuilding::FlipResp, LogicBuildManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoBuilding::BuildingUpReq>(ProtoManager::Process<ProtoBuilding::BuildingUpReq, ProtoBuilding::BuildingUpResp, LogicBuildManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoBuilding::UpgradeStarSpeedUpReq>(ProtoManager::Process<ProtoBuilding::UpgradeStarSpeedUpReq, ProtoBuilding::UpgradeStarSpeedUpResp, LogicBuildManager>);
	//???????????????
	dispatcher.registerMessageCallback<ProtoBuilding::RemoveBarrierReq>(ProtoManager::Process<ProtoBuilding::RemoveBarrierReq, ProtoBuilding::RemoveBarrierResp, LogicBuildManager>);

	dispatcher.registerMessageCallback<ProtoBuilding::SellDecorateReq>(ProtoManager::Process<ProtoBuilding::SellDecorateReq, ProtoBuilding::SellDecorateResq, LogicBuildManager>);
	//????????????
	dispatcher.registerMessageCallback<User::SpeedUpReq>(ProtoManager::Process<User::SpeedUpReq, User::SpeedUpResp, LogicQueueManager>);
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoBuilding::ViewAdReduceBuildTimeReq>(ProtoManager::Process<ProtoBuilding::ViewAdReduceBuildTimeReq, ProtoBuilding::ViewAdReduceBuildTimeResp, LogicBuildManager>);
	//????????????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoBuilding::GetViewAdReduceBuildTimeReq>(ProtoManager::Process<ProtoBuilding::GetViewAdReduceBuildTimeReq, ProtoBuilding::GetViewAdReduceBuildTimeResp, LogicBuildManager>);

	//---------------???????????????
	//??????
	dispatcher.registerMessageCallback<ProtoProduce::PlantCropReq>(ProtoManager::Process<ProtoProduce::PlantCropReq, ProtoProduce::PlantCropResp, LogicProductLineManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoProduce::ReapCropReq>(ProtoManager::Process<ProtoProduce::ReapCropReq, ProtoProduce::ReapCropResp, LogicProductLineManager>);

	//---------------????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::ExpandQueueReq>(ProtoManager::Process<ProtoProduce::ExpandQueueReq, ProtoProduce::ExpandQueueResp, LogicProductLineManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoProduce::JoinQueueReq>(ProtoManager::Process<ProtoProduce::JoinQueueReq, ProtoProduce::JoinQueueResp, LogicProductLineManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::FetchProductReq>(ProtoManager::Process<ProtoProduce::FetchProductReq, ProtoProduce::FetchProductResp, LogicProductLineManager>);

	//----------------????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::AdoptAnimalReq>(ProtoManager::Process<ProtoProduce::AdoptAnimalReq, ProtoProduce::AdoptAnimalResp, LogicProductLineManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::FeedAnimalReq>(ProtoManager::Process<ProtoProduce::FeedAnimalReq, ProtoProduce::FeedAnimalResp, LogicProductLineManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::ObtainProductReq>(ProtoManager::Process<ProtoProduce::ObtainProductReq, ProtoProduce::ObtainProductResp, LogicProductLineManager>);

	//---------------????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::ReapFruitReq>(ProtoManager::Process<ProtoProduce::ReapFruitReq, ProtoProduce::ReapFruitResp, LogicProductLineManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoProduce::SeekHelpReq>(ProtoManager::Process<ProtoProduce::SeekHelpReq, ProtoProduce::SeekHelpResp, LogicProductLineManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoProduce::CutFruitTreeReq>(ProtoManager::Process<ProtoProduce::CutFruitTreeReq, ProtoProduce::CutFruitTreeResp, LogicProductLineManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::OfferHelpReq>(ProtoManager::ProcessNoReply<ProtoProduce::OfferHelpReq, LogicProductLineManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoProduce::CSOfferHelpReq>(ProtoManager::ProcessNoReplyNoUID<ProtoProduce::CSOfferHelpReq, LogicProductLineManager>);
	//??????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoProduce::CSOfferHelpResp>(ProtoManager::ProcessNoReplyNoUID<ProtoProduce::CSOfferHelpResp, LogicProductLineManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoProduce::ConfirmHelpReq>(ProtoManager::Process<ProtoProduce::ConfirmHelpReq, ProtoProduce::ConfirmHelpResp, LogicProductLineManager>);

	//GM
	dispatcher.registerMessageCallback<ProtoGM::GMCmdReq>(ProtoManager::ProcessNoReply<ProtoGM::GMCmdReq, LogicGM>);

	//????????????
	dispatcher.registerMessageCallback<ProtoNotify::GetNotifyReq>(ProtoManager::Process<ProtoNotify::GetNotifyReq, ProtoNotify::GetNotifyResp, LogicNotifyManager>);

	//-----------------??????
	//????????????
	dispatcher.registerMessageCallback<ProtoOrder::OrderQueryReq>(ProtoManager::Process<ProtoOrder::OrderQueryReq, ProtoOrder::OrderResp, LogicOrderManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoOrder::StartOrderReq>(ProtoManager::Process<ProtoOrder::StartOrderReq, ProtoOrder::StartOrderResp, LogicOrderManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoOrder::DeleteOrderReq>(ProtoManager::Process<ProtoOrder::DeleteOrderReq, ProtoOrder::DeleteOrderResp, LogicOrderManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoOrder::TruckQueryReq>(ProtoManager::Process<ProtoOrder::TruckQueryReq, ProtoOrder::TruckResp, LogicOrderManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoOrder::RewardOrderReq>(ProtoManager::Process<ProtoOrder::RewardOrderReq, ProtoOrder::RewardOrderResp, LogicOrderManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoOrder::GetOrderBonusInfoReq>(ProtoManager::Process<ProtoOrder::GetOrderBonusInfoReq, ProtoOrder::GetOrderBonusInfoResp, LogicOrderManager>);
	//?????????
	dispatcher.registerMessageCallback<ProtoOrder::ViewAdReq>(ProtoManager::Process<ProtoOrder::ViewAdReq, ProtoOrder::ViewAdResp, LogicOrderManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoOrder::BuyOrderBonusReq>(ProtoManager::Process<ProtoOrder::BuyOrderBonusReq, ProtoOrder::BuyOrderBonusResp, LogicOrderManager>);

	//-------------??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoShop::GetShopReq>(ProtoManager::Process<ProtoShop::GetShopReq, ProtoShop::GetShopResp, LogicShopManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShop::ShelfPropsReq>(ProtoManager::Process<ProtoShop::ShelfPropsReq, ProtoShop::ShelfPropsResp, LogicShopManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShop::ShelfUnLockReq>(ProtoManager::Process<ProtoShop::ShelfUnLockReq, ProtoShop::ShelfUnLockResp, LogicShopManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoShop::VisitOtherShopReq>(ProtoManager::ProcessNoReply<ProtoShop::VisitOtherShopReq, LogicShopManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoShop::CSVisitOtherShopReq>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSVisitOtherShopReq, LogicShopManager>);
	//???????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoShop::CSVisitOtherShopResp>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSVisitOtherShopResp, LogicShopManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoShop::PurchaseReq>(ProtoManager::ProcessNoReply<ProtoShop::PurchaseReq, LogicShopManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShop::CSPurchaseReq>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSPurchaseReq, LogicShopManager>);
	//????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoShop::CSPurchaseResp>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSPurchaseResp, LogicShopManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoShop::ConfirmPaymentReq>(ProtoManager::Process<ProtoShop::ConfirmPaymentReq, ProtoShop::ConfirmPaymentResp, LogicShopManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoShop::ModifyShelfInfoReq>(ProtoManager::Process<ProtoShop::ModifyShelfInfoReq, ProtoShop::ModifyShelfInfoResp, LogicShopManager>);
	//???????????????????????????
	dispatcher.registerMessageCallback<ProtoShop::ViewAdRecycleItemReq>(ProtoManager::Process<ProtoShop::ViewAdRecycleItemReq, ProtoShop::ViewAdRecycleItemResp, LogicShopManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShop::BuyShopItemBySystemReq>(ProtoManager::Process<ProtoShop::BuyShopItemBySystemReq, ProtoShop::BuyShopItemBySystemResp, LogicShopManager>);

	//-------------??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoFriend::GetAllFriendsReq>(ProtoManager::ProcessNoReply<ProtoFriend::GetAllFriendsReq, LogicFriendManager>);
	//?????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriend::GetFriendHelpInfoReq>(ProtoManager::ProcessNoReply<ProtoFriend::GetFriendHelpInfoReq, LogicFriendManager>);
	//???????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriend::CSGetFriendHelpInfoReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSGetFriendHelpInfoReq, LogicFriendManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoFriend::ConcernReq>(ProtoManager::ProcessNoReply<ProtoFriend::ConcernReq, LogicFriendManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoFriend::CSConcernReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSConcernReq, LogicFriendManager>);
	//????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriend::CSConcernResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSConcernResp, LogicFriendManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoFriend::CancelConcernReq>(ProtoManager::ProcessNoReply<ProtoFriend::CancelConcernReq, LogicFriendManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoFriend::CSCancelConcernReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSCancelConcernReq, LogicFriendManager>);
	//??????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriend::CSCancelConcernResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSCancelConcernResp, LogicFriendManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoFriend::RemoveFansReq>(ProtoManager::Process<ProtoFriend::RemoveFansReq, ProtoFriend::RemoveFansResp, LogicFriendManager>);

	//--------??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoAdvertise::GetAdInfoReq>(ProtoManager::Process<ProtoAdvertise::GetAdInfoReq, ProtoAdvertise::GetAdInfoResp, LogicAdvertiseManager>);
	//???????????????cd
	dispatcher.registerMessageCallback<ProtoAdvertise::UpdateAdCdReq>(ProtoManager::Process<ProtoAdvertise::UpdateAdCdReq, ProtoAdvertise::UpdateAdCdResp, LogicAdvertiseManager>);

	//------??????
	//?????????????????? ??????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::GetDynamicInfoReq>(ProtoManager::Process<ProtoDynamicInfo::GetDynamicInfoReq, ProtoDynamicInfo::GetDynamicInfoResp, LogicDynamicInfoManager>);
	//?????????????????? ??????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::DeleteDynamicInfoReq>(ProtoManager::ProcessNoReply<ProtoDynamicInfo::DeleteDynamicInfoReq, LogicDynamicInfoManager>);
	//???????????????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::RequestOtherUserMakeDy>(ProtoManager::ProcessNoReplyNoUID<ProtoDynamicInfo::RequestOtherUserMakeDy, LogicDynamicInfoManager>);
	//??????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::HasNewDynamicInfoReq>(ProtoManager::Process<ProtoDynamicInfo::HasNewDynamicInfoReq, ProtoDynamicInfo::HasNewDynamicInfoResp, LogicDynamicInfoManager>);
	//?????????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::ClickOrderHelpReq>(ProtoManager::Process<ProtoDynamicInfo::ClickOrderHelpReq, ProtoDynamicInfo::ClickOrderHelpResp, LogicDynamicInfoManager>);

	//------?????????
	//??????????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::GetMessageBoardReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::GetMessageBoardReq, LogicMessageBoardManager>);
	//??????????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::DeleteMessageBoardReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::DeleteMessageBoardReq, LogicMessageBoardManager>);
	//????????????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoMessageBoard::HasNewLeaveMessageReq>(ProtoManager::Process<ProtoMessageBoard::HasNewLeaveMessageReq, ProtoMessageBoard::HasNewLeaveMessageResp, LogicMessageBoardManager>);
	//?????? ?????????????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::GetMasterVisiableMsgReq>(ProtoManager::ProcessNoReplyNoUID<ProtoMessageBoard::GetMasterVisiableMsgReq, LogicMessageBoardManager>);
	//?????? ?????????????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::DeleteMyMsgOverServerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoMessageBoard::DeleteMyMsgOverServerReq, LogicMessageBoardManager>);
	//?????? ????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::LeaveMessageReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::LeaveMessageReq, LogicMessageBoardManager>);
	//?????? ????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::AnswerLeaveMessageReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::AnswerLeaveMessageReq, LogicMessageBoardManager>);
	//?????? ???????????????????????? ?????? ?????????????????????????????????
	dispatcher.registerMessageCallback<ProtoMessageBoard::SendLeaveMsgOverServerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoMessageBoard::SendLeaveMsgOverServerReq, LogicMessageBoardManager>);

	//------????????????
	//??????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoMessageBoard::GetFeedbackReq>(ProtoManager::Process<ProtoMessageBoard::GetFeedbackReq, ProtoMessageBoard::GetFeedbackResp, LogicMessageBoardManager>);
	//?????????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoMessageBoard::SendFeedbackReq>(ProtoManager::Process<ProtoMessageBoard::SendFeedbackReq, ProtoMessageBoard::SendFeedbackResp, LogicMessageBoardManager>);
	//????????????????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoMessageBoard::DelFeedbackReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::DelFeedbackReq, LogicMessageBoardManager>);

	//------????????????
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::LocationHelpReq>(ProtoManager::ProcessNoReply<ProtoDynamicInfo::LocationHelpReq, LogicLocationHelpManager>);
	//???????????????????????? ????????????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoDynamicInfo::HandleLocationHelpReq>(ProtoManager::Process<ProtoDynamicInfo::HandleLocationHelpReq, ProtoDynamicInfo::HandleLocationHelpResq, LogicLocationHelpManager>);

	//------????????????
	//???????????????????????? ??????
	dispatcher.registerMessageCallback<ProtoFriendOrder::GetFriendOrderReq>(ProtoManager::Process<ProtoFriendOrder::GetFriendOrderReq, ProtoFriendOrder::GetFriendOrderResp, LogicFriendOrderManager>);
	//?????????????????? ??????
	dispatcher.registerMessageCallback<ProtoFriendOrder::SendFriendOrderReq>(ProtoManager::Process<ProtoFriendOrder::SendFriendOrderReq, ProtoFriendOrder::SendFriendOrderResp, LogicFriendOrderManager>);
	//?????? ??????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::SendOtherServerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::SendOtherServerReq, LogicFriendOrderManager>);
	//????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::ClickFriendOrderReq>(ProtoManager::ProcessNoReply<ProtoFriendOrder::ClickFriendOrderReq, LogicFriendOrderManager>);
	//?????? ?????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::RecallSourceFoReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::RecallSourceFoReq, LogicFriendOrderManager>);
	//?????? ????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::ChangeFoStatusReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::ChangeFoStatusReq, LogicFriendOrderManager>);
	//??????????????????????????? ????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::BuyFriendOrderReq>(ProtoManager::ProcessNoReply<ProtoFriendOrder::BuyFriendOrderReq, LogicFriendOrderManager>);
	//?????? ????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::RecallCanBuyFoReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::RecallCanBuyFoReq, LogicFriendOrderManager>);
	//?????? ????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::AnswerWhetherCanBuyReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::AnswerWhetherCanBuyReq, LogicFriendOrderManager>);
	//????????????????????? ???????????????????????????????????????????????????????????????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::GetOrderRewardsReq>(ProtoManager::Process<ProtoFriendOrder::GetOrderRewardsReq, ProtoFriendOrder::GetOrderRewardsResp, LogicFriendOrderManager>);
	//???????????????????????????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendOrder::CostDiamondReq>(ProtoManager::Process<ProtoFriendOrder::CostDiamondReq, ProtoFriendOrder::CostDiamondResp, LogicFriendOrderManager>);

	//------??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoTask::GetTaskReq>(ProtoManager::Process<ProtoTask::GetTaskReq, ProtoTask::GetTaskResp, LogicTaskManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoTask::RewardTaskReq>(ProtoManager::Process<ProtoTask::RewardTaskReq, ProtoTask::RewardTaskResp, LogicTaskManager>);
	//-----????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoMission::GetCurMissionReq>(ProtoManager::Process<ProtoMission::GetCurMissionReq, ProtoMission::GetCurMissionResp, LogicMissionManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoMission::RewardMissionReq>(ProtoManager::Process<ProtoMission::RewardMissionReq, ProtoMission::RewardMissionResp, LogicMissionManager>);

	//--------vip
	//????????????
	dispatcher.registerMessageCallback<ProtoVIP::GetVIPGiftReq>(ProtoManager::Process<ProtoVIP::GetVIPGiftReq, ProtoVIP::GetVIPGiftResp, LogicVIPManager>);
	//vip????????????????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoVIP::VIPProductSpeedUpReq>(ProtoManager::Process<ProtoVIP::VIPProductSpeedUpReq, ProtoVIP::VIPProductSpeedUpResp, LogicVIPManager>);
	//vip ???????????????
	dispatcher.registerMessageCallback<ProtoVIP::VIPRemoveOrderCDReq>(ProtoManager::Process<ProtoVIP::VIPRemoveOrderCDReq, ProtoVIP::VIPRemoveOrderCDResp, LogicVIPManager>);
	//vip??????????????????
	dispatcher.registerMessageCallback<ProtoVIP::VIPShelfUnLockReq>(ProtoManager::Process<ProtoVIP::VIPShelfUnLockReq, ProtoVIP::VIPShelfUnLockResp, LogicVIPManager>);
	//vip??????????????????
	dispatcher.registerMessageCallback<ProtoVIP::VIPAddProductQueueReq>(ProtoManager::Process<ProtoVIP::VIPAddProductQueueReq, ProtoVIP::VIPAddProductQueueResp, LogicVIPManager>);
	//????????????vip??????
	dispatcher.registerMessageCallback<ProtoVIP::RandomVIPGiftReq>(ProtoManager::Process<ProtoVIP::RandomVIPGiftReq, ProtoVIP::RandomVIPGiftResp, LogicVIPManager>);

	//------??????
	//????????????
	dispatcher.registerMessageCallback<ProtoShipping::UnlockDockReq>(ProtoManager::Process<ProtoShipping::UnlockDockReq, ProtoShipping::UnlockDockResp, LogicShippingManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShipping::UnveilDockReq>(ProtoManager::Process<ProtoShipping::UnveilDockReq, ProtoShipping::UnveilDockResp, LogicShippingManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoShipping::PackBoxReq>(ProtoManager::Process<ProtoShipping::PackBoxReq, ProtoShipping::PackBoxResp, LogicShippingManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShipping::SeekAidReq>(ProtoManager::Process<ProtoShipping::SeekAidReq, ProtoShipping::SeekAidResp, LogicShippingManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoShipping::OfferAidReq>(ProtoManager::ProcessNoReply<ProtoShipping::OfferAidReq, LogicShippingManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoShipping::CSOfferAidReq>(ProtoManager::ProcessNoReplyNoUID<ProtoShipping::CSOfferAidReq, LogicShippingManager>);
	//???????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoShipping::CSOfferAidResp>(ProtoManager::ProcessNoReplyNoUID<ProtoShipping::CSOfferAidResp, LogicShippingManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoShipping::LeaveDockReq>(ProtoManager::Process<ProtoShipping::LeaveDockReq, ProtoShipping::LeaveDockResp, LogicShippingManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoShipping::SetPlayStatusReq>(ProtoManager::Process<ProtoShipping::SetPlayStatusReq, ProtoShipping::SetPlayStatusResp, LogicShippingManager>);
	//
	dispatcher.registerMessageCallback<ProtoShipping::GetShipBonusInfoReq>(ProtoManager::Process<ProtoShipping::GetShipBonusInfoReq, ProtoShipping::GetShipBonusInfoResp, LogicShippingManager>);

	//????????????
	dispatcher.registerMessageCallback<User::PurchaseCoinReq>(ProtoManager::Process<User::PurchaseCoinReq, User::PurchaseCoinResp, LogicUserManager>);
	//?????????????????????????????????
	dispatcher.registerMessageCallback<User::CSPushLoginMsg>(ProtoManager::ProcessNoReplyNoUID<User::CSPushLoginMsg, LogicUserManager>);

	//--------------npc??????
	//????????????
	dispatcher.registerMessageCallback<ProtoNPCSeller::GetPropsDiscountReq>(ProtoManager::Process<ProtoNPCSeller::GetPropsDiscountReq, ProtoNPCSeller::GetPropsDiscountResp, LogicNPCSellerManager>);
	//??????npc??????
	dispatcher.registerMessageCallback<ProtoNPCSeller::ResponseNPCSellerReq>(ProtoManager::Process<ProtoNPCSeller::ResponseNPCSellerReq, ProtoNPCSeller::ResponseNPCSellerResp, LogicNPCSellerManager>);
	//??????npc????????????
	dispatcher.registerMessageCallback<ProtoNPCSeller::ChangeNPCSellerStatusReq>(ProtoManager::Process<ProtoNPCSeller::ChangeNPCSellerStatusReq, ProtoNPCSeller::ChangeNPCSellerStatusResp, LogicNPCSellerManager>);

	//-------------npc??????
	//??????npc
	dispatcher.registerMessageCallback<ProtoNPCUser::RequestNPCUser>(ProtoManager::Process<ProtoNPCUser::RequestNPCUser, ProtoNPCUser::NPCUser, LogicNPCShopManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoNPCUser::GetNPCShopReq>(ProtoManager::Process<ProtoNPCUser::GetNPCShopReq, ProtoNPCUser::GetNPCShopResp, LogicNPCShopManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoNPCUser::PurchaseReq>(ProtoManager::Process<ProtoNPCUser::PurchaseReq, ProtoNPCUser::PurchaseResp, LogicNPCShopManager>);

	//--------------??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAlliance>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAlliance, LogicAllianceManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceBC, LogicAllianceManager>);
	//??????????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceBC, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::GetAllianceFunctionReq>(ProtoManager::ProcessNoReply<ProtoAlliance::GetAllianceFunctionReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceFunctionBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceFunctionBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceFunctionBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceFunctionBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::GetNotifyReq>(ProtoManager::ProcessNoReply<ProtoAlliance::GetNotifyReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceNotifyBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceNotifyBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::GetMemberReq>(ProtoManager::ProcessNoReply<ProtoAlliance::GetMemberReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceMemberBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceMemberBC, LogicAllianceManager>);

	//??????????????????--
	dispatcher.registerMessageCallback<ProtoAlliance::CheckNameAvailableReq>(ProtoManager::Process<ProtoAlliance::CheckNameAvailableReq, ProtoAlliance::CheckNameAvailableResp, LogicAllianceManager>);

	//????????????--
	dispatcher.registerMessageCallback<ProtoAlliance::CreateAllianceReq>(ProtoManager::Process<ProtoAlliance::CreateAllianceReq, ProtoAlliance::CreateAllianceResp, LogicAllianceManager>);

	//????????????--
	dispatcher.registerMessageCallback<ProtoAlliance::RecommendllianceReq>(ProtoManager::Process<ProtoAlliance::RecommendllianceReq, ProtoAlliance::RecommendllianceResp, LogicAllianceManager>);

	//??????????????????--
	dispatcher.registerMessageCallback<ProtoAlliance::GetPartAllianceInfoReq>(ProtoManager::Process<ProtoAlliance::GetPartAllianceInfoReq, ProtoAlliance::GetPartAllianceInfoResp, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::ApplyJoinReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ApplyJoinReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestApplyJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestApplyJoinBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyApplyJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyApplyJoinBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::ApproveJoinReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ApproveJoinReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestApproveJoinUserBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestApproveJoinUserBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestApproveJoinAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestApproveJoinAllianceBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyApproveJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyApproveJoinBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::ExitAllianceReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ExitAllianceReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestExitAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestExitAllianceBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyExitAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyExitAllianceBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::InviteJoinReq>(ProtoManager::ProcessNoReply<ProtoAlliance::InviteJoinReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestInviteJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestInviteJoinBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestInviteJoinUserBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestInviteJoinUserBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyInviteJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyInviteJoinBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::AcceptInviteReq>(ProtoManager::ProcessNoReply<ProtoAlliance::AcceptInviteReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAcceptInviteBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAcceptInviteBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAcceptInviteBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAcceptInviteBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::ManipulateMemberReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ManipulateMemberReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestManipulateMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestManipulateMemberBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyManipulateMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyManipulateMemberBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::KickOutReq>(ProtoManager::ProcessNoReply<ProtoAlliance::KickOutReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestKickOutBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestKickOutBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestKickOutMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestKickOutMemberBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyKickOutBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyKickOutBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::TransferReq>(ProtoManager::ProcessNoReply<ProtoAlliance::TransferReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestTransferBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestTransferBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyTransferBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyTransferBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::EditAllianceReq>(ProtoManager::ProcessNoReply<ProtoAlliance::EditAllianceReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestEditAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestEditAllianceBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyEditAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyEditAllianceBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::SeekDonationReq>(ProtoManager::ProcessNoReply<ProtoAlliance::SeekDonationReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestSeekDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestSeekDonationBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplySeekDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplySeekDonationBC, LogicAllianceManager>);

	//?????????cd--
	dispatcher.registerMessageCallback<ProtoAlliance::CutUpDonationCDReq>(ProtoManager::Process<ProtoAlliance::CutUpDonationCDReq, ProtoAlliance::CutUpDonationCDResp, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::OfferDonationReq>(ProtoManager::ProcessNoReply<ProtoAlliance::OfferDonationReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestOfferDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestOfferDonationBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyOfferDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyOfferDonationBC, LogicAllianceManager>);

	//?????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::FetchDonationReq>(ProtoManager::ProcessNoReply<ProtoAlliance::FetchDonationReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestFetchDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestFetchDonationBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyFetchDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyFetchDonationBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::SendNotifyReq>(ProtoManager::ProcessNoReply<ProtoAlliance::SendNotifyReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestSendNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestSendNotifyBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplySendNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplySendNotifyBC, LogicAllianceManager>);

	//????????????
	dispatcher.registerMessageCallback<ProtoAlliance::DelNotifyReq>(ProtoManager::ProcessNoReply<ProtoAlliance::DelNotifyReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestDelNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestDelNotifyBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyDelNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyDelNotifyBC, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestUpdateMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestUpdateMemberBC, LogicAllianceManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAddMemberHelpTimesBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAddMemberHelpTimesBC, LogicAllianceManager>);



	//??????????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceSetFlag>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceSetFlag, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceSetFlagBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceSetFlagBC, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberProgress>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceMemberProgress, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberProgressBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberProgressBC, LogicAllianceManager>);


	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceInfo>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceInfo, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceInfoBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceInfoBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceOrderBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOperateOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceOperateOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOperateOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceOperateOrderBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceOperateOrder>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceOperateOrder, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberDelOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceMemberDelOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberDelOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberDelOrderBC, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC, LogicAllianceManager>);

	//???????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceBuyOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceBuyOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceBuyOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceBuyOrderBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceBuyOrder>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceBuyOrder, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceRewardBC, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeGradeReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceTakeGradeReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceTakeGradeReward>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceTakeGradeReward, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeStageReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceTakeStageReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeStageRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceTakeStageRewardBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceTakeStageReward>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceTakeStageReward, LogicAllianceManager>);

	//????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceRefreshStageReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceRefreshStageReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceRefreshStageReward>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceRefreshStageReward, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberOrderLog>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceMemberOrderLog, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberOrderLogBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberOrderLogBC, LogicAllianceManager>);

	//??????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRacePersonOrderLog>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRacePersonOrderLog, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRacePersonOrderLogBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRacePersonOrderLogBC, LogicAllianceManager>);

	//????????????????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::SetAllianceRaceGroupPointBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::SetAllianceRaceGroupPointBC, LogicAllianceManager>);

	//??????????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceGroupMember>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceGroupMember, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceGroupMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceGroupMemberBC, LogicAllianceManager>);

	//???????????????????????????
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceWatchAd>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceWatchAd, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceWatchAdBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceWatchAdBC, LogicAllianceManager>);


	//????????????
	dispatcher.registerMessageCallback<ProtoAssistor::OpenAssistorReq>(ProtoManager::Process<ProtoAssistor::OpenAssistorReq, ProtoAssistor::OpenAssistorResp, UserManager>);
	dispatcher.registerMessageCallback<ProtoAssistor::UseAssistorReq>(ProtoManager::Process<ProtoAssistor::UseAssistorReq, ProtoAssistor::UseAssistorResp, UserManager>);

	//----npc??????
	//??????npc????????????
	dispatcher.registerMessageCallback<ProtoNPCCustomer::GetNPCCustomerReq>(ProtoManager::Process<ProtoNPCCustomer::GetNPCCustomerReq, ProtoNPCCustomer::GetNPCCustomerResp, LogicCustomerManager>);
	//???????????????npc??????
	dispatcher.registerMessageCallback<ProtoNPCCustomer::SellPropsReq>(ProtoManager::Process<ProtoNPCCustomer::SellPropsReq, ProtoNPCCustomer::SellPropsResp, LogicCustomerManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoNPCCustomer::RefuseSellPropsReq>(ProtoManager::Process<ProtoNPCCustomer::RefuseSellPropsReq, ProtoNPCCustomer::RefuseSellPropsResp, LogicCustomerManager>);

	//----????????????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoRandomBox::OpenBoxReq>(ProtoManager::Process<ProtoRandomBox::OpenBoxReq, ProtoRandomBox::OpenBoxResp, LogicRandomBoxManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoRandomBox::BuyBoxGiftReq>(ProtoManager::Process<ProtoRandomBox::BuyBoxGiftReq, ProtoRandomBox::BuyBoxGiftResp, LogicRandomBoxManager>);

	//????????????
	dispatcher.registerMessageCallback<User::SignInRewardsReq>(ProtoManager::Process<User::SignInRewardsReq, User::SignInRewardsResp, SignInActivity>);
	dispatcher.registerMessageCallback<ProtoActivity::GameAcitivityStatusReq>(ProtoManager::Process<ProtoActivity::GameAcitivityStatusReq, ProtoActivity::GameAcitivityStatusResp, LogicGameActivityManager>);

	//------?????????
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoMailDog::GetMailDogInfoReq>(ProtoManager::Process<ProtoMailDog::GetMailDogInfoReq, ProtoMailDog::GetMailDogInfoResp, LogicMailDogManager>);

	//------??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoRotaryTable::GetRotaryTableInfoReq>(ProtoManager::Process<ProtoRotaryTable::GetRotaryTableInfoReq, ProtoRotaryTable::GetRotaryTableInfoResp, RotaryTableActivity>);
	//??????
	dispatcher.registerMessageCallback<ProtoRotaryTable::DrawRotaryTableReq>(ProtoManager::Process<ProtoRotaryTable::DrawRotaryTableReq, ProtoRotaryTable::DrawRotaryTableResp, RotaryTableActivity>);
	//??????
	dispatcher.registerMessageCallback<ProtoRotaryTable::ShareReq>(ProtoManager::Process<ProtoRotaryTable::ShareReq, ProtoRotaryTable::ShareResp, RotaryTableActivity>);

	//-----?????????
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoFriendlyTree::GetFriendlyTreeReq>(ProtoManager::Process<ProtoFriendlyTree::GetFriendlyTreeReq, ProtoFriendlyTree::GetFriendlyTreeResp, LogicFriendlyTreeManager>);
	//???????????????
	dispatcher.registerMessageCallback<ProtoFriendlyTree::RewardFriendlyTreeReq>(ProtoManager::Process<ProtoFriendlyTree::RewardFriendlyTreeReq, ProtoFriendlyTree::RewardFriendlyTreeResp, LogicFriendlyTreeManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoFriendlyTree::WaterFriendlyTreeReq>(ProtoManager::ProcessNoReply<ProtoFriendlyTree::WaterFriendlyTreeReq, LogicFriendlyTreeManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoFriendlyTree::CSWaterFriendlyTreeReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendlyTree::CSWaterFriendlyTreeReq, LogicFriendlyTreeManager>);
	//????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendlyTree::CSWaterFriendlyTreeResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendlyTree::CSWaterFriendlyTreeResp, LogicFriendlyTreeManager>);
	//---------qqgame??????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivityTencent::RewardStatusReq>(ProtoManager::Process<ProtoActivityTencent::RewardStatusReq, ProtoActivityTencent::RewardStatusResp, LogicActivityTencent>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetBlueDailyAward>(ProtoManager::Process<ProtoActivityTencent::GetBlueDailyAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetBlueGrowAward>(ProtoManager::Process<ProtoActivityTencent::GetBlueGrowAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetQQgamePrivilegeDailyAward>(ProtoManager::Process<ProtoActivityTencent::GetQQgamePrivilegeDailyAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetQQgamePrivilegeGrowAward>(ProtoManager::Process<ProtoActivityTencent::GetQQgamePrivilegeGrowAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//---------????????????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoAccessAd::RewardViewAdReq>(ProtoManager::Process<ProtoAccessAd::RewardViewAdReq, ProtoAccessAd::RewardViewAdResp, LogicAccessAdManager>);
	//??????ts
	dispatcher.registerMessageCallback<ProtoAccessAd::GetLastViewAdTsReq>(ProtoManager::Process<ProtoAccessAd::GetLastViewAdTsReq, ProtoAccessAd::GetLastViewAdTsResp, LogicAccessAdManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoAccessAd::GetBallonInfoReq>(ProtoManager::Process<ProtoAccessAd::GetBallonInfoReq, ProtoAccessAd::GetBallonInfoResp, LogicAccessAdManager>);
	//???????????????
	dispatcher.registerMessageCallback<ProtoAccessAd::CommonlViewAdReq>(ProtoManager::Process<ProtoAccessAd::CommonlViewAdReq, ProtoAccessAd::CommonlViewAdResp, LogicAccessAdManager>);
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoAccessAd::GetScarecrowInfoReq>(ProtoManager::Process<ProtoAccessAd::GetScarecrowInfoReq, ProtoAccessAd::GetScarecrowInfoResp, LogicAccessAdManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoAccessAd::ScarecrowViewAdReq>(ProtoManager::Process<ProtoAccessAd::ScarecrowViewAdReq, ProtoAccessAd::ScarecrowViewAdResp, LogicAccessAdManager>);
	//-------------????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoActivity::FundPurchaseReq>(ProtoManager::Process<ProtoActivity::FundPurchaseReq, ProtoActivity::FundPurchaseResp, LogicFundActivityManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivity::RewardFundGiftReq>(ProtoManager::Process<ProtoActivity::RewardFundGiftReq, ProtoActivity::RewardFundGiftResp, LogicFundActivityManager>);

	//-------------4399??????????????????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivity::Reward4399RechargeGiftReq>(ProtoManager::Process<ProtoActivity::Reward4399RechargeGiftReq, ProtoActivity::Reward4399RechargeGiftResp, Recharge4399ActivityManager>);
	//-------------4399??????????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoActivity::Reward4399DailyGiftReq>(ProtoManager::Process<ProtoActivity::Reward4399DailyGiftReq, ProtoActivity::Reward4399DailyGiftResp, Daily4399ActivityManager>);
	dispatcher.registerMessageCallback<ProtoActivity::UseCardReq>(ProtoManager::Process<ProtoActivity::UseCardReq, ProtoActivity::UseCardResp, Daily4399ActivityManager>);

	//----------??????????????????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoCard::GetCardReq>(ProtoManager::Process<ProtoCard::GetCardReq, ProtoCard::GetCardResp, LogicCardManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoCard::RewardMonthCardReq>(ProtoManager::Process<ProtoCard::RewardMonthCardReq, ProtoCard::RewardMonthCardResp, LogicCardManager>);
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoCard::RewardLifeCardReq>(ProtoManager::Process<ProtoCard::RewardLifeCardReq, ProtoCard::RewardLifeCardResp, LogicCardManager>);

	//---------????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoTheme::ThemeInfoReq>(ProtoManager::Process<ProtoTheme::ThemeInfoReq, ProtoTheme::ThemeInfoResp, LogicThemeManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoTheme::ThemeBuyReq>(ProtoManager::Process<ProtoTheme::ThemeBuyReq, ProtoTheme::ThemeBuyResp, LogicThemeManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoTheme::ThemeUseReq>(ProtoManager::Process<ProtoTheme::ThemeUseReq, ProtoTheme::ThemeInfoResp, LogicThemeManager>);

	//---------????????????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperInfoReq>(ProtoManager::Process<ProtoKeeper::KeeperInfoReq, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//?????????????????????
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperBuyTime>(ProtoManager::Process<ProtoKeeper::KeeperBuyTime, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//???????????????
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperWatchAds>(ProtoManager::Process<ProtoKeeper::KeeperWatchAds, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperSetTask>(ProtoManager::Process<ProtoKeeper::KeeperSetTask, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//??????
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperUpgrade>(ProtoManager::Process<ProtoKeeper::KeeperUpgrade, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperSetAutoFeed>(ProtoManager::Process<ProtoKeeper::KeeperSetAutoFeed, ProtoKeeper::KeeperSetAutoFeedResp, LogicKeeperManager>);

	//??????
	//gm?????????
	dispatcher.registerMessageCallback<Admin::SysMail>(ProtoManager::ProcessNoUID<Admin::SysMail, Admin::ReplySysMail, LogicSysMailManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::RequestSysMail>(ProtoManager::Process<User::RequestSysMail, User::ReplySysMail, LogicSysMailManager>);
	//????????????
	dispatcher.registerMessageCallback<User::RequestMailRead>(ProtoManager::Process<User::RequestMailRead, User::ReplyMailRead, LogicSysMailManager>);
	//????????????
	dispatcher.registerMessageCallback<User::RequestMailGet>(ProtoManager::Process<User::RequestMailGet, User::ReplyMailGet, LogicSysMailManager>);
	//??????
	dispatcher.registerMessageCallback<User::RequestMailDel>(ProtoManager::ProcessNoReply<User::RequestMailDel, LogicSysMailManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::RequestMailAllGet>(ProtoManager::Process<User::RequestMailAllGet, User::ReplyMailAllGet, LogicSysMailManager>);
	//????????????
	dispatcher.registerMessageCallback<User::RequestMailAllDel>(ProtoManager::ProcessNoReply<User::RequestMailAllDel, LogicSysMailManager>);
	//??????????????????
	dispatcher.registerMessageCallback<User::ReqSendMailBC>(ProtoManager::ProcessNoReplyNoUID<User::ReqSendMailBC, LogicSysMailManager>);

	//????????????
	//????????????
	dispatcher.registerMessageCallback<ProtoFriendWorker::SetFriendWorkerReq>(ProtoManager::ProcessNoReply<ProtoFriendWorker::SetFriendWorkerReq, LogicFriendWorkerManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoFriendWorker::CSSetFriendWorkerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendWorker::CSSetFriendWorkerReq, LogicFriendWorkerManager>);
	//??????????????????????????????
	dispatcher.registerMessageCallback<ProtoFriendWorker::CSSetFriendWorkerResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendWorker::CSSetFriendWorkerResp, LogicFriendWorkerManager>);
	//??????????????????
	dispatcher.registerMessageCallback<ProtoFriendWorker::GetWorkerSpeedUpReq>(ProtoManager::Process<ProtoFriendWorker::GetWorkerSpeedUpReq, ProtoFriendWorker::GetWorkerSpeedUpResp, LogicFriendWorkerManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoFriendWorker::SelectWorkerReq>(ProtoManager::Process<ProtoFriendWorker::SelectWorkerReq, ProtoFriendWorker::SelectWorkerResp, LogicFriendWorkerManager>);
	//????????????
	dispatcher.registerMessageCallback<ProtoFriendWorker::ThanksWorkerReq>(ProtoManager::Process<ProtoFriendWorker::ThanksWorkerReq, ProtoFriendWorker::ThanksWorkerResp, LogicFriendWorkerManager>);

	//??????
	dispatcher.registerMessageCallback<ProtoPet::UnlockPetResidenceReq>(ProtoManager::Process<ProtoPet::UnlockPetResidenceReq, ProtoPet::UnlockPetResidenceResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::GetUnlockPetInfoReq>(ProtoManager::Process<ProtoPet::GetUnlockPetInfoReq, ProtoPet::GetUnlockPetInfoResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::UnlockPetReq>(ProtoManager::Process<ProtoPet::UnlockPetReq, ProtoPet::UnlockPetResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::FeedPetReq>(ProtoManager::Process<ProtoPet::FeedPetReq, ProtoPet::FeedPetResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::TeasePetReq>(ProtoManager::Process<ProtoPet::TeasePetReq, ProtoPet::TeasePetResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::ChangePetNameReq>(ProtoManager::Process<ProtoPet::ChangePetNameReq, ProtoPet::ChangePetNameResp, LogicPetManager>);

	//???????????????
	//??????????????????
	dispatcher.registerMessageCallback<ProtoActivity::GetNewShareInfoReq>(ProtoManager::Process<ProtoActivity::GetNewShareInfoReq, ProtoActivity::GetNewShareInfoResp, LogicNewShareActivity>);
	//??????
	dispatcher.registerMessageCallback<ProtoActivity::NewShareReq>(ProtoManager::Process<ProtoActivity::NewShareReq, ProtoActivity::NewShareResp, LogicNewShareActivity>);
	//??????
	dispatcher.registerMessageCallback<ProtoActivity::RewardNewShareReq>(ProtoManager::Process<ProtoActivity::RewardNewShareReq, ProtoActivity::RewardNewShareResp, LogicNewShareActivity>);


}

void LogicManager::RegMemoryManager()
{
	//???????????????????????????????????????
	m_memoryManager.push_back(ResourceManager::Instance());
	m_memoryManager.push_back(AsynManager::Instance());
	m_memoryManager.push_back(NotifyManager::Instance());
	//???????????????????????????????????????

	//??????
	m_memoryManager.push_back(AdvertiseManager::Instance());
	m_memoryManager.push_back(MemoryAllianceManager::Instance());
	m_memoryManager.push_back(MemoryAllianceRaceGroupManager::Instance());
	m_memoryManager.push_back(MemoryActivityManager::Instance());
	m_memoryManager.push_back(DynamicInfoManager::Instance());
	m_memoryManager.push_back(FriendOrderManager::Instance());

	m_memoryManager.push_back(MessageBoardManager::Instance());
	m_memoryManager.push_back(CDKeyManager::Instance());
	m_memoryManager.push_back(MemorySysMailManager::Instance());

}

void LogicManager::RegDataManager()
{
	m_dataManager.push_back(BaseManager::Instance());
	m_dataManager.push_back(DataGameActivityManager::Instance());
	m_dataManager.push_back(DataChargeHistoryManager::Instance());
	m_dataManager.push_back(DataBuildingMgr::Instance());
	m_dataManager.push_back(DataCroplandManager::Instance());
	m_dataManager.push_back(DataItemManager::Instance());
	m_dataManager.push_back(DataProduceequipManager::Instance());
	m_dataManager.push_back(DataAnimalManager::Instance());
	m_dataManager.push_back(DataEquipmentStarManager::Instance());
	m_dataManager.push_back(DataOrderManager::Instance());
	m_dataManager.push_back(DataShopManager::Instance());
	m_dataManager.push_back(DataFruitManager::Instance());
	m_dataManager.push_back(DataTruckManager::Instance());
	m_dataManager.push_back(DataConcernManager::Instance());
	m_dataManager.push_back(DataFansManager::Instance());
	m_dataManager.push_back(DataAidRecordManager::Instance());
	m_dataManager.push_back(DataTaskManager::Instance());
	m_dataManager.push_back(DataShippingManager::Instance());
	m_dataManager.push_back(DataShippingboxManager::Instance());
	m_dataManager.push_back(DataNPCSellerManager::Instance());

	m_dataManager.push_back(DataNPCShopManager::Instance());
	m_dataManager.push_back(DataVIPGiftManager::Instance());
	//??????
	m_dataManager.push_back(DataAllianceManager::Instance());
	m_dataManager.push_back(DataAllianceMemberManager::Instance());
	m_dataManager.push_back(DataInvitedListManager::Instance());
	m_dataManager.push_back(DataAllianceApplyManager::Instance());
	m_dataManager.push_back(DataAllianceDonationManager::Instance());
	m_dataManager.push_back(DataAllianceNotifyManager::Instance());

	m_dataManager.push_back(DataMissionManager::Instance());
	m_dataManager.push_back(DataMailDogManager::Instance());
	m_dataManager.push_back(DataFriendlyTreeManager::Instance());
	m_dataManager.push_back(DataShopSellCoinManager::Instance());
	m_dataManager.push_back(DataFriendWorkerManager::Instance());

	// ??????
	m_dataManager.push_back(DataKeeperManager::Instance());
	m_dataManager.push_back(DataKeeperTaskManager::Instance());
	// ????????????
	m_dataManager.push_back(DataSysmailManager::Instance());

	//????????????
	m_dataManager.push_back(DataUserFeedbackManager::Instance());

	//??????
	m_dataManager.push_back(DataPetManager::Instance());
}

void LogicManager::RegBattleManager()
{
	m_battleManager.push_back(LogicUserManager::Instance());
	m_battleManager.push_back(LogicNotifyManager::Instance());
	m_battleManager.push_back(LogicBuildManager::Instance());
	m_battleManager.push_back(LogicProductLineManager::Instance());
	m_battleManager.push_back(LogicPropsManager::Instance());
	m_battleManager.push_back(LogicQueueManager::Instance());
	m_battleManager.push_back(LogicRoutineManager::Instance());
	m_battleManager.push_back(LogicOrderManager::Instance());
	m_battleManager.push_back(LogicShopManager::Instance());
	m_battleManager.push_back(LogicAdvertiseManager::Instance());
	m_battleManager.push_back(LogicDynamicInfoManager::Instance());
	m_battleManager.push_back(LogicLocationHelpManager::Instance());
	m_battleManager.push_back(LogicFriendOrderManager::Instance());
	m_battleManager.push_back(LogicMessageBoardManager::Instance());
	m_battleManager.push_back(LogicFriendManager::Instance());
	m_battleManager.push_back(LogicTaskManager::Instance());
	m_battleManager.push_back(LogicShippingManager::Instance());
	m_battleManager.push_back(LogicVIPManager::Instance());
	m_battleManager.push_back(LogicNPCSellerManager::Instance());
	m_battleManager.push_back(LogicNPCShopManager::Instance());
	m_battleManager.push_back(LogicAllianceManager::Instance());
	m_battleManager.push_back(LogicMissionManager::Instance());
	m_battleManager.push_back(LogicCustomerManager::Instance());
	m_battleManager.push_back(LogicRandomBoxManager::Instance());
	m_battleManager.push_back(LogicMailDogManager::Instance());
	m_battleManager.push_back(RotaryTableActivity::Instance());
	m_battleManager.push_back(LogicFriendlyTreeManager::Instance());
	m_battleManager.push_back(LogicAccessAdManager::Instance());
	m_battleManager.push_back(LogicKeeperManager::Instance());
	m_battleManager.push_back(LogicCardManager::Instance());
	m_battleManager.push_back(LogicShopSellCoinManager::Instance());
	m_battleManager.push_back(LogicSysMailManager::Instance());
	m_battleManager.push_back(LogicFriendWorkerManager::Instance());
	m_battleManager.push_back(LogicPetManager::Instance());
	m_battleManager.push_back(LogicNewShareActivity::Instance());
	m_battleManager.push_back(LogicXsgReportManager::Instance());
	//???????????????????????????????????????
	m_battleManager.push_back(UserManager::Instance());
	//???????????????????????????????????????
}

void LogicManager::RegActivityManager()
{
	m_activityManager.push_back(RechargeActivity::Instance());
	m_activityManager.push_back(DailyShareActivity::Instance());
	m_activityManager.push_back(CropsActivity::Instance());
	m_activityManager.push_back(OrderActivity::Instance());
	m_activityManager.push_back(SignInActivity::Instance());
	m_activityManager.push_back(LogicFundActivityManager::Instance());
	m_activityManager.push_back(Recharge4399ActivityManager::Instance());
	m_activityManager.push_back(Daily4399ActivityManager::Instance());
}

bool LogicManager::IsDataManagerWorking()
{
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		if((*it)->IsWorking())
			return true;
	}
	return false;
}

bool LogicManager::IsDataManagerFull()
{
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		if((*it)->IsFull())
			return true;
	}
	return false;
}

bool LogicManager::IsDataManagerNeedClear()
{
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		if((*it)->IsNeedClear())
			return true;
	}
	return false;
}

bool LogicManager::IsMemoryManagerNeedClear()
{
	for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
	{
		if((*it)->IsNeedClear())
			return true;
	}
	return false;
}

void LogicManager::DoDataManagerSave(unsigned uid)
{
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		try
		{
			(*it)->DoSave(uid);
		}
		catch(const std::exception&) {}
	}

}

void LogicManager::DoDataManagerAllianceSave(unsigned aid)
{
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		try
		{
			(*it)->DoAllianceSave(aid);
		}
		catch(const std::exception&) {}
	}

}

void LogicManager::DoDataManagerClear(unsigned uid)
{
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
	{
		try
		{
			(*it)->DoClear(uid);
		}
		catch(const std::exception&) {}
	}
}

void LogicManager::DoAllianceManagerClear(unsigned alliance_id)
{
	for(vector<DataSingletonBase*>::iterator it = m_dataManager.begin(); it != m_dataManager.end(); ++it)
	{
		try
		{
			(*it)->DoAllianceClear(alliance_id);
		}

		catch(const std::exception&) {}
	}
}

void LogicManager::DoMemoryManagerClear(unsigned uid)
{
	for(vector<DataSingletonBase*>::iterator it=m_memoryManager.begin();it!=m_memoryManager.end();++it)
	{
		try
		{
			(*it)->DoClear(uid);
		}
		catch(const std::exception&) {}
	}
}

void LogicManager::Addfd(unsigned uid, unsigned fd)
{
	m_fdmap[fd] = uid;
	m_uidmap[uid] = fd;
}

void LogicManager::Erasefd(unsigned fd)
{
	if(m_fdmap.count(fd))
	{
		m_uidmap.erase(m_fdmap[fd]);
		m_fdmap.erase(fd);
	}
}

void LogicManager::Eraseuid(unsigned uid)
{
	if(m_uidmap.count(uid))
	{
		m_fdmap.erase(m_uidmap[uid]);
		m_uidmap.erase(uid);
	}
}

unsigned LogicManager::Getfd(unsigned uid)
{
	if(m_uidmap.count(uid))
		return m_uidmap[uid];
	return -1;
}

unsigned LogicManager::Getuid(unsigned fd)
{
	if(m_fdmap.count(fd))
		return m_fdmap[fd];
	return -1;
}

void LogicManager::EraseLeaveList(unsigned uid)
{
	for(list<pair<unsigned, unsigned> >::iterator it = m_leaveList.begin();it != m_leaveList.end();++it)
	{
		if(it->second == uid)
		{
			m_leaveList.erase(it);
			return;
		}
	}
}

void LogicManager::ClearUser(bool send)
{
	vector<unsigned> uids;
	UMI->GetOnlineUsers(uids);

	info_log("kick close %u", uids.size());
	for(vector<unsigned>::iterator it=uids.begin();it!=uids.end();++it)
	{
		if(send)
			sendKickMsg(Getfd(*it), "server_close");
		offline(*it);
	}

	m_fdmap.clear();
	m_uidmap.clear();
	m_leaveList.clear();
	channelId = -1; 
	m_fd = -1;
}

unsigned LogicManager::GetOpenDays()
{
	return CTime::GetDayInterval(SecOpenTime, Time::GetGlobalTime());
}

void LogicManager::CheckSave(){
	debug_log("---------------------------");
	debug_log("CheckSave!");
	debug_log("---------------------------");
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
		(*it)->OnCheck();
}

void LogicManager::DataLog(){
	debug_log("---------------------------");
	for(vector<DataSingletonBase*>::iterator it=m_dataManager.begin();it!=m_dataManager.end();++it)
		(*it)->DebugLog();
	debug_log("---------------------------");
}

void LogicManager::TryClear()
{
	debug_log("---------------------------");
	debug_log("%s",IsDataManagerWorking()?"DataManager Working":"DataManager not Working");
	debug_log("%s",IsDataManagerNeedClear()?"DataManager Need Clear":"DataManager not Need Clear");
	vector<unsigned> uids;
	BaseManager::Instance()->TryClear(uids);
	debug_log("clear num: %u",uids.size());
	debug_log("---------------------------");
	debug_log("%s",IsMemoryManagerNeedClear()?"MemoryManager Need Clear":"MemoryManager not Need Clear");
	vector<unsigned> uids1;
	ResourceManager::Instance()->TryClear(uids1);
	debug_log("clear num: %u",uids1.size());
	debug_log("---------------------------");
}

void LogicManager::CheckMin()
{
	info_log("Online:%u",m_fdmap.size());

	User::ServerTime * msg = new User::ServerTime;
	msg->set_ts(now);
	broadcastMsg(msg);

	for(vector<ActivitySingletonBase*>::iterator it=m_activityManager.begin();it!=m_activityManager.end();++it)
	{
		try
		{
			if((*it)->IsOn())
				(*it)->OnMin();
		}
		catch(const std::exception&) {}
	}

	LogicFriendOrderManager::Instance()->CheckClearFoInfo();
}

void LogicManager::CheckHour()
{
	m_timer = 0;

	if(LogicCommonUtil::GetHourByTime(Time::GetGlobalTime()) == 0)
		CheckDay();

	for(vector<ActivitySingletonBase*>::iterator it=m_activityManager.begin();it!=m_activityManager.end();++it)
	{
		try
		{
			if((*it)->IsOn())
				(*it)->OnHour();
		}
		catch(const std::exception&) {}
	}

	//??????????????????
	LogicAdvertiseManager::Instance()->DelOldAd();
	//??????????????????
	LogicDynamicInfoManager::Instance()->CheckClearDyInfo();
	//??????????????????
	LogicMessageBoardManager::Instance()->CheckClearMsgInfo();
}

void LogicManager::CheckDay()
{
	for(vector<ActivitySingletonBase*>::iterator it=m_activityManager.begin();it!=m_activityManager.end();++it)
	{
		try
		{
			(*it)->CheckDay();
		}
		catch(const std::exception&) {}
	}

	LogicUserManager::Instance()->EveryDayAction();
	MemorySysMailManager::Instance()->OnDay();

	//?????????????????????????????????????????????
	LogicFriendOrderManager::Instance()->CheckRecycleOldSourceFo();
	//??????????????????????????????
	LogicMessageBoardManager::Instance()->UpdateFeedbackTimes();
	//???????????????????????????????????????
	LogicAllianceManager::Instance()->UpdateWatchAd();

	//4399??????????????????0?????????????????????
	Daily4399ActivityManager::Instance()->CheckDaily();
}

void LogicManager::OnReload()
{
	for(vector<ActivitySingletonBase*>::iterator it=m_activityManager.begin();it!=m_activityManager.end();++it)
	{
		try
		{
			(*it)->OnReload();
		}
		catch(const std::exception&) {}
	}
}

void LogicManager::CheckSig(){
	switch(m_signum)
	{
	case e_Sig_Save:
		CheckSave();
		break;
	case e_Sig_data_log:
		DataLog();
		break;
	case e_Sig_try_clear:
		TryClear();
		break;
	case e_Sig_print_info:
		LogicResourceManager::Instance()->Print();
		break;
	default:
		break;
	}
	m_signum = 0;
}

template<class Type>
void LogicManager::ObjInit(vector<Type*>& obj){
	int ret1 = 0;
	int ret2 = 0;
	for(auto it=obj.begin();it!=obj.end();++it)
	{
		try
		{
			if(has_member_Init<Type>::value)
				ret1 = (*it)->Init();
			if(has_member_OnInit<Type>::value)
				ret2 = (*it)->OnInit();
			if(ret1 || ret2)
			{
				error_log("%s Init error!", typeid(*it).name());
				return;
			}
		}
		catch(const std::exception&)
		{
			error_log("%s Init error!", typeid(*it).name());
			return;
		}
	}

} 
