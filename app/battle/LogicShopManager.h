#ifndef LOGIC_SHOP_MANAGER_H
#define LOGIC_SHOP_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicShopManager :public BattleSingleton, public CSingleton<LogicShopManager>
{
private:
	friend class CSingleton<LogicShopManager>;
	LogicShopManager(){}

public:
	enum{
		type_of_confirm_payment_normal = 0,
		type_of_confirm_payment_viewad = 1,

		shelf_of_common = 0,//普通货架
		shelf_of_vip    = 1,//VIP货架
		shelf_of_invite = 2,//因邀请获取到的货架
	};
	virtual void CallDestroy() { Destroy();}

	//初始商店信息
	int NewUser(unsigned uid);

	//查询商店信息
	int Process(unsigned uid, ProtoShop::GetShopReq* req, ProtoShop::GetShopResp* resp);

	//商店物品上架
	int Process(unsigned uid, ProtoShop::ShelfPropsReq* req, ProtoShop::ShelfPropsResp* resp);

	//货架解锁
	int Process(unsigned uid, ProtoShop::ShelfUnLockReq* req, ProtoShop::ShelfUnLockResp* resp);

	//确认收钱
	int Process(unsigned uid, ProtoShop::ConfirmPaymentReq* req, ProtoShop::ConfirmPaymentResp* resp);

	//购买
	int Process(unsigned uid, ProtoShop::PurchaseReq* req);

	//跨服购买
	int Process(ProtoShop::CSPurchaseReq* req);

	//处理跨服后购买商店后的信息
	int Process(ProtoShop::CSPurchaseResp* req);

	//访问玩家商店
	int Process(unsigned uid, ProtoShop::VisitOtherShopReq* req);

	//跨服访问商店请求
	int Process(ProtoShop::CSVisitOtherShopReq* req);

	//处理跨服访问商店的回应消息
	int Process(ProtoShop::CSVisitOtherShopResp* resp);

	//修改商店数据
	int Process(unsigned uid, ProtoShop::ModifyShelfInfoReq* req, ProtoShop::ModifyShelfInfoResp* resp);

	//看广告回收物品
	int Process(unsigned uid, ProtoShop::ViewAdRecycleItemReq* req, ProtoShop::ViewAdRecycleItemResp* resp);

	//系统购买
	int Process(unsigned uid, ProtoShop::BuyShopItemBySystemReq* req, ProtoShop::BuyShopItemBySystemResp* resp);

	//添加货架
	int AddShelf(unsigned uid);

	//获取玩家商店货架数目(不包含vip解锁的货架)
	int GetShelfCnt(unsigned uid);

	//获取VIP解锁的货架
	int GetVIPShelfCnt(unsigned uid);

	//获取因邀请而解锁的货架数目
	int GetShelfCntByInvite(unsigned uid);

	//删除广告的同时更新商店货架里的广告标记并且由系统购买商店里的东西
	int UpdateAdFlag(unsigned uid,unsigned shelf_ud);

	//购买商品增加动态消息 同服
	bool AddBuyDyInfo(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin);

	//购买商品增加动态消息 跨服
	bool AddBuyDyInfoOverServer(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin);

	//邀请解锁商店货架
	void InviteUnlockShopShelf(unsigned uid);

private:
	//上架物品
	int ShelfPros(unsigned uid,unsigned shelf_ud,unsigned props_id,unsigned props_cnt,unsigned props_price,unsigned ad_flag,ProtoShop::ShelfPropsResp* resp);

	//访问玩家商店
	int VisitOtherShop(unsigned othuid, ProtoShop::VisitOtherShopResp * resp);

	//购买商店物品前校验
	int BeforeBuyShopItemCheck(unsigned uid, DataShop &shop, bool firstCheck = true);

	//处理购买
	int handleShopPurchase(unsigned uid,const DataShop &shop,ProtoShop::PurchaseResp* resp);
};


#endif //LOGIC_SHOP_MANAGER_H
