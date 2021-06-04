#ifndef _LOGIC_MANAGER_H_
#define _LOGIC_MANAGER_H_

#include "Kernel.h"
#include "DataInc.h"

class LogicManager : public CSingleton<LogicManager>
{
private:
	friend class CSingleton<LogicManager>;
	LogicManager();
	virtual ~LogicManager();
public:
	bool Initialize();
	void process(const muduo::net::TcpConnectionPtr& conn, CFirePacket* packet);
	bool SendData(CFirePacket* packet, TcpConnectionPtr conn = nullptr);
	bool sendMsg(unsigned uid, Message* msg, bool delmsg = true);//主动推送用
	bool sendMsgFD(unsigned fd, Message* msg, bool delmsg = true);//主动推送用
	bool sendMsgGroup(set<unsigned>& uid, Message* msg, bool delmsg = true);//主动推送用
	bool broadcastMsg(Message* msg);//广播给所有在线用户
	bool sendKickMsg(unsigned fd, string reason);//发送掉线包
	void forceKick(unsigned uid, string reason);
	void offline(unsigned uid);

	void SetReplyProtocol(Message* msg) {pReplyProtocol = msg;}
	void SetNeedDelReply(bool d = false) {needDelReply = d;}

	bool IsDataManagerWorking();
	bool IsDataManagerFull();
	bool IsDataManagerNeedClear();
	bool IsMemoryManagerNeedClear();
	void DoDataManagerSave(unsigned uid);
	void DoDataManagerAllianceSave(unsigned aid);
	void DoDataManagerClear(unsigned uid);
	void DoAllianceManagerClear(unsigned alliance_id);
	void DoMemoryManagerClear(unsigned uid);

	void Addfd(unsigned uid, unsigned fd);
	void Erasefd(unsigned fd);
	void Eraseuid(unsigned uid);
	unsigned Getfd(unsigned uid);
	unsigned Getuid(unsigned fd);
	unsigned Getfd(){return m_fd;}
	unsigned Getuid(){return Getuid(m_fd);}//获取当前请求对应的uid，登陆包不可用

	void EraseLeaveList(unsigned uid);

	void SetErrMsg(const string &msg){m_errmsg = msg;}

	int32_t getchannelId() {return channelId;}
	void ClearUser(bool send);

	unsigned GetOpenDays();
	unsigned GetTimerTS() {return m_timer;}

	void CheckSave();
	void DataLog();
	void TryClear();

	template<class Type>
	void ObjInit(vector<Type*>& obj);
	void SaveConn(const muduo::net::TcpConnectionPtr& conn){
		m_pConn = conn;
	}
private:
	void clientProcess(CFirePacket* packet);
	void deliverProcess(CFirePacket* packet);
	void adminProcess(CFirePacket* packet);
	void botProcess(CFirePacket* packet);
	void heartProcess(CFirePacket* packet);
	void timerProcess();
	void onTimer2();
	void preOffline(CFirePacket* packet);
	void battleProcess(CFirePacket* packet);
	void forwardProcess(CFirePacket* packet);

	//注册协议回调
	void RegProto();
	//注册dbc数据管理器
	void RegDataManager();
	//注册共享内存管理器
	void RegMemoryManager();
	//注册逻辑管理器
	void RegBattleManager();
	//注册活动管理器
	void RegActivityManager();

	void CheckSig();
	void CheckMin();
	void CheckHour();
	void CheckDay();
	void OnReload();
public:
	static uint32_t GlobalMilliTime; //程序启动到目前经过的毫秒数
	static uint32_t ServerId;
	static uint32_t SecOpenTime;
	static bool IsClosed;
	static bool IsPreClosed;
	static uint64_t StartMilliTime;
	static bool NeedReloadConfig;
	static int m_signum;
private:
	uint32_t m_fd;
	int32_t channelId;
	unsigned m_timer;
	unsigned m_last_hour_ts;
	uint32_t lastLoopTime;
	Message* pReplyProtocol;
	muduo::net::TcpConnectionPtr m_pConn;

	bool needDelReply;

	ProtobufDispatcher dispatcher;
	list<pair<unsigned, unsigned> > m_leaveList;
	map<unsigned, unsigned> m_fdmap, m_uidmap;

	vector<DataSingletonBase*> m_dataManager;
	vector<DataSingletonBase*> m_memoryManager;
	vector<BattleSingleton*> m_battleManager;
	vector<ActivitySingletonBase*> m_activityManager;

	string m_errmsg;
};

#endif
