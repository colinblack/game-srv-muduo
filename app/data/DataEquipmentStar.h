#include "Kernel.h"

struct DataEquipmentStar
{
	uint32_t uid;
	uint32_t id;
	uint32_t usedtime;
	uint16_t star;

	DataEquipmentStar()
	{
		uid = 0;
		id = 0;
		star = 0;
		usedtime = 0;
	}

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_id(id);
		msg->set_star(star);
		msg->set_usedtime(usedtime);
	}

	void FromMessage(const ProtoUser::EquipmentStarCPP * msg)
	{
		star = msg->star();
		usedtime = msg->usedtime();
	}
};

class CDataEquipmentStar: public DBCBase<DataEquipmentStar, DB_EQUIPMENT_STAR>
{
public:
	virtual int Get(DataEquipmentStar &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(star);
		DBCREQ_NEED(usedtime);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, star);
		DBCREQ_GET_INT(data, usedtime);

		return 0;
	}

	virtual int Get(vector<DataEquipmentStar> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(star);
		DBCREQ_NEED(usedtime);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, star);
		DBCREQ_ARRAY_GET_INT(data, usedtime);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataEquipmentStar &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, star);
		DBCREQ_SET_INT(data, usedtime);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataEquipmentStar &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, star);
		DBCREQ_SET_INT(data, usedtime);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataEquipmentStar &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
