#ifndef DATA_PET_H_
#define DATA_PET_H_

#include "Kernel.h"

struct DataPet
{
    uint32_t uid;
    uint32_t id;   //宠物id
    uint32_t teaseEndts;//逗养结束ts
    uint32_t normalEndts;//正常状态结束ts
    uint32_t teaseFlag;//是否逗养过
    char name[BASE_NAME_LEN];//名字

    DataPet():
    	uid(0),
    	id(0),
    	teaseEndts(0),
    	normalEndts(0),
		teaseFlag(0)
	{
    	memset(name, 0, sizeof(name));
	}

    DataPet(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	teaseEndts(0),
    	normalEndts(0),
		teaseFlag(0)
    {
    	memset(name, 0, sizeof(name));
    }


    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_petid(id);
		msg->set_teaseendts(teaseEndts);
		msg->set_normalendts(normalEndts);
		msg->set_teaseflag(teaseFlag);
		msg->set_name(string(name));
	}

    void FromMessage(const ProtoUser::UnlockPetCPP * msg)
    {
    	teaseEndts = msg->teaseendts();
    	normalEndts=msg->normalendts();
    	teaseFlag=msg->teaseflag();
    	memset(name, 0, sizeof(name));
    	strncpy(name, msg->name().c_str(), sizeof(name)-1);
    }

};

class CDataPet :public DBCBase<DataPet, DB_PET>
{
public:
	virtual int Get(DataPet &data);
	virtual int Get(vector<DataPet> &data);
	virtual int Add(DataPet &data);
	virtual int Set(DataPet &data);
	virtual int Del(DataPet &data);
};

#endif //DATA_PET_H_
