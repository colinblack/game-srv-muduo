#ifndef LOGIC_ACCESSAD_MANAGER_H
#define LOGIC_ACCESSAD_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicAccessAdManager :public BattleSingleton, public CSingleton<LogicAccessAdManager>
{
private:
	friend class CSingleton<LogicAccessAdManager>;
	LogicAccessAdManager(){actid = e_Activity_FriendlyTree;userDataId = e_Activity_UserData_1;}

public:
	//对应此活动表存储索引
	enum{
		activiy_table_save_index_4   = 3,//用于存储下一次观看广告可领取奖励ts

		e_Activity_UserData_1_index_3 = 3,//当天使用气球看广告的次数
		e_Activity_UserData_1_index_4 = 4,//等级看广告获取奖励时,存储等级,用于校验
		e_Activity_UserData_1_index_5 = 5,//存储稻草人加成时间结束ts
		e_Activity_UserData_1_index_6 = 6,//存储稻草人下一次会出现的ts
		e_Activity_UserData_1_index_7 = 7,//当天试用稻草人看广告的次数
		e_Activity_UserData_1_index_8 = 8,//下一次气球可看广告的cd时间



		type_of_ballon_veiw_ad        = 0,//气球看广告
		type_of_upgrage_veiw_ad       = 1,//升级看广告
		type_of_max_view_ad
	};

	virtual void CallDestroy() { Destroy();}

	//获取上次看广告的ts
	int Process(unsigned uid,ProtoAccessAd::GetLastViewAdTsReq *req,ProtoAccessAd::GetLastViewAdTsResp *resp);
	//领取看广告奖励
	int Process(unsigned uid,ProtoAccessAd::RewardViewAdReq *req,ProtoAccessAd::RewardViewAdResp *resp);

	//获取气球信息
	int Process(unsigned uid,ProtoAccessAd::GetBallonInfoReq *req,ProtoAccessAd::GetBallonInfoResp *resp);

	//通用看广告处理
	int Process(unsigned uid, ProtoAccessAd::CommonlViewAdReq* req, ProtoAccessAd::CommonlViewAdResp* resp);

	//获取稻草人信息
	int Process(unsigned uid, ProtoAccessAd::GetScarecrowInfoReq* req, ProtoAccessAd::GetScarecrowInfoResp* resp);

	//稻草人看广告
	int Process(unsigned uid, ProtoAccessAd::ScarecrowViewAdReq* req, ProtoAccessAd::ScarecrowViewAdResp* resp);

	//稻草人农作物加成
	float ScarecrowBonus(unsigned uid);

	//重置每日看广告次数
	int ResetViewAdCnt(unsigned uid);
private:
	//处理气球看广告奖励
	int HandleBallonViewAd(unsigned uid,ProtoAccessAd::CommonlViewAdResp* resp);

	//处理升级看广告奖励
	int HandleUpgardeViewAd(unsigned uid,ProtoAccessAd::CommonlViewAdResp* resp);

	int actid;
	int userDataId;
};


#endif //LOGIC_ACCESSAD_MANAGER_H
