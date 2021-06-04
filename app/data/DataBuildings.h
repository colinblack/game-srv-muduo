#ifndef DATA_BUILDING_H_
#define DATA_BUILDING_H_
#include "Kernel.h"

struct DataBuildings
{
	uint32_t uid;
	uint32_t done_time;  //结束时间
	uint16_t id;  //建筑ud
	uint16_t build_id;	//建筑id
	uint16_t position;	//建筑位置.格子数
	uint8_t direct; 	//朝向.0-右 1-下
	uint8_t level;  //建筑等级

	DataBuildings():
		uid(0),
		done_time(0),
		id(0),
		build_id(0),
		position(0),
		direct(0),
		level(0)
	{

	}

	DataBuildings(unsigned uid_, unsigned ud_):
		uid(uid_),
		done_time(0),
		id(ud_),
		build_id(0),
		position(0),
		direct(0),
		level(0)
	{

	}

	void ResetTime()
	{
		done_time = 0;
	}

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_ud(id);
		msg->set_buildid(build_id);

		unsigned xpos = position/1000;
		unsigned ypos = position - xpos*1000;

		msg->add_position(xpos);
		msg->add_position(ypos);
		msg->set_direct(direct);
		msg->set_donetime(done_time);
		msg->set_level(level);
	}

	void FromMessage(const ProtoUser::BuildingsCPP * msg)
	{
		build_id = msg->buildid();

		unsigned xpos = msg->position(0u);
		unsigned ypos = msg->position(1u);

		position = xpos * 1000 + ypos;

		direct = msg->direct();
		done_time = msg->donetime();
		level = msg->level();
	}
};

class CDataBuildings :public DBCBase<DataBuildings, DB_BUILD>
{
public:
	virtual int Get(DataBuildings &data);
	virtual int Get(vector<DataBuildings> &data);
	virtual int Add(DataBuildings &data);
	virtual int Set(DataBuildings &data);
	virtual int Del(DataBuildings &data);
};

#endif /* DATA_BUILDING_H_ */
