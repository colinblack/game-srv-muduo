/*
 * DataTruck.h
 *
 *  Created on: 2018-3-5
 *      Author: Administrator
 */

#ifndef DATATRUCK_H_
#define DATATRUCK_H_
#include "Kernel.h"

struct DataTruck
{
	uint32_t uid;
	uint32_t slot;
	uint32_t storage_id;
	uint32_t level_id;
	uint32_t order_id;
	uint32_t state;
	uint32_t endts;
	uint32_t coin;
	uint32_t exp;
	DataTruck():uid(0),slot(0),storage_id(0),level_id(0),order_id(0),state(0),endts(0),coin(0),exp(0)
	{

	}

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_state(state);
		msg->set_end_ts(endts);
		msg->set_coin(coin);
		msg->set_exp(exp);
	}

	void FromMessage(const ProtoUser::TruckCPP * msg)
	{
		state = msg->state();
		endts = msg->end_ts();
		coin = msg->coin();
		exp = msg->exp();
	}

	void reset_time()
	{
		endts = 0;
	}
};

class CDataTruck :public DBCBase<DataTruck, DB_TRUCK>
{
public:
	virtual int Get(DataTruck &data);
	virtual int Add(DataTruck &data);
	virtual int Set(DataTruck &data);
	virtual int Del(DataTruck &data);
};
#endif /* DATATRUCK_H_ */
