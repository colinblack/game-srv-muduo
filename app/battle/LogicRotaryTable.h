/*
 * LogicRotaryTable.h
 *
 *  Created on: 2018年6月19日
 *      Author: summer
 */

#ifndef APP_BATTLE_LOGICROTARYTABLE_H_
#define APP_BATTLE_LOGICROTARYTABLE_H_

#include "ServerInc.h"

class RotaryTableActivity : public BattleSingleton, public CSingleton<RotaryTableActivity>
{
private:
	friend class CSingleton<RotaryTableActivity>;
	RotaryTableActivity() {actid = e_Activity_RotaryTable;}
	int actid;
public:
	enum{
		daily_free_draw_count_save_index        = 10,//每日免费抽奖次数存储索引
		daily_used_free_draw_count_save_index   = 11,//每日已使用免费抽奖次数存储索引
		daily_friendly_draw_count_save_index   = 12,//每日已使用友情值抽奖次数存储索引

		item_type_cash              = 1,//转盘内容为钻石
		item_type_coin              = 2,//转盘内容为金币
		item_type_props             = 3,//转盘内容为物品
	};

	void CallDestroy() {Destroy();}

	//获取转盘信息
	int Process(unsigned uid,ProtoRotaryTable::GetRotaryTableInfoReq *req,ProtoRotaryTable::GetRotaryTableInfoResp *resp);

	//抽奖
	int Process(unsigned uid,ProtoRotaryTable::DrawRotaryTableReq *req,ProtoRotaryTable::DrawRotaryTableResp *resp);

	//分享
	int Process(unsigned uid,ProtoRotaryTable::ShareReq *req,ProtoRotaryTable::ShareResp *resp);


	//重置活动数据
	void ResetActivity(unsigned uid);

	//统计分享次数
	void AddShareCount(unsigned uid);

private:
	//数据存取函数
	unsigned Get16Low(const unsigned & data);
	unsigned Get16High(const unsigned & data);
	void SetSaveData(unsigned & data, const unsigned & high,const unsigned & low);

	//获取区间随机数
	int GetRandom(int start,int end);

	//从data中随机出size个物品放入rlt中，不重复
	int GetRandomItemList(unsigned uid,vector<unsigned> data,int size,vector<unsigned> & rlt,DataGameActivity & activity);

	//获取转盘数据
	void SetActivityData(unsigned uid,DataGameActivity & activity);
};

#endif /* APP_BATTLE_LOGICROTARYTABLE_H_ */
