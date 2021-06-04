#ifndef DATA_CHARGE_HISTORY_H_
#define DATA_CHARGE_HISTORY_H_
#include "Kernel.h"

struct DataChargeHistory
{
    uint32_t uid;
    uint32_t ts;
    uint32_t cash;

    DataChargeHistory():
		uid(0),
		ts(0),
		cash(0)
	{

	}

    void Reset()
    {
    	ts = 0;
    	cash = 0;
    }

    template<class T>
	void SetMessage(T * msg)
	{
		msg->set_cash(cash);
		msg->set_ts(ts);
	}

    void FromMessage(const ProtoUser::ChargeItem * msg)
    {
    	cash = msg->cash();
    	ts = msg->ts();
    }
};

class CDataChargeHistory :public DBCBase<DataChargeHistory, DB_CHARGE_HISTORY>
{
public:
	virtual int Get(DataChargeHistory &data);
	virtual int Get(vector<DataChargeHistory> &data);
	virtual int Add(DataChargeHistory &data);
	virtual int Set(DataChargeHistory &data);
	virtual int Del(DataChargeHistory &data);
};

#endif //DATA_CHARGE_HISTORY_H_
