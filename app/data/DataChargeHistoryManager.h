#ifndef DATA_CHARGE_HISTORY_MANAGER_H_
#define DATA_CHARGE_HISTORY_MANAGER_H_

#include "Kernel.h"
#include "DataChargeHistory.h"

class DataChargeHistoryManager : public DataSingleton<DataChargeHistory, DB_CHARGE_HISTORY, DB_CHARGE_HISTORY_FULL, CDataChargeHistory, DB_CHARGE_HISTORY_FULL>,
	public CSingleton<DataChargeHistoryManager>
{
private:
	friend class CSingleton<DataChargeHistoryManager>;
	DataChargeHistoryManager(){};
	virtual ~DataChargeHistoryManager(){}

	typedef map<uint32_t, map<uint32_t, uint32_t> > MapMap;  //uid->times =>index
	typedef map<uint32_t, map<uint32_t, uint32_t> >::iterator MapMap_ITER;
	MapMap m_map;

	int NewChargeHistory(unsigned uid, unsigned cash);
public:
	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();

	int LoginCheck(unsigned uid);

	int CheckBuff(unsigned uid);
	int AddBuff(DataChargeHistory & datacharge);
	int LoadBuff(unsigned uid);

	void DoClear(unsigned uid);
	void DoSave(unsigned uid);

	void FullMessage(unsigned uid, User::AccumulateCharge * msg);

	void FullMessage(unsigned uid, google::protobuf::RepeatedPtrField<ProtoUser::ChargeItem >* msg);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	//添加充值记录
	int AddChargeHistory(unsigned uid, unsigned cash);
	//获取最近的充值记录对应的index
	int GetLatiestCharge(unsigned uid, unsigned & index);

	DataChargeHistory& GetChargeHistory(unsigned index);

	//获取所有充值数据
	int GetChargeHistoryList(unsigned uid, vector<unsigned> & indexs);

	bool UpdateChargeHistory(DataChargeHistory& datacharge);

	//删除充值记录
	bool DeleteChargeHistory(DataChargeHistory & datacharge);

	bool DeleteDBC(unsigned index);

};

#endif /* DATA_CHARGE_HISTORY_MANAGER_H_ */
