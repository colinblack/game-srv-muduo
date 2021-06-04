/*
 * LogicRotaryTable.h
 *
 *  Created on: 2018年6月19日
 *      Author: summer
 */

#ifndef APP_BATTLE_LOGICNEWSHARE_H_
#define APP_BATTLE_LOGICNEWSHARE_H_

#include "ServerInc.h"

class LogicNewShareActivity : public BattleSingleton, public CSingleton<LogicNewShareActivity>
{
private:
	friend class CSingleton<LogicNewShareActivity>;
	LogicNewShareActivity() {actid = e_Activity_New_Share;}
	int actid;
public:
	enum{
		e_Activity_New_Share_index_0  = 0,//用于存储下一次可分享的ts
		e_Activity_New_Share_index_1  = 1,//用于存储领取标记
		e_Activity_New_Share_index_2  = 2,//用于存储领取标记
		e_Activity_New_Share_index_3  = 3,//用于存储领取标记
		e_Activity_New_Share_index_4  = 4,//用于存储领取标记
		e_Activity_New_Share_index_5  = 5,//用于存储领当日分享次数

	};

	void CallDestroy() {Destroy();}

	//获取活动数据
	int Process(unsigned uid, ProtoActivity::GetNewShareInfoReq* req, ProtoActivity::GetNewShareInfoResp* resp);

	//领取奖励
	int Process(unsigned uid, ProtoActivity::RewardNewShareReq* req, ProtoActivity::RewardNewShareResp* resp);

	//分享
	int Process(unsigned uid, ProtoActivity::NewShareReq* req, ProtoActivity::NewShareResp* resp);

	//重置每日分享活动数据
	int ResetNewShareData(unsigned uid);
private:
	//设置指定标记位
	void SetDataFlag(unsigned & data,unsigned index);

	//获取指定位置索引位
	unsigned GetDataFlag(unsigned data,unsigned index);

};

#endif /* APP_BATTLE_LOGICNEWSHARE_H_ */
