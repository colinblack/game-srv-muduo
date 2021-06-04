#include "ServerInc.h"
#include "BattleServer.h"


int LogicMessageBoardManager::Process(unsigned uid, ProtoMessageBoard::GetMessageBoardReq *reqmsg)
{
	unsigned msg_show_max = PER_USER_MAX_LEAVE_MSG;		//最多留言条数
	unsigned msg_show_cnt = 0;							//实际可以展示的留言条数

	MsgInfoItem* msgItem = NULL;
	msgItem = (MsgInfoItem*)malloc(sizeof(MsgInfoItem)*msg_show_max);
	if(msgItem == NULL)
	{
		error_log("init is failed");
		throw std::runtime_error("init is failed");
	}

	unsigned masteruid = reqmsg->masteruid();
	if(uid == masteruid)							//在自家查看留言
	{
		ProtoMessageBoard::GetMessageBoardResp * respmsg = new ProtoMessageBoard::GetMessageBoardResp;

		//获取留言条数
		msg_show_cnt = GetMsgInfo(uid,msgItem,msg_show_max);

		//设置返回信息
		for(unsigned idx = 0; idx < msg_show_cnt; idx++)
		{
			SetLeaveMsgRespMsg(msgItem[idx],respmsg->add_arraymsgbrd());
		}

		//将消息推送给前端
		LogicManager::Instance()->sendMsg(uid,respmsg);
	}
	else											//在别人家访问留言板
	{
		if(CMI->IsNeedConnectByUID(masteruid))		//跨服
		{
			ProtoMessageBoard::GetMasterVisiableMsgReq * reqmsg = new ProtoMessageBoard::GetMasterVisiableMsgReq;
			reqmsg->set_myuid(uid);
			reqmsg->set_masteruid(masteruid);

			BMI->BattleConnectNoReplyByUID(masteruid, reqmsg);
		}
		else										//同服
		{
			ProtoMessageBoard::GetMessageBoardResp * respmsg = new ProtoMessageBoard::GetMessageBoardResp;

			//获取可见的留言条数
			msg_show_cnt = GetVisiableMsgInfo(uid,masteruid,msgItem,msg_show_max);

			//设置返回信息
			for(unsigned idx = 0; idx < msg_show_cnt; idx++)
			{
				SetLeaveMsgRespMsg(msgItem[idx],respmsg->add_arraymsgbrd());
			}

			//将消息推送给前端
			LogicManager::Instance()->sendMsg(uid,respmsg);
		}
	}


	delete [] msgItem;
	return 0;
}

int LogicMessageBoardManager::Process(ProtoMessageBoard::GetMasterVisiableMsgReq *msg)
{
	unsigned myuid = msg->myuid();
	unsigned master = msg->masteruid();

	unsigned msg_show_max = PER_USER_MAX_LEAVE_MSG;		//最多留言条数
	unsigned msg_show_cnt = 0;							//实际可以展示的留言条数

	MsgInfoItem* msgItem = NULL;
	msgItem = (MsgInfoItem*)malloc(sizeof(MsgInfoItem)*msg_show_max);
	if(msgItem == NULL)
	{
		error_log("init is failed");
		throw std::runtime_error("init is failed");
	}

	ProtoMessageBoard::GetMessageBoardResp * respmsg = new ProtoMessageBoard::GetMessageBoardResp;

	//获取可见的留言条数
	msg_show_cnt = GetVisiableMsgInfo(myuid,master,msgItem,msg_show_max);

	//设置返回信息
	for(unsigned idx = 0; idx < msg_show_cnt; idx++)
	{
		SetLeaveMsgRespMsg(msgItem[idx],respmsg->add_arraymsgbrd());
	}

	//将消息推送给前端
	LogicManager::Instance()->sendMsg(myuid,respmsg);

}

