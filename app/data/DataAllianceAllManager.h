#ifndef DATA_ALLIANCE_ALL_MANAGER_H_
#define DATA_ALLIANCE_ALL_MANAGER_H_

#include "Kernel.h"

#include "DBCSimpleAllianceTemplate.h"
#include "DBCMultipleAllianceTemplate.h"
#include "DataAlliance.h"
#include "DataAllianceMember.h"
#include "DataAllianceApply.h"
#include "DataAllianceDonation.h"
#include "DataAllianceNotify.h"

//商会
class DataAllianceManager : public DBCSimpleAllianceTemplate<DataAlliance, DB_ALLIANCE, CDataAlliance>
	, public CSingleton<DataAllianceManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataAllianceManager";
	}

	bool IsAllianceNeedClear()
	{
		return GetFreeCount() * 10 / MAX_BUFF <= 1;
	}

	template<class T>
	void SetMessage(unsigned alliance_id, T* msg);
	template<class T>
	void SetMessage1(unsigned alliance_id, T* msg);
};


template<class T>
void DataAllianceManager::SetMessage(unsigned alliance_id, T* msg)
{
	//判断是否存在
	if (IsExist(alliance_id))
	{
		GetData(alliance_id).SetMessage(msg->mutable_alliance());
	}
}
template<class T>
void DataAllianceManager::SetMessage1(unsigned alliance_id, T* msg)
{
	//判断是否存在
	if (IsExist(alliance_id))
	{
		GetData(alliance_id).SetMessage(msg);
	}
}

//商会成员
class DataAllianceMemberManager : public DBCMultipleAllianceTemplate<DataAllianceMember, DB_ALLIANCE_MEMBER, DB_ALLIANCE_MEMBER_FULL, CDataAllianceMember>
	,public CSingleton<DataAllianceMemberManager>
{
public:
	virtual void CallDestroy() 	{ Destroy();}

	const char* name() const
	{
		return "DataAllianceMemberManager";
	}

	//获取商会总的成员数量
	unsigned GetMemberCount(unsigned alliance_id) const;
	//获取在线商会成员数量
	unsigned GetMemberOnlineNum(unsigned alliance_id)const;
	//获取各职位的成员数量
	unsigned GetPositionCount(unsigned alliance_id, unsigned position) const;
};

//商会申请列表
class DataAllianceApplyManager : public DBCMultipleAllianceTemplate<DataAllianceApply, DB_ALLIANCE_APPLY, DB_ALLIANCE_APPLY_FULL, CDataAllianceApply>
	,public CSingleton<DataAllianceApplyManager>
{
public:
	virtual void CallDestroy() 	{ Destroy();}

	const char* name() const
	{
		return "DataAllianceApplyManager";
	}

	int FullMessage(unsigned alliance_id, unsigned maxcount, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceApplyCPP >* msg);
};

//商会捐收
class DataAllianceDonationManager : public DBCMultipleAllianceTemplate<DataAllianceDonation, DB_ALLIANCE_DONATION, DB_ALLIANCE_DONATION_FULL, CDataAllianceDonation>
	,public CSingleton<DataAllianceDonationManager>
{
public:
	virtual void CallDestroy() 	{ Destroy();}

	const char* name() const
	{
		return "DataAllianceDonationManager";
	}

	int FullMessage(unsigned alliance_id, unsigned uid, unsigned maxcount, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceDonationCPP >* msg);
};

//商会通知
class DataAllianceNotifyManager : public DBCMultipleAllianceTemplate<DataAllianceNotify, DB_ALLIANCE_NOTIFY, DB_ALLIANCE_NOTIFY_FULL, CDataAllianceNotify>
	,public CSingleton<DataAllianceNotifyManager>
{
public:
	virtual void CallDestroy() 	{ Destroy();}

	const char* name() const
	{
		return "DataAllianceNotifyManager";
	}

	int FullMessage(unsigned alliance_id, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceNotifyCPP >* msg);

	//理事或者会长职位变化时删除通知
	int ResetNotifyWhenChange(unsigned uid);

	//获取可用的通知id
	unsigned GetFreeNotifyId(unsigned alliance_id, unsigned uid, unsigned postion);

	unsigned GetNextId() const;
};


#endif //DATA_ALLIANCE_ALL_MANAGER_H_
