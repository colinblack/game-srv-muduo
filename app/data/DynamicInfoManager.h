/*
 * DynamicInfoManager.h
 *
 *  Created on: 2018-07-11
 *      Author: que
 */

#ifndef DYNAMIC_INFO_MANAGER_H_
#define DYNAMIC_INFO_MANAGER_H_


#include "Kernel.h"

struct DyInfoItem
{
	uint32_t dyidx;			//本条动态的索引
	uint32_t ts;			//动态发生时间
	uint32_t type_id;		//动态类型id
	uint32_t op_uid;		//对方玩家uid
	uint32_t product_id;	//卖出的商品id
	uint32_t coin;			//卖出商品收获的金币
	uint32_t windex;		//对方回复留言的内容索引

	DyInfoItem():
		dyidx(0),
		ts(Time::GetGlobalTime()),
		type_id(0),
		op_uid(0),
		coin(0),
		product_id(0),
		windex(0)
	{

	}

	void Set(const DyInfoItem& item)
	{
		dyidx      = item.dyidx;
		ts		   = item.ts;
		type_id    = item.type_id;
		op_uid     = item.op_uid;
		product_id = item.product_id;
		coin  	   = item.coin;
		windex     = item.windex;
	}

	void Clear()
	{
		dyidx = 0;
		type_id = 0;
		op_uid 	= 0;
		product_id = 0;
		coin = 0;
		ts = 0;
		windex = 0;
	}

};

struct DyInfoUserItem
{
	uint32_t uid;			//动态拥有者uid
	uint32_t last_off_time;	//最后下线时间
	DyInfoItem dyInfo[PER_USER_MAX_DYNAMIC_INFO];

	DyInfoUserItem()
	{
		uid = 0;
		last_off_time = 0;
	}

	void Clear()
	{
		uid = 0;
		last_off_time = 0;
		for(unsigned idx = 0;idx < PER_USER_MAX_DYNAMIC_INFO;++idx)
		{
			dyInfo[idx].Clear();
		}
	}
};

struct DynamicInfoData
{
	DyInfoUserItem item[MAX_PLAYER_SIZE_DY];
};


class DynamicInfoManager : public MemorySingleton<DynamicInfoData, MEMORY_DYNAMIC_INFO>, public CSingleton<DynamicInfoManager>
{
private:
	friend class CSingleton<DynamicInfoManager>;
	DynamicInfoManager();
	virtual ~DynamicInfoManager(){}

	static const int MAX_SIZE = MAX_PLAYER_SIZE_DY; 		//总容量（根据人数而定）
	set<unsigned> m_freeIndex;								//剩余动态索引(0-MAX_PLAYER_SIZE_DY)
	map<unsigned,unsigned> m_map_uid_uidex;					//玩家uid与item数组索引映射
	map<unsigned, deque<unsigned> >  m_dyinfomap;	        //动态拥有者uid与dyInfo数组索引即uid和dyidx对应关系
	map<unsigned, bool>	m_map_has_new;						//动态拥有者是否有新的动态
public:

	bool IsFull();											//index是否已满

	unsigned GetFreeIndex();								//获取未被使用的index

	map<unsigned,deque<unsigned> > & GetDyInfoMap();		//获取动态map uid和dyidx映射

	DyInfoItem & GetDyInfoItem(unsigned uid,unsigned dyidx);//从共享内存中获取一条动态数据

	unsigned FindFreePlace(deque<unsigned> &deq);			//为找一个还未被占用的dyidx

	deque<unsigned>::iterator SearchPlaceForNormal(unsigned uid,deque<unsigned> &deq);	//为普通动态找一个插入deque的位置

	int GetUserDyIndex(uint32_t uid);						//通过uid获取index

	bool AddDyInfo(uint32_t uid,DyInfoItem &dyItem);		//重要接口 增加一条动态 考虑替换算法

	bool UpdateOffLineTime(uint32_t uid,uint32_t offtime);	//更新玩家下线时间

	bool CheckClearDyInfo();								//共享内存淘汰策略

	bool CheckClearDemo();									//内网测试共享内存淘汰策略用接口

	bool DeleteUserDyInfo(uint32_t uid);					//删除某个玩家的所有动态(共享内存清零，释放相关内存)

	bool DeleteOneDyInfo(uint32_t uid,uint32_t dyidx);		//删除玩家的一条动态

	bool HasNewDyInfo(unsigned uid);						//是否有新的动态消息

	bool SetHasNewDy(unsigned uid,bool has);				//更新动态消息状态

	bool DegradeDy(unsigned uid,unsigned dyidx);			//将置顶动态降级为普通动态

	int GetFriendOrderIndex(unsigned uid,unsigned dyidx);	//获取好友订单动态的相关index

public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();									//从磁盘读取共享内存数据和初始化相关内存数据

};
#endif
