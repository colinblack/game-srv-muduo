#ifndef LOGIC_BUILDING_MANAGER_H_
#define LOGIC_BUILDING_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"
#include "LogicQueueManager.h"

//建筑建造
class DataBuildRoutine : public DataRoutineBase
{
public:
	DataBuildRoutine(unsigned uid, unsigned endts, unsigned ud):
		DataRoutineBase(uid, endts, ud)
	{

	}

	virtual void CheckUd(unsigned buildud);

	//获取需要扣的钻石数以及剩余时间
	virtual void GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type);

	virtual void SingleRoutineEnd(unsigned buildud, ProtoPush::PushBuildingsCPP * msg);
};

class LogicBuildManager : public BattleSingleton, public CSingleton<LogicBuildManager>
{
private:
	friend class CSingleton<LogicBuildManager>;
	LogicBuildManager();
	int userActId;
public:
	enum
	{
		grid_ypos_len = 1000,  //y坐标表示的范围

		e_Activity_UserData_1_index_11 = 11,//每日看广告减少建筑cd的使用次数
	};

	virtual void CallDestroy() { Destroy();}

	virtual int OnInit();

	int NewUser(unsigned uid);

	int CheckLogin(unsigned uid);

	//建造
	int Process(unsigned uid, ProtoBuilding::BuildReq* req, ProtoBuilding::BuildResp* resp);

	//生产设备揭幕
	int Process(unsigned uid, ProtoBuilding::UnveilBuildReq* req, ProtoBuilding::UnveilBuildResp* resp);

	//移动
	int Process(unsigned uid, ProtoBuilding::MoveReq* req, ProtoBuilding::MoveResp* resp);

	//翻转
	int Process(unsigned uid, ProtoBuilding::FlipReq* req, ProtoBuilding::FlipResp* resp);

	//升级(粮仓、货仓)
	int Process(unsigned uid, ProtoBuilding::BuildingUpReq *req, ProtoBuilding::BuildingUpResp* resp);

	//建筑星级加速
	int Process(unsigned uid, ProtoBuilding::UpgradeStarSpeedUpReq *req, ProtoBuilding::UpgradeStarSpeedUpResp* resp);

	//拆除障碍物
	int Process(unsigned uid, ProtoBuilding::RemoveBarrierReq *req, ProtoBuilding::RemoveBarrierResp* resp);

	//变卖装饰物
	int Process(unsigned uid, ProtoBuilding::SellDecorateReq* req, ProtoBuilding::SellDecorateResq* resp);

	//地块解锁
	int Process(unsigned uid, ProtoBuilding::ExpandMapReq* req, ProtoBuilding::ExpandMapResp* resp);

	//看广告减少建筑cd时间
	int Process(unsigned uid, ProtoBuilding::ViewAdReduceBuildTimeReq* req, ProtoBuilding::ViewAdReduceBuildTimeResp* resp);

	//获取看广告减建筑cd的次数
	int Process(unsigned uid, ProtoBuilding::GetViewAdReduceBuildTimeReq* req, ProtoBuilding::GetViewAdReduceBuildTimeResp* resp);

	//每日重置看广告减建筑cd的次数
	int ResetViewAdReduceBuildTime(unsigned uid);

	//获取仓库的剩余可用空间.type: 1-粮仓 2-货仓
	int GetStorageRestSpace(unsigned uid, unsigned type);

	//建造经验奖励
	int BuildExpReward(unsigned uid, unsigned build_id, DataCommon::CommonItemsCPP * msg);

	//获取所有生产数目
	int GetAllProductBuildNum(unsigned uid);

private:
	//建造
	int Build(unsigned uid, unsigned build_id, unsigned xpos, unsigned ypos, unsigned direct,ProtoBuilding::BuildResp *resp);

	//建造前的处理
	int CheckAndCostBeforeBuild(unsigned uid, unsigned build_id, unsigned & wait_time, ProtoBuilding::BuildResp *resp);

	//建筑揭幕
	int UnVeil(unsigned uid, unsigned buildud, ProtoBuilding::UnveilBuildResp * resp);

