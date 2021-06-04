#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_

#include "Kernel.h"
#include "DBCSimpleTemplate.h"
#include "DBCMultipleTemplate.h"
#include "DataGameActivity.h"

//活动
class DataGameActivityManager : public DataSingleton<DataGameActivity, DB_GAME_ACTIVITY, DB_GAME_ACTIVITY_FULL, CDataGameActivity, DB_GAME_ACTIVITY_FULL>, public CSingleton<DataGameActivityManager>
{
	typedef DataSingleton<DataGameActivity, DB_GAME_ACTIVITY, DB_GAME_ACTIVITY_FULL, CDataGameActivity, DB_GAME_ACTIVITY_FULL> base;
private:
	friend class CSingleton<DataGameActivityManager>;
	DataGameActivityManager(){};
	virtual ~DataGameActivityManager(){}

	typedef map<uint32_t, map<uint32_t, uint32_t> > MapMap;  //uid->id =>index
	typedef map<uint32_t, map<uint32_t, uint32_t> >::iterator MapMap_ITER;
	MapMap m_map;
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	DataGameActivity & GetUserActivity(unsigned uid, unsigned id);

	int CheckBuff(unsigned uid);
	int AddBuff(DataGameActivity & activity);
	int LoadBuff(unsigned uid);

	void DoClear(unsigned uid);
	void DoSave(unsigned uid);

	bool UpdateActivity(DataGameActivity & activity);

	void DelItem(unsigned uid, unsigned id);

	bool IsExistItem(unsigned uid, unsigned id);

	void FullMessage(unsigned uid,::google::protobuf::RepeatedPtrField< ::ProtoUser::ActivityCPP >* msg);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};

#endif  //DATA_MANAGER_H_
