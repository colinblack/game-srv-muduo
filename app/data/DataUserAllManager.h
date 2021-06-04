#ifndef DATA_USER_ALL_MANAGER_H_
#define DATA_USER_ALL_MANAGER_H_

#include "Kernel.h"
#include "DBCMultipleTemplate.h"
#include "DBCSimpleTemplate.h"
#include "DataBuildings.h"
#include "DataCropLand.h"
#include "DataItem.h"
#include "DataProduceEquip.h"
#include "DataAnimal.h"
#include "DataOrder.h"
#include "DataEquipmentStar.h"
#include "DataFruit.h"
#include "DataConcern.h"
#include "DataFans.h"
#include "DataShipping.h"
#include "DataShippingBox.h"
#include "DataInvitedList.h"

class DataBuildingMgr: public DBCMultipleTemplate<DataBuildings, DB_BUILD, DB_BUILD_FULL, CDataBuildings>
	,public CSingleton<DataBuildingMgr>
{
public:
	virtual void CallDestroy() {
		Destroy();
	}

	const char* name() const {
		return "DataBuildings";
	}

	int Init(unsigned uid);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	DataBuildings & AddNewBuilding(unsigned uid, unsigned build_id);

	//获取指定建筑id的数目
	int GetBuildNum(unsigned uid, uint16_t build_id);

	//获取所有已建筑数目
	int GetAllBuildNum(unsigned uid);

	unsigned GetUserNextUd(unsigned uid);


	//适合获取唯一存在的建筑，如果不是唯一的，那么ud值是第一个
	unsigned GetBuildUd(unsigned uid, uint16_t build_id);

	//获取用户的建筑信息
	unsigned GetBuildInfo(unsigned uid,map<uint16_t, vector<uint16_t> > & builds);

	//删除建筑
	int DelBuild(unsigned uid, unsigned ud);

private:
	map<unsigned, uint16_t> m_userMaxUd;
	map<unsigned, map<uint16_t, vector<uint16_t> > > m_BuildUd; //uid =》( build->ud)
};

class DataItemManager : public DBCMultipleTemplate<DataItem, DB_ITEM, DB_ITEM_FULL, CDataItem>
	,public CSingleton<DataItemManager>
{
public:
	virtual void CallDestroy() { Destroy();}
	const char* name() const  { return "DataItemManager"; }

	int Init(unsigned uid);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	//获取下一个ud
	unsigned GetUserNextUd(unsigned uid);

	//判断道具id用户是否存在
	bool IsPropsExist(unsigned uid, unsigned propsid);

	//适合获取唯一存在的建筑，如果不是唯一的，那么ud值是第一个
	unsigned GetPropsUd(unsigned uid, unsigned propsid);

	unsigned GetItemCount(uint32_t uid, uint32_t propsid);
	//新增道具
	int AddNewProps(DataItem & propsitem);

	//删除装备
	int DelProps(DataItem & propsitem);

	//获取当前物品列表
	int GetPropsIdList(unsigned uid,vector<unsigned> & propsList);

private:
	map<unsigned, uint32_t> m_userMaxUd;
	map<unsigned, map<uint32_t, vector<uint32_t> > > m_PropsUd; //uid =》(propsid->ud)
};

class DataCroplandManager: public DBCMultipleTemplate<DataCropland, DB_CROPLAND, DB_CROPLAND_FULL, CDataCropland>
	,public CSingleton<DataCroplandManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataCroplandManager";
	}

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	unsigned int AddNewCropLand(unsigned uid, unsigned cropud);
};

class DataProduceequipManager : public DBCMultipleTemplate<DataProduceequip, DB_PRODUCEEQUIP, DB_PRODUCEEQUIP_FULL, CDataProduceequip>
	,public CSingleton<DataProduceequipManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataProduceequipManager";
	}

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	unsigned int AddNewEquip(unsigned uid, unsigned equipud);

	int PartMessage(unsigned uid, google::protobuf::RepeatedPtrField<User::OthProduceCPP >* msg);
};

class DataAnimalManager : public DBCMultipleTemplate<DataAnimal, DB_ANIMAL, DB_ANIMAL_FULL, CDataAnimal>
	,public CSingleton<DataAnimalManager>
{
public:
	enum
	{
		animal_ud_begin = 10000000,  //ud起始值
	};

	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataAnimalManager";
	}

	int Init(unsigned uid);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	//获取下一个ud
	unsigned GetUserNextUd(unsigned uid);

	//获取指定动物住所已容纳的动物数目
	unsigned GetBuildAdoptedNum(unsigned uid, unsigned buildud) const;

	//获取指定动物的领养数目
	unsigned GetAdoptedNum(unsigned uid, unsigned animalid) const;

	//动物
	DataAnimal & AddNewAnimal(unsigned uid, unsigned animalid, unsigned buildud);