	bool IsNoExpReward(unsigned type);

	//移动
	int Move(unsigned uid, unsigned build_ud, unsigned direct,unsigned xpos, unsigned ypos, ProtoBuilding::MoveResp *resp);
	//翻转
	int Flip(unsigned uid, unsigned build_ud, ProtoBuilding::FlipResp *resp);

	//设备升星立即完成
	int StarSpeedUp(unsigned uid, unsigned id, ProtoBuilding::UpgradeStarSpeedUpResp * resp);

	//拆除障碍物
	int RemoveBarrier(unsigned uid, unsigned id, ProtoBuilding::RemoveBarrierResp * resp);

	//根据建筑id,原点id，计算所占的所有格子
	int SeteBuildGrid(unsigned uid, unsigned xpos, unsigned ypos, uint8_t direct, unsigned build_id, int * status,bool isbuild, bool isbarrier = false);

	bool IsIntersect(unsigned uid, unsigned ud, int * build_status);

	//校验扩展地图格子是否已解锁
	bool CheckMapGridIsUnlock(unsigned uid,unsigned xpos,unsigned x_offset,unsigned ypos,unsigned y_offset);

	//地块扩展
	int LandExpand(unsigned uid, ProtoBuilding::ExpandMapReq* req, ProtoBuilding::ExpandMapResp* resp);

	template<class P, class T>
	int CostBeforeBuild(unsigned uid, T& cfg, string reason,int num, int level, unsigned & wait_time, ProtoBuilding::BuildResp *resp);

	template<class T>
	bool IsInteractBetween(unsigned uid, int * build_status, T& buildmap);
private:
	//map<unsigned, >  ;  //障碍物格子数
};

template<class P, class T>
int LogicBuildManager::CostBeforeBuild(unsigned uid, T& cfg, string reason, int num, int level, unsigned & wait_time,
		ProtoBuilding::BuildResp *resp)
{
	//获取等级索引只用于判断当前的用户等级能够建造的最大数目。
	BuildCfgWrap buildcfgwrap;
	int levelindex = buildcfgwrap.GetLevelIndex(cfg.need_level(), level);

	//判断当前数目是否超过要求
	if (num >= cfg.gain_num(levelindex))
	{
		error_log("build num already max. uid=%u,build_id=%u", uid, cfg.id());
		throw runtime_error("build_num_max");
	}

	//建造相关的索引，而当前这个索引，是用于判断当前建造的建筑该用哪个索引的配置，比如消耗和等待时间
	int build_index = buildcfgwrap.GetGainNumIndex(cfg.gain_num(), num + 1);

	//处理消耗，获取当前建造的数目的下一个，对应的正确消耗
	P::Instance()->CommonProcess(uid, cfg.need_cost(build_index), reason, resp->mutable_commons());

	if (cfg.wait_time_size() > 0)
	{
		wait_time = cfg.wait_time(build_index);
	}

	return 0;
}

template<class T>
bool LogicBuildManager::IsInteractBetween(unsigned uid, int * build_status, T& buildmap)
{
	char conflict[10] = {0};
	string errmsg;

	for(unsigned i = 0; i < GRID_LENGTH; ++i)
	{
		if ((buildmap[i] & build_status[i]) > 0)
		{
			//记录哪些格相交
#ifdef _DEBUG_BUILD
			int result = buildmap[i] & build_status[i];
			bitset<32> bitres(result);

			for(size_t j = 0; j < bitres.size(); ++j)
			{
				if (bitres[j])
				{
					//记录哪些格子冲突
					int tempgrid = i*INT_BITS + j;

					sprintf(conflict, "%d|%d,", tempgrid/MAP_LENGTH, tempgrid%MAP_LENGTH);

					errmsg += conflict;
				}
			}

			debug_log("grid conflict. uid=%u,grid=%s.", uid, errmsg.c_str());
#endif
			return true;
		}
	}

	return false;
}

#endif //LOGIC_BUILDING_MANAGER_H_
