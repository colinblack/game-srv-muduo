#include "LogicGameActivityManager.h"

//活动小红点对应保存在数组中的索引值
unsigned LogicGameActivityManager::index_array[ACTIVE_MAX_NUM] = {12, 5, 0, 0, 3};

void LogicGameActivityManager::FullMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
{
	RechargeActivity::Instance()->SetMessage(uid, msg);
	DailyShareActivity::Instance()->SetMessage(uid, msg);
	SignInActivity::Instance()->SetMessage(uid, msg);
	OrderActivity::Instance()->SetMessage(uid, msg);
	CropsActivity::Instance()->SetMessage(uid, msg);
	LogicFundActivityManager::Instance()->SetMessage(uid,msg);
	Recharge4399ActivityManager::Instance()->SetMessage(uid,msg);
	Daily4399ActivityManager::Instance()->SetMessage(uid,msg);
}

unsigned LogicGameActivityManager::FullStatusMarks(unsigned uid)
{
	//返回所有活动标记
	unsigned active_marks = 0;
	for(int i = 0; i < ACTIVE_MAX_NUM; ++i)
	{
		DataGameActivity & act = DataGameActivityManager::Instance()->GetUserActivity(uid, i+1);
		unsigned flag = act.actdata[index_array[i]];
		if(1 == flag)
			active_marks |= (1 << i);
	}
	return active_marks;
}


int LogicGameActivityManager::Process(unsigned uid, ProtoActivity::GameAcitivityStatusReq * req, ProtoActivity::GameAcitivityStatusResp* resp)
{
	unsigned actid = req->id();
	unsigned index = index_array[actid-1];
	unsigned active_marks = 0;
	switch(actid)
	{
	case e_Activity_Recharge:
		active_marks = RechargeActivity::Instance()->ProcessRedPoint(uid, index);
		break;
	case e_Activity_DailyShare:
		active_marks = DailyShareActivity::Instance()->ProcessRedPoint(uid, index);
		break;
	case e_Activity_Crops:
		active_marks = CropsActivity::Instance()->ProcessRedPoint(uid, index);
		break;
	case e_Activity_Order:
		active_marks = OrderActivity::Instance()->ProcessRedPoint(uid, index);
		break;
	case e_Activity_SignIn:
		active_marks = SignInActivity::Instance()->ProcessRedPoint(uid, index);
		break;
	default:
		break;
	}
	active_marks = FullStatusMarks(uid);
	debug_log("[LogicGameActivityManager::Process]uid=%u, active_marks=%u, actid=%u", uid, active_marks, actid);
	resp->set_status(active_marks);

	return 0;
}


