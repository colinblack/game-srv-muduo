#ifndef LOGIC_MESSAGE_BOARD_MANAGER_H_
#define LOGIC_MESSAGE_BOARD_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"
#include "ProtoMessageBoard.pb.h"
#include "ProtoDynamicInfo.pb.h"


#define DAILY_FEEDBACK_TIMES 3
#define MAX_FEEDBACK_COUNT 10

class  LogicMessageBoardManager : public BattleSingleton, public CSingleton<LogicMessageBoardManager>
{
private:
	friend class CSingleton<LogicMessageBoardManager>;
	LogicMessageBoardManager() {}

public:
	virtual void CallDestroy() { Destroy();}

	//前端主动获取留言消息 同服
	int Process(unsigned uid, ProtoMessageBoard::GetMessageBoardReq *reqmsg);

	//前端请求删除留言消息 同服
	int Process(unsigned uid, ProtoMessageBoard::DeleteMessageBoardReq *reqmsg);

	//前端请求更新有留言状态 同服
	int Process(unsigned uid, ProtoMessageBoard::HasNewLeaveMessageReq *reqmsg, ProtoMessageBoard::HasNewLeaveMessageResp * respmsg);

	//跨服 观看别人家的留言板内容
	int Process(ProtoMessageBoard::GetMasterVisiableMsgReq *msg);

	//跨服 在别人家删除自己的留言
	int Process(ProtoMessageBoard::DeleteMyMsgOverServerReq *msg);

	//同服 在别人家给他留言
	int Process(unsigned uid, ProtoMessageBoard::LeaveMessageReq *reqmsg);

	//同服 在自己家回复留言
	int Process(unsigned uid, ProtoMessageBoard::AnswerLeaveMessageReq *reqmsg);

	//跨服 在别人家给他留言 或者 在自己家回复别人的留言
	int Process(ProtoMessageBoard::SendLeaveMsgOverServerReq *msg);

	//后台主动推送是否有新的留言消息
	bool NotifyNewMsg2Client(unsigned uid);

	//产生留言消息统一接口
	int ProduceOneMsgInfo(unsigned uid, MsgInfoItem & msgitem);

	//获取留言条数
	int GetMsgInfo(unsigned uid,MsgInfoItem *msgItem,unsigned max_count);

	//在别人家获取可见留言
	int GetVisiableMsgInfo(unsigned myuid,unsigned masteruid,MsgInfoItem *msgItem,unsigned max_count);

	//设置留言返回信息
	bool SetLeaveMsgRespMsg(const MsgInfoItem& msgItem,ProtoMessageBoard::MessageInfo *msg);

	//更新玩家下线时间
	bool UpdateOffLine(unsigned uid,unsigned offtime);

	//内网调试时测试淘汰策略
	bool CheckClearDemo();

	//共享内存满后，执行淘汰策略
	bool CheckClearMsgInfo();

	//每日更新剩余反馈次数
	bool UpdateFeedbackTimes();

	//获取玩家的反馈消息 同服
	int Process(unsigned uid,ProtoMessageBoard::GetFeedbackReq *reqmsg,ProtoMessageBoard::GetFeedbackResp *respmsg);

	//玩家发送系统反馈消息 同服
	int Process(unsigned uid,ProtoMessageBoard::SendFeedbackReq *reqmsg,ProtoMessageBoard::SendFeedbackResp *respmsg);

	//玩家删除自己的反馈消息 同服
	int Process(unsigned uid,ProtoMessageBoard::DelFeedbackReq *repmsg);

private:

	//给玩家增加一条留言
	int AddOneMsgInShm(unsigned uid,MsgInfoItem & msgitem);

private:

	//玩家当天剩余反馈次数
	map<unsigned,unsigned> m_map_rest_times;
};


#endif