int LogicMessageBoardManager::Process(unsigned uid, ProtoMessageBoard::DeleteMessageBoardReq *reqmsg)
{
	unsigned masteruid = reqmsg->masteruid();
	unsigned msgidx = reqmsg->id();
	unsigned typeId = reqmsg->typeid_();
	unsigned senderuid = reqmsg->senderuid();

	if(uid == masteruid)							//在自己家删除留言
	{
		MessageBoardManager::Instance()->DeleteOneMsgInfo(uid,msgidx);
	}
	else if(uid != masteruid && uid == senderuid)	//在别人家删除自己的留言
	{
		if(CMI->IsNeedConnectByUID(masteruid))		//跨服
		{
			ProtoMessageBoard::DeleteMyMsgOverServerReq *msg = new ProtoMessageBoard::DeleteMyMsgOverServerReq;
			msg->set_myuid(uid);
			msg->set_masteruid(masteruid);
			msg->set_id(msgidx);
			BMI->BattleConnectNoReplyByUID(masteruid, msg);
		}
		else										//同服
		{
			MessageBoardManager::Instance()->DeleteOneMsgInfo(masteruid,msgidx);
		}
	}
	else
	{
		return 1;
	}


	return 0;
}

int LogicMessageBoardManager::Process(ProtoMessageBoard::DeleteMyMsgOverServerReq *msg)
{
//	unsigned myuid = msg->myuid();
	unsigned master = msg->masteruid();
	unsigned msgidx = msg->id();

	MessageBoardManager::Instance()->DeleteOneMsgInfo(master,msgidx);

	return 0;
}

int LogicMessageBoardManager::Process(unsigned uid, ProtoMessageBoard::LeaveMessageReq *reqmsg)
{
	unsigned master = reqmsg->masteruid();
	unsigned typeId = reqmsg->typeid_();
	string words(reqmsg->words());

	if(uid == master)				//这是在别人家留言
	{
		return 1;
	}
	if(typeId != TYPE_MSG_SECRET_SEND && typeId != TYPE_MSG_PUBLIC_SEND)	//类型不对
	{
		return 1;
	}

	//敏感词过滤
	String::Trim(words);

	if (words.empty())
	{
		error_log("words empty. name=%s", words.c_str());
		throw runtime_error("words_empty");
	}

	if(!StringFilter::Check(words))
	{
		error_log("sensitive words. name=%s", words.c_str());
		throw runtime_error("sensitive_words");
	}

	unsigned msgidx = 0;
	if(CMI->IsNeedConnectByUID(master))			//跨服
	{
		ProtoMessageBoard::SendLeaveMsgOverServerReq *msg = new ProtoMessageBoard::SendLeaveMsgOverServerReq;
		msg->set_model(1);
		msg->set_sender(uid);
		msg->set_receiver(master);
		msg->set_typeid_(typeId);
		msg->set_words(words);
		BMI->BattleConnectNoReplyByUID(master, msg);
	}
	else										//同服
	{
		MsgInfoItem msgitem;
		msgitem.senderUid = uid;
		msgitem.typeId = typeId;
		msgitem.receiverUid = master;
		strcpy(msgitem.words,words.c_str());
		msgidx = ProduceOneMsgInfo(master,msgitem);

		//将刚才的玩家留言推送给前端
		ProtoMessageBoard::LeaveMessageResp * respmsg = new ProtoMessageBoard::LeaveMessageResp;
		ProtoMessageBoard::MessageInfo *msg = respmsg->mutable_newmsg();
		msg->set_id(msgidx);
		msg->set_ts(Time::GetGlobalTime());
		msg->set_senderuid(uid);
		msg->set_typeid_(typeId);
		msg->set_words(words);
		msg->set_receiveruid(master);
		LogicManager::Instance()->sendMsg(uid,respmsg);
	}

	return 0;
}

