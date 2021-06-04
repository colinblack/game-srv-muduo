#ifndef DATA_MailDog_H_
#define DATA_MailDog_H_
#include "Kernel.h"

struct DataMailDog
{
    uint32_t uid;
    uint32_t id;   //标志统计项
    uint32_t value;//对应统计项的值
    DataMailDog():
    	uid(0),
    	id(0),
    	value(0)
	{

	}

    DataMailDog(unsigned uid_, unsigned ud)
    	:uid(uid_),
    	id(ud),
    	value(0)
    {

    }

    template<class T>
	void SetMessage(T *msg)
	{
		msg->set_id(id);
		msg->set_value(value);
	}

	void FromMessage(const ProtoUser::MaidDogCPP * msg)
	{
		value = msg->value();
	}
};

class CDataMailDog :public DBCBase<DataMailDog, DB_MAILDOG>
{
public:
	virtual int Get(DataMailDog &data);
	virtual int Get(vector<DataMailDog> &data);
	virtual int Add(DataMailDog &data);
	virtual int Set(DataMailDog &data);
	virtual int Del(DataMailDog &data);
};

#endif //DATA_MailDog_H_
