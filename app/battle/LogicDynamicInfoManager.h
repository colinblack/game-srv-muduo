#ifndef LOGIC_DYNAMIC_INFO_MANAGER_H_
#define LOGIC_DYNAMIC_INFO_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"
#include "ProtoDynamicInfo.pb.h"

#define MAX_PRODUCE_UNIQUE_ID 1000000000

struct DynamicInfoAttach
{
	uint32_t op_uid;		//对方玩家uid
	uint32_t product_id;	//卖出的商品id
	uint32_t coin;			//卖出商品收获的金币
	string   words;			//对方回复留言的内容

	DynamicInfoAttach()
	{
		op_uid = 0;
		product_id = 0;
		coin = 0;
	}
};

class  LogicDynamicInfoManager : public BattleSingleton, public CSingleton<LogicDynamicInfoManager>
{
private:
	friend class CSingleton<LogicDynamicInfoManager>;
	LogicDynamicInfoManager() {}

public:
	virtual void CallDestroy() { Destroy();}

	//前端主动获取动态消息 同服
	int Process(unsigned uid, ProtoDynamicInfo::GetDynamicInfoReq *reqmsg, ProtoDynamicInfo::GetDynamicInfoResp * respmsg);

	//前端请求删除动态消息 同服
	int Process(unsigned uid, ProtoDynamicInfo::DeleteDynamicInfoReq *reqmsg);

	//前端请求更新有动态状态 同服
	int Process(unsigned uid, ProtoDynamicInfo::HasNewDynamicInfoReq *reqmsg, ProtoDynamicInfo::HasNewDynamicInfoResp * respmsg);

	//跨服访问好友产生动态 跨服
	int Process(ProtoDynamicInfo::RequestOtherUserMakeDy *msg);

	//后台主动推送是否有新的动态消息
	bool NotifyNewDy2Client(unsigned uid);

	//产生动态消息统一接口
	bool ProduceOneDyInfo(unsigned uid, unsigned type_id, DynamicInfoAttach *pattach = new DynamicInfoAttach);

	//获取动态条数
	int GetDyInfo(unsigned uid,DyInfoItem dyItem[],unsigned max_count);

	//设置动态返回信息
	bool SetDyRespMsg(unsigned uid,const DyInfoItem& adItem,ProtoDynamicInfo::DynamicInfo *msg);

	//更新玩家下线时间
	bool UpdateOffLine(unsigned uid,unsigned offtime);

	//内网调试时测试淘汰策略
	bool CheckClearDemo();

	//共享内存满后，执行淘汰策略
	bool CheckClearDyInfo();

	//将置顶动态消息降级为普通消息
	bool Degrade2Normal(unsigned uid,unsigned dyidx);

	int Process(unsigned uid,ProtoDynamicInfo::ClickOrderHelpReq *reqmsg,ProtoDynamicInfo::ClickOrderHelpResp *respmsg);

private:

	//给玩家增加一条动态
	bool AddOneDyInShm(unsigned uid,DyInfoItem & dyitem);
};


#endif