private:
	map<unsigned, uint32_t> m_userMaxUd;
	map<unsigned, map<uint32_t, vector<uint32_t> > > m_AnimalIdUd; //uid =》(animal->ud)  动物id相关的映射
	map<unsigned, map<uint32_t, vector<uint32_t> > > m_BuildUd; //uid =》(buildud->ud)  动物住所相关的映射
};

//设备升星
class DataEquipmentStarManager : public DBCMultipleTemplate<DataEquipmentStar, DB_EQUIPMENT_STAR, DB_EQUIPMENT_STAR_FULL, CDataEquipmentStar>
	,public CSingleton<DataEquipmentStarManager>
{
public:
	virtual void CallDestroy() 	{ Destroy();}

	const char* name() const
	{
		return "DataEquipment_starManager";
	}

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);
};

class DataOrderManager : public DBCMultipleTemplate<DataOrder, DB_ORDER, DB_ORDER_FULL, CDataOrder> ,
	public CSingleton<DataOrderManager>
{
public:
	virtual void CallDestroy(){Destroy();}
	const char* name() const{return "DataOrderManager";}
	int Init(unsigned uid);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	DataOrder & AddNewOrder(uint32_t uid, uint32_t slot, uint32_t storage_id, uint32_t level_id, char order_id[]);
	bool GetOrderInfo(uint32_t uid, uint32_t slot, uint32_t & storage_id, uint32_t & level_id, uint32_t & order_id);
//private:
//	map<uint32_t, vector<uint32_t> > m_Orders;
};

//果树生产
class DataFruitManager : public DBCMultipleTemplate<DataFruit, DB_FRUIT, DB_FRUIT_FULL, CDataFruit>
	,public CSingleton<DataFruitManager>
{
public: virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataFruitManager";
	}

	void FullSpecialMessage(unsigned uid, google::protobuf::RepeatedPtrField<ProtoProduce::FruitCPP> * msg);

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	unsigned int AddNewFruit(unsigned uid, unsigned treeud);

	//删除果树
	int DelFruitTree(DataFruit & fruit);
};

//关注列表
class DataConcernManager : public DBCMultipleTemplate<DataConcern, DB_CONCERN, DB_CONCERN_FULL, CDataConcern>
	,public CSingleton<DataConcernManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataConcernManager";
	}
};

//粉丝列表
class DataFansManager : public DBCMultipleTemplate<DataFans, DB_FANS, DB_FANS_FULL, CDataFans>
	,public CSingleton<DataFansManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataFansManager";
	}
};

//航运
class DataShippingManager : public DBCSimpleTemplate<DataShipping, DB_SHIPPING, CDataShipping>
	, public CSingleton<DataShippingManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataShippingManager";
	}

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	template<class T>
	void SetMessage(unsigned uid, T msg);
};

template<class T>
void DataShippingManager::SetMessage(unsigned uid, T msg)
{
	//判断是否存在
	if (IsExist(uid))
	{
		GetData(uid).SetMessage(msg->mutable_shipping());
	}
}

//航运箱子
class DataShippingboxManager : public DBCMultipleTemplate<DataShippingbox, DB_SHIPPINGBOX, DB_SHIPPINGBOX_FULL, CDataShippingbox>
	, public CSingleton<DataShippingboxManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataShippingboxManager";
	}

	void FromMessage(unsigned uid, const ProtoArchive::UserData & data);

	void ResetBoxes(unsigned uid);

	void FullSpecialMessage(unsigned uid, google::protobuf::RepeatedPtrField<ProtoShipping::ShippingBoxCPP> * msg);
};

//公会邀请列表
class DataInvitedListManager : public DBCMultipleTemplate<DataInvitedList, DB_INVITED_LIST, DB_INVITED_LIST_FULL, CDataInvitedList>
	,public CSingleton<DataInvitedListManager>
{
public:
	virtual void CallDestroy()
	{
		Destroy();
	}

	const char* name() const
	{
		return "DataInvitedListManager";
	}

	void SetMessage(unsigned uid, unsigned alliance_id, ProtoAlliance::AllianceInvitedCPP * msg);

	void FullMessage(unsigned uid, unsigned maxcount, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceInvitedCPP> * msg);
};

#endif //DATA_USER_ALL_MANAGER_H_
