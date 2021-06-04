/*
 * DataOrder.h
 *
 *  Created on: 2018-2-28
 *      Author: Administrator
 */

#ifndef DATAORDER_H_
#define DATAORDER_H_

#include "Kernel.h"

struct DataOrder
{
	uint32_t uid;
	uint32_t id;
	uint32_t storage_id;
	uint32_t level_id;
	char order_id[ORDER_LENGTH];
	uint32_t coin;
	uint32_t exp;
	uint32_t end_ts;
	DataOrder():uid(0),id(0),storage_id(0),level_id(0),coin(0),exp(0),end_ts(0)
	{
		memset(order_id, 0, sizeof(order_id));
	}

	template<class T>
	void SetMessage(T * msg)
	{
		msg->set_slot(id);
		msg->set_coin(coin);
		msg->set_exp(exp);
		msg->set_end_ts(end_ts);
		msg->set_storageid(storage_id);
		msg->set_levelid(level_id);
		msg->set_orderid(order_id,ORDER_LENGTH);
	}

	void FromMessage(const ProtoUser::OrderCPP * msg)
	{
		coin = msg->coin();
		exp = msg->exp();
		end_ts = msg->end_ts();
		storage_id = msg->storageid();
		level_id = msg->levelid();
		memset(order_id, 0, sizeof(order_id));
		strncpy(order_id, msg->orderid().c_str(), sizeof(order_id)-1);
	}

	void reset_time()
	{
		end_ts = 0;
	}
};

class CDataOrder :public DBCBase<DataOrder, DB_ORDER>
{
public:
	virtual int Get(DataOrder &data);
	virtual int Get(vector<DataOrder> &data);
	virtual int Add(DataOrder &data);
	virtual int Set(DataOrder &data);
	virtual int Del(DataOrder &data);
};
#endif /* DATAORDER_H_ */
