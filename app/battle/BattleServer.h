#ifndef _BATTLE_SERVER_H_
#define _BATTLE_SERVER_H_

#include "ServerInc.h"
#include "muduo/base/AsyncLogging.h"
#include <memory>

class BattleServer : public CSingleton<BattleServer> {
private:
	friend class CSingleton<BattleServer>;
	BattleServer();	
    virtual  ~BattleServer();

public:
    bool Initialize();
	void SetTimerCB(TimerCallback cb, double  interval); //设置定时器回调 
	bool BattleConInit();
	bool Run();

	//跨服消息
	TcpConnectionPtr GetBattleClient(uint32_t serverId);
	int BattleConnectNoReplyByUID(unsigned uid, Message* msg, bool d = true);
	int BattleConnectNoReplyByAID(unsigned aid, Message* msg, bool d = true);
	int BattleConnectNoReplyByZoneID(unsigned zoneId, Message* msg, bool d = true);
	int	BattleConnectNoReply(unsigned serverid, Message* msg, bool d = true);

public:
	static muduo::AsyncLogging* asyncLog;
	static void LogOutPut(const char* msg, int len);
	static void LogFlush();

private:
	bool OnReceive(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime);
	void OnConnection(const TcpConnectionPtr& conn);

	InetAddress GetAddr();

private:
    EventLoop* loop_;
    std::unique_ptr<TcpServer> pTcpSvr_;
	map<uint32_t, std::unique_ptr<TcpClient>>  battleConns_;
};


#endif
