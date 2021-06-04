/*
 * MessageBoardManager.h
 *
 *  Created on: 2018-07-11
 *      Author: que
 */

#ifndef MESSAGE_BOARD_MANAGER_H_
#define MESSAGE_BOARD_MANAGER_H_


#include "Kernel.h"

struct MsgInfoItem
{
	unsigned msgidx;
	unsigned senderUid;
	unsigned receiverUid;
	unsigned typeId;
	unsigned ts;
	char words[18*4];

	MsgInfoItem():
		msgidx(0),
		senderUid(0),
		receiverUid(0),
		ts(Time::GetGlobalTime()),
		typeId(0)
	{
		memset(words,0,sizeof(words));
	}

	void Set(const MsgInfoItem& item)
	{
		msgidx     = item.msgidx;
		senderUid  = item.senderUid;
		receiverUid  = item.receiverUid;
		ts		   = item.ts;
		typeId    = item.typeId;
		memcpy(words,item.words,sizeof(words));
	}

	void Clear()
	{
		msgidx = 0;
		senderUid = 0;
		receiverUid = 0;
		ts = 0;
		typeId = 0;
		memset(words,0,sizeof(words));
	}

};

struct MsgInfoUserItem
{
	uint32_t uid;			//留言拥有者uid
	uint32_t last_off_time;	//最后下线时间
	MsgInfoItem msgInfo[PER_USER_MAX_LEAVE_MSG];

	MsgInfoUserItem()
	{
		uid = 0;
		last_off_time = 0;
	}

	void Clear()
	{
		uid = 0;
		last_off_time = 0;
		for(unsigned idx = 0;idx < PER_USER_MAX_LEAVE_MSG;++idx)
		{
			msgInfo[idx].Clear();
		}
	}
};

struct MessageBoardData
{
	MsgInfoUserItem item[MAX_PLAYER_SIZE_MSG];
};


class MessageBoardManager : public MemorySingleton<MessageBoardData, MEMORY_MESSAGE_BOARD>, public CSingleton<MessageBoardManager>
{
private:
	friend class CSingleton<MessageBoardManager>;
	MessageBoardManager();
	virtual ~MessageBoardManager(){}

	static const int MAX_SIZE = MAX_PLAYER_SIZE_MSG; 		//总容量（根据人数而定）
	set<unsigned> m_freeIndex;								//剩余留言索引(0-MAX_PLAYER_SIZE_Msg)
	map<unsigned,unsigned> m_map_uid_uidex;					//玩家uid与item数组索引映射
	map<unsigned, deque<unsigned> >  m_Msginfomap;	        //留言拥有者uid与MsgInfo数组索引即uid和Msgidx对应关系
	map<unsigned, bool>	m_map_has_new;						//留言拥有者是否有新的留言
public:

	bool IsFull();											//index是否已满

	unsigned GetFreeIndex();								//获取未被使用的index

	map<unsigned,deque<unsigned> > & GetMsgInfoMap();		//获取留言map uid和Msgidx映射

	MsgInfoItem & GetMsgInfoItem(unsigned uid,unsigned Msgidx);//从共享内存中获取一条留言数据

	unsigned FindFreePlace(deque<unsigned> &deq);			//为找一个还未被占用的Msgidx

	int GetUserMsgIndex(uint32_t uid);						//通过uid获取index

	int AddMsgInfo(uint32_t uid,MsgInfoItem &MsgItem);		//重要接口 增加一条留言 考虑替换算法

	bool UpdateOffLineTime(uint32_t uid,uint32_t offtime);	//更新玩家下线时间

	bool CheckClearMsgInfo();								//共享内存淘汰策略

	bool CheckClearDemo();									//内网测试共享内存淘汰策略用接口

	bool DeleteUserMsgInfo(uint32_t uid);					//删除某个玩家的所有留言(共享内存清零，释放相关内存)

	bool DeleteOneMsgInfo(uint32_t uid,uint32_t Msgidx);	//删除玩家的一条留言

	bool HasNewMsgInfo(unsigned uid);						//是否有新的留言消息

	bool SetHasNewMsg(unsigned uid,bool has);				//更新留言消息状态

public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();									//从磁盘读取共享内存数据和初始化相关内存数据

};
#endif
