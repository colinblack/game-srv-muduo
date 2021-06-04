#ifndef DATA_ALLIANCE_DONATION_H_
#define DATA_ALLIANCE_DONATION_H_

#include "Kernel.h"

#define DataAllianceDonation_donate_person_LENG (40)
#define DataAllianceDonation_donate_person_name_LENG (320)

struct DataAllianceDonation
{
	uint32_t alliance_id;  //商会id
	uint32_t id;  //用户id
	uint32_t propsid;  //物品
	uint32_t applyts;  //发出捐收的时间
	uint32_t level;//等级
	uint8_t count;  //要求数量
	uint8_t donate_count;  //捐助数量
	uint8_t fetch_count;  //提取数量
	uint8_t status;  //捐收状态

	char donate_person[DataAllianceDonation_donate_person_LENG];  //捐助者信息.char转换成int

	char username[BASE_NAME_LEN];   //创建者名称
	char donate_person_name[DataAllianceDonation_donate_person_name_LENG];   //捐收者名称

	DataAllianceDonation()
	{
		alliance_id = 0;
		id = 0;
		propsid = 0;
		applyts = 0;
		level = 0;
		count = 0;
		donate_count = 0;
		fetch_count = 0;
		status = 0;
		memset(donate_person, 0, sizeof(donate_person));
		memset(username, 0, sizeof(username));
		memset(donate_person_name, 0, sizeof(donate_person_name));
	}

	void Reset()
	{
		propsid = 0;
		applyts = 0;
		level = 0;
		count = 0;
		donate_count = 0;
		fetch_count = 0;
		status = donation_processing;
		memset(donate_person, 0, sizeof(donate_person));
		memset(donate_person_name, 0, sizeof(donate_person_name));
	}

	void SetMessage(ProtoAlliance::AllianceDonationCPP * msg)
	{
		msg->set_applyuid(id);
		msg->set_propsid(propsid);
		msg->set_count(count);
		msg->set_donatecount(donate_count);
		msg->set_fetchcount(fetch_count);
		msg->set_status(status);
		msg->set_name(username);
		msg->set_applyts(applyts);
		msg->set_level(level);

		map<unsigned, pair<unsigned, string> > donatecnt;

		//捐赠者
		for(int i = 0; i < donate_count; ++i)
		{
			int donateuid = *(reinterpret_cast<int*>(donate_person + i * sizeof(int)) );

			//设置名称
			if (!donatecnt.count(donateuid))
			{
				string name(donate_person_name + i*BASE_NAME_LEN, BASE_NAME_LEN);
				name.append("\0");
				donatecnt[donateuid].second = name;
			}

			donatecnt[donateuid].first += 1;
		}

		for(map<unsigned, pair<unsigned, string> >::iterator uiter = donatecnt.begin(); uiter != donatecnt.end(); ++uiter)
		{
			msg->add_donateuid(uiter->first);
			msg->add_donatetimes(uiter->second.first);
			msg->add_donatename(uiter->second.second.c_str());
		}
	}

	void Donate(unsigned donate_uid, const char * name)
	{
		//增加捐赠记录
		//队列还有空余空间，插入.num可以直接当下标使用
		int * donateuid = reinterpret_cast<int*>(donate_person + donate_count*sizeof(int));
		*donateuid = donate_uid;

		//设置捐收者名称
		snprintf(donate_person_name + donate_count*BASE_NAME_LEN, BASE_NAME_LEN, name);

		donate_count += 1;

		if (donate_count >= count)
		{
			status = donation_finish;
		}
	}
};

class CDataAllianceDonation: public DBCBase<DataAllianceDonation, DB_ALLIANCE_DONATION>
{
public:
	virtual int Get(DataAllianceDonation &data)
	{
		DBCREQ_DECLARE(DBC::GetRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(propsid);
		DBCREQ_NEED(count);
		DBCREQ_NEED(donate_count);
		DBCREQ_NEED(fetch_count);
		DBCREQ_NEED(status);
		DBCREQ_NEED(donate_person);
		DBCREQ_NEED(applyts);
		DBCREQ_NEED(level);

		DBCREQ_EXEC;

		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, alliance_id);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_INT(data, propsid);
		DBCREQ_GET_INT(data, count);
		DBCREQ_GET_INT(data, donate_count);
		DBCREQ_GET_INT(data, fetch_count);
		DBCREQ_GET_INT(data, status);
		DBCREQ_GET_CHAR(data, donate_person, DataAllianceDonation_donate_person_LENG);
		DBCREQ_GET_INT(data, applyts);
		DBCREQ_GET_INT(data, level);

		return 0;
	}

	virtual int Get(vector<DataAllianceDonation> &data)
	{
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].alliance_id);
		DBCREQ_SET_KEY(data[0].alliance_id);

		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(alliance_id);
		DBCREQ_NEED(id);
		DBCREQ_NEED(propsid);
		DBCREQ_NEED(count);
		DBCREQ_NEED(donate_count);
		DBCREQ_NEED(fetch_count);
		DBCREQ_NEED(status);
		DBCREQ_NEED(donate_person);
		DBCREQ_NEED(applyts);
		DBCREQ_NEED(level);

		DBCREQ_EXEC;

		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, alliance_id);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_INT(data, propsid);
		DBCREQ_ARRAY_GET_INT(data, count);
		DBCREQ_ARRAY_GET_INT(data, donate_count);
		DBCREQ_ARRAY_GET_INT(data, fetch_count);
		DBCREQ_ARRAY_GET_INT(data, status);
		DBCREQ_ARRAY_GET_CHAR(data, donate_person,	DataAllianceDonation_donate_person_LENG);
		DBCREQ_ARRAY_GET_INT(data, applyts);
		DBCREQ_ARRAY_GET_INT(data, level);

		DBCREQ_ARRAY_GET_END();

		return 0;
	}

	virtual int Add(DataAllianceDonation &data)
	{
		DBCREQ_DECLARE(DBC::InsertRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, propsid);
		DBCREQ_SET_INT(data, count);
		DBCREQ_SET_INT(data, donate_count);
		DBCREQ_SET_INT(data, fetch_count);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_CHAR(data, donate_person, DataAllianceDonation_donate_person_LENG);
		DBCREQ_SET_INT(data, applyts);
		DBCREQ_SET_INT(data, level);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Set(DataAllianceDonation &data)
	{
		DBCREQ_DECLARE(DBC::UpdateRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_INT(data, propsid);
		DBCREQ_SET_INT(data, count);
		DBCREQ_SET_INT(data, donate_count);
		DBCREQ_SET_INT(data, fetch_count);
		DBCREQ_SET_INT(data, status);
		DBCREQ_SET_CHAR(data, donate_person, DataAllianceDonation_donate_person_LENG);
		DBCREQ_SET_INT(data, applyts);
		DBCREQ_SET_INT(data, level);

		DBCREQ_EXEC;

		return 0;
	}

	virtual int Del(DataAllianceDonation &data)
	{
		DBCREQ_DECLARE(DBC::DeleteRequest, data.alliance_id);
		DBCREQ_SET_KEY(data.alliance_id);
		DBCREQ_SET_CONDITION(EQ, id, data.id);

		DBCREQ_EXEC;

		return 0;
	}
};

#endif //DATA_ALLIANCE_DONATION_H_
