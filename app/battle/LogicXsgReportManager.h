#ifndef LOGIC_XSG_MANAGER_H
#define LOGIC_XSG_MANAGER_H

#include "DcLogger.h"
#include "Common.h"
#include "Kernel.h"
#include <bitset>

class LogicXsgReportManager:public BattleSingleton, public CSingleton<LogicXsgReportManager>
{
private:
	friend class CSingleton<LogicXsgReportManager>;
	LogicXsgReportManager();
	map<unsigned,string>user_openid_map;

public:
	virtual void CallDestroy();
private:
	 static DcLogger* m_dclogger;

public:
	 //是否为微信平台
	 bool IsWXPlatform(unsigned uid);

	 //设置基本参数
	 template<class T>
	 void SetBaseParam(unsigned uid,string openid,T &base);

	 /*****************************服务端通用化行为上报******************************/
	 //1.登入成功上报
	 void XSGLoginReport(unsigned uid,const string openid);

	 //2.登出上报
	 void XSGLogOutReport(unsigned uid);

	 //3.角色充值
	 void XSGRechargeReport(unsigned uid,string itemid,unsigned charge,string orderid,string channelOrderId);

	 //4.充值获得钻石
	 void XSGRechargeGetDiamondReport(unsigned uid,unsigned cash,unsigned total);

	 //5.非充值获取钻石
	 void XSGGetDiamondReport(unsigned uid,string reason,unsigned add,unsigned total,bool isbind = false);

	 //6.消耗钻石
	 void XSGCostDiamondReport(unsigned uid,unsigned sub,unsigned total,string reason);

	 //7.金币购买
	 void XSGPurchaseCoinReport(unsigned uid,unsigned purchase_count,unsigned total);

	 //8.非购买获得金币
	 void XSGGetCoinReport(unsigned uid,unsigned count,unsigned total,string reason);

	 //9.金币消耗
	 void XSGCostCoinReport(unsigned uid,unsigned sub,unsigned total,string reason);

	 //10.角色升级
	 void XSGLevelUpReport(unsigned uid);

	 //11.任务开始上报
	 void XSGMissionStartReport(unsigned uid,unsigned missionid,unsigned missiontype);

	 //12.任务结束上报
	 void XSGMissionEndReport(unsigned uid,unsigned missionid,unsigned missiontype);

	 //13.玩家商店交易
	 void XSGShopTradeReport(unsigned uid,string tradeid,string tradetype,unsigned coin,string coinType,unsigned itemid,unsigned itemnum);



	 /*****************************服务端个性化行为上报******************************/
	 //1.建造
	 void XSGBuildReport(unsigned uid,unsigned buildtype,unsigned buildid,unsigned buildnum);

	 //2.生产设备生产
	 void XSGEquipmentReport(unsigned uid,unsigned equipmentId,unsigned itemid,unsigned itemnum);

	 //3.作物收获
	 void XSGGetHaverstReport(unsigned uid,unsigned itemtype,unsigned itemid,unsigned itemnum);

	 //4.清除障碍物
	 void XSGDestroyBarrierReport(unsigned uid,unsigned item_num,unsigned use_num);

	 //5.动物喂食
	 void XSGFeedAnimalReport(unsigned uid,unsigned animalid,unsigned itemid,unsigned itemnum);

	 //6.粮仓货仓升级
	 void XSGStorageUpReport(unsigned uid,unsigned stroageid,unsigned space,string itemid,string itemnum);

	 //7.道具增减
	 void XSGItemChangeReport(unsigned uid,int flag ,unsigned itemid,unsigned itemnum,string reason);

	 //8.发布商品
	 void XSGShopShelfItemReport(unsigned uid,unsigned itemid,unsigned itemnum,unsigned price);

	 //9.转盘
	 void XSGRotaryDrawReport(unsigned uid,unsigned itemid,unsigned itemnum);
};

#endif
