#ifndef LOGIC_RANDOMBOX_MANAGER_H
#define LOGIC_RANDOMBOX_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicRandomBoxManager :public BattleSingleton, public CSingleton<LogicRandomBoxManager>
{
private:
	friend class CSingleton<LogicRandomBoxManager>;
	LogicRandomBoxManager(){}
	enum{
		boxt_of_free   = 1,
		box_of_charge  = 2,

		free_box_of_props = 1,
		free_box_of_diamond = 2,
		free_box_of_coin    = 3,

		normal_of_get_randombox_gift = 0,//普通获取随机礼包方式
		viewad_of_get_randombox_gift = 1,//看广告或分享或许随机礼包
	};
	map<unsigned,unsigned>m_giftid; // uid -> giftid
public:

	virtual void CallDestroy() { Destroy();}

	//打开随机宝箱
	int Process(unsigned uid,ProtoRandomBox::OpenBoxReq *req,ProtoRandomBox::OpenBoxResp *resp);

	//购买礼包
	int Process(unsigned uid,ProtoRandomBox::BuyBoxGiftReq *req,ProtoRandomBox::BuyBoxGiftResp *resp);
private:
	//打开箱子后续处理
	int HandleOpenBox(unsigned uid,unsigned boxtype,ProtoRandomBox::OpenBoxResp *resp);

	//打开免费箱子处理
	int HandleOpenFreeBox(unsigned uid,ProtoRandomBox::OpenBoxResp *resp);

	//打开收费箱子处理
	int HandleOpenChargeBox(unsigned uid,ProtoRandomBox::OpenBoxResp *resp);

	//生成随机数[start-end]
	int GetRandom(int start,int end);


};


#endif //LOGIC_RANDOMBOX_MANAGER_H
