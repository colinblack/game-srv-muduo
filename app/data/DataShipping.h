#include "Kernel.h"

struct DataShipping
{
	uint32_t uid;
	uint32_t badge;
	uint32_t unlock_endts;
	uint32_t arrive_ts;
	uint8_t public_aid_times;
	uint8_t commercial_aid_times;
	uint8_t status;
	uint8_t play_status;
	uint8_t view_ad_pack_cnt;

	DataShipping()
	{
		uid = 0;
		badge = 0;
		unlock_endts = 0;
		arrive_ts = 0;
		public_aid_times = 0;
		commercial_aid_times = 0;
		status = 0;
		play_status = 0;
		view_ad_pack_cnt = 0;
	}

	void Reset()
	{
		public_aid_times = 0;
		commercial_aid_times = 0;
		play_status = 0;
		view_ad_pack_cnt = 0;
	}

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_badge(badge);
		msg->set_unlockendts(unlock_endts);
		msg->set_arrivets(arrive_ts);
		msg->set_publicaidtimes(public_aid_times);
		msg->set_commercialaidtimes(commercial_aid_times);
		msg->set_status(status);
		msg->set_playstatus(play_status);
		msg->set_viewadpackcnt(view_ad_pack_cnt);

	}

	void FromMessage(const ProtoUser::ShippingCPP * msg)
	{
		badge = msg->badge();
		unlock_endts = msg->unlockendts();
		arrive_ts = msg->arrivets();
		public_aid_times = msg->publicaidtimes();
		commercial_aid_times = msg->commercialaidtimes();
		status = msg->status();
		play_status = msg->playstatus();
		view_ad_pack_cnt = msg->viewadpackcnt();
	}
};

class CDataShipping: public DBCBase<DataShipping, DB_SHIPPING>
{
public:
	virtual int Get(DataShipping &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(badge);
		DBCREQ_NEED(unlock_endts);
		DBCREQ_NEED(arrive_ts);
		DBCREQ_NEED(public_aid_times);
		DBCREQ_NEED(commercial_aid_times);
		DBCREQ_NEED(status);
		DBCREQ_NEED(play_status);
		DBCREQ_NEED(view_ad_pack_cnt);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, badge);
		DBCREQ_GET_INT(data, unlock_endts);
		DBCREQ_GET_INT(data, arrive_ts);
		DBCREQ_GET_INT(data, public_aid_times);
		DBCREQ_GET_INT(data, commercial_aid_times);
		DBCREQ_GET_INT(data, status);
		DBCREQ_GET_INT(data, play_status);
		DBCREQ_GET_INT(data, view_ad_pack_cnt);

		return 0;
	}

	virtual int Add(DataShipping &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, badge);
		DBCREQ_SET_INT(data, unlock_endts);
		DBCREQ_SET_INT(data, arrive_ts);
		DBCREQ_SET_INT(data, public_aid_times);
		DBCREQ_SET_INT(data, commercial_aid_times);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, play_status);
		DBCREQ_SET_INT(data, view_ad_pack_cnt);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataShipping &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_SET_INT(data, badge);
		DBCREQ_SET_INT(data, unlock_endts);
		DBCREQ_SET_INT(data, arrive_ts);
		DBCREQ_SET_INT(data, public_aid_times);
		DBCREQ_SET_INT(data, commercial_aid_times);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_INT(data, play_status);
		DBCREQ_SET_INT(data, view_ad_pack_cnt);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataShipping &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);

		DBCREQ_EXEC;

		return 0;
	}
};
