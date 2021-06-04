
#include "DataBase.h"
#include "DataInc.h"

void DataBase::AddExp(int exp_)
{
	if (exp_ < 0)
	{
		error_log("params error. uid=%u,exp=%d", uid, exp_);
		return ;
	}

	if (exp_ == 0) return ;

	exp += exp_;

	const UserCfg::User& userCfg = UserCfgWrap().User();


	//更新用户level数据
	int levelSize = userCfg.user_exp_size();

	if (exp >=  userCfg.user_exp(levelSize - 1))
	{
		exp =  userCfg.user_exp(levelSize - 1);
		level = levelSize;
	}
	else
	{
		for (int i = level; i < levelSize; i++)
		{
			if (exp < (uint64_t)userCfg.user_exp(i))
			{
				level = i;
				break;
			}
		}
	}
}

int CDataBase::Get(DataBase &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(register_platform);
	DBCREQ_NEED(register_time);
	DBCREQ_NEED(last_login_platform);
	DBCREQ_NEED(last_login_time);
	DBCREQ_NEED(login_times);
	DBCREQ_NEED(login_days);
	DBCREQ_NEED(last_active_time);
	DBCREQ_NEED(last_off_time);
	DBCREQ_NEED(forbid_ts);
	DBCREQ_NEED(forbid_reason);
	DBCREQ_NEED(tutorial_stage);
	DBCREQ_NEED(name);
	DBCREQ_NEED(fig);
	DBCREQ_NEED(exp);
	DBCREQ_NEED(level);
	DBCREQ_NEED(acccharge);
	DBCREQ_NEED(viplevel);
	DBCREQ_NEED(first_recharge);
	DBCREQ_NEED(alliance_id);
	DBCREQ_NEED(cash);
	DBCREQ_NEED(coin);
	DBCREQ_NEED(barrier);
	DBCREQ_NEED(next_create_ad_ts);
	DBCREQ_NEED(vip_reward_daily_gift_ts);
	DBCREQ_NEED(vip_daily_speed_product_cnt);
	DBCREQ_NEED(vip_daily_remove_ordercd_cnt);
	DBCREQ_NEED(allian_allow_ts);
	DBCREQ_NEED(next_donation_ts);
	DBCREQ_NEED(helptimes);
	DBCREQ_NEED(npc_shop_update_ts);
	DBCREQ_NEED(switch_status);
	DBCREQ_NEED(share_reward_daily_gift_ts);
	DBCREQ_NEED(vip_daily_gift_refresh_ts);
	DBCREQ_NEED(assist_start_ts);
	DBCREQ_NEED(assist_end_ts);
	DBCREQ_NEED(expand_map_1);
	DBCREQ_NEED(expand_map_2);
	DBCREQ_NEED(expand_map_3);
	DBCREQ_NEED(npc_customer1_propsid);
	DBCREQ_NEED(npc_customer1_propscnt);
	DBCREQ_NEED(npc_customer2_propsid);
	DBCREQ_NEED(npc_customer2_propscnt);
	DBCREQ_NEED(next_random_box_refresh_ts);
	DBCREQ_NEED(flag);
	DBCREQ_NEED(npc_customer1_next_visit_ts);
	DBCREQ_NEED(npc_customer2_next_visit_ts);
	DBCREQ_NEED(friendly_value);
	DBCREQ_NEED(blue_info);
	DBCREQ_NEED(version);
	DBCREQ_NEED(prosperity);
	DBCREQ_NEED(accthumbsup);
	DBCREQ_NEED(inviteuid);
	DBCREQ_NEED(isUnlockPetResidence);
	DBCREQ_NEED(pxgksChannel);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, register_platform);
	DBCREQ_GET_INT(data, register_time);
	DBCREQ_GET_INT(data, last_login_platform);
	DBCREQ_GET_INT(data, last_login_time);
	DBCREQ_GET_INT(data, login_times);
	DBCREQ_GET_INT(data, login_days);
	DBCREQ_GET_INT(data, last_active_time);
	DBCREQ_GET_INT(data, last_off_time);
	DBCREQ_GET_INT(data, forbid_ts);
	DBCREQ_GET_CHAR(data, forbid_reason, BASE_FORBID_REASON_LEN);
	DBCREQ_GET_INT(data, tutorial_stage);
	DBCREQ_GET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_GET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_GET_INT(data, exp);
	DBCREQ_GET_INT(data, level);
	DBCREQ_GET_INT(data, acccharge);
	DBCREQ_GET_INT(data, viplevel);
	DBCREQ_GET_INT(data, first_recharge);
	DBCREQ_GET_INT(data, alliance_id);
	DBCREQ_GET_INT(data, cash);
	DBCREQ_GET_INT(data, coin);
	DBCREQ_GET_BINARY(data, barrier, BARRIER_LENGTH);
	DBCREQ_GET_INT(data, next_create_ad_ts);
	DBCREQ_GET_INT(data, vip_reward_daily_gift_ts);
	DBCREQ_GET_INT(data, vip_daily_speed_product_cnt);
	DBCREQ_GET_INT(data, vip_daily_remove_ordercd_cnt);
	DBCREQ_GET_INT(data, allian_allow_ts);
	DBCREQ_GET_INT(data, next_donation_ts);
	DBCREQ_GET_INT(data, helptimes);
	DBCREQ_GET_INT(data, npc_shop_update_ts);
	DBCREQ_GET_INT(data, switch_status);
	DBCREQ_GET_INT(data, share_reward_daily_gift_ts);
	DBCREQ_GET_INT(data, vip_daily_gift_refresh_ts);
	DBCREQ_GET_INT(data, assist_start_ts);
	DBCREQ_GET_INT(data, assist_end_ts);
	DBCREQ_GET_BINARY(data, expand_map_1, EXPAND_MAPGRID_LENGTH);
	DBCREQ_GET_BINARY(data, expand_map_2, EXPAND_MAPGRID_LENGTH);
	DBCREQ_GET_BINARY(data, expand_map_3, EXPAND_MAPGRID_LENGTH);
	DBCREQ_GET_INT(data, npc_customer1_propsid);
	DBCREQ_GET_INT(data, npc_customer1_propscnt);
	DBCREQ_GET_INT(data, npc_customer2_propsid);
	DBCREQ_GET_INT(data, npc_customer2_propscnt);
	DBCREQ_GET_INT(data, next_random_box_refresh_ts);
	DBCREQ_GET_INT(data, flag);
	DBCREQ_GET_INT(data, npc_customer1_next_visit_ts);
	DBCREQ_GET_INT(data, npc_customer2_next_visit_ts);
	DBCREQ_GET_INT(data, friendly_value);
	DBCREQ_GET_INT(data, blue_info);
	DBCREQ_GET_INT(data, version);
	DBCREQ_GET_INT(data, prosperity);
	DBCREQ_GET_INT(data, accthumbsup);
	DBCREQ_GET_INT(data, inviteuid);
	DBCREQ_GET_INT(data, isUnlockPetResidence);
	DBCREQ_GET_BINARY(data, pxgksChannel, 64);


	return 0;
}