int LogicMessageBoardManager::Process(unsigned uid, ProtoMessageBoard::AnswerLeaveMessageReq *reqmsg)
{
	unsigned sender = reqmsg->senderuid();
	unsigned sourceType = reqmsg->typeid_();
	string words(reqmsg->words());

	//敏感词过滤
	String::Trim(words);

	if (words.empty())
	{
		error_log("words empty. name=%s", words.c_str());
		throw runtime_error("words_empty");
	}

	if(!StringFilter::Check(words))
	{
		error_log("sensitive words. name=%s", words.c_str());
		throw runtime_error("sensitive_words");
	}

	unsigned msgidx = 0;
	unsigned typeId = TYPE_MSG_PUBLIC_SEND;
	if(uid != sender)			//在自己家给别人留言
	{
		if(IS_SECRET_MSG(sourceType))
		{
			typeId = TYPE_MSG_SECRET_SEND;
		}
		else
		{
			typeId = TYPE_MSG_PUBLIC_SEND;
		}

		if(CMI->IsNeedConnectByUID(sender))			//跨服
		{

			//通过动态去创建一条留言
			ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
			msg->set_myuid(uid);
			msg->set_othuid(sender);
			msg->set_productid(typeId);
			msg->set_typeid_(TYPE_DY_ANSWER);
			msg->set_words(words);

			BMI->BattleConnectNoReplyByUID(sender, msg);
		}
		else										//同服
		{
			DynamicInfoAttach *pattach = new DynamicInfoAttach;		//通过动态去创建一条留言
			pattach->op_uid = uid;
			pattach->product_id = typeId;
			pattach->words.assign(words);
			LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(sender,TYPE_DY_ANSWER,pattach);

		}
	}

	//在自己家给自己或者他人留言
	MsgInfoItem msgitem;
	msgitem.senderUid = uid;
	msgitem.typeId = typeId + 1;
	msgitem.receiverUid = sender;
	strcpy(msgitem.words,words.c_str());
	msgidx = AddOneMsgInShm(uid,msgitem);

	//将刚才的玩家留言推送给前端
	ProtoMessageBoard::LeaveMessageResp * respmsg = new ProtoMessageBoard::LeaveMessageResp;
	ProtoMessageBoard::MessageInfo *msg = respmsg->mutable_newmsg();
	msg->set_id(msgidx);
	msg->set_ts(Time::GetGlobalTime());
	msg->set_senderuid(uid);
	msg->set_typeid_(typeId+1);
	msg->set_words(words);
	msg->set_receiveruid(sender);
	LogicManager::Instance()->sendMsg(uid,respmsg);

	return 0;
}

int LogicMessageBoardManager::Process(ProtoMessageBoard::SendLeaveMsgOverServerReq *msg)
{
	unsigned model = msg->model();
	unsigned sender = msg->sender();
	unsigned master = msg->receiver();
	unsigned typeId = msg->typeid_();
	string words(msg->words());

	unsigned msgidx = 0;
	if(1 == model)					//别人在我家给我留言
	{
		MsgInfoItem msgitem;
		msgitem.senderUid = sender;
		msgitem.typeId = typeId;
		msgitem.receiverUid = master;
		strcpy(msgitem.words,words.c_str());
		msgidx = ProduceOneMsgInfo(master,msgitem);
	}
	else if(2 == model)				//在自家回复别人的留言
	{
		MsgInfoItem msgitem;
		msgitem.senderUid = sender;
		msgitem.receiverUid = master;
		if(IS_SECRET_MSG(typeId))
		{
			msgitem.typeId = TYPE_MSG_SECRET_SEND;
		}
		else
		{
			msgitem.typeId = TYPE_MSG_PUBLIC_SEND;
		}
		strcpy(msgitem.words,words.c_str());
		msgidx = ProduceOneMsgInfo(master,msgitem);
		typeId = msgitem.typeId;
	}
	else
	{
		return 1;
	}

	//将刚才的玩家留言推送给前端
	ProtoMessageBoard::LeaveMessageResp * respmsg = new ProtoMessageBoard::LeaveMessageResp;
	ProtoMessageBoard::MessageInfo *pnewmsg = respmsg->mutable_newmsg();
	pnewmsg->set_id(msgidx);
	pnewmsg->set_ts(Time::GetGlobalTime());
	pnewmsg->set_senderuid(sender);
	pnewmsg->set_typeid_(typeId);
	pnewmsg->set_words(words);
	pnewmsg->set_receiveruid(master);
	LogicManager::Instance()->sendMsg(sender,respmsg);

	return 0;
}

bool LogicMessageBoardManager::NotifyNewMsg2Client(unsigned uid)
{
	bool ret = false;

	//检查是否有留言
	if(MessageBoardManager::Instance()->HasNewMsgInfo(uid))
	{
		ret = true;
	}

	ProtoMessageBoard::HasNewLeaveMessageResp * respmsg = new ProtoMessageBoard::HasNewLeaveMessageResp;
	respmsg->set_hasnewmsg(ret);

	//发送留言消息到client
	LogicManager::Instance()->sendMsg(uid,respmsg);

	return ret;
}

