#include "Kernel.h"

struct DataShippingbox
{
	uint32_t uid;
	uint32_t id;
	uint32_t propsid;
	uint32_t count;
	uint32_t aid_uid;
	uint32_t coin;
	uint32_t exp;

	int8_t status;
	int8_t aid_status;

	DataShippingbox()
	{
		uid = 0;
		id = 0;
		propsid = 0;
		count = 0;
		aid_status = 0;
		aid_uid = 0;
		status = 0;
		coin = 0;
		exp = 0;
	}

	void Reset()
	{
		propsid = 0;
		count = 0;
		aid_status = 0;
		aid_uid = 0;
		status = 0;
		coin = 0;
		exp = 0;
	}

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_boxid(id);
		msg->set_propsid(propsid);
		msg->set_count(count);
		msg->set_aidstatus(aid_status);
		msg->set_status(status);
		msg->set_coin(coin);
		msg->set_exp(exp);
	}

	void FromMessage(const ProtoUser::ShippingBoxCPP * msg)
	{
		propsid = msg->propsid();
		count = msg->count();
		aid_status = msg->aidstatus();
		status = msg->status();
		coin = msg->coin();
		exp = msg->exp();
	}
};

class CDataShippingbox: public DBCBase<DataShippingbox, DB_SHIPPINGBOX>
{
public:
	virtual int Get(DataShippingbox &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(propsid);
		DBCREQ_NEED(count);
		DBCREQ_NEED(aid_status);
		DBCREQ_NEED(aid_uid);
		DBCREQ_NEED(status);
		DBCREQ_NEED(coin);
		DBCREQ_NEED(exp);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, propsid);
		DBCREQ_GET_INT(data, count);
		DBCREQ_GET_INT(data, aid_status);
		DBCREQ_GET_INT(data, aid_uid);
		DBCREQ_GET_INT(data, status);
		DBCREQ_GET_INT(data, coin);
		DBCREQ_GET_INT(data, exp);

		return 0;
	}

	virtual int Get(vector<DataShippingbox> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);

		data.clear();

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(propsid);
		DBCREQ_NEED(count);
		DBCREQ_NEED(aid_status);
		DBCREQ_NEED(aid_uid);
		DBCREQ_NEED(status);
		DBCREQ_NEED(coin);
		DBCREQ_NEED(exp);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, propsid);
		DBCREQ_ARRAY_GET_INT(data, count);
		DBCREQ_ARRAY_GET_INT(data, aid_status);
		DBCREQ_ARRAY_GET_INT(data, aid_uid);
		DBCREQ_ARRAY_GET_INT(data, status);
		DBCREQ_ARRAY_GET_INT(data, coin);
		DBCREQ_ARRAY_GET_INT(data, exp);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataShippingbox &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, propsid);
		DBCREQ_SET_INT(data, count);
		DBCREQ_SET_INT(data, aid_status);
		DBCREQ_SET_INT(data, aid_uid);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, coin);
		DBCREQ_SET_INT(data, exp);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataShippingbox &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, propsid);
		DBCREQ_SET_INT(data, count);
		DBCREQ_SET_INT(data, aid_status);
		DBCREQ_SET_INT(data, aid_uid);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, coin);
		DBCREQ_SET_INT(data, exp);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataShippingbox &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
