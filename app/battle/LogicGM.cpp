/*
 * LogicGM.cpp
 *
 *  Created on: 2017-7-21
 *      Author: dawx62fac
 */

#include "LogicGM.h"


void GMCmd::split(std::string text, const char key)
{
	v_args_.clear();
	do
	{
		text.erase(0, text.find_first_not_of(key));
		if (text.empty())
		{
			break;
		}

		std::string::size_type p = text.find_first_of(key);
		v_args_.push_back(text.substr(0, p));
		text.erase(0, p);

	} while (1);

}


template<>
int GMCmd::get_arg<int>(int index) const
{
	check_args_index(index);

	int v = 0;
	if (Convert::StringToInt(v, v_args_[index + 1]))
	{
		return v;
	}
	else
	{
		throw std::runtime_error("the_args_not_int_type");
	}
}


template<>
std::string GMCmd::get_arg<std::string>(int index) const
{
	check_args_index(index);

	return v_args_[index + 1];
}

////////////////////////////////////////////////////////////////////////////////////
LogicGM::LogicGM()
{
}

bool LogicGM::HandleUserBase(const GMCmd& gm, DBCUserBaseWrap& user)
{
	CommonGiftConfig::BaseItem  baseitem;
	string name;

	if (gm.cmd() == string("pass"))
	{
		//user.Obj().npc_pass = gm.get_arg<int>(0);
	}
	else if (gm.cmd() == string("cash"))
	{
		baseitem.set_cash(gm.get_arg<int>(0));
	}
	else if (gm.cmd() == string("accharge"))
	{
		user.Obj().acccharge += gm.get_arg<int>(0);
		LogicUserManager::Instance()->NotifyRecharge(user.Obj().uid, gm.get_arg<int>(0));
		user.RefreshVIPLevel(false);
		user.Save();
	}
	else if (gm.cmd() == string("coin"))
	{
		baseitem.set_coin(gm.get_arg<int>(0));
	}
	else if (gm.cmd() == string("name"))
	{
		name = gm.get_arg<string>(0);
		memset(user.Obj().name, 0, sizeof(user.Obj().name));
		strncpy(user.Obj().name, name.c_str(), BASE_NAME_LEN-1);
		user.Save();
		OfflineResourceItem& rmi = GET_RMI(user.Obj().uid);
		memset(rmi.name, 0, sizeof(rmi.name));
		strncpy(rmi.name, name.c_str(), BASE_NAME_LEN-1);
	}
	else if (gm.cmd() == string("level"))
	{
		int new_level = gm.get_arg<int>(0);
		const UserCfg::User& userCfg = UserCfgWrap().User();
		//更新用户level数据
		int levelSize = userCfg.user_exp_size();
		if (new_level > 0 && new_level < levelSize )
		{
			unsigned old_level = user.Obj().level;
			user.Obj().exp =  userCfg.user_exp(new_level - 1);
			user.Obj().level = new_level;

			//user.OnUserUpgradeReward(old_level);
		}
		else
		{
			throw std::runtime_error("param error");
		}

		return true;
	}
	else if (gm.cmd() == string("exp"))
	{
		baseitem.set_exp(gm.get_arg<int>(0));
	}
	else if(gm.cmd() == string("mcard"))
	{
		//月卡处理
		const ConfigCard::MonthCardCPP &monthcard = ConfigManager::Instance()->card.m_config.monthcard();

		//1.添加钻石奖励
		unsigned cash = monthcard.first_reward().based().cash();
		baseitem.set_cash(cash);

		//2.添加累积充值
		user.Obj().acccharge = cash;
		user.RefreshVIPLevel(false);
		user.Save();

		//3.添加月卡处理
		LogicCardManager::Instance()->HandleCardPurchase(user.Obj().uid,MONTH_CARD_ID);
	}
	else if(gm.cmd() == string("lcard"))
	{
		//终生卡处理
		const ConfigCard::LifeCardCPP &liftcard = ConfigManager::Instance()->card.m_config.lifecard();

		//1.添加钻石奖励
		unsigned cash = liftcard.first_reward().based().cash();
		baseitem.set_cash(cash);

		//2.添加累积充值
		user.Obj().acccharge = cash;
		user.RefreshVIPLevel(false);
		user.Save();

		//3.添加终生卡处理
		LogicCardManager::Instance()->HandleCardPurchase(user.Obj().uid,LIFE_CARD_ID);
	}
	else
	{
		return false;
	}

	user.BaseProcess(baseitem, msg_.mutable_common()->mutable_userbase(), "GM");

	return true;
}

bool LogicGM::HandleProps(const GMCmd& gm, unsigned uid)
{
	if (gm.cmd() == string("equip"))
	{
		unsigned eq_id = gm.get_arg<int>(0);
		int count = gm.get_arg<int>(1);
		if(count > 0)
		{
			if (count >= 100000 )
				count = 100000;
			LogicPropsManager::Instance()->AddProps(uid, eq_id, count, "gm_op", msg_.mutable_common()->mutable_props());
		}
		else {
			unsigned ud = DataItemManager::Instance()->GetPropsUd(uid, eq_id);
			LogicPropsManager::Instance()->CostProps(uid,ud,eq_id,-count,"gm_op", msg_.mutable_common()->mutable_props());
		}


	}
	else
	{
		return false;
	}

	return true;
}

void LogicGM::SyncInfo(unsigned uid)
{
	try
	{
		LogicManager::Instance()->sendMsg(uid, &msg_, false);
	}
	catch(const std::exception& e)
	{
		error_log("uid:%u, %s", uid, e.what());
	}
}

int LogicGM::Process(unsigned uid, ProtoGM::GMCmdReq* req)
{
	msg_.Clear();

	Common::Login info;
	if(!UMI->GetUserInfo(uid, info) || info.platform() != 0)
	{
		throw std::runtime_error("gm_only_support_test_platform");
	}

	DBCUserBaseWrap user(uid);
	if(user.Obj().register_platform != 0)
	{
		throw std::runtime_error("gm_only_support_test_platform");
	}

	GMCmd gm(req->cmd());
	do
	{
		if (HandleUserBase(gm, user)) break;
		if (HandleProps(gm, uid)) break;

		throw std::runtime_error("unknown gm_cmd");

	} while(0);

	if (msg_.ByteSize() > 0)
	{
		SyncInfo(uid);
	}

	return 0;
}
