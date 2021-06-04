#include "DataShop.h"

//////////////////////////////////////////////////////////////////
int CDataShop::Get(DataShop &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(buyer_uid);
	DBCREQ_NEED(props_id);
	DBCREQ_NEED(props_cnt);
	DBCREQ_NEED(props_price);
	DBCREQ_NEED(ad_flag);
	DBCREQ_NEED(sell_flag);
	DBCREQ_NEED(vip_shelf_flag);
	DBCREQ_NEED(name);
	DBCREQ_NEED(fig);

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	DBCREQ_GET_BEGIN();
	DBCREQ_GET_INT(data, uid);
	DBCREQ_GET_INT(data, id);
	DBCREQ_GET_INT(data, buyer_uid);
	DBCREQ_GET_INT(data, props_id);
	DBCREQ_GET_INT(data, props_cnt);
	DBCREQ_GET_INT(data, props_price);
	DBCREQ_GET_INT(data, ad_flag);
	DBCREQ_GET_INT(data, sell_flag);
	DBCREQ_GET_INT(data, vip_shelf_flag);
	DBCREQ_GET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_GET_CHAR(data, fig, BASE_FIG_LEN);

	return 0;
}

int CDataShop::Get(vector<DataShop> &data)
{
	if (0 == data.size())
	{
		return R_ERR_PARAM;
	}

	DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
	DBCREQ_SET_KEY(data[0].uid);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(uid);
	DBCREQ_NEED(id);
	DBCREQ_NEED(buyer_uid);
	DBCREQ_NEED(props_id);
	DBCREQ_NEED(props_cnt);
	DBCREQ_NEED(props_price);
	DBCREQ_NEED(ad_flag);
	DBCREQ_NEED(sell_flag);
	DBCREQ_NEED(vip_shelf_flag);
	DBCREQ_NEED(name);
	DBCREQ_NEED(fig);

	data.clear();

	DBCREQ_EXEC;

	DBCREQ_ARRAY_GET_BEGIN(data);
	DBCREQ_ARRAY_GET_INT(data, uid);
	DBCREQ_ARRAY_GET_INT(data, id);
	DBCREQ_ARRAY_GET_INT(data, buyer_uid);
	DBCREQ_ARRAY_GET_INT(data, props_id);
	DBCREQ_ARRAY_GET_INT(data, props_cnt);
	DBCREQ_ARRAY_GET_INT(data, props_price);
	DBCREQ_ARRAY_GET_INT(data, ad_flag);
	DBCREQ_ARRAY_GET_INT(data, sell_flag);
	DBCREQ_ARRAY_GET_INT(data, vip_shelf_flag);
	DBCREQ_ARRAY_GET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_ARRAY_GET_CHAR(data, fig, BASE_FIG_LEN);

	DBCREQ_ARRAY_GET_END();

	return 0;
}

int CDataShop::Add(DataShop &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);

	DBCREQ_SET_INT(data, id);
	DBCREQ_SET_INT(data, buyer_uid);
	DBCREQ_SET_INT(data, props_id);
	DBCREQ_SET_INT(data, props_cnt);
	DBCREQ_SET_INT(data, props_price);
	DBCREQ_SET_INT(data, ad_flag);
	DBCREQ_SET_INT(data, sell_flag);
	DBCREQ_SET_INT(data, vip_shelf_flag);
	DBCREQ_SET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_EXEC;

	return 0;
}

int CDataShop::Set(DataShop &data)
{
	DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_SET_INT(data, buyer_uid);
	DBCREQ_SET_INT(data, props_id);
	DBCREQ_SET_INT(data, props_cnt);
	DBCREQ_SET_INT(data, props_price);
	DBCREQ_SET_INT(data, ad_flag);
	DBCREQ_SET_INT(data, sell_flag);
	DBCREQ_SET_INT(data, vip_shelf_flag);
	DBCREQ_SET_CHAR(data, name, BASE_NAME_LEN);
	DBCREQ_SET_CHAR(data, fig, BASE_FIG_LEN);
	DBCREQ_EXEC;

	return 0;
}

int CDataShop::Del(DataShop &data)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
	DBCREQ_SET_KEY(data.uid);
	DBCREQ_SET_CONDITION(EQ, id, data.id);

	DBCREQ_EXEC;

	return 0;
}
