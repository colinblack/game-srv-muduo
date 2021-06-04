#include "BattleServer.h"
#include  <utility>
#include "Util.h"

using namespace std::placeholders;

muduo::AsyncLogging* BattleServer::asyncLog = nullptr;


void BattleServer::LogOutPut(const char* msg, int len){
	asyncLog->append(msg, len);
}
void BattleServer::LogFlush(){
}
static void OnReloadConfig()
{
	LogicManager::NeedReloadConfig = true;
}
static void OnExit()
{
	LogicManager::IsPreClosed = true;
}
static void OnSigNum(int signum,siginfo_t *info,void *myact)
{
	LogicManager::m_signum = signum;
}

InetAddress FromString(const string &sAddress)
{
    vector<string> vecAddress;
    CBasic::StringSplitTrim(sAddress, ":", vecAddress);
    if(vecAddress.size() != 2)
    {
        return {};
    }
    unsigned port;
    if(!Convert::StringToUInt(port, vecAddress[1]))
    {
        return {};
    }
    if(port > UINT16_MAX)
    {
        return {};
    }
	
    return std::move(InetAddress(vecAddress[0], port));
}

static bool ParseAddress(vector<InetAddress> &vecAddress, const string &sAddress)
{
	if(sAddress.empty())
	{
		return false;
	}
	vector<string> vecStrAddress;
	CBasic::StringSplitTrim(sAddress, ",", vecStrAddress);
	for(vector<string>::iterator itr = vecStrAddress.begin(); itr != vecStrAddress.end(); itr++)
	{
		InetAddress address = FromString(*itr);
		vecAddress.push_back(address);
	}
	return vecAddress.size() != 0;
}


bool BattleServer::Initialize() {
    if(!Kernel::Init())
    {
        fatal_log("[Kernel::Init fail][server=Toywar]");
        return false;
    }

	if(!System::InitDaemon())
	{
		fatal_log("[System::InitDaemon fail][error=%d]", errno);
		return false;
	}

	string path = Config::GetPath(CONFIG_LOG_PATH) + "muduo";
  	asyncLog = new muduo::AsyncLogging(path, 500*1000*1000);
	asyncLog->start();
	Logger::setLogLevel(Logger::DEBUG);
	Logger::setOutput(BattleServer::LogOutPut);
//	muduo::Logger::setFlush(BattleServer::LogFlush);

   	loop_ = new EventLoop;
    std::unique_ptr<TcpServer> tmp(new TcpServer(loop_, std::forward<InetAddress>(GetAddr()), "BattleServer"));
    pTcpSvr_ = std::move(tmp);
	pTcpSvr_->setMessageCallback(std::bind(&BattleServer::OnReceive, this,  _1,  _2, _3));
	pTcpSvr_->setConnectionCallback(std::bind(&BattleServer::OnConnection, this, _1));


	return true;
}

bool BattleServer::Run() {
	pTcpSvr_->start();
    loop_->loop();
    return true;
}


BattleServer::BattleServer(){

}


BattleServer::~BattleServer(){
    delete loop_;
}

bool BattleServer::OnReceive(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime){
	uint32_t decodeSize = 0;
	do
	{
		CFirePacket* packet = new CFirePacket;
		packet->channelId = conn->getChannelId();
		bool decodeSuccess = packet->Decode(buf);
		///TODO: decode size error auto close
		decodeSize = packet->GetDecodeSize();
		if(decodeSize > 0){
			if(decodeSuccess){
				LogicManager::Instance()->SaveConn(conn);
				LogicManager::Instance()->process(conn, packet);
			}
			else{
				error_log("[Decode fail] \n");
				delete packet;
				return false;
			}
			delete packet;
		}
		else{
			delete packet;
		}
	}while(decodeSize > 0);

	return true;
}


void BattleServer::SetTimerCB(TimerCallback cb, double  interval){
	loop_->runAfter(interval, std::move(cb));
}

InetAddress BattleServer::GetAddr(){
    vector<InetAddress> listenAddress;
    if(!ParseAddress(listenAddress,Config::GetValue("server_listen")))
    {
        fatal_log("[ParseAddress fail]");
		return {};
    }

	return std::move(listenAddress.front());	
}



bool BattleServer::BattleConInit(){
	uint32_t serverId = 0;
	for(auto& v : ConfigManager::Instance()->m_server){
		serverId = v.first;
		if(Config::GetIntValue(CONFIG_SRVID) == serverId)
			continue; 
		const Demo::Server& c = ConfigManager::Instance()->GetServer(serverId);
		InetAddress addr(c.host().c_str(), c.port());
		Fmt f("server%03d", v.second);
		string s(f.data(), f.length());
		battleConns_.insert(std::make_pair(serverId, std::unique_ptr<TcpClient>(new TcpClient(loop_, addr, s))));
		battleConns_[serverId]->connect();	
	}
}



TcpConnectionPtr BattleServer::GetBattleClient(uint32_t serverId){
	if(battleConns_.count(serverId)){
		return battleConns_[serverId]->connection();
	}	
	
	return nullptr;
}

void BattleServer::OnConnection(const TcpConnectionPtr& conn){
    LOG_DEBUG << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
		<< conn->getChannelId()
        << (conn->connected() ? "UP" : "DOWN");
}

//指定uid的battle之间的无回复跨服请求，对方的消息注册应该使用 ProcessNoReplyNoUID
int BattleServer::BattleConnectNoReplyByUID(unsigned uid, Message* msg, bool d){
	return BattleConnectNoReply(Config::GetZoneByUID(uid), msg, d);
}

//指定aid的battle之间的无回复跨服请求，对方的消息注册应该使用 ProcessNoReplyNoUID
int BattleServer::BattleConnectNoReplyByAID(unsigned aid, Message* msg, bool d)
{
	return BattleConnectNoReply(Config::GetZoneByAID(aid), msg, d);
}
//指定zoneId的battle之间的无回复跨服请求，对方的消息注册应该使用 ProcessNoReplyNoUID
int BattleServer::BattleConnectNoReplyByZoneID(unsigned zoneId, Message* msg, bool d)
{
	return BattleConnectNoReply(zoneId, msg, d);
}

//指定serverid的battle之间的无回复跨服请求，对方的消息注册应该使用 ProcessNoReplyNoUID
int BattleServer::BattleConnectNoReply(unsigned serverid, Message* msg, bool d)
{
	try
	{
		auto conn = GetBattleClient(serverid);
		if(conn == nullptr){
			return R_ERROR;
		}
		CFirePacket* packet = new CFirePacket(PROTOCOL_EVENT_BATTLE_CONNECT, d);
		packet->m_msg = msg;
		LogicManager::Instance()->SendData(packet, conn);
	}
	catch(const std::exception& e)
	{
		error_log("BattleConnect error %s", e.what());
		return R_ERROR;
	}

	return R_SUCCESS;
}