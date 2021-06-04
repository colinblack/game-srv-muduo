#ifndef LOGIC_ADVERTISE_MANAGER_H_
#define LOGIC_ADVERTISE_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"

class  LogicAdvertiseManager : public BattleSingleton, public CSingleton<LogicAdvertiseManager>
{
private:
	friend class CSingleton<LogicAdvertiseManager>;
	LogicAdvertiseManager() {};

public:
	virtual void CallDestroy() { Destroy();}

	//获取广告信息
	int Process(unsigned uid, ProtoAdvertise::GetAdInfoReq *reqmsg, ProtoAdvertise::GetAdInfoResp * respmsg);

	//更新发广告cd
	int Process(unsigned uid, ProtoAdvertise::UpdateAdCdReq *reqmsg, ProtoAdvertise::UpdateAdCdResp * respmsg);


	//创建广告
	int CreateAd(unsigned uid,unsigned shelf_ud);

	//定时删除到期广告
	int DelOldAd();

	//删除广告
	int DelAd(unsigned uid,unsigned shelf_ud);

	//设置广告返回信息
	int SetAdRespMsg(const AdvertiseItem& adItem,ProtoAdvertise::AdInfoCPP *msg);

	//随机获得指定数目的广告信息
	int GetAdInfo(unsigned uid,AdvertiseItem adItem[], int count);

	//玩家是否能查询到别的玩家发的广告
	bool isQueryOtherAd(unsigned uid,unsigned otheruid,unsigned shelf_ud);
private:
	//添加广告
	int AddAd(unsigned uid,unsigned shelf_ud,unsigned props_id,unsigned props_cnt,unsigned props_price);
};


#endif  //LOGIC_ADVERTISE_MANAGER_H_
