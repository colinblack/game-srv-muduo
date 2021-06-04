#ifndef DATA_NPCSELLER_H_
#define DATA_NPCSELLER_H_
#include "Kernel.h"

struct DataNPCSeller
{
    uint32_t uid;
    uint32_t props_id;
    uint32_t props_cnt;
    uint32_t npc_next_visit_ts;
    uint32_t props_discount;
    uint32_t npc_seller_status;

    DataNPCSeller():
    	uid(0),
    	props_id(0),
    	props_cnt(0),
    	npc_next_visit_ts(0),
    	props_discount(0),
    	npc_seller_status(0)
	{

	}

    DataNPCSeller(unsigned uid_)
    	:uid(uid_),
    	props_id(0),
		props_cnt(0),
		npc_next_visit_ts(0),
		props_discount(0),
		npc_seller_status(0)
    {

    }

	void SetMessage(ProtoNPCSeller::NPCSellerCPP *msg)
	{
		msg->set_propsid(props_id);
		msg->set_propscnt(props_cnt);
		msg->set_propsdiscount(props_discount);
		msg->set_npcnextvisitts(npc_next_visit_ts);
		msg->set_npcsellerstatus(npc_seller_status);
	}
};

class CDataNPCSeller :public DBCBase<DataNPCSeller, DB_NPCSELLER>
{
public:
	virtual int Get(DataNPCSeller &data);
	virtual int Add(DataNPCSeller &data);
	virtual int Set(DataNPCSeller &data);
	virtual int Del(DataNPCSeller &data);
};

#endif //DATA_NPCSELLER_H_
