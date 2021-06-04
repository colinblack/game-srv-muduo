#include "LogicIdCtrl.h"

CLogicIdCtrl::CLogicIdCtrl()
{

}

int CLogicIdCtrl::GetNextId(int key, uint64_t &nextId, unsigned serverid)
{
	int ret;

	ret = dbIdCtrl.GetId(key, nextId, serverid);

	if(ret != 0)
	{
		error_log("GetId fail. key=%d,serverid=%u,ret=%u", key,serverid,ret);
		throw runtime_error("db_get_id_ctrl_fail");
	}

	nextId++;
	ret = dbIdCtrl.SetId(key, nextId, serverid);

	if(ret != 0)
	{
		error_log("SetId fail. key=%d,nextId=%llu,serverid=%u,ret=%u", key, nextId, serverid, ret);
		throw runtime_error("db_set_id_ctrl_fail");
	}

	return 0;
}

int CLogicIdCtrl::GetCurrentId(int key, uint64_t &currId, unsigned serverid)
{
	int ret;

	ret = dbIdCtrl.GetId(key, currId, serverid);

	if(ret != 0)
	{
		error_log("getId fail. key=%d,serverid=%u", key, serverid);
		throw runtime_error("get_id_fail");
	}

	return 0;
}
