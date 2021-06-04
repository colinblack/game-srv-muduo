/*
 * LogicGM.h
 *
 *  Created on: 2017-7-21
 *      Author: dawx62fac
 */

#ifndef LOGICGM_H_
#define LOGICGM_H_

#include "ServerInc.h"

class DBCUserBaseWrap;

class GMCmd
{
public:
	GMCmd(std::string text)
	{
		if (text.empty())
		{
			throw std::runtime_error("cmd_content_empty");
		}

		split(text, ' ');
	}

	const std::string& cmd() const
	{
		if (v_args_.size() < 1)
		{
			throw std::runtime_error("not_parsed_cmd");
		}

		return v_args_[0];
	}

	template<typename T> T get_arg(int index) const;
private:
	void split(std::string text, const char key=' ');

	void check_args_index(int index) const
	{
		if ((int)v_args_.size() <= index + 1)
		{
			throw std::runtime_error("not_valid_args_index");
		}
	}

private:
	std::vector<std::string> v_args_;
};

class LogicGM : public BattleSingleton, public CSingleton<LogicGM>
{
private:
	friend class CSingleton<LogicGM>;
	LogicGM();

	void SyncInfo(unsigned uid);

	bool HandleUserBase(const GMCmd& gm, DBCUserBaseWrap& user);
	bool HandleProps(const GMCmd& gm, unsigned uid);

public:
	virtual void CallDestroy() { Destroy();}

	//GM
	int Process(unsigned uid, ProtoGM::GMCmdReq* req);
private:
	ProtoGM::SyncInfo  msg_;
};


#endif /* LOGICGM_H_ */
