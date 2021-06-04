#ifndef LOGIC_FRIEND_MANAGER_H
#define LOGIC_FRIEND_MANAGER_H

#include "ServerInc.h"

class LogicFriendManager: public BattleSingleton, public CSingleton<LogicFriendManager>
{
public:
	enum {
		GET_CONCERN_FRIEND = 1,
		GET_FANS_FRIEND = 2,
		GET_HELP_FRIEND = 3
	};

	virtual void CallDestroy()
	{
		Destroy();
	}

	//请求好友数据
	int Process(unsigned uid, ProtoFriend::GetAllFriendsReq* req);

	//请求需要帮助的好友数据
	int Process(unsigned uid, ProtoFriend::GetFriendHelpInfoReq* req);

	//处理跨服获取好友帮助信息
	int Process(ProtoFriend::CSGetFriendHelpInfoReq* req);

	//关注
	int Process(unsigned uid, ProtoFriend::ConcernReq* req);

	//跨服关注
	int Process(ProtoFriend::CSConcernReq* req);

	//处理跨服关注返回的消息
	int Process(ProtoFriend::CSConcernResp* req);

	//取消关注
	int Process(unsigned uid, ProtoFriend::CancelConcernReq* req);

	//跨服取消关注
	int Process(ProtoFriend::CSCancelConcernReq* req);

	//处理跨服取消关注的返回消息
	int Process(ProtoFriend::CSCancelConcernResp* req);

	//删除粉丝
	int Process(unsigned uid, ProtoFriend::RemoveFansReq *req, ProtoFriend::RemoveFansResp* resp);

	//同服关注好友添加动态消息
	bool AddConcernDyInfo(unsigned uid,unsigned other_uid);

	//跨服关注好友添加动态消息
	bool AddConcernDyInfoOverServer(unsigned uid,unsigned other_uid);

private:

	//关注
	int Concern(unsigned uid, unsigned othuid, ProtoFriend::ConcernResp * resp);

	//取消关注
	int CancelConcern(unsigned uid, unsigned othuid, ProtoFriend::CancelConcernResp * resp);

	//删除粉丝
	int RemoveFans(unsigned uid, unsigned othuid, ProtoFriend::RemoveFansResp * resp);

	void GetConcernFriendInfo(unsigned uid,ProtoFriend::GetAllFriendsResp *resp);

	void GetFansFriendInfo(unsigned uid,ProtoFriend::GetAllFriendsResp *resp);

	void GetAidFriendInfo(unsigned uid,ProtoFriend::GetAllFriendsResp *resp);
};


#endif //LOGIC_FRIEND_MANAGER_H
