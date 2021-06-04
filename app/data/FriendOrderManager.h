/*
 * FriendOrderManager.h
 *
 *  Created on: 2018-07-20
 *      Author: que
 */

#ifndef FRIEND_ORDER_MANAGER_H_
#define FRIEND_ORDER_MANAGER_H_


#include "Kernel.h"

struct FoInfoItem
{
	unsigned status;		//订单状态
	unsigned senderUid;		//订单发送者
	unsigned deadtime;		//订单超时时间
	unsigned sourceId;		//源订单ID（唯一标记订单）
	unsigned productId;		//请求物品id
	unsigned count;			//请求物品数量
	unsigned coin;			//价格
	unsigned owneruid;		//好友订单拥有者uid
	unsigned helperuid;		//订单的购买者

	FoInfoItem():
		status(STATUS_FO_UNDEFINED),
		senderUid(0),
		deadtime(0),
		sourceId(0),
		productId(0),
		count(0),
		coin(0),
		owneruid(0),
		helperuid(0)
	{

	}

	void Set(const FoInfoItem& item)
	{
		status     = item.status;
		senderUid  = item.senderUid;
		deadtime   = item.deadtime;
		sourceId   = item.sourceId;
		productId  = item.productId;
		coin  	   = item.coin;
		count      = item.count;
		owneruid   = item.owneruid;
		helperuid  = item.helperuid;
	}

	void Clear()
	{
		status 		= STATUS_FO_UNDEFINED;
		senderUid 	= 0;
		deadtime 	= 0;
		sourceId 	= 0;
		coin 		= 0;
		productId 	= 0;
		count 		= 0;
		owneruid    = 0;
		helperuid   = 0;
	}

};


struct FriendOrderData
{
	FoInfoItem item[MAX_FRIEND_ORDER_NUM];
};


class FriendOrderManager : public MemorySingleton<FriendOrderData, MEMORY_FRIEND_ORDER>, public CSingleton<FriendOrderManager>
{
private:
	friend class CSingleton<FriendOrderManager>;
	FriendOrderManager();
	virtual ~FriendOrderManager(){}

	static const int MAX_SIZE = MAX_FRIEND_ORDER_NUM; 		//总容量
	set<unsigned> m_freeIndex;								//剩余好友订单索引(0-MAX_FRIEND_ORDER_NUM)
	map<unsigned, deque<unsigned> >  m_foinfomap;	        //好友订单拥有者uid与foidx数组索引即uid和foidx对应关系

public:

	bool IsFull();											//index是否已满

	unsigned GetFreeIndex();								//获取未被使用的index

	map<unsigned,deque<unsigned> > & GetFoInfoMap();		//获取好友订单map uid和foidx映射

	bool AddFoInfo(uint32_t uid,FoInfoItem &foItem,int basket);	//重要接口 增加一条好友订单 考虑替换算法

	bool CheckClearFo();							    		//共享内存淘汰策略

	bool CheckRecyleSourceFo();									//系统自动回收 超时很久的源订单

	bool DeleteOneSourceOrder(unsigned uid,unsigned basket);	//删除某个玩家的某个源订单

	deque<unsigned>::iterator SearchPlaceForNormalOrder(deque<unsigned> &deq);	//为好友订单找一个插入deque的位置

	unsigned GetSourceFoSize(deque<unsigned> &deq);				//获取玩家源订单的个数

	FoInfoItem & GetFoInfoItem(unsigned foidx);					//获取共享内存某条订单数据

	unsigned GetSourceFoStatus(unsigned senderuid,unsigned sourceId);//获取源订单的状态

	bool ChangeShmFoStatus(unsigned uid,unsigned index,unsigned newStatus);//改变订单的状态

	unsigned GetFoIndex(unsigned senderuid,unsigned myuid);		//获取deque索引

	bool AddHelperUid(unsigned senderuid,unsigned sourceId,unsigned helperuid);	//增加帮助者信息

public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();									//从磁盘读取共享内存数据和初始化相关内存数据

};
#endif
