#ifndef DATA_AID_RECORD_MANAGER_H
#define DATA_AID_RECORD_MANAGER_H

#include "DataAidRecord.h"

//这个结构，主要就是为了实现好友中的近期帮助好友记录
class DataAidRecordManager : public DataSingleton<DataAidRecord, DB_AID_RECORD, DB_AID_RECORD_FULL, CDataAidRecord, DB_AID_RECORD_FULL>
	, public CSingleton<DataAidRecordManager>
{
private:
	friend class CSingleton<DataAidRecordManager>;
	DataAidRecordManager(){};
	virtual ~DataAidRecordManager(){}

	typedef map<unsigned, map<unsigned, map<unsigned, unsigned> > > MMMAP;

	MMMAP m_map; //uid->ts->othuid->index
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	int CheckBuff(unsigned uid);
	int AddBuff(DataAidRecord & datarecord);
	int LoadBuff(unsigned uid);

	void DoClear(unsigned uid);
	void DoSave(unsigned uid);

	//添加记录
	int AddAidRecord(unsigned uid, unsigned aid, unsigned ts);

	//获取近期帮助好友记录
	int GetRecentAidRecord(unsigned uid, vector<unsigned> & indexs);

	DataAidRecord & GetDataByIndex(unsigned index);

private:
	int AddNewRecord(unsigned uid, unsigned aid, unsigned ts);

	void ClearDateRecord(unsigned uid, unsigned ts);
};

#endif //DATA_AID_RECORD_MANAGER_H
