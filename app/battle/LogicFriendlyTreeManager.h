#ifndef LOGIC_FRIENDLYTREE_MANAGER_H
#define LOGIC_FRIENDLYTREE_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicFriendlyTreeManager :public BattleSingleton, public CSingleton<LogicFriendlyTreeManager>
{
private:
	friend class CSingleton<LogicFriendlyTreeManager>;
	LogicFriendlyTreeManager(){actid = e_Activity_FriendlyTree;}

public:
	virtual void CallDestroy() { Destroy();}

	//对应此活动表存储索引
	enum{
		activiy_table_save_index_1   = 0,//对应活动表的第一个字段用于存储友情树下一次可浇水的ts
		activiy_table_save_index_2   = 1,//对应活动表的第二个字段用于存储友情树的状态
	};

	//友情树状态
	enum{
		status_growth    = 0,//浇水成长阶段
		status_haverst   = 1,//收获阶段
		status_regrowth  = 2,//再生长阶段
	};

	enum{
		water_code_success                  = 0,//浇水成功
		water_code_tree_regrowth            = 1,//树处于枯萎状态
		water_code_tree_harvest             = 2,//树处于收获状态
		water_code_tree_cd_error            = 3,//个人浇水cd未到
		water_code_level_error              = 4,//等级未解锁
		water_code_friendly_max_error       = 5,//友情值已到上线
	};

	//获取友情树基本信息
	int Process(unsigned uid, ProtoFriendlyTree::GetFriendlyTreeReq* req, ProtoFriendlyTree::GetFriendlyTreeResp* resp);

	//浇水
	int Process(unsigned uid, ProtoFriendlyTree::WaterFriendlyTreeReq* req);

	//跨服浇水
	int Process(ProtoFriendlyTree::CSWaterFriendlyTreeReq* req);

	//处理跨服浇水返回
	int Process(ProtoFriendlyTree::CSWaterFriendlyTreeResp* req);

	//领取友情值
	int Process(unsigned uid, ProtoFriendlyTree::RewardFriendlyTreeReq* req, ProtoFriendlyTree::RewardFriendlyTreeResp* resp);

	//设置友情树基本信息
	void FullMessage(unsigned uid,ProtoFriendlyTree::FriendlyTreeCPP *msg);

	//同服到好友庄园浇水添加动态消息
	bool AddWaterDyInfo(unsigned uid,unsigned other_uid);

	//跨服到好友庄园浇水添加动态消息
	bool AddWaterDyInfoOverServer(unsigned uid,unsigned other_uid);

private:

	//浇水处理
	int WaterTree(unsigned uid,unsigned othuid,ProtoFriendlyTree::WaterFriendlyTreeResp* resp);

	//浇水次数是否已满
	bool isFull(unsigned uid);

	//设置浇水者信息
	void SetWaterTreeMessage(const DataFriendlyTree &tree,ProtoFriendlyTree::FriendlyTreeBasicCPP *msg);

	//设置树状态信息
	void SetTreeStatusMessage(unsigned uid,ProtoFriendlyTree::FriendlyTreeStatusCPP *msg);

	//推送消息
	int PushMessage(unsigned uid);

	//获取对应浇水者下一次可浇水时间
	unsigned GetWaterTS(unsigned uid,unsigned othuid);


	int actid;
};


#endif //LOGIC_FRIENDLYTREE_MANAGER_H
