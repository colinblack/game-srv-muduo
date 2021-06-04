/*
 * ProtoManager.cpp
 *
 *  Created on: 2016-8-20
 *      Author: Ralf
 */

#include "ServerInc.h"

CurCMD ProtoManager::m_CurCMD = e_CMD_none;

int ProtoManager::DisCardMessage(Message* message)
{
	error_log("Discarding %s.", message->GetTypeName().c_str());
	LogicManager::Instance()->SetErrMsg("msg_discard");
	return R_ERR_PARAM;
}
int ProtoManager::ProcessLogin(Common::Login* msg)
{
	int ret = 0;
	try
	{
		ret = UserManager::Instance()->ProcessLogin(msg);
	}
	catch(runtime_error & e)
	{
		ret = R_ERROR;
	}
	if(ret)
	{
		info_log("kick login error, uid=%u, fd=%u", msg->uid(), LogicManager::Instance()->Getfd());
		LogicManager::Instance()->forceKick(msg->uid(), "login_error");
	}
	m_CurCMD = e_CMD_none;
	return 0;
}