int LogicMessageBoardManager::GetMsgInfo(unsigned uid,MsgInfoItem *msgItem,unsigned max_count)
{

	map<unsigned,deque<unsigned> > & msgMap = MessageBoardManager::Instance()->GetMsgInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = msgMap.find(uid);
	if(it_map == msgMap.end())	//no uid
	{
		return 0;
	}
	if(!it_map->second.size())	//no MessageBoard
	{
		return 0;
	}
	int show_count = 0;
	int info_size = it_map->second.size();
	if(info_size > max_count)
	{
		info_size = max_count;
	}
	for(unsigned j = 0;j < info_size;++j)		//从0开始遍历deque
	{
		unsigned msgidx = it_map->second[j];

		msgItem[show_count].Set(MessageBoardManager::Instance()->GetMsgInfoItem(uid,msgidx));
		++show_count;

	}
	return show_count;

}

int LogicMessageBoardManager::GetVisiableMsgInfo(unsigned myuid,unsigned masteruid,MsgInfoItem *msgItem,unsigned max_count)
{
	map<unsigned,deque<unsigned> > & msgMap = MessageBoardManager::Instance()->GetMsgInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = msgMap.find(masteruid);
	if(it_map == msgMap.end())	//no uid
	{
		return 0;
	}
	if(!it_map->second.size())	//no MessageBoard
	{
		return 0;
	}
	int show_count = 0;
	int info_size = it_map->second.size();
	if(info_size > max_count)
	{
		info_size = max_count;
	}

	for(unsigned jdx = 0;jdx < info_size;++jdx)
	{
		unsigned msgidx = it_map->second[jdx];
		MsgInfoItem & item = MessageBoardManager::Instance()->GetMsgInfoItem(masteruid,msgidx);
		unsigned type = item.typeId;
		unsigned sender = item.senderUid;
		unsigned receiver = item.receiverUid;
		if(IS_PUBLIC_MSG(type) || sender == myuid || receiver == myuid)
		{
			msgItem[show_count].Set(item);
			++show_count;
		}

	}

	return show_count;
}

bool LogicMessageBoardManager::SetLeaveMsgRespMsg(const MsgInfoItem& msgItem,ProtoMessageBoard::MessageInfo *msg)
{
	msg->set_id(msgItem.msgidx);
	msg->set_senderuid(msgItem.senderUid);
	msg->set_ts(msgItem.ts);
	msg->set_typeid_(msgItem.typeId);
	msg->set_words(msgItem.words);
	msg->set_receiveruid(msgItem.receiverUid);
	return true;
}

int LogicMessageBoardManager::ProduceOneMsgInfo(unsigned uid,MsgInfoItem & msgitem)
{
	bool has = true;

	//添加一条留言
	int msgidx = AddOneMsgInShm(uid,msgitem);
	if(-1 == msgidx)
	{
		error_log("AddOneDyInShm fail");
		return -1;
	}

	//设置新留言为true/false
	if(!MessageBoardManager::Instance()->SetHasNewMsg(uid,has))
	{
		error_log("SetHasNewMap fail");
		return -1;
	}

	if(UserManager::Instance()->IsOnline(uid))
	{
		//判断并通知client有新留言
		NotifyNewMsg2Client(uid);
	}

	return msgidx;
}

int LogicMessageBoardManager::Process(unsigned uid, ProtoMessageBoard::HasNewLeaveMessageReq *reqmsg, ProtoMessageBoard::HasNewLeaveMessageResp * respmsg)
{
	//改变留言状态为false
	MessageBoardManager::Instance()->SetHasNewMsg(uid,false);

	//回包
	respmsg->set_hasnewmsg(false);

	return 0;
}

int LogicMessageBoardManager::AddOneMsgInShm(unsigned uid,MsgInfoItem & msgitem)
{
	return MessageBoardManager::Instance()->AddMsgInfo(uid,msgitem);
}