int CDataBase::Add(DataBase &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_INT(data, register_platform);
	DBCREQ_SET_INT(data, register_time);
	DBCREQ_SET_INT(data, last_login_platform);
	DBCREQ_SET_INT(data, last_login_time);
	DBCREQ_SET_INT(data, login_times);
	DBCREQ_SET_INT(data, login_days);
	DBCREQ_SET_INT(data, last_active_time);
	DBCREQ_SET_INT(data, last_off_time);
	DBCREQ_SET_INT(data, forbid_ts);
	DBCREQ_SET_CHAR(data, forbid_reason, BASE_FORBID_REASON_LEN);
	DBCREQ_SET_INT(data, tutorial_stage);
	DBCREQ_SET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_SET_INT(data, exp);
	DBCREQ_SET_INT(data, level);
	DBCREQ_SET_INT(data, acccharge);
	DBCREQ_SET_INT(data, viplevel);
	DBCREQ_SET_INT(data, cash);
	DBCREQ_SET_INT(data, coin);
	DBCREQ_SET_INT(data, first_recharge);
	DBCREQ_SET_INT(data, alliance_id);
	DBCREQ_SET_BINARY(data, barrier, BARRIER_LENGTH);
	DBCREQ_SET_INT(data, next_create_ad_ts);
	DBCREQ_SET_INT(data, vip_reward_daily_gift_ts);
	DBCREQ_SET_INT(data, vip_daily_speed_product_cnt);
	DBCREQ_SET_INT(data, vip_daily_remove_ordercd_cnt);
	DBCREQ_SET_INT(data, allian_allow_ts);
	DBCREQ_SET_INT(data, next_donation_ts);
	DBCREQ_SET_INT(data, helptimes);
	DBCREQ_SET_INT(data, npc_shop_update_ts);
	DBCREQ_SET_INT(data, switch_status);
	DBCREQ_SET_INT(data, share_reward_daily_gift_ts);
	DBCREQ_SET_INT(data, vip_daily_gift_refresh_ts);
	DBCREQ_SET_INT(data, assist_start_ts);
	DBCREQ_SET_INT(data, assist_end_ts);
	DBCREQ_SET_BINARY(data, expand_map_1, EXPAND_MAPGRID_LENGTH);
	DBCREQ_SET_BINARY(data, expand_map_2, EXPAND_MAPGRID_LENGTH);
	DBCREQ_SET_BINARY(data, expand_map_3, EXPAND_MAPGRID_LENGTH);
	DBCREQ_SET_INT(data, npc_customer1_propsid);
	DBCREQ_SET_INT(data, npc_customer1_propscnt);
	DBCREQ_SET_INT(data, npc_customer2_propsid);
	DBCREQ_SET_INT(data, npc_customer2_propscnt);
	DBCREQ_SET_INT(data, next_random_box_refresh_ts);
	DBCREQ_SET_INT(data, flag);
	DBCREQ_SET_INT(data, npc_customer1_next_visit_ts);
	DBCREQ_SET_INT(data, npc_customer2_next_visit_ts);
	DBCREQ_SET_INT(data, friendly_value);
	DBCREQ_SET_INT(data, blue_info);
	DBCREQ_SET_INT(data, version);
	DBCREQ_SET_INT(data, prosperity);
	DBCREQ_SET_INT(data, accthumbsup);
	DBCREQ_SET_INT(data, inviteuid);
	DBCREQ_SET_INT(data, isUnlockPetResidence);
	DBCREQ_SET_BINARY(data, pxgksChannel, 64);
	DBCREQ_EXEC;
	return 0;
}

