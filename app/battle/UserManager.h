/*
 * UserManager.h
 *
 *  Created on: 2016-8-16
 *      Author: Ralf
 */

#ifndef USERMANAGER_H_
#define USERMANAGER_H_

#include "ServerInc.h"

class UserManager : public BattleSingleton, public CSingleton<UserManager>
{
private:
	friend class CSingleton<UserManager>;
	UserManager(){};
	virtual ~UserManager(){}

	map<unsigned, Common::Login> m_infomap;
	set<unsigned> m_save, m_alliance_save;

	//CDataName m_dbName;

	int _requestOtherUser(unsigned othuid, User::OtherUser* reply);
public:
	virtual void CallDestroy() {Destroy();}
	virtual void OnTimer2();

	int ProcessLogin(Common::Login* msg);
	int Process(unsigned myuid, User::RequestOtherUser* msg);
	int Process(User::RequestOtherUserBC* msg);
	int Process(unsigned uid, Common::ChangeName* msg, Common::ReplyChangeName* resp);
	int Process(Admin::AddCash* msg, Admin::ReplyAddCash* resp);
	int Process(unsigned uid, User::Tutorialstage* msg);
	int Process(unsigned uid, User::SwitchStatus* msg);
	int Process(Admin::AsycAdd* req, Admin::AsycAddResp* resp);
	int Process(Admin::RequestForbidTS* req, Admin::ReplyForbidTS* resp);
	int Process(Admin::SetForbidTS* req);
	int Process(Admin::SetAllianceRaceGroup* req);
	int Process(Admin::SetActivity* req);

	int Process(unsigned uid, User::SetVersion* msg);
	int Process(unsigned uid, User::SetFlag* msg);
	int Process(unsigned uid, Common::ShutDown* msg);

	int Process(unsigned uid, User::HeartBeatReq * req, User::HeartBeatResp * resp);

	int Process(uint32_t uid, User::ReqNewMsg* req, User::ReplyNewMsg* resp);

	//存档的导出
	int Process(ProtoArchive::ExportReq* req, ProtoArchive::ExportResp* resp);

	//存档的导入
	int Process(ProtoArchive::ImportReq* req, ProtoArchive::ImportResp* resp);

	//物品助手
	int Process(unsigned uid, ProtoAssistor::OpenAssistorReq* msg, ProtoAssistor::OpenAssistorResp* resp);
	int Process(unsigned uid, ProtoAssistor::UseAssistorReq* msg, ProtoAssistor::UseAssistorResp* resp);

	int CheckUser(unsigned uid);
	int AddUser(unsigned uid);
	int LoadUser(unsigned uid);

	int OnNewUser(unsigned uid, Common::Login* msg);
	int OnUserLogin(unsigned uid, Common::Login* msg);
	int UserOffLine(unsigned uid);

	void CheckActive();
	void CheckSave();
	void CheckClear();

	void GetOnlineUsers(std::vector<unsigned>& users);
	const map<unsigned, Common::Login>& GetUser(){return m_infomap;}
	bool IsOnline(unsigned uid) {return m_infomap.count(uid);}
	bool GetUserInfo(unsigned uid, Common::Login& info)
	{
		if(m_infomap.count(uid))
		{
			info = m_infomap[uid];
			return true;
		}
		return false;
	}

	std::string GetOpenId(unsigned uid);

	int LoadArchives(unsigned uid);

	int SyncSave(unsigned uid);
	int AllianceSave(unsigned aid);

	//同服访问好友庄园添加动态消息
	bool AddVisitDyInfo(unsigned uid,unsigned other_uid);
	//跨服访问好友庄园添加动态消息
	bool AddVisitDyInfoOverServer(unsigned uid,unsigned other_uid);
	//仙人加速功能激活
	bool OnFairySpeedUpOpen(uint32_t uid);
	bool IsFairySpeedUpCrop(uint32_t uid);
	bool IsFairySpeedUpEquip(uint32_t uid);
	bool IsFairySpeedUpFarm(uint32_t uid);
	float GetFairySpeedUpCrop(uint32_t uid);
	float GetFairySpeedUpEquip(uint32_t uid);
	float GetFairySpeedUpFarm(uint32_t uid);
	void SendFlagInfo(uint32_t uid,bool IsNeedCost = false);
	bool SendNewMsgToUser(uint32_t uid, uint32_t type);
	bool SendNewMsgToAll(uint32_t type);
};

#endif /* USERMANAGER_H_ */
