/*
 * ProtoManager.h
 *
 *  Created on: 2016-8-20
 *      Author: Ralf
 */

#ifndef PROTOMANAGER_H_
#define PROTOMANAGER_H_

#include "Kernel.h"
#include "DataInc.h"

/*
 * 回调管理器
 * 必须是static函数才可以注册回调
 * 这里实现具体的回调逻辑
 */
class ProtoManager
{
public:
	static int DisCardMessage(Message* message);
	static CurCMD m_CurCMD;
	static int ProcessLogin(Common::Login* msg);

	/**************通用模版**********/
	//new出回包，自动删除
	template<class T, class G, class H>
	static int Process(T* msg)
	{
		int ret = 0;
		G* resp = new G;
		try
		{
			ret = H::Instance()->Process(LogicManager::Instance()->Getuid(), msg, resp);
			if (ret != 0)
				delete resp;
			else
				LogicManager::Instance()->SetReplyProtocol(resp);

		}
		catch(const std::exception& e)
		{
			delete resp;
			LogicManager::Instance()->SetErrMsg(e.what());
			return R_ERROR;
		}

		return ret;
	}

	//不new回包，不删除
	template<class T, class G, class H>
	static int ProcessNoNew(T* msg)
	{
		int ret = 0;
		G* resp = NULL;
		try
		{
			ret = H::Instance()->Process(LogicManager::Instance()->Getuid(), msg, resp);
			if(ret == 0)
			{
				LogicManager::Instance()->SetReplyProtocol(resp);
				LogicManager::Instance()->SetNeedDelReply();
			}

		}
		catch(const std::exception& e)
		{
			LogicManager::Instance()->SetErrMsg(e.what());
			return R_ERROR;
		}

		return ret;
	}

	//无回包
	template<class T, class H>
	static int ProcessNoReply(T* msg)
	{
		int ret = 0;
		try
		{
			ret = H::Instance()->Process(LogicManager::Instance()->Getuid(), msg);
		}
		catch(const std::exception& e)
		{
			LogicManager::Instance()->SetErrMsg(e.what());
			return R_ERROR;
		}

		return ret;
	}

	//new出回包，自动删除，支付或者gm发来的无uid的包
	template<class T, class G, class H>
	static int ProcessNoUID(T* msg)
	{
		int ret = 0;
		G* resp = new G;
		try
		{
			ret = H::Instance()->Process(msg, resp);
			if (ret != 0)
				delete resp;
			else
				LogicManager::Instance()->SetReplyProtocol(resp);

		}
		catch(const std::exception& e)
		{
			delete resp;
			LogicManager::Instance()->SetErrMsg(e.what());
			return R_ERROR;
		}

		return ret;
	}

	//无回包，支付或者gm发来的无uid的包
	template<class T, class H>
	static int ProcessNoReplyNoUID(T* msg)
	{
		int ret = 0;
		try
		{
			ret = H::Instance()->Process(msg);
		}
		catch(const std::exception& e)
		{
			LogicManager::Instance()->SetErrMsg(e.what());
			return R_ERROR;
		}

		return ret;
	}
};


#endif /* PROTOMANAGER_H_ */