int CDataBase::Set(DataBase &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_INT(data, register_platform);
	DBCREQ_SET_INT(data, register_time);
	DBCREQ_SET_INT(data, last_login_platform);
	DBCREQ_SET_INT(data, last_login_time);
	DBCREQ_SET_INT(data, login_times);
	DBCREQ_SET_INT(data, login_days);
	DBCREQ_SET_INT(data, last_active_time);
	DBCREQ_SET_INT(data, last_off_time);
	DBCREQ_SET_INT(data, forbid_ts);
	DBCREQ_SET_CHAR(data, forbid_reason, BASE_FORBID_REASON_LEN);
	DBCREQ_SET_INT(data, tutorial_stage);
	DBCREQ_SET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_SET_INT(data, exp);
	DBCREQ_SET_INT(data, level);
	DBCREQ_SET_INT(data, acccharge);
	DBCREQ_SET_INT(data, viplevel);
	DBCREQ_SET_INT(data, cash);
	DBCREQ_SET_INT(data, coin);
	DBCREQ_SET_INT(data, first_recharge);
	DBCREQ_SET_INT(data, alliance_id);
	DBCREQ_SET_BINARY(data, barrier, BARRIER_LENGTH);
	DBCREQ_SET_INT(data, next_create_ad_ts);
	DBCREQ_SET_INT(data, vip_reward_daily_gift_ts);
	DBCREQ_SET_INT(data, vip_daily_speed_product_cnt);
	DBCREQ_SET_INT(data, vip_daily_remove_ordercd_cnt);
	DBCREQ_SET_INT(data, allian_allow_ts);
	DBCREQ_SET_INT(data, next_donation_ts);
	DBCREQ_SET_INT(data, helptimes);
	DBCREQ_SET_INT(data, npc_shop_update_ts);
	DBCREQ_SET_INT(data, switch_status);
	DBCREQ_SET_INT(data, share_reward_daily_gift_ts);
	DBCREQ_SET_INT(data, vip_daily_gift_refresh_ts);
	DBCREQ_SET_INT(data, assist_start_ts);
	DBCREQ_SET_INT(data, assist_end_ts);
	DBCREQ_SET_BINARY(data, expand_map_1, EXPAND_MAPGRID_LENGTH);
	DBCREQ_SET_BINARY(data, expand_map_2, EXPAND_MAPGRID_LENGTH);
	DBCREQ_SET_BINARY(data, expand_map_3, EXPAND_MAPGRID_LENGTH);
	DBCREQ_SET_INT(data, npc_customer1_propsid);
	DBCREQ_SET_INT(data, npc_customer1_propscnt);
	DBCREQ_SET_INT(data, npc_customer2_propsid);
	DBCREQ_SET_INT(data, npc_customer2_propscnt);
	DBCREQ_SET_INT(data, next_random_box_refresh_ts);
	DBCREQ_SET_INT(data, flag);
	DBCREQ_SET_INT(data, npc_customer1_next_visit_ts);
	DBCREQ_SET_INT(data, npc_customer2_next_visit_ts);
	DBCREQ_SET_INT(data, friendly_value);
	DBCREQ_SET_INT(data, blue_info);
	DBCREQ_SET_INT(data, version);
	DBCREQ_SET_INT(data, prosperity);
	DBCREQ_SET_INT(data, accthumbsup);
	DBCREQ_SET_INT(data, inviteuid);
	DBCREQ_SET_INT(data, isUnlockPetResidence);
	DBCREQ_SET_BINARY(data, pxgksChannel, 64);

	DBCREQ_EXEC;
	return 0;
}

int CDataBase::Del(DataBase &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_EXEC;

	return 0;
}


