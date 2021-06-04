#include "Kernel.h"

struct DataCropland {
	uint32_t uid;
	uint32_t plant;
	uint32_t harvest_time;

	uint16_t id;
	uint8_t  status;

	DataCropland()
	{
		uid = 0;
		id = 0;
		plant = 0;
		harvest_time = 0;
		status = status_free;
	}

	template<class T>
	void SetMessage(T *msg)
	{
		msg->set_ud(id);
		msg->set_status(status);
		msg->set_plant(plant);
		msg->set_harvesttime(harvest_time);
	}

	void FromMessage(const ProtoUser::CropLandCPP * msg)
	{
		status = msg->status();
		plant = msg->plant();
		harvest_time = msg->harvesttime();
	}

	//收获
	void Harvest()
	{
		status = status_free;
		plant = 0;
	}

	//成熟
	void Mature()
	{
		status = status_harvest;
		harvest_time = 0;
	}
};

class CDataCropland: public DBCBase<DataCropland, DB_CROPLAND>
{
public:
	virtual int Get(DataCropland &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(plant);
		DBCREQ_NEED(status);
		DBCREQ_NEED(harvest_time);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, plant);
		DBCREQ_GET_INT(data, status);
		DBCREQ_GET_INT(data, harvest_time);

		return 0;
	}

	virtual int Get(vector<DataCropland> &data)
	{
		if (data.empty())
			return R_ERROR;

		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(plant);
		DBCREQ_NEED(status);
		DBCREQ_NEED(harvest_time);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, plant);
		DBCREQ_ARRAY_GET_INT(data, status);
		DBCREQ_ARRAY_GET_INT(data, harvest_time);
		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataCropland &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, plant);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, harvest_time);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataCropland &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, plant);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, harvest_time);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataCropland &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};
