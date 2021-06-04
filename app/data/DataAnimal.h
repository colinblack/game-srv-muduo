#include "Kernel.h"

struct DataAnimal
{
	uint32_t uid;
	uint32_t id;
	uint32_t full_time;

	uint16_t residence_ud;
	uint16_t animal_id;
	uint8_t status;
	int8_t produce_type;	// 0:手动生产,1:助手生产

	DataAnimal()
	{
		uid = 0;
		id = 0;
		full_time = 0;

		residence_ud = 0;
		animal_id = 0;
		status = status_hungry;
		produce_type = PRODUCE_TYPE_MAN;
	}

	template<class T>
	void SetMessage(T *msg)
	{
		msg->set_ud(id);
		msg->set_residenceud(residence_ud);
		msg->set_animalid(animal_id);
		msg->set_fulltime(full_time);
		msg->set_status(status);
//		msg->set_keeper(produce_type);
	}

	void FromMessage(const ProtoUser::AnimalCPP * msg)
	{
		residence_ud = msg->residenceud();
		animal_id = msg->animalid();
		full_time = msg->fulltime();
		status = msg->status();
	}

	void Full()
	{
		//动物饱腹
		status = status_full;
		full_time = 0;
	}

	void Obtain()
	{
		status = status_hungry;
	}
};

class CDataAnimal: public DBCBase<DataAnimal, DB_ANIMAL>
{
public:
	virtual int Get(DataAnimal &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(residence_ud);
		DBCREQ_NEED(animal_id);
		DBCREQ_NEED(status);
		DBCREQ_NEED(full_time);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, residence_ud);
		DBCREQ_GET_INT(data, animal_id);
		DBCREQ_GET_INT(data, status);
		DBCREQ_GET_INT(data, full_time);
		return 0;
	}

	virtual int Get(vector<DataAnimal> &data)
	{
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(residence_ud);
		DBCREQ_NEED(animal_id);
		DBCREQ_NEED(status);
		DBCREQ_NEED(full_time);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, residence_ud);
		DBCREQ_ARRAY_GET_INT(data, animal_id);
		DBCREQ_ARRAY_GET_INT(data, status);
		DBCREQ_ARRAY_GET_INT(data, full_time);
		DBCREQ_ARRAY_GET_END();
		return 0;
	}

	virtual int Add(DataAnimal &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);

		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, residence_ud);
		DBCREQ_SET_INT(data, animal_id);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, full_time);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAnimal &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, residence_ud);
		DBCREQ_SET_INT(data, animal_id);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, full_time);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAnimal &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
