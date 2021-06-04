#ifndef DATA_ALLIANCE_MAPPING_H_
#define DATA_ALLIANCE_MAPPING_H_

#include "Kernel.h"

struct DataAllianceMapping
{
	uint32_t alliance_id;
	char alliance_name[BASE_NAME_LEN];//名字

	DataAllianceMapping():
		alliance_id(0)
	{
		memset(alliance_name, 0, sizeof(alliance_name));
	}

	void Clear()
	{
		alliance_id = 0;
		memset(alliance_name, 0, sizeof(alliance_name));
	}
};

class CDataAllianceMapping :public DBCBase<DataAllianceMapping, DB_ALLIANCE_MAPPING>
{
public:
	virtual int Get(DataAllianceMapping &data);
	virtual int Add(DataAllianceMapping &data);
	virtual int Del(const char * name);
};

#endif /* DATA_ALLIANCE_MAPPING_H_ */
