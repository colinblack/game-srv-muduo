/*
 * LogicSysMail.h
 *
 *  Created on: 2018-2-8
 *      Author: Ralf
 */

#ifndef LOGICSYSMAIL_H_
#define LOGICSYSMAIL_H_

#include "BattleServer.h"

class LogicSysMailManager: public BattleSingleton, public CSingleton<
		LogicSysMailManager> {
public:
	virtual void CallDestroy() {
		Destroy();
	}
	enum
	{
		e_Activity_UserData_1_index_12 = 12,//每日看广告减少建筑cd的使用次数
	};
	LogicSysMailManager(){userActId = e_Activity_UserData_1;};
	void OnLogin(unsigned uid, unsigned ts);
	void SetMessage(unsigned uid, User::Base* msg);
	int Process(Admin::SysMail *req, Admin::ReplySysMail *resp);
	int Process(unsigned uid, User::RequestSysMail *req, User::ReplySysMail *resp);
	int Process(unsigned uid, User::RequestMailRead *req, User::ReplyMailRead *resp);
	int Process(unsigned uid, User::RequestMailGet *req, User::ReplyMailGet *resp);
	int Process(unsigned uid, User::RequestMailDel *req);
	int Process(unsigned uid, User::RequestMailAllGet *req, User::ReplyMailAllGet *resp);
	int Process(unsigned uid, User::RequestMailAllDel *req);
	int Process(User::ReqSendMailBC *req);

	bool ExistNewMsg(unsigned uid);
	int DoReward(unsigned uid, const string& sys, const CommonGiftConfig::CommonModifyItem& reward);
	int DoRewardLocal(MemorySysmailItem& ms);

	//查询是否为指定平台
	bool ValidPlatForm(unsigned uid);

	//游戏内添加邮件
	int AddMail(unsigned uid,string title,string content,string reward);

	//对于指定平台,游戏内给玩家发送邮件
	int AddMail(unsigned uid,bool NewUser);

	//首充发送邮件提醒
	int FirstRechargeAddMail(unsigned uid);

private:
	void Get(unsigned uid, unsigned ts, ProtoReward::RewardInfo* msg);
	int userActId;
};

#endif /* LOGICSYSMAIL_H_ */
