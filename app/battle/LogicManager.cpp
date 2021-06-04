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

			//@add oulong 20161009 首充
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

		//小米平台充值邮件提醒
		if(LogicSysMailManager::Instance()->ValidPlatForm(uid))
		{
			LogicSysMailManager::Instance()->FirstRechargeAddMail(uid);
		}

		//西山居充值上报
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
		/*禁止同步接口
		ErrorRet::ErrorRet* reply = new ErrorRet::ErrorRet;
		reply->set_errorret(ret);
		reply->set_errormsg(m_errmsg);
		reply->set_requestmsg(packet->m_msg->GetTypeName());
		pReplyProtocol = reply;
		needDelReply = true;
		*/
	}

	/*禁止同步接口
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
	//todo 此处是原来的心跳，应该无用
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
		//todo 此处是下线的逻辑处理
		UserManager::Instance()->UserOffLine(uid);
		//队列离线处理
		LogicQueueManager::Instance()->Offline(uid);
		//删除定时任务管理器中该用户的任务
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
	//抛弃挤压消息，避免消息阻塞引起的雪崩
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
	m_leaveList.push_back(make_pair(Time::GetGlobalTime(), Getuid(packet->fd)));
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
	//登录
	dispatcher.registerMessageCallback<Common::Login>(ProtoManager::ProcessLogin);
	//读取别人数据
	dispatcher.registerMessageCallback<User::RequestOtherUser>(ProtoManager::ProcessNoReply<User::RequestOtherUser, UserManager>);
	//跨服读取别人数据
	dispatcher.registerMessageCallback<User::RequestOtherUserBC>(ProtoManager::ProcessNoReplyNoUID<User::RequestOtherUserBC, UserManager>);
	//新手
	dispatcher.registerMessageCallback<User::Tutorialstage>(ProtoManager::ProcessNoReply<User::Tutorialstage, UserManager>);
	//开关状态
	dispatcher.registerMessageCallback<User::SwitchStatus>(ProtoManager::ProcessNoReply<User::SwitchStatus, UserManager>);
	//起名
	dispatcher.registerMessageCallback<Common::ChangeName>(ProtoManager::Process<Common::ChangeName, Common::ReplyChangeName, UserManager>);
	/***********GM********************/
	//加钱
	dispatcher.registerMessageCallback<Admin::AddCash>(ProtoManager::ProcessNoUID<Admin::AddCash, Admin::ReplyAddCash, UserManager>);
	//批量加资源或道具
	dispatcher.registerMessageCallback<Admin::AsycAdd>(ProtoManager::ProcessNoUID<Admin::AsycAdd, Admin::AsycAddResp, UserManager>);
	//查询封号
	dispatcher.registerMessageCallback<Admin::RequestForbidTS>(ProtoManager::ProcessNoUID<Admin::RequestForbidTS, Admin::ReplyForbidTS, UserManager>);
	//封号
	dispatcher.registerMessageCallback<Admin::SetForbidTS>(ProtoManager::ProcessNoReplyNoUID<Admin::SetForbidTS, UserManager>);
	//设置商会竞赛分组
	dispatcher.registerMessageCallback<Admin::SetAllianceRaceGroup>(ProtoManager::ProcessNoReplyNoUID<Admin::SetAllianceRaceGroup, UserManager>);
	//设置活动状态
	dispatcher.registerMessageCallback<Admin::SetActivity>(ProtoManager::ProcessNoReplyNoUID<Admin::SetActivity, UserManager>);
	//设置版本号
	dispatcher.registerMessageCallback<User::SetVersion>(ProtoManager::ProcessNoReply<User::SetVersion, UserManager>);
	//设置标志位
	dispatcher.registerMessageCallback<User::SetFlag>(ProtoManager::ProcessNoReply<User::SetFlag, UserManager>);
	//请求是否有新消息
	dispatcher.registerMessageCallback<User::ReqNewMsg>(ProtoManager::Process<User::ReqNewMsg, User::ReplyNewMsg, UserManager>);
	//心跳请求
	dispatcher.registerMessageCallback<User::HeartBeatReq>(ProtoManager::Process<User::HeartBeatReq, User::HeartBeatResp, UserManager>);

	//获取点赞信息
	dispatcher.registerMessageCallback<User::GetThumbsUpReq>(ProtoManager::Process<User::GetThumbsUpReq, User::GetThumbsUpResp, LogicUserManager>);
	//排行榜点赞
	dispatcher.registerMessageCallback<User::RankThumbsUpReq>(ProtoManager::ProcessNoReply<User::RankThumbsUpReq, LogicUserManager>);
	//跨服记录点赞
	dispatcher.registerMessageCallback<User::CSRankThumbsUpReq>(ProtoManager::ProcessNoReplyNoUID<User::CSRankThumbsUpReq, LogicUserManager>);
	//点赞跨服返回
	dispatcher.registerMessageCallback<User::CSRankThumbsUpResp>(ProtoManager::ProcessNoReplyNoUID<User::CSRankThumbsUpResp, LogicUserManager>);
	//获取世界频道求助信息
	dispatcher.registerMessageCallback<User::GetWorldChannelHelpCntReq>(ProtoManager::Process<User::GetWorldChannelHelpCntReq, User::GetWorldChannelHelpCntResp, LogicUserManager>);
	//世界频道求助
	dispatcher.registerMessageCallback<User::WorldChannelHelpReq>(ProtoManager::Process<User::WorldChannelHelpReq, User::WorldChannelHelpResp, LogicUserManager>);
	//新手引导分享奖励
	dispatcher.registerMessageCallback<User::NewUserGuideShareReq>(ProtoManager::Process<User::NewUserGuideShareReq, User::NewUserGuideShareResp, LogicUserManager>);
	//升级领取奖励
	dispatcher.registerMessageCallback<ProtoPush::RewardLevelUpReq>(ProtoManager::Process<ProtoPush::RewardLevelUpReq, ProtoPush::RewardLevelUpResp, LogicUserManager>);
	//cdkey
	dispatcher.registerMessageCallback<User::UseCdKeyReq>(ProtoManager::Process<User::UseCdKeyReq, User::UseCdKeyResp, LogicUserManager>);

	//导出
	dispatcher.registerMessageCallback<ProtoArchive::ExportReq>(ProtoManager::ProcessNoUID<ProtoArchive::ExportReq, ProtoArchive::ExportResp, UserManager>);
	//导入
	dispatcher.registerMessageCallback<ProtoArchive::ImportReq>(ProtoManager::ProcessNoUID<ProtoArchive::ImportReq, ProtoArchive::ImportResp, UserManager>);
	//钻石消耗
	dispatcher.registerMessageCallback<User::CostCashReq>(ProtoManager::Process<User::CostCashReq, User::CostCashResp, LogicUserManager>);
	//每日分享奖励
	dispatcher.registerMessageCallback<User::ShareRewardsReq>(ProtoManager::Process<User::ShareRewardsReq, User::ShareRewardsResp, DailyShareActivity>);
	dispatcher.registerMessageCallback<User::ShareTotalRewardsReq>(ProtoManager::Process<User::ShareTotalRewardsReq, User::ShareTotalRewardsResp, DailyShareActivity>);
	dispatcher.registerMessageCallback<User::DaliyShareReq>(ProtoManager::Process<User::DaliyShareReq, User::DaliyShareResp, DailyShareActivity>);

	//首充领奖励
	dispatcher.registerMessageCallback<ProtoReward::GetFirstRechargeRewardReq>(ProtoManager::Process<ProtoReward::GetFirstRechargeRewardReq, ProtoReward::GetFirstRechargeRewardResp, LogicUserManager>);
	//关注公众号奖励
	dispatcher.registerMessageCallback<ProtoReward::GetFollowPublicRewardReq>(ProtoManager::Process<ProtoReward::GetFollowPublicRewardReq, ProtoReward::GetFollowPublicRewardResp, LogicUserManager>);
	//收获产品看广告获得奖励
	dispatcher.registerMessageCallback<ProtoReward::FetchProductWatchAdsRewardReq>(ProtoManager::Process<ProtoReward::FetchProductWatchAdsRewardReq, ProtoReward::FetchProductWatchAdsRewardResp, LogicUserManager>);

	dispatcher.registerMessageCallback<Common::ShutDown>(ProtoManager::ProcessNoReply<Common::ShutDown, UserManager>);
	//购买材料
	dispatcher.registerMessageCallback<User::BuyMaterialReq>(ProtoManager::Process<User::BuyMaterialReq, User::BuyMaterialResp, LogicPropsManager>);
	//看广告获取加速卡
	dispatcher.registerMessageCallback<User::ViewAdGetSpeedUpCardReq>(ProtoManager::Process<User::ViewAdGetSpeedUpCardReq, User::ViewAdGetSpeedUpCardResp, LogicPropsManager>);

	//内政
	//建造
	dispatcher.registerMessageCallback<ProtoBuilding::BuildReq>(ProtoManager::Process<ProtoBuilding::BuildReq, ProtoBuilding::BuildResp, LogicBuildManager>);
	//生产设备揭幕
	dispatcher.registerMessageCallback<ProtoBuilding::UnveilBuildReq>(ProtoManager::Process<ProtoBuilding::UnveilBuildReq, ProtoBuilding::UnveilBuildResp, LogicBuildManager>);
	//移动
	dispatcher.registerMessageCallback<ProtoBuilding::MoveReq>(ProtoManager::Process<ProtoBuilding::MoveReq, ProtoBuilding::MoveResp, LogicBuildManager>);
	//翻转
	dispatcher.registerMessageCallback<ProtoBuilding::FlipReq>(ProtoManager::Process<ProtoBuilding::FlipReq, ProtoBuilding::FlipResp, LogicBuildManager>);
	//仓库升级
	dispatcher.registerMessageCallback<ProtoBuilding::BuildingUpReq>(ProtoManager::Process<ProtoBuilding::BuildingUpReq, ProtoBuilding::BuildingUpResp, LogicBuildManager>);
	//设备星级加速
	dispatcher.registerMessageCallback<ProtoBuilding::UpgradeStarSpeedUpReq>(ProtoManager::Process<ProtoBuilding::UpgradeStarSpeedUpReq, ProtoBuilding::UpgradeStarSpeedUpResp, LogicBuildManager>);
	//拆除障碍物
	dispatcher.registerMessageCallback<ProtoBuilding::RemoveBarrierReq>(ProtoManager::Process<ProtoBuilding::RemoveBarrierReq, ProtoBuilding::RemoveBarrierResp, LogicBuildManager>);

	dispatcher.registerMessageCallback<ProtoBuilding::SellDecorateReq>(ProtoManager::Process<ProtoBuilding::SellDecorateReq, ProtoBuilding::SellDecorateResq, LogicBuildManager>);
	//通用加速
	dispatcher.registerMessageCallback<User::SpeedUpReq>(ProtoManager::Process<User::SpeedUpReq, User::SpeedUpResp, LogicQueueManager>);
	//看广告加速建筑
	dispatcher.registerMessageCallback<ProtoBuilding::ViewAdReduceBuildTimeReq>(ProtoManager::Process<ProtoBuilding::ViewAdReduceBuildTimeReq, ProtoBuilding::ViewAdReduceBuildTimeResp, LogicBuildManager>);
	//获取每日看广告加速建筑的剩余次数
	dispatcher.registerMessageCallback<ProtoBuilding::GetViewAdReduceBuildTimeReq>(ProtoManager::Process<ProtoBuilding::GetViewAdReduceBuildTimeReq, ProtoBuilding::GetViewAdReduceBuildTimeResp, LogicBuildManager>);

	//---------------地块生产线
	//种植
	dispatcher.registerMessageCallback<ProtoProduce::PlantCropReq>(ProtoManager::Process<ProtoProduce::PlantCropReq, ProtoProduce::PlantCropResp, LogicProductLineManager>);
	//收割
	dispatcher.registerMessageCallback<ProtoProduce::ReapCropReq>(ProtoManager::Process<ProtoProduce::ReapCropReq, ProtoProduce::ReapCropResp, LogicProductLineManager>);

	//---------------设备生产
	//扩展队列
	dispatcher.registerMessageCallback<ProtoProduce::ExpandQueueReq>(ProtoManager::Process<ProtoProduce::ExpandQueueReq, ProtoProduce::ExpandQueueResp, LogicProductLineManager>);
	//放入生产队列
	dispatcher.registerMessageCallback<ProtoProduce::JoinQueueReq>(ProtoManager::Process<ProtoProduce::JoinQueueReq, ProtoProduce::JoinQueueResp, LogicProductLineManager>);
	//取回仓库
	dispatcher.registerMessageCallback<ProtoProduce::FetchProductReq>(ProtoManager::Process<ProtoProduce::FetchProductReq, ProtoProduce::FetchProductResp, LogicProductLineManager>);

	//----------------动物生产
	//领养动物
	dispatcher.registerMessageCallback<ProtoProduce::AdoptAnimalReq>(ProtoManager::Process<ProtoProduce::AdoptAnimalReq, ProtoProduce::AdoptAnimalResp, LogicProductLineManager>);
	//喂养动物
	dispatcher.registerMessageCallback<ProtoProduce::FeedAnimalReq>(ProtoManager::Process<ProtoProduce::FeedAnimalReq, ProtoProduce::FeedAnimalResp, LogicProductLineManager>);
	//获取产品
	dispatcher.registerMessageCallback<ProtoProduce::ObtainProductReq>(ProtoManager::Process<ProtoProduce::ObtainProductReq, ProtoProduce::ObtainProductResp, LogicProductLineManager>);

	//---------------果树生产
	//果树收割
	dispatcher.registerMessageCallback<ProtoProduce::ReapFruitReq>(ProtoManager::Process<ProtoProduce::ReapFruitReq, ProtoProduce::ReapFruitResp, LogicProductLineManager>);
	//求助
	dispatcher.registerMessageCallback<ProtoProduce::SeekHelpReq>(ProtoManager::Process<ProtoProduce::SeekHelpReq, ProtoProduce::SeekHelpResp, LogicProductLineManager>);
	//砍树
	dispatcher.registerMessageCallback<ProtoProduce::CutFruitTreeReq>(ProtoManager::Process<ProtoProduce::CutFruitTreeReq, ProtoProduce::CutFruitTreeResp, LogicProductLineManager>);
	//提供帮助
	dispatcher.registerMessageCallback<ProtoProduce::OfferHelpReq>(ProtoManager::ProcessNoReply<ProtoProduce::OfferHelpReq, LogicProductLineManager>);
	//跨服提供帮助
	dispatcher.registerMessageCallback<ProtoProduce::CSOfferHelpReq>(ProtoManager::ProcessNoReplyNoUID<ProtoProduce::CSOfferHelpReq, LogicProductLineManager>);
	//处理跨服提供帮助后的返回信息
	dispatcher.registerMessageCallback<ProtoProduce::CSOfferHelpResp>(ProtoManager::ProcessNoReplyNoUID<ProtoProduce::CSOfferHelpResp, LogicProductLineManager>);
	//确认帮助
	dispatcher.registerMessageCallback<ProtoProduce::ConfirmHelpReq>(ProtoManager::Process<ProtoProduce::ConfirmHelpReq, ProtoProduce::ConfirmHelpResp, LogicProductLineManager>);

	//GM
	dispatcher.registerMessageCallback<ProtoGM::GMCmdReq>(ProtoManager::ProcessNoReply<ProtoGM::GMCmdReq, LogicGM>);

	//通知系统
	dispatcher.registerMessageCallback<ProtoNotify::GetNotifyReq>(ProtoManager::Process<ProtoNotify::GetNotifyReq, ProtoNotify::GetNotifyResp, LogicNotifyManager>);

	//-----------------订单
	//订单查询
	dispatcher.registerMessageCallback<ProtoOrder::OrderQueryReq>(ProtoManager::Process<ProtoOrder::OrderQueryReq, ProtoOrder::OrderResp, LogicOrderManager>);
	//卡车运输
	dispatcher.registerMessageCallback<ProtoOrder::StartOrderReq>(ProtoManager::Process<ProtoOrder::StartOrderReq, ProtoOrder::StartOrderResp, LogicOrderManager>);
	//撕单
	dispatcher.registerMessageCallback<ProtoOrder::DeleteOrderReq>(ProtoManager::Process<ProtoOrder::DeleteOrderReq, ProtoOrder::DeleteOrderResp, LogicOrderManager>);
	//卡车查询
	dispatcher.registerMessageCallback<ProtoOrder::TruckQueryReq>(ProtoManager::Process<ProtoOrder::TruckQueryReq, ProtoOrder::TruckResp, LogicOrderManager>);
	//卡车奖励
	dispatcher.registerMessageCallback<ProtoOrder::RewardOrderReq>(ProtoManager::Process<ProtoOrder::RewardOrderReq, ProtoOrder::RewardOrderResp, LogicOrderManager>);
	//获取订单加成信息
	dispatcher.registerMessageCallback<ProtoOrder::GetOrderBonusInfoReq>(ProtoManager::Process<ProtoOrder::GetOrderBonusInfoReq, ProtoOrder::GetOrderBonusInfoResp, LogicOrderManager>);
	//看广告
	dispatcher.registerMessageCallback<ProtoOrder::ViewAdReq>(ProtoManager::Process<ProtoOrder::ViewAdReq, ProtoOrder::ViewAdResp, LogicOrderManager>);
	//花钻购买订单加成
	dispatcher.registerMessageCallback<ProtoOrder::BuyOrderBonusReq>(ProtoManager::Process<ProtoOrder::BuyOrderBonusReq, ProtoOrder::BuyOrderBonusResp, LogicOrderManager>);

	//-------------商店
	//获取商店信息
	dispatcher.registerMessageCallback<ProtoShop::GetShopReq>(ProtoManager::Process<ProtoShop::GetShopReq, ProtoShop::GetShopResp, LogicShopManager>);
	//商品上架
	dispatcher.registerMessageCallback<ProtoShop::ShelfPropsReq>(ProtoManager::Process<ProtoShop::ShelfPropsReq, ProtoShop::ShelfPropsResp, LogicShopManager>);
	//解锁货架
	dispatcher.registerMessageCallback<ProtoShop::ShelfUnLockReq>(ProtoManager::Process<ProtoShop::ShelfUnLockReq, ProtoShop::ShelfUnLockResp, LogicShopManager>);
	//访问玩家商店
	dispatcher.registerMessageCallback<ProtoShop::VisitOtherShopReq>(ProtoManager::ProcessNoReply<ProtoShop::VisitOtherShopReq, LogicShopManager>);
	//跨服访问商店
	dispatcher.registerMessageCallback<ProtoShop::CSVisitOtherShopReq>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSVisitOtherShopReq, LogicShopManager>);
	//处理跨服访问商店回应的消息
	dispatcher.registerMessageCallback<ProtoShop::CSVisitOtherShopResp>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSVisitOtherShopResp, LogicShopManager>);

	//购买接口
	dispatcher.registerMessageCallback<ProtoShop::PurchaseReq>(ProtoManager::ProcessNoReply<ProtoShop::PurchaseReq, LogicShopManager>);
	//跨服购买
	dispatcher.registerMessageCallback<ProtoShop::CSPurchaseReq>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSPurchaseReq, LogicShopManager>);
	//处理跨服购买后的回应消息
	dispatcher.registerMessageCallback<ProtoShop::CSPurchaseResp>(ProtoManager::ProcessNoReplyNoUID<ProtoShop::CSPurchaseResp, LogicShopManager>);

	//确认收钱
	dispatcher.registerMessageCallback<ProtoShop::ConfirmPaymentReq>(ProtoManager::Process<ProtoShop::ConfirmPaymentReq, ProtoShop::ConfirmPaymentResp, LogicShopManager>);
	//修改货架订单信息
	dispatcher.registerMessageCallback<ProtoShop::ModifyShelfInfoReq>(ProtoManager::Process<ProtoShop::ModifyShelfInfoReq, ProtoShop::ModifyShelfInfoResp, LogicShopManager>);
	//商店看广告回收物品
	dispatcher.registerMessageCallback<ProtoShop::ViewAdRecycleItemReq>(ProtoManager::Process<ProtoShop::ViewAdRecycleItemReq, ProtoShop::ViewAdRecycleItemResp, LogicShopManager>);
	//系统购买
	dispatcher.registerMessageCallback<ProtoShop::BuyShopItemBySystemReq>(ProtoManager::Process<ProtoShop::BuyShopItemBySystemReq, ProtoShop::BuyShopItemBySystemResp, LogicShopManager>);

	//-------------好友
	//获得好友数据
	dispatcher.registerMessageCallback<ProtoFriend::GetAllFriendsReq>(ProtoManager::ProcessNoReply<ProtoFriend::GetAllFriendsReq, LogicFriendManager>);
	//获取需要帮助的好友列表
	dispatcher.registerMessageCallback<ProtoFriend::GetFriendHelpInfoReq>(ProtoManager::ProcessNoReply<ProtoFriend::GetFriendHelpInfoReq, LogicFriendManager>);
	//跨服获取需要帮助的好友列表
	dispatcher.registerMessageCallback<ProtoFriend::CSGetFriendHelpInfoReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSGetFriendHelpInfoReq, LogicFriendManager>);
	//关注
	dispatcher.registerMessageCallback<ProtoFriend::ConcernReq>(ProtoManager::ProcessNoReply<ProtoFriend::ConcernReq, LogicFriendManager>);
	//跨服关注
	dispatcher.registerMessageCallback<ProtoFriend::CSConcernReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSConcernReq, LogicFriendManager>);
	//处理跨服关注后的返回消息
	dispatcher.registerMessageCallback<ProtoFriend::CSConcernResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSConcernResp, LogicFriendManager>);
	//取消关注
	dispatcher.registerMessageCallback<ProtoFriend::CancelConcernReq>(ProtoManager::ProcessNoReply<ProtoFriend::CancelConcernReq, LogicFriendManager>);
	//跨服取消关注
	dispatcher.registerMessageCallback<ProtoFriend::CSCancelConcernReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSCancelConcernReq, LogicFriendManager>);
	//处理跨服取消关注后的返回消息
	dispatcher.registerMessageCallback<ProtoFriend::CSCancelConcernResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriend::CSCancelConcernResp, LogicFriendManager>);
	//删除粉丝
	dispatcher.registerMessageCallback<ProtoFriend::RemoveFansReq>(ProtoManager::Process<ProtoFriend::RemoveFansReq, ProtoFriend::RemoveFansResp, LogicFriendManager>);

	//--------广告
	//获取广告信息
	dispatcher.registerMessageCallback<ProtoAdvertise::GetAdInfoReq>(ProtoManager::Process<ProtoAdvertise::GetAdInfoReq, ProtoAdvertise::GetAdInfoResp, LogicAdvertiseManager>);
	//花钻秒广告cd
	dispatcher.registerMessageCallback<ProtoAdvertise::UpdateAdCdReq>(ProtoManager::Process<ProtoAdvertise::UpdateAdCdReq, ProtoAdvertise::UpdateAdCdResp, LogicAdvertiseManager>);

	//------动态
	//获取动态信息 同服
	dispatcher.registerMessageCallback<ProtoDynamicInfo::GetDynamicInfoReq>(ProtoManager::Process<ProtoDynamicInfo::GetDynamicInfoReq, ProtoDynamicInfo::GetDynamicInfoResp, LogicDynamicInfoManager>);
	//删除动态信息 同服
	dispatcher.registerMessageCallback<ProtoDynamicInfo::DeleteDynamicInfoReq>(ProtoManager::ProcessNoReply<ProtoDynamicInfo::DeleteDynamicInfoReq, LogicDynamicInfoManager>);
	//接收跨服访问产生动态消息 跨服
	dispatcher.registerMessageCallback<ProtoDynamicInfo::RequestOtherUserMakeDy>(ProtoManager::ProcessNoReplyNoUID<ProtoDynamicInfo::RequestOtherUserMakeDy, LogicDynamicInfoManager>);
	//请求更新有动态状态 同服
	dispatcher.registerMessageCallback<ProtoDynamicInfo::HasNewDynamicInfoReq>(ProtoManager::Process<ProtoDynamicInfo::HasNewDynamicInfoReq, ProtoDynamicInfo::HasNewDynamicInfoResp, LogicDynamicInfoManager>);
	//点击好友订单动态跳转 同服
	dispatcher.registerMessageCallback<ProtoDynamicInfo::ClickOrderHelpReq>(ProtoManager::Process<ProtoDynamicInfo::ClickOrderHelpReq, ProtoDynamicInfo::ClickOrderHelpResp, LogicDynamicInfoManager>);

	//------留言板
	//前端主动获取留言消息
	dispatcher.registerMessageCallback<ProtoMessageBoard::GetMessageBoardReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::GetMessageBoardReq, LogicMessageBoardManager>);
	//前端请求删除留言消息
	dispatcher.registerMessageCallback<ProtoMessageBoard::DeleteMessageBoardReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::DeleteMessageBoardReq, LogicMessageBoardManager>);
	//前端请求更新有留言状态 同服
	dispatcher.registerMessageCallback<ProtoMessageBoard::HasNewLeaveMessageReq>(ProtoManager::Process<ProtoMessageBoard::HasNewLeaveMessageReq, ProtoMessageBoard::HasNewLeaveMessageResp, LogicMessageBoardManager>);
	//跨服 观看别人家的留言板内容
	dispatcher.registerMessageCallback<ProtoMessageBoard::GetMasterVisiableMsgReq>(ProtoManager::ProcessNoReplyNoUID<ProtoMessageBoard::GetMasterVisiableMsgReq, LogicMessageBoardManager>);
	//跨服 在别人家删除自己的留言
	dispatcher.registerMessageCallback<ProtoMessageBoard::DeleteMyMsgOverServerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoMessageBoard::DeleteMyMsgOverServerReq, LogicMessageBoardManager>);
	//同服 在别人家给他留言
	dispatcher.registerMessageCallback<ProtoMessageBoard::LeaveMessageReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::LeaveMessageReq, LogicMessageBoardManager>);
	//同服 在自己家回复留言
	dispatcher.registerMessageCallback<ProtoMessageBoard::AnswerLeaveMessageReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::AnswerLeaveMessageReq, LogicMessageBoardManager>);
	//跨服 在别人家给他留言 或者 在自己家回复别人的留言
	dispatcher.registerMessageCallback<ProtoMessageBoard::SendLeaveMsgOverServerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoMessageBoard::SendLeaveMsgOverServerReq, LogicMessageBoardManager>);

	//------用户反馈
	//获取玩家的反馈消息 同服
	dispatcher.registerMessageCallback<ProtoMessageBoard::GetFeedbackReq>(ProtoManager::Process<ProtoMessageBoard::GetFeedbackReq, ProtoMessageBoard::GetFeedbackResp, LogicMessageBoardManager>);
	//玩家发送系统反馈消息 同服
	dispatcher.registerMessageCallback<ProtoMessageBoard::SendFeedbackReq>(ProtoManager::Process<ProtoMessageBoard::SendFeedbackReq, ProtoMessageBoard::SendFeedbackResp, LogicMessageBoardManager>);
	//玩家删除自己的反馈消息 同服
	dispatcher.registerMessageCallback<ProtoMessageBoard::DelFeedbackReq>(ProtoManager::ProcessNoReply<ProtoMessageBoard::DelFeedbackReq, LogicMessageBoardManager>);

	//------定向请求
	//定向请求好友帮助
	dispatcher.registerMessageCallback<ProtoDynamicInfo::LocationHelpReq>(ProtoManager::ProcessNoReply<ProtoDynamicInfo::LocationHelpReq, LogicLocationHelpManager>);
	//处理好友帮助消息 后台收到请求后将其降级为普通消息
	dispatcher.registerMessageCallback<ProtoDynamicInfo::HandleLocationHelpReq>(ProtoManager::Process<ProtoDynamicInfo::HandleLocationHelpReq, ProtoDynamicInfo::HandleLocationHelpResq, LogicLocationHelpManager>);

	//------好友订单
	//获取全部好友订单 同服
	dispatcher.registerMessageCallback<ProtoFriendOrder::GetFriendOrderReq>(ProtoManager::Process<ProtoFriendOrder::GetFriendOrderReq, ProtoFriendOrder::GetFriendOrderResp, LogicFriendOrderManager>);
	//发起订单请求 同服
	dispatcher.registerMessageCallback<ProtoFriendOrder::SendFriendOrderReq>(ProtoManager::Process<ProtoFriendOrder::SendFriendOrderReq, ProtoFriendOrder::SendFriendOrderResp, LogicFriendOrderManager>);
	//跨服 接收好友订单
	dispatcher.registerMessageCallback<ProtoFriendOrder::SendOtherServerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::SendOtherServerReq, LogicFriendOrderManager>);
	//点击好友订单中的某个订单
	dispatcher.registerMessageCallback<ProtoFriendOrder::ClickFriendOrderReq>(ProtoManager::ProcessNoReply<ProtoFriendOrder::ClickFriendOrderReq, LogicFriendOrderManager>);
	//跨服 查询源订单状态
	dispatcher.registerMessageCallback<ProtoFriendOrder::RecallSourceFoReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::RecallSourceFoReq, LogicFriendOrderManager>);
	//跨服 更新好友订单状态
	dispatcher.registerMessageCallback<ProtoFriendOrder::ChangeFoStatusReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::ChangeFoStatusReq, LogicFriendOrderManager>);
	//点击“没问题”按钮 购买该条好友订单
	dispatcher.registerMessageCallback<ProtoFriendOrder::BuyFriendOrderReq>(ProtoManager::ProcessNoReply<ProtoFriendOrder::BuyFriendOrderReq, LogicFriendOrderManager>);
	//跨服 购买该条好友订单
	dispatcher.registerMessageCallback<ProtoFriendOrder::RecallCanBuyFoReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::RecallCanBuyFoReq, LogicFriendOrderManager>);
	//跨服 处理购买成功与否
	dispatcher.registerMessageCallback<ProtoFriendOrder::AnswerWhetherCanBuyReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendOrder::AnswerWhetherCanBuyReq, LogicFriendOrderManager>);
	//回收发出的订单 后台给玩家增加物品，并改变该条源订单为冷却状态或者直接删除该条订单
	dispatcher.registerMessageCallback<ProtoFriendOrder::GetOrderRewardsReq>(ProtoManager::Process<ProtoFriendOrder::GetOrderRewardsReq, ProtoFriendOrder::GetOrderRewardsResp, LogicFriendOrderManager>);
	//花费钻石清除冷却时间或者删除正在发送的订单
	dispatcher.registerMessageCallback<ProtoFriendOrder::CostDiamondReq>(ProtoManager::Process<ProtoFriendOrder::CostDiamondReq, ProtoFriendOrder::CostDiamondResp, LogicFriendOrderManager>);

	//------任务
	//获取所有任务
	dispatcher.registerMessageCallback<ProtoTask::GetTaskReq>(ProtoManager::Process<ProtoTask::GetTaskReq, ProtoTask::GetTaskResp, LogicTaskManager>);
	//领取任务
	dispatcher.registerMessageCallback<ProtoTask::RewardTaskReq>(ProtoManager::Process<ProtoTask::RewardTaskReq, ProtoTask::RewardTaskResp, LogicTaskManager>);
	//-----单线任务
	//获取任务
	dispatcher.registerMessageCallback<ProtoMission::GetCurMissionReq>(ProtoManager::Process<ProtoMission::GetCurMissionReq, ProtoMission::GetCurMissionResp, LogicMissionManager>);
	//领取任务
	dispatcher.registerMessageCallback<ProtoMission::RewardMissionReq>(ProtoManager::Process<ProtoMission::RewardMissionReq, ProtoMission::RewardMissionResp, LogicMissionManager>);

	//--------vip
	//获取礼包
	dispatcher.registerMessageCallback<ProtoVIP::GetVIPGiftReq>(ProtoManager::Process<ProtoVIP::GetVIPGiftReq, ProtoVIP::GetVIPGiftResp, LogicVIPManager>);
	//vip生产设备生产产品使用免费次数加速
	dispatcher.registerMessageCallback<ProtoVIP::VIPProductSpeedUpReq>(ProtoManager::Process<ProtoVIP::VIPProductSpeedUpReq, ProtoVIP::VIPProductSpeedUpResp, LogicVIPManager>);
	//vip 免撕单等待
	dispatcher.registerMessageCallback<ProtoVIP::VIPRemoveOrderCDReq>(ProtoManager::Process<ProtoVIP::VIPRemoveOrderCDReq, ProtoVIP::VIPRemoveOrderCDResp, LogicVIPManager>);
	//vip添加商店货架
	dispatcher.registerMessageCallback<ProtoVIP::VIPShelfUnLockReq>(ProtoManager::Process<ProtoVIP::VIPShelfUnLockReq, ProtoVIP::VIPShelfUnLockResp, LogicVIPManager>);
	//vip增加生产队列
	dispatcher.registerMessageCallback<ProtoVIP::VIPAddProductQueueReq>(ProtoManager::Process<ProtoVIP::VIPAddProductQueueReq, ProtoVIP::VIPAddProductQueueResp, LogicVIPManager>);
	//随机生成vip礼包
	dispatcher.registerMessageCallback<ProtoVIP::RandomVIPGiftReq>(ProtoManager::Process<ProtoVIP::RandomVIPGiftReq, ProtoVIP::RandomVIPGiftResp, LogicVIPManager>);

	//------航运
	//码头解锁
	dispatcher.registerMessageCallback<ProtoShipping::UnlockDockReq>(ProtoManager::Process<ProtoShipping::UnlockDockReq, ProtoShipping::UnlockDockResp, LogicShippingManager>);
	//码头揭幕
	dispatcher.registerMessageCallback<ProtoShipping::UnveilDockReq>(ProtoManager::Process<ProtoShipping::UnveilDockReq, ProtoShipping::UnveilDockResp, LogicShippingManager>);
	//装箱
	dispatcher.registerMessageCallback<ProtoShipping::PackBoxReq>(ProtoManager::Process<ProtoShipping::PackBoxReq, ProtoShipping::PackBoxResp, LogicShippingManager>);
	//请求帮助
	dispatcher.registerMessageCallback<ProtoShipping::SeekAidReq>(ProtoManager::Process<ProtoShipping::SeekAidReq, ProtoShipping::SeekAidResp, LogicShippingManager>);
	//提供帮助
	dispatcher.registerMessageCallback<ProtoShipping::OfferAidReq>(ProtoManager::ProcessNoReply<ProtoShipping::OfferAidReq, LogicShippingManager>);
	//跨服提供帮助
	dispatcher.registerMessageCallback<ProtoShipping::CSOfferAidReq>(ProtoManager::ProcessNoReplyNoUID<ProtoShipping::CSOfferAidReq, LogicShippingManager>);
	//处理跨服提供帮助的返回消息
	dispatcher.registerMessageCallback<ProtoShipping::CSOfferAidResp>(ProtoManager::ProcessNoReplyNoUID<ProtoShipping::CSOfferAidResp, LogicShippingManager>);
	//离港
	dispatcher.registerMessageCallback<ProtoShipping::LeaveDockReq>(ProtoManager::Process<ProtoShipping::LeaveDockReq, ProtoShipping::LeaveDockResp, LogicShippingManager>);
	//设置动画状态
	dispatcher.registerMessageCallback<ProtoShipping::SetPlayStatusReq>(ProtoManager::Process<ProtoShipping::SetPlayStatusReq, ProtoShipping::SetPlayStatusResp, LogicShippingManager>);
	//
	dispatcher.registerMessageCallback<ProtoShipping::GetShipBonusInfoReq>(ProtoManager::Process<ProtoShipping::GetShipBonusInfoReq, ProtoShipping::GetShipBonusInfoResp, LogicShippingManager>);

	//金币购买
	dispatcher.registerMessageCallback<User::PurchaseCoinReq>(ProtoManager::Process<User::PurchaseCoinReq, User::PurchaseCoinResp, LogicUserManager>);
	//跨服推送好友的登录信息
	dispatcher.registerMessageCallback<User::CSPushLoginMsg>(ProtoManager::ProcessNoReplyNoUID<User::CSPushLoginMsg, LogicUserManager>);

	//--------------npc商人
	//获取折扣
	dispatcher.registerMessageCallback<ProtoNPCSeller::GetPropsDiscountReq>(ProtoManager::Process<ProtoNPCSeller::GetPropsDiscountReq, ProtoNPCSeller::GetPropsDiscountResp, LogicNPCSellerManager>);
	//回应npc商人
	dispatcher.registerMessageCallback<ProtoNPCSeller::ResponseNPCSellerReq>(ProtoManager::Process<ProtoNPCSeller::ResponseNPCSellerReq, ProtoNPCSeller::ResponseNPCSellerResp, LogicNPCSellerManager>);
	//改变npc商人状态
	dispatcher.registerMessageCallback<ProtoNPCSeller::ChangeNPCSellerStatusReq>(ProtoManager::Process<ProtoNPCSeller::ChangeNPCSellerStatusReq, ProtoNPCSeller::ChangeNPCSellerStatusResp, LogicNPCSellerManager>);

	//-------------npc商店
	//访问npc
	dispatcher.registerMessageCallback<ProtoNPCUser::RequestNPCUser>(ProtoManager::Process<ProtoNPCUser::RequestNPCUser, ProtoNPCUser::NPCUser, LogicNPCShopManager>);
	//获取商店信息
	dispatcher.registerMessageCallback<ProtoNPCUser::GetNPCShopReq>(ProtoManager::Process<ProtoNPCUser::GetNPCShopReq, ProtoNPCUser::GetNPCShopResp, LogicNPCShopManager>);
	//商店购买
	dispatcher.registerMessageCallback<ProtoNPCUser::PurchaseReq>(ProtoManager::Process<ProtoNPCUser::PurchaseReq, ProtoNPCUser::PurchaseResp, LogicNPCShopManager>);

	//--------------商会
	//获取商会信息
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAlliance>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAlliance, LogicAllianceManager>);
	//跨服获取商会信息
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceBC, LogicAllianceManager>);
	//跨服获取商会信息返回
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceBC, LogicAllianceManager>);

	//获取商会功能信息
	dispatcher.registerMessageCallback<ProtoAlliance::GetAllianceFunctionReq>(ProtoManager::ProcessNoReply<ProtoAlliance::GetAllianceFunctionReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceFunctionBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceFunctionBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceFunctionBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceFunctionBC, LogicAllianceManager>);

	//获取商会通知
	dispatcher.registerMessageCallback<ProtoAlliance::GetNotifyReq>(ProtoManager::ProcessNoReply<ProtoAlliance::GetNotifyReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceNotifyBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceNotifyBC, LogicAllianceManager>);

	//获取商会成员
	dispatcher.registerMessageCallback<ProtoAlliance::GetMemberReq>(ProtoManager::ProcessNoReply<ProtoAlliance::GetMemberReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceMemberBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceMemberBC, LogicAllianceManager>);

	//商会名称校验--
	dispatcher.registerMessageCallback<ProtoAlliance::CheckNameAvailableReq>(ProtoManager::Process<ProtoAlliance::CheckNameAvailableReq, ProtoAlliance::CheckNameAvailableResp, LogicAllianceManager>);

	//创建商会--
	dispatcher.registerMessageCallback<ProtoAlliance::CreateAllianceReq>(ProtoManager::Process<ProtoAlliance::CreateAllianceReq, ProtoAlliance::CreateAllianceResp, LogicAllianceManager>);

	//商会推荐--
	dispatcher.registerMessageCallback<ProtoAlliance::RecommendllianceReq>(ProtoManager::Process<ProtoAlliance::RecommendllianceReq, ProtoAlliance::RecommendllianceResp, LogicAllianceManager>);

	//商会批量查询--
	dispatcher.registerMessageCallback<ProtoAlliance::GetPartAllianceInfoReq>(ProtoManager::Process<ProtoAlliance::GetPartAllianceInfoReq, ProtoAlliance::GetPartAllianceInfoResp, LogicAllianceManager>);

	//申请加入商会
	dispatcher.registerMessageCallback<ProtoAlliance::ApplyJoinReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ApplyJoinReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestApplyJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestApplyJoinBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyApplyJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyApplyJoinBC, LogicAllianceManager>);

	//批准入会操作
	dispatcher.registerMessageCallback<ProtoAlliance::ApproveJoinReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ApproveJoinReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestApproveJoinUserBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestApproveJoinUserBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestApproveJoinAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestApproveJoinAllianceBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyApproveJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyApproveJoinBC, LogicAllianceManager>);

	//退出商会
	dispatcher.registerMessageCallback<ProtoAlliance::ExitAllianceReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ExitAllianceReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestExitAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestExitAllianceBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyExitAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyExitAllianceBC, LogicAllianceManager>);

	//邀请入会
	dispatcher.registerMessageCallback<ProtoAlliance::InviteJoinReq>(ProtoManager::ProcessNoReply<ProtoAlliance::InviteJoinReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestInviteJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestInviteJoinBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestInviteJoinUserBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestInviteJoinUserBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyInviteJoinBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyInviteJoinBC, LogicAllianceManager>);

	//接受邀请
	dispatcher.registerMessageCallback<ProtoAlliance::AcceptInviteReq>(ProtoManager::ProcessNoReply<ProtoAlliance::AcceptInviteReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAcceptInviteBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAcceptInviteBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAcceptInviteBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAcceptInviteBC, LogicAllianceManager>);

	//成员职位调整
	dispatcher.registerMessageCallback<ProtoAlliance::ManipulateMemberReq>(ProtoManager::ProcessNoReply<ProtoAlliance::ManipulateMemberReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestManipulateMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestManipulateMemberBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyManipulateMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyManipulateMemberBC, LogicAllianceManager>);

	//踢出成员
	dispatcher.registerMessageCallback<ProtoAlliance::KickOutReq>(ProtoManager::ProcessNoReply<ProtoAlliance::KickOutReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestKickOutBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestKickOutBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestKickOutMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestKickOutMemberBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyKickOutBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyKickOutBC, LogicAllianceManager>);

	//转任会长
	dispatcher.registerMessageCallback<ProtoAlliance::TransferReq>(ProtoManager::ProcessNoReply<ProtoAlliance::TransferReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestTransferBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestTransferBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyTransferBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyTransferBC, LogicAllianceManager>);

	//编辑商会
	dispatcher.registerMessageCallback<ProtoAlliance::EditAllianceReq>(ProtoManager::ProcessNoReply<ProtoAlliance::EditAllianceReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestEditAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestEditAllianceBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyEditAllianceBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyEditAllianceBC, LogicAllianceManager>);

	//发起捐收
	dispatcher.registerMessageCallback<ProtoAlliance::SeekDonationReq>(ProtoManager::ProcessNoReply<ProtoAlliance::SeekDonationReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestSeekDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestSeekDonationBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplySeekDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplySeekDonationBC, LogicAllianceManager>);

	//秒捐收cd--
	dispatcher.registerMessageCallback<ProtoAlliance::CutUpDonationCDReq>(ProtoManager::Process<ProtoAlliance::CutUpDonationCDReq, ProtoAlliance::CutUpDonationCDResp, LogicAllianceManager>);

	//提供捐收
	dispatcher.registerMessageCallback<ProtoAlliance::OfferDonationReq>(ProtoManager::ProcessNoReply<ProtoAlliance::OfferDonationReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestOfferDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestOfferDonationBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyOfferDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyOfferDonationBC, LogicAllianceManager>);

	//提取已捐收物品
	dispatcher.registerMessageCallback<ProtoAlliance::FetchDonationReq>(ProtoManager::ProcessNoReply<ProtoAlliance::FetchDonationReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestFetchDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestFetchDonationBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyFetchDonationBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyFetchDonationBC, LogicAllianceManager>);

	//发布通知
	dispatcher.registerMessageCallback<ProtoAlliance::SendNotifyReq>(ProtoManager::ProcessNoReply<ProtoAlliance::SendNotifyReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestSendNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestSendNotifyBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplySendNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplySendNotifyBC, LogicAllianceManager>);

	//删除通知
	dispatcher.registerMessageCallback<ProtoAlliance::DelNotifyReq>(ProtoManager::ProcessNoReply<ProtoAlliance::DelNotifyReq, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestDelNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestDelNotifyBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyDelNotifyBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyDelNotifyBC, LogicAllianceManager>);

	//跨服更新成员信息
	dispatcher.registerMessageCallback<ProtoAlliance::RequestUpdateMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestUpdateMemberBC, LogicAllianceManager>);
	//跨服更新成员信息
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAddMemberHelpTimesBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAddMemberHelpTimesBC, LogicAllianceManager>);



	//设置商会成员竞赛标志
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceSetFlag>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceSetFlag, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceSetFlagBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceSetFlagBC, LogicAllianceManager>);

	//竞赛订单完成进度
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberProgress>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceMemberProgress, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberProgressBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberProgressBC, LogicAllianceManager>);


	//查询竞赛信息
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceInfo>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceInfo, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceInfoBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceInfoBC, LogicAllianceManager>);

	//查询竞赛订单
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceOrderBC, LogicAllianceManager>);

	//竞赛操作订单
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOperateOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceOperateOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceOperateOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceOperateOrderBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceOperateOrder>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceOperateOrder, LogicAllianceManager>);

	//竞赛成员删除订单
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberDelOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceMemberDelOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberDelOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberDelOrderBC, LogicAllianceManager>);

	//竞赛成员更新订单
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC, LogicAllianceManager>);

	//竞赛买订单
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceBuyOrder>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceBuyOrder, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceBuyOrderBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceBuyOrderBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceBuyOrder>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceBuyOrder, LogicAllianceManager>);

	//查询竞赛奖励
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceRewardBC, LogicAllianceManager>);

	//领取竞赛等级奖励
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeGradeReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceTakeGradeReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceTakeGradeReward>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceTakeGradeReward, LogicAllianceManager>);

	//领取竞赛阶段奖励
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeStageReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceTakeStageReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceTakeStageRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceTakeStageRewardBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceTakeStageReward>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceTakeStageReward, LogicAllianceManager>);

	//刷新竞赛阶段奖励
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceRefreshStageReward>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceRefreshStageReward, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::ReplyAllianceRaceRefreshStageReward>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::ReplyAllianceRaceRefreshStageReward, LogicAllianceManager>);

	//竞赛成员日志
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberOrderLog>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceMemberOrderLog, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceMemberOrderLogBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceMemberOrderLogBC, LogicAllianceManager>);

	//竞赛个人日志
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRacePersonOrderLog>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRacePersonOrderLog, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRacePersonOrderLogBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRacePersonOrderLogBC, LogicAllianceManager>);

	//跨服设置商会竞赛分组积分
	dispatcher.registerMessageCallback<ProtoAlliance::SetAllianceRaceGroupPointBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::SetAllianceRaceGroupPointBC, LogicAllianceManager>);

	//请求商会竞赛分组成员
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceGroupMember>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceGroupMember, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceGroupMemberBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceGroupMemberBC, LogicAllianceManager>);

	//看广告增加商会积分
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceWatchAd>(ProtoManager::ProcessNoReply<ProtoAlliance::RequestAllianceRaceWatchAd, LogicAllianceManager>);
	dispatcher.registerMessageCallback<ProtoAlliance::RequestAllianceRaceWatchAdBC>(ProtoManager::ProcessNoReplyNoUID<ProtoAlliance::RequestAllianceRaceWatchAdBC, LogicAllianceManager>);


	//物品助手
	dispatcher.registerMessageCallback<ProtoAssistor::OpenAssistorReq>(ProtoManager::Process<ProtoAssistor::OpenAssistorReq, ProtoAssistor::OpenAssistorResp, UserManager>);
	dispatcher.registerMessageCallback<ProtoAssistor::UseAssistorReq>(ProtoManager::Process<ProtoAssistor::UseAssistorReq, ProtoAssistor::UseAssistorResp, UserManager>);

	//----npc顾客
	//获取npc顾客信息
	dispatcher.registerMessageCallback<ProtoNPCCustomer::GetNPCCustomerReq>(ProtoManager::Process<ProtoNPCCustomer::GetNPCCustomerReq, ProtoNPCCustomer::GetNPCCustomerResp, LogicCustomerManager>);
	//出售物品给npc顾客
	dispatcher.registerMessageCallback<ProtoNPCCustomer::SellPropsReq>(ProtoManager::Process<ProtoNPCCustomer::SellPropsReq, ProtoNPCCustomer::SellPropsResp, LogicCustomerManager>);
	//拒绝出售
	dispatcher.registerMessageCallback<ProtoNPCCustomer::RefuseSellPropsReq>(ProtoManager::Process<ProtoNPCCustomer::RefuseSellPropsReq, ProtoNPCCustomer::RefuseSellPropsResp, LogicCustomerManager>);

	//----随机宝箱
	//打开随机宝箱
	dispatcher.registerMessageCallback<ProtoRandomBox::OpenBoxReq>(ProtoManager::Process<ProtoRandomBox::OpenBoxReq, ProtoRandomBox::OpenBoxResp, LogicRandomBoxManager>);
	//购买宝箱礼包
	dispatcher.registerMessageCallback<ProtoRandomBox::BuyBoxGiftReq>(ProtoManager::Process<ProtoRandomBox::BuyBoxGiftReq, ProtoRandomBox::BuyBoxGiftResp, LogicRandomBoxManager>);

	//签到奖励
	dispatcher.registerMessageCallback<User::SignInRewardsReq>(ProtoManager::Process<User::SignInRewardsReq, User::SignInRewardsResp, SignInActivity>);
	dispatcher.registerMessageCallback<ProtoActivity::GameAcitivityStatusReq>(ProtoManager::Process<ProtoActivity::GameAcitivityStatusReq, ProtoActivity::GameAcitivityStatusResp, LogicGameActivityManager>);

	//------邮件狗
	//获取邮件狗信息
	dispatcher.registerMessageCallback<ProtoMailDog::GetMailDogInfoReq>(ProtoManager::Process<ProtoMailDog::GetMailDogInfoReq, ProtoMailDog::GetMailDogInfoResp, LogicMailDogManager>);

	//------转盘
	//获取转盘信息
	dispatcher.registerMessageCallback<ProtoRotaryTable::GetRotaryTableInfoReq>(ProtoManager::Process<ProtoRotaryTable::GetRotaryTableInfoReq, ProtoRotaryTable::GetRotaryTableInfoResp, RotaryTableActivity>);
	//抽奖
	dispatcher.registerMessageCallback<ProtoRotaryTable::DrawRotaryTableReq>(ProtoManager::Process<ProtoRotaryTable::DrawRotaryTableReq, ProtoRotaryTable::DrawRotaryTableResp, RotaryTableActivity>);
	//分享
	dispatcher.registerMessageCallback<ProtoRotaryTable::ShareReq>(ProtoManager::Process<ProtoRotaryTable::ShareReq, ProtoRotaryTable::ShareResp, RotaryTableActivity>);

	//-----友情树
	//获取友情树信息
	dispatcher.registerMessageCallback<ProtoFriendlyTree::GetFriendlyTreeReq>(ProtoManager::Process<ProtoFriendlyTree::GetFriendlyTreeReq, ProtoFriendlyTree::GetFriendlyTreeResp, LogicFriendlyTreeManager>);
	//领取友情值
	dispatcher.registerMessageCallback<ProtoFriendlyTree::RewardFriendlyTreeReq>(ProtoManager::Process<ProtoFriendlyTree::RewardFriendlyTreeReq, ProtoFriendlyTree::RewardFriendlyTreeResp, LogicFriendlyTreeManager>);
	//浇水
	dispatcher.registerMessageCallback<ProtoFriendlyTree::WaterFriendlyTreeReq>(ProtoManager::ProcessNoReply<ProtoFriendlyTree::WaterFriendlyTreeReq, LogicFriendlyTreeManager>);
	//跨服浇水
	dispatcher.registerMessageCallback<ProtoFriendlyTree::CSWaterFriendlyTreeReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendlyTree::CSWaterFriendlyTreeReq, LogicFriendlyTreeManager>);
	//处理跨服浇水返回
	dispatcher.registerMessageCallback<ProtoFriendlyTree::CSWaterFriendlyTreeResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendlyTree::CSWaterFriendlyTreeResp, LogicFriendlyTreeManager>);
	//---------qqgame活动
	//获取奖励状态
	dispatcher.registerMessageCallback<ProtoActivityTencent::RewardStatusReq>(ProtoManager::Process<ProtoActivityTencent::RewardStatusReq, ProtoActivityTencent::RewardStatusResp, LogicActivityTencent>);
	//蓝钻每日奖励
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetBlueDailyAward>(ProtoManager::Process<ProtoActivityTencent::GetBlueDailyAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//蓝钻成长奖励
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetBlueGrowAward>(ProtoManager::Process<ProtoActivityTencent::GetBlueGrowAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//特权每日奖励
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetQQgamePrivilegeDailyAward>(ProtoManager::Process<ProtoActivityTencent::GetQQgamePrivilegeDailyAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//特权成长奖励
	dispatcher.registerMessageCallback<ProtoActivityTencent::GetQQgamePrivilegeGrowAward>(ProtoManager::Process<ProtoActivityTencent::GetQQgamePrivilegeGrowAward, ProtoActivityTencent::GetRewardResp, LogicActivityTencent>);
	//---------广告接入
	//领取观看奖励
	dispatcher.registerMessageCallback<ProtoAccessAd::RewardViewAdReq>(ProtoManager::Process<ProtoAccessAd::RewardViewAdReq, ProtoAccessAd::RewardViewAdResp, LogicAccessAdManager>);
	//获取ts
	dispatcher.registerMessageCallback<ProtoAccessAd::GetLastViewAdTsReq>(ProtoManager::Process<ProtoAccessAd::GetLastViewAdTsReq, ProtoAccessAd::GetLastViewAdTsResp, LogicAccessAdManager>);
	//获取气球信息
	dispatcher.registerMessageCallback<ProtoAccessAd::GetBallonInfoReq>(ProtoManager::Process<ProtoAccessAd::GetBallonInfoReq, ProtoAccessAd::GetBallonInfoResp, LogicAccessAdManager>);
	//看广告处理
	dispatcher.registerMessageCallback<ProtoAccessAd::CommonlViewAdReq>(ProtoManager::Process<ProtoAccessAd::CommonlViewAdReq, ProtoAccessAd::CommonlViewAdResp, LogicAccessAdManager>);
	//获取稻草人信息
	dispatcher.registerMessageCallback<ProtoAccessAd::GetScarecrowInfoReq>(ProtoManager::Process<ProtoAccessAd::GetScarecrowInfoReq, ProtoAccessAd::GetScarecrowInfoResp, LogicAccessAdManager>);
	//稻草人看广告
	dispatcher.registerMessageCallback<ProtoAccessAd::ScarecrowViewAdReq>(ProtoManager::Process<ProtoAccessAd::ScarecrowViewAdReq, ProtoAccessAd::ScarecrowViewAdResp, LogicAccessAdManager>);
	//-------------基金活动
	//购买基金
	dispatcher.registerMessageCallback<ProtoActivity::FundPurchaseReq>(ProtoManager::Process<ProtoActivity::FundPurchaseReq, ProtoActivity::FundPurchaseResp, LogicFundActivityManager>);
	//每日领取奖励
	dispatcher.registerMessageCallback<ProtoActivity::RewardFundGiftReq>(ProtoManager::Process<ProtoActivity::RewardFundGiftReq, ProtoActivity::RewardFundGiftResp, LogicFundActivityManager>);

	//-------------4399首冲翻倍活动
	//领取翻倍奖励
	dispatcher.registerMessageCallback<ProtoActivity::Reward4399RechargeGiftReq>(ProtoManager::Process<ProtoActivity::Reward4399RechargeGiftReq, ProtoActivity::Reward4399RechargeGiftResp, Recharge4399ActivityManager>);
	//-------------4399每日充值活动
	//领取奖励
	dispatcher.registerMessageCallback<ProtoActivity::Reward4399DailyGiftReq>(ProtoManager::Process<ProtoActivity::Reward4399DailyGiftReq, ProtoActivity::Reward4399DailyGiftResp, Daily4399ActivityManager>);
	dispatcher.registerMessageCallback<ProtoActivity::UseCardReq>(ProtoManager::Process<ProtoActivity::UseCardReq, ProtoActivity::UseCardResp, Daily4399ActivityManager>);

	//----------月卡与终生卡
	//获取月卡信息
	dispatcher.registerMessageCallback<ProtoCard::GetCardReq>(ProtoManager::Process<ProtoCard::GetCardReq, ProtoCard::GetCardResp, LogicCardManager>);
	//领取月卡奖励
	dispatcher.registerMessageCallback<ProtoCard::RewardMonthCardReq>(ProtoManager::Process<ProtoCard::RewardMonthCardReq, ProtoCard::RewardMonthCardResp, LogicCardManager>);
	//领取终生卡奖励
	dispatcher.registerMessageCallback<ProtoCard::RewardLifeCardReq>(ProtoManager::Process<ProtoCard::RewardLifeCardReq, ProtoCard::RewardLifeCardResp, LogicCardManager>);

	//---------付费主题
	//查询主题
	dispatcher.registerMessageCallback<ProtoTheme::ThemeInfoReq>(ProtoManager::Process<ProtoTheme::ThemeInfoReq, ProtoTheme::ThemeInfoResp, LogicThemeManager>);
	//购买主题
	dispatcher.registerMessageCallback<ProtoTheme::ThemeBuyReq>(ProtoManager::Process<ProtoTheme::ThemeBuyReq, ProtoTheme::ThemeBuyResp, LogicThemeManager>);
	//应用主题
	dispatcher.registerMessageCallback<ProtoTheme::ThemeUseReq>(ProtoManager::Process<ProtoTheme::ThemeUseReq, ProtoTheme::ThemeInfoResp, LogicThemeManager>);

	//---------农场助手
	//查询农场助手
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperInfoReq>(ProtoManager::Process<ProtoKeeper::KeeperInfoReq, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//花钻石奖励时间
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperBuyTime>(ProtoManager::Process<ProtoKeeper::KeeperBuyTime, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//看广告奖励
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperWatchAds>(ProtoManager::Process<ProtoKeeper::KeeperWatchAds, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//设置任务
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperSetTask>(ProtoManager::Process<ProtoKeeper::KeeperSetTask, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//升级
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperUpgrade>(ProtoManager::Process<ProtoKeeper::KeeperUpgrade, ProtoKeeper::KeeperInfoResp, LogicKeeperManager>);
	//设置自动喂养
	dispatcher.registerMessageCallback<ProtoKeeper::KeeperSetAutoFeed>(ProtoManager::Process<ProtoKeeper::KeeperSetAutoFeed, ProtoKeeper::KeeperSetAutoFeedResp, LogicKeeperManager>);

	//邮件
	//gm发邮件
	dispatcher.registerMessageCallback<Admin::SysMail>(ProtoManager::ProcessNoUID<Admin::SysMail, Admin::ReplySysMail, LogicSysMailManager>);
	//获取所有邮件
	dispatcher.registerMessageCallback<User::RequestSysMail>(ProtoManager::Process<User::RequestSysMail, User::ReplySysMail, LogicSysMailManager>);
	//设置已读
	dispatcher.registerMessageCallback<User::RequestMailRead>(ProtoManager::Process<User::RequestMailRead, User::ReplyMailRead, LogicSysMailManager>);
	//获取附件
	dispatcher.registerMessageCallback<User::RequestMailGet>(ProtoManager::Process<User::RequestMailGet, User::ReplyMailGet, LogicSysMailManager>);
	//删除
	dispatcher.registerMessageCallback<User::RequestMailDel>(ProtoManager::ProcessNoReply<User::RequestMailDel, LogicSysMailManager>);
	//快速获取附件
	dispatcher.registerMessageCallback<User::RequestMailAllGet>(ProtoManager::Process<User::RequestMailAllGet, User::ReplyMailAllGet, LogicSysMailManager>);
	//快速删除
	dispatcher.registerMessageCallback<User::RequestMailAllDel>(ProtoManager::ProcessNoReply<User::RequestMailAllDel, LogicSysMailManager>);
	//跨服发送邮件
	dispatcher.registerMessageCallback<User::ReqSendMailBC>(ProtoManager::ProcessNoReplyNoUID<User::ReqSendMailBC, LogicSysMailManager>);

	//好友长工
	//设置长工
	dispatcher.registerMessageCallback<ProtoFriendWorker::SetFriendWorkerReq>(ProtoManager::ProcessNoReply<ProtoFriendWorker::SetFriendWorkerReq, LogicFriendWorkerManager>);
	//跨服设置长工
	dispatcher.registerMessageCallback<ProtoFriendWorker::CSSetFriendWorkerReq>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendWorker::CSSetFriendWorkerReq, LogicFriendWorkerManager>);
	//处理跨服设置长工返回
	dispatcher.registerMessageCallback<ProtoFriendWorker::CSSetFriendWorkerResp>(ProtoManager::ProcessNoReplyNoUID<ProtoFriendWorker::CSSetFriendWorkerResp, LogicFriendWorkerManager>);
	//获取长工信息
	dispatcher.registerMessageCallback<ProtoFriendWorker::GetWorkerSpeedUpReq>(ProtoManager::Process<ProtoFriendWorker::GetWorkerSpeedUpReq, ProtoFriendWorker::GetWorkerSpeedUpResp, LogicFriendWorkerManager>);
	//选择长工
	dispatcher.registerMessageCallback<ProtoFriendWorker::SelectWorkerReq>(ProtoManager::Process<ProtoFriendWorker::SelectWorkerReq, ProtoFriendWorker::SelectWorkerResp, LogicFriendWorkerManager>);
	//感谢长工
	dispatcher.registerMessageCallback<ProtoFriendWorker::ThanksWorkerReq>(ProtoManager::Process<ProtoFriendWorker::ThanksWorkerReq, ProtoFriendWorker::ThanksWorkerResp, LogicFriendWorkerManager>);

	//宠物
	dispatcher.registerMessageCallback<ProtoPet::UnlockPetResidenceReq>(ProtoManager::Process<ProtoPet::UnlockPetResidenceReq, ProtoPet::UnlockPetResidenceResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::GetUnlockPetInfoReq>(ProtoManager::Process<ProtoPet::GetUnlockPetInfoReq, ProtoPet::GetUnlockPetInfoResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::UnlockPetReq>(ProtoManager::Process<ProtoPet::UnlockPetReq, ProtoPet::UnlockPetResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::FeedPetReq>(ProtoManager::Process<ProtoPet::FeedPetReq, ProtoPet::FeedPetResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::TeasePetReq>(ProtoManager::Process<ProtoPet::TeasePetReq, ProtoPet::TeasePetResp, LogicPetManager>);
	dispatcher.registerMessageCallback<ProtoPet::ChangePetNameReq>(ProtoManager::Process<ProtoPet::ChangePetNameReq, ProtoPet::ChangePetNameResp, LogicPetManager>);

	//新活动分享
	//获取活动信息
	dispatcher.registerMessageCallback<ProtoActivity::GetNewShareInfoReq>(ProtoManager::Process<ProtoActivity::GetNewShareInfoReq, ProtoActivity::GetNewShareInfoResp, LogicNewShareActivity>);
	//分享
	dispatcher.registerMessageCallback<ProtoActivity::NewShareReq>(ProtoManager::Process<ProtoActivity::NewShareReq, ProtoActivity::NewShareResp, LogicNewShareActivity>);
	//领取
	dispatcher.registerMessageCallback<ProtoActivity::RewardNewShareReq>(ProtoManager::Process<ProtoActivity::RewardNewShareReq, ProtoActivity::RewardNewShareResp, LogicNewShareActivity>);


}

void LogicManager::RegMemoryManager()
{
	//下面的放在最前，顺序不要变
	m_memoryManager.push_back(ResourceManager::Instance());
	m_memoryManager.push_back(AsynManager::Instance());
	m_memoryManager.push_back(NotifyManager::Instance());
	//上面的放在最前，顺序不要变

	//广告
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
	//商会
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

	// 助手
	m_dataManager.push_back(DataKeeperManager::Instance());
	m_dataManager.push_back(DataKeeperTaskManager::Instance());
	// 系统邮件
	m_dataManager.push_back(DataSysmailManager::Instance());

	//用户反馈
	m_dataManager.push_back(DataUserFeedbackManager::Instance());

	//宠物
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
	//下面的放在最后，顺序不要变
	m_battleManager.push_back(UserManager::Instance());
	//上面的放在最后，顺序不要变
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

	//定时删除广告
	LogicAdvertiseManager::Instance()->DelOldAd();
	//定时删除动态
	LogicDynamicInfoManager::Instance()->CheckClearDyInfo();
	//定时删除留言
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

	//系统自动回收超时很久的好友订单
	LogicFriendOrderManager::Instance()->CheckRecycleOldSourceFo();
	//用户反馈次数每日更新
	LogicMessageBoardManager::Instance()->UpdateFeedbackTimes();
	//商会竞赛看广告任务每日更新
	LogicAllianceManager::Instance()->UpdateWatchAd();

	//4399每日充值活动0点定时重置数据
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
