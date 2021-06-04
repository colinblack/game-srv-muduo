#ifndef LOGIC_SHIPPING_MANAGER_H_
#define LOGIC_SHIPPING_MANAGER_H_

#include "LogicQueueManager.h"

//航运定时任务
class DataShippingRoutine : public DataRoutineBase
{
public:
	DataShippingRoutine(unsigned uid, unsigned endts, unsigned ud):
		DataRoutineBase(uid, endts, ud)
	{

	}

	//ud在该类中无意义
	virtual void GetPriceAndATime(unsigned ud, int & cash, int & diffts, int &type);

	//定时任务结束
	virtual void SingleRoutineEnd(unsigned ud, ProtoPush::PushBuildingsCPP * msg);
};

class LogicShippingManager :public BattleSingleton, public CSingleton<LogicShippingManager>
{
private:
	friend class CSingleton<LogicShippingManager>;
	LogicShippingManager();
	int userActId;

public:
	enum
	{
		status_init = 0,  //航运状态初始化
		status_unlock = 1, //解锁中
		status_unveil = 2,  //等待揭幕
		status_wait_arrive = 3,  //等待船只到港
		status_packing = 4,  //装货中

		box_status_empty = 0, //空箱
		box_status_full = 1, //满箱

		aid_type_public = 1, //公共援助
		aid_type_commercial = 2, //公会援助

		aid_status_none = 0, //无援助
		aid_stauts_public = 1,  //公共援助中
		aid_status_commercial = 2, //公会援助中
		aid_stauts_pulic_done = 3,  //公共援助已完成
		aid_status_commercial_done = 4, //公会援助已完成

		e_Activity_UserData_1_index_2 = 2,//VIP每日航运加成的使用的次数

		e_Activity_UserData_1_index_9 = 9,//用于存储航运满箱时点券物品id
		e_Activity_UserData_1_index_10 = 10,//用于存储航运满箱时点券物品数量

		common_pack_box  = 1,//普通装箱
		view_ad_pack_box = 2,//看广告装箱
	};

	virtual void CallDestroy() { Destroy();}

	int CheckLogin(unsigned uid);

	// 设置船帮助状态
	int SetShippingboxAidStatus(DataShippingbox& box, int8_t status);
	//船到时间，自动离港
	int AutoLeaveDock(DataShipping & dataship, ProtoPush::PushBuildingsCPP * msg);

	//解锁
	int Process(unsigned uid, ProtoShipping::UnlockDockReq* req, ProtoShipping::UnlockDockResp* resp);

	//揭幕
	int Process(unsigned uid, ProtoShipping::UnveilDockReq* req, ProtoShipping::UnveilDockResp* resp);

	//装箱
	int Process(unsigned uid, ProtoShipping::PackBoxReq* req, ProtoShipping::PackBoxResp* resp);

	//请求援助
	int Process(unsigned uid, ProtoShipping::SeekAidReq* req, ProtoShipping::SeekAidResp* resp);

	//提供援助
	int Process(unsigned uid, ProtoShipping::OfferAidReq* req);

	//跨服提供援助
	int Process(ProtoShipping::CSOfferAidReq* req);

	//处理跨服援助后的返回消息
	int Process(ProtoShipping::CSOfferAidResp* req);

	//离港
	int Process(unsigned uid, ProtoShipping::LeaveDockReq* req, ProtoShipping::LeaveDockResp* resp);

	//设置播放动画状态
	int Process(unsigned uid, ProtoShipping::SetPlayStatusReq* req, ProtoShipping::SetPlayStatusResp* resp);

	//援助装船添加动态消息 同服
	bool AddShipDyInfo(unsigned uid,unsigned other_uid);

	//援助装船添加动态消息 跨服
	bool AddShipDyInfoOverServer(unsigned uid,unsigned other_uid);

	//公会援助次数已满添加动态消息
	bool AddCAidDyInfo(unsigned uid);

	//获取航运VIP加成信息
	int Process(unsigned uid, ProtoShipping::GetShipBonusInfoReq* req, ProtoShipping::GetShipBonusInfoResp* resp);

	//重置每日航运加成次数
	int ResetShipBonusCnt(unsigned uid);

	//航运加成奖励
	int ShipBonus(unsigned uid,unsigned &coin,unsigned &exp);

private:
	//解锁码头
	int UnlockDock(unsigned uid, ProtoShipping::UnlockDockResp * resp);

	//码头揭幕
	int UnveilDock(unsigned uid, ProtoShipping::UnveilDockResp * resp);

	//装箱
	int PackBox(unsigned uid, unsigned type,unsigned boxid, ProtoShipping::PackBoxResp * resp);

	//请求援助
	int SeekAid(unsigned uid, unsigned boxid, unsigned type, ProtoShipping::SeekAidResp * resp);

	//提供援助
	int OfferAid(unsigned uid, unsigned othuid, unsigned boxid, ProtoShipping::OfferAidResp * resp);

	//离港
	int LeaveDock(unsigned uid, ProtoShipping::LeaveDockResp * resp);

	//设置播放动画状态
	int SetPlayStatus(unsigned uid, ProtoShipping::SetPlayStatusResp * resp);

	int PackBoxUnderlying(unsigned uid, unsigned type,DataShippingbox & box, DataCommon::CommonItemsCPP* msg = NULL);

	//分配货箱
	int DistributeBoxes(DataShipping & dataship);

	unsigned GetPrePareTime(unsigned uid);

	//根据用户等级，获取已解锁的可以航运的物品
	int GetShippingItem(unsigned level, int boxnum, vector<unsigned> & items, vector<unsigned> & count);

	//判定船运是否满箱
	bool IsFullBox(unsigned uid);

	//航运满箱奖励
	int FullBoxReward(unsigned uid,unsigned & propsid,unsigned & propscnt);
};

#endif //LOGIC_SHIPPING_MANAGER_H_