bool LogicMessageBoardManager::UpdateOffLine(unsigned uid,unsigned offtime)
{
	if(MessageBoardManager::Instance()->UpdateOffLineTime(uid,offtime))
	{
		return true;
	}
	return false;
}

bool LogicMessageBoardManager::CheckClearDemo()
{
	if(MessageBoardManager::Instance()->CheckClearDemo())
	{
		return true;
	}
	return false;
}

bool LogicMessageBoardManager::CheckClearMsgInfo()
{
	if(MessageBoardManager::Instance()->CheckClearMsgInfo())
	{
		return true;
	}
	return false;
}

bool LogicMessageBoardManager::UpdateFeedbackTimes()
{
	m_map_rest_times.clear();
	return true;
}

int LogicMessageBoardManager::Process(unsigned uid,ProtoMessageBoard::GetFeedbackReq *reqmsg,ProtoMessageBoard::GetFeedbackResp *respmsg)
{

	map<unsigned,unsigned>::iterator it_map = m_map_rest_times.find(uid);
	if(it_map == m_map_rest_times.end())
	{
		m_map_rest_times[uid] = DAILY_FEEDBACK_TIMES;
	}

	bool exist = DataUserFeedbackManager::Instance()->IsExist(uid);
	if(!exist)
	{
		respmsg->set_resttimes(m_map_rest_times[uid]);
		return 0;
	}

	vector<unsigned> vec_ids;
	DataUserFeedbackManager::Instance()->GetIds(uid,vec_ids);

	if(vec_ids.size() > MAX_FEEDBACK_COUNT)	//推送给前端的最大反馈数
	{
		std::sort(vec_ids.begin(), vec_ids.end(), std::greater<int>());	//按时间从大到小排序
	}

	vector<unsigned>::iterator it_vec = vec_ids.begin();
	unsigned count = MAX_FEEDBACK_COUNT;
	for(;it_vec != vec_ids.end() && count;++it_vec,--count)
	{
		DataFeedback feedback = DataUserFeedbackManager::Instance()->GetData(uid,*it_vec);
		ProtoMessageBoard::FeedbackInfo *msg = respmsg->add_arrayfeedback();
		msg->set_id(feedback.id);
		msg->set_ts(feedback.id);
		msg->set_words(string(feedback.contents));
	}
	while(it_vec != vec_ids.end())		//每个玩家限存MAX_FEEDBACK_COUNT条反馈
	{
		DataUserFeedbackManager::Instance()->DelItem(uid,*it_vec);
		++it_vec;
	}
	respmsg->set_resttimes(m_map_rest_times[uid]);

	return 0;
}

int LogicMessageBoardManager::Process(unsigned uid,ProtoMessageBoard::SendFeedbackReq *reqmsg,ProtoMessageBoard::SendFeedbackResp *respmsg)
{
	string words(reqmsg->words());

	if( 0 == m_map_rest_times[uid])	//当日反馈次数已用完
	{
		return 1;
	}

	//敏感词过滤
	String::Trim(words);

	if (words.empty())
	{
		error_log("words empty. name=%s", words.c_str());
		throw runtime_error("words_empty");
	}

	if(!StringFilter::Check(words))
	{
		error_log("sensitive words. name=%s", words.c_str());
		throw runtime_error("sensitive_words");
	}

	unsigned ts = Time::GetGlobalTime();
	DataFeedback & feedback = DataUserFeedbackManager::Instance()->GetData(uid,ts);
	feedback.id = ts;
	feedback.uid = uid;
	strcpy(feedback.contents,words.c_str());
	DataUserFeedbackManager::Instance()->UpdateItem(feedback);
	feedback.SetMessage(respmsg->mutable_yourfeedback());

	if(m_map_rest_times[uid])
	{
		--m_map_rest_times[uid];
	}

	return 0;
}

int LogicMessageBoardManager::Process(unsigned uid,ProtoMessageBoard::DelFeedbackReq *repmsg)
{
	unsigned id = repmsg->id();
	bool has = DataUserFeedbackManager::Instance()->IsExistItem(uid,id);
	if(has)
	{
		DataUserFeedbackManager::Instance()->DelItem(uid,id);
	}
	else
	{
		throw runtime_error("feedback_not_exist");
	}

	return 0;
}
