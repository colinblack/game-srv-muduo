#ifndef LOGIC_FRIEND_ORDER_MANAGER_H_
#define LOGIC_FRIEND_ORDER_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"
#include "ProtoFriendOrder.pb.h"


class  LogicFriendOrderManager : public BattleSingleton, public CSingleton<LogicFriendOrderManager>
{
private:
	friend class CSingleton<LogicFriendOrderManager>;
	LogicFriendOrderManager() {}

public:
	virtual void CallDestroy() { Destroy();}

	//前端主动获取订单消息 同服
	int Process(unsigned uid, ProtoFriendOrder::GetFriendOrderReq *reqmsg, ProtoFriendOrder::GetFriendOrderResp * respmsg);

	//发起订单请求
	int Process(unsigned uid,ProtoFriendOrder::SendFriendOrderReq *resqmsg,ProtoFriendOrder::SendFriendOrderResp *respmsg);

	//跨服 接收好友订单
	int Process(ProtoFriendOrder::SendOtherServerReq *resqmsg);

	//点击好友订单中的某个订单
	int Process(unsigned uid,ProtoFriendOrder::ClickFriendOrderReq *resqmsg);

	//跨服 查询源订单状态
	int Process(ProtoFriendOrder::RecallSourceFoReq *resqmsg);

	//跨服 更新好友订单状态
	int Process(ProtoFriendOrder::ChangeFoStatusReq *resqmsg);

	//点击“没问题”按钮 购买该条好友订单
	int Process(unsigned uid,ProtoFriendOrder::BuyFriendOrderReq *resqmsg);

	//跨服 购买该条好友订单
	int Process(ProtoFriendOrder::RecallCanBuyFoReq *resqmsg);

	//跨服 处理购买成功与否
	int Process(ProtoFriendOrder::AnswerWhetherCanBuyReq *resqmsg);

	//回收发出的订单 后台给玩家增加物品，并改变该条源订单为冷却状态或者直接删除该条订单
	int Process(unsigned uid,ProtoFriendOrder::GetOrderRewardsReq *resqmsg,ProtoFriendOrder::GetOrderRewardsResp *respmsg);

	//花费钻石清除冷却时间或者删除正在发送的订单
	int Process(unsigned uid,ProtoFriendOrder::CostDiamondReq *resqmsg,ProtoFriendOrder::CostDiamondResp *respmsg);

	//定时清理无效的订单
	bool CheckClearFoInfo();

	//系统自动回收过期很久的超时订单
	bool CheckRecycleOldSourceFo();

	//获取订单条数
	int GetFoInfo(unsigned uid,FoInfoItem foItem[],unsigned max_count);

	//设置订单返回信息
	bool SetFoRespMsg(const FoInfoItem& adItem,ProtoFriendOrder::FriendOrderInfo *msg,unsigned index);

	//发起订单增加动态消息 同服
	bool AddBuyDyInfo(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin);

	//发起订单增加动态消息 跨服
	bool AddBuyDyInfoOverServer(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin);

private:

	//给接收者增加一条好友订单 跨服
	bool AddNormalFoOverServer(unsigned senderUid,unsigned receiverUid,FoInfoItem & foitem);
};


#endif
