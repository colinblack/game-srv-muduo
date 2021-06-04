#include "DataUserAllManager.h"
#include "ResourceManager.h"

int DataBuildingMgr::Init(unsigned uid)
{
	Online(uid);

	//初始化建筑ud
	unsigned maxud = 0;  //初始化ud = 1
	map<uint32_t, uint32_t>::reverse_iterator reviter = m_map[uid].rbegin();

	if (reviter != m_map[uid].rend() && maxud < reviter->first)
	{
		maxud = reviter->first;
	}

	m_userMaxUd[uid] = maxud;

	if (m_BuildUd.count(uid))
	{
		m_BuildUd[uid].clear();
	}

	//初始化建筑ud与build_id之间的映射关系
	for(; reviter != m_map[uid].rend(); ++reviter)
	{
		unsigned index = reviter->second;
		m_BuildUd[uid][m_data->data[index].build_id].push_back(reviter->first);
	}

	return 0;
}

void DataBuildingMgr::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataBuildings & build = GetDataByIndex(indexs[i]);

		DelBuild(build.uid, build.id);
	}

	//新增新数据
	unsigned maxud = 0;
	for(int i = 0; i < data.builds_size(); ++i)
	{
		unsigned build_id = data.builds(i).buildid();
		unsigned build_ud = data.builds(i).ud();
		DataBuildings & build = GetData(uid, build_ud);
		build.FromMessage(&data.builds(i));
		m_BuildUd[uid][build_id].push_back(build_ud);
		maxud = max(maxud, build_ud);
	}
	m_userMaxUd[uid] = maxud;
}

unsigned DataBuildingMgr::GetUserNextUd(unsigned uid)
{
	m_userMaxUd[uid] = m_userMaxUd[uid] + 1;

	return m_userMaxUd[uid];
}

DataBuildings & DataBuildingMgr::AddNewBuilding(unsigned uid, unsigned build_id)
{
	unsigned build_ud = GetUserNextUd(uid);

	DataBuildings & databuild = GetData(uid, build_ud);
	databuild.build_id = build_id;

	m_BuildUd[uid][build_id].push_back(build_ud);

	return databuild;
}

unsigned DataBuildingMgr::GetBuildUd(unsigned uid, uint16_t build_id)
{
	if (m_BuildUd.count(uid)
			&& m_BuildUd[uid].count(build_id)
			&& m_BuildUd[uid][build_id].size() > 0)
	{
		return m_BuildUd[uid][build_id][0u];
	}

	return -1;
}

unsigned DataBuildingMgr::GetBuildInfo(unsigned uid,map<uint16_t, vector<uint16_t> > &builds)
{
	map<unsigned, map<uint16_t, vector<uint16_t> > >::iterator it = m_BuildUd.find(uid);
	if(it != m_BuildUd.end())
	{
		map<uint16_t, vector<uint16_t> >::iterator itor = it->second.begin();
		for(; itor != it->second.end(); itor++)
		{
			builds[itor->first] = itor->second;
		}
	}

}

int DataBuildingMgr::DelBuild(unsigned uid, unsigned ud)
{
	if (!m_map.count(uid) || !m_map[uid].count(ud))
	{
		error_log("get build error. uid=%u, id=%u", uid, ud);
		throw std::runtime_error("get_build_error");
	}

	uint16_t build_id = GetData(uid, ud).build_id;

	//删除ud
	DelItem(uid, ud);

	//删除id映射
	vector<uint16_t>::iterator piter = find(m_BuildUd[uid][build_id].begin(), m_BuildUd[uid][build_id].end(), ud);

	if (piter != m_BuildUd[uid][build_id].end())
	{
		//删除
		m_BuildUd[uid][build_id].erase(piter);
	}

	if (m_BuildUd[uid][build_id].empty())
	{
		m_BuildUd[uid].erase(build_id);
	}

	return 0;
}

int DataBuildingMgr::GetBuildNum(unsigned uid, uint16_t build_id)
{
	if (m_BuildUd.count(uid) && m_BuildUd[uid].count(build_id))
	{
		return m_BuildUd[uid][build_id].size();
	}

	return 0;
}

int DataBuildingMgr::GetAllBuildNum(unsigned uid)
{
	if (!m_BuildUd.count(uid))
	{
		return 0;
	}

	map<uint16_t, vector<uint16_t> >::iterator viter = m_BuildUd[uid].begin();

	int nums = 0;

	for (; viter != m_BuildUd[uid].end(); ++viter)
	{
		nums += viter->second.size();
	}

	return nums;
}

int DataItemManager::Init(unsigned uid)
{
	Online(uid);

	//初始化道具ud
	unsigned maxud = 0;  //初始化ud = 1
	map<uint32_t, uint32_t>::reverse_iterator reviter = m_map[uid].rbegin();

	if (reviter != m_map[uid].rend() && maxud < reviter->first)
	{
		maxud = reviter->first;
	}

	m_userMaxUd[uid] = maxud;

	if (m_PropsUd.count(uid))
	{
		m_PropsUd[uid].clear();
	}

	//初始化道具ud与props_id之间的映射关系
	for(; reviter != m_map[uid].rend(); ++reviter)
	{
		unsigned index = reviter->second;
		m_PropsUd[uid][m_data->data[index].props_id].push_back(reviter->first);
	}

	return 0;
}

void DataItemManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataItem & item = GetDataByIndex(indexs[i]);

		DelProps(item);
	}

	//删除旧有的最大ud值
	m_userMaxUd.erase(uid);

	unsigned maxud = 0;
	//新增新数据
	for(int i = 0; i < data.item_size(); ++i)
	{
		unsigned id = data.item(i).ud();

		if (maxud < id)
		{
			maxud = id;
		}

		DataItem & item = GetData(uid, id);
		item.FromMessage(&data.item(i));
		m_PropsUd[item.uid][item.props_id].push_back(item.id);
	}

	//设置当前用户的最大ud值
	m_userMaxUd[uid] = maxud;
}

unsigned DataItemManager::GetUserNextUd(unsigned uid)
{
	m_userMaxUd[uid] = m_userMaxUd[uid] + 1;

	return m_userMaxUd[uid];
}

bool DataItemManager::IsPropsExist(unsigned uid, unsigned propsid)
{
	if (m_PropsUd.count(uid)
			&& m_PropsUd[uid].count(propsid)
			&& m_PropsUd[uid][propsid].size() > 0)
	{
		return true;
	}

	return false;
}

unsigned DataItemManager::GetPropsUd(unsigned uid, unsigned propsid)
{
	if (m_PropsUd.count(uid)
			&& m_PropsUd[uid].count(propsid)
			&& m_PropsUd[uid][propsid].size() > 0)
	{
		return m_PropsUd[uid][propsid][0u];
	}

	return -1;
}
unsigned DataItemManager::GetItemCount(uint32_t uid, uint32_t propsid)
{
	unsigned propsud = GetPropsUd(uid,propsid);
	if(propsud != -1){
		DataItem & item = GetData(uid,propsud);
		return item.item_cnt;
	}
	return 0;
}
int DataItemManager::AddNewProps(DataItem & propsitem)
{
	unsigned index = NewItem(propsitem.uid, propsitem.id);

	DataItem & item = GetDataByIndex(index);
	item = propsitem;

	m_PropsUd[propsitem.uid][propsitem.props_id].push_back(propsitem.id);

	return 0;
}

int DataItemManager::DelProps(DataItem & propsitem)
{
	unsigned uid = propsitem.uid;
	unsigned ud = propsitem.id;

	if (!m_map.count(uid) || !m_map[uid].count(ud))
	{
		error_log("get props error. uid=%u, id=%u", uid, ud);
		throw std::runtime_error("get_props_error");
	}

	unsigned index = m_map[uid][ud];
	//将删除操作立即加入到dbc的更新队列中
	//mark删除状态
	m_data->MarkDel(index);
	//添加至操作队列
	AddSave(index);

	//清除ud映射
	m_map[uid].erase(ud);

	//删除id的映射
	unsigned propsid = propsitem.props_id;

	vector<unsigned>::iterator piter = find(m_PropsUd[uid][propsid].begin(), m_PropsUd[uid][propsid].end(), ud);

	if (piter != m_PropsUd[uid][propsid].end())
	{
		//删除
		m_PropsUd[uid][propsid].erase(piter);
	}

	if (m_PropsUd[uid][propsid].empty())
	{
		m_PropsUd[uid].erase(propsid);
	}

	return 0;
}

int DataItemManager::GetPropsIdList(unsigned uid,vector<unsigned> & propsList)
{
	map<unsigned, map<uint32_t, vector<uint32_t> > >::iterator it = m_PropsUd.find(uid);
	if(it != m_PropsUd.end())
	{
		map<uint32_t, vector<uint32_t> >::iterator itor = it->second.begin();
		for(; itor != it->second.end(); itor++)
		{
			propsList.push_back(itor->first);
		}
	}
	return 0;
}

void DataCroplandManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataCropland & cropland = GetDataByIndex(indexs[i]);

		DelItem(cropland.uid, cropland.id);
	}

	//新增新数据
	for(int i = 0; i < data.cropland_size(); ++i)
	{
		unsigned id = data.cropland(i).ud();

		unsigned index = AddNewCropLand(uid, id);
		DataCropland & cropland = GetDataByIndex(index);
		cropland.FromMessage(&data.cropland(i));
	}
}

unsigned int DataCroplandManager::AddNewCropLand(unsigned uid, unsigned cropud)
{
	//判断ud是否存在，如果存在，则不重复添加
	if (IsExistItem(uid, cropud))
	{
		return GetIndex(uid, cropud);
	}

	unsigned index = NewItem(uid, cropud);

	return index;
}

void DataProduceequipManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataProduceequip & equip = GetDataByIndex(indexs[i]);

		DelItem(equip.uid, equip.id);
	}

	//新增新数据
	for(int i = 0; i < data.equipments_size(); ++i)
	{
		unsigned id = data.equipments(i).ud();

		unsigned index = AddNewEquip(uid, id);
		DataProduceequip & equip = GetDataByIndex(index);
		equip.FromMessage(&data.equipments(i));
	}
}

unsigned int DataProduceequipManager::AddNewEquip(unsigned uid, unsigned equipud)
{
	//判断ud是否存在，如果存在，则不重复添加
	if (IsExistItem(uid, equipud))
	{
		return GetIndex(uid, equipud);
	}

	unsigned index = NewItem(uid, equipud);

	return index;
}

int DataProduceequipManager::PartMessage(unsigned uid, google::protobuf::RepeatedPtrField<User::OthProduceCPP >* msg)
{
	vector<unsigned> indexs;

	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		GetDataByIndex(indexs[i]).SetPartMessage(msg->Add());
	}

	return 0;
}

int DataAnimalManager::Init(unsigned uid)
{
	Online(uid);

	//初始化建筑ud
	unsigned maxud = animal_ud_begin;  //初始化ud
	map<uint32_t, uint32_t>::reverse_iterator reviter = m_map[uid].rbegin();

	if (reviter != m_map[uid].rend() && maxud < reviter->first)
	{
		maxud = reviter->first;
	}

	m_userMaxUd[uid] = maxud;

	if (m_AnimalIdUd.count(uid))
	{
		m_AnimalIdUd[uid].clear();
	}

	if (m_BuildUd.count(uid))
	{
		m_BuildUd[uid].clear();
	}

	//初始化建筑ud与build_id之间的映射关系
	for(; reviter != m_map[uid].rend(); ++reviter)
	{
		unsigned mindex = reviter->second;
		m_AnimalIdUd[uid][m_data->data[mindex].animal_id].push_back(reviter->first);
		m_BuildUd[uid][m_data->data[mindex].residence_ud].push_back(reviter->first);
	}

	return 0;
}

void DataAnimalManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataAnimal & animal = GetDataByIndex(indexs[i]);

		DelItem(animal.uid, animal.id);
	}

	//删除旧有的最大ud值
	m_AnimalIdUd.erase(uid);
	m_BuildUd.erase(uid);
	m_userMaxUd.erase(uid);

	unsigned maxud = 0;
	//新增新数据
	for(int i = 0; i < data.animals_size(); ++i)
	{
		unsigned id = data.animals(i).ud();

		if (maxud < id)
		{
			maxud = id;
		}

		unsigned animal_ud = data.animals(i).ud();
		unsigned buildud = data.animals(i).residenceud();
		unsigned animalid = data.animals(i).animalid();
		DataAnimal & dataanimal = GetData(uid, animal_ud);
		dataanimal.FromMessage(&data.animals(i));
		m_BuildUd[uid][buildud].push_back(animal_ud);
		m_AnimalIdUd[uid][animalid].push_back(animal_ud);
	}

	//设置当前用户的最大ud值
	m_userMaxUd[uid] = maxud;
}

unsigned DataAnimalManager::GetUserNextUd(unsigned uid)
{
	m_userMaxUd[uid] = m_userMaxUd[uid] + 1;

	return m_userMaxUd[uid];
}

unsigned DataAnimalManager::GetBuildAdoptedNum(unsigned uid, unsigned buildud) const
{
	if (m_BuildUd.count(uid) && m_BuildUd.at(uid).count(buildud))
	{
		return m_BuildUd.at(uid).at(buildud).size();
	}

	return 0;
}

unsigned DataAnimalManager::GetAdoptedNum(unsigned uid, unsigned animalid) const
{
	if (m_AnimalIdUd.count(uid) && m_AnimalIdUd.at(uid).count(animalid))
	{
		return m_AnimalIdUd.at(uid).at(animalid).size();
	}

	return 0;
}

DataAnimal & DataAnimalManager::AddNewAnimal(unsigned uid, unsigned animalid, unsigned buildud)
{
	unsigned animal_ud = GetUserNextUd(uid);

	DataAnimal & dataanimal = GetData(uid, animal_ud);

	dataanimal.animal_id = animalid;
	dataanimal.residence_ud = buildud;

	m_BuildUd[uid][buildud].push_back(animal_ud);
	m_AnimalIdUd[uid][animalid].push_back(animal_ud);

	return dataanimal;
}

void DataEquipmentStarManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataEquipmentStar & equipstar = GetDataByIndex(indexs[i]);

		DelItem(equipstar.uid, equipstar.id);
	}

	//新增新数据
	for(int i = 0; i < data.equipstar_size(); ++i)
	{
		unsigned id = data.equipstar(i).id();

		DataEquipmentStar & equipstar = GetData(uid, id);
		equipstar.FromMessage(&data.equipstar(i));
	}
}

int DataOrderManager::Init(unsigned uid)
{
	Online(uid);
	return 0;
}

void DataOrderManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataOrder & dataOrder = GetDataByIndex(indexs[i]);

		DelItem(dataOrder.uid, dataOrder.id);
	}

	//新增新数据
	for(int i = 0; i < data.orders_size(); ++i)
	{
		unsigned id = data.orders(i).slot();

		DataOrder & dataOrder = GetData(uid, id);
		dataOrder.FromMessage(&data.orders(i));
	}
}

DataOrder & DataOrderManager::AddNewOrder(unsigned uid, unsigned slot, unsigned storage_id, unsigned level_id, char order_id[])
{
	DataOrder & dataOrder = GetData(uid, slot);
	dataOrder.id = slot;
	dataOrder.storage_id = storage_id;
	dataOrder.level_id = level_id;
	memset(dataOrder.order_id, 0, ORDER_LENGTH);
	memcpy(dataOrder.order_id, order_id, ORDER_LENGTH);

	return dataOrder;
}

bool DataOrderManager::GetOrderInfo(uint32_t uid, uint32_t slot, uint32_t & storage_id, uint32_t & level_id, uint32_t & order_id)
{
	return true;
}

void DataFruitManager::FullSpecialMessage(unsigned uid, google::protobuf::RepeatedPtrField<ProtoProduce::FruitCPP> * msg)
{
	//设置基本信息
	vector<unsigned> indexs;

	GetIndexs(uid, indexs);

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		ProtoProduce::FruitCPP * fruitmsg = msg->Add();
		DataFruit & fruit = GetDataByIndex(indexs[i]);

		//设置果树的基本信息
		fruit.SetMessage(fruitmsg);

		//判断是否有援助的人
		if (fruit.aid_uid > 0)
		{
			//设置该数据的头像和名称
			fruitmsg->set_helpuid(fruit.aid_uid);
		}
	}
}

void DataFruitManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataFruit & fruit = GetDataByIndex(indexs[i]);

		DelItem(fruit.uid, fruit.id);
	}

	//新增新数据
	for(int i = 0; i < data.fruits_size(); ++i)
	{
		unsigned id = data.fruits(i).ud();

		DataFruit & fruit = GetData(uid, id);
		fruit.FromMessage(&data.fruits(i));
	}
}

unsigned int DataFruitManager::AddNewFruit(unsigned uid, unsigned treeud)
{
	//判断ud是否存在，如果存在，则不重复添加
	if (IsExistItem(uid, treeud))
	{
		return GetIndex(uid, treeud);
	}

	unsigned index = NewItem(uid, treeud);

	return index;
}

int DataFruitManager::DelFruitTree(DataFruit & fruit)
{
	unsigned uid = fruit.uid;
	unsigned ud = fruit.id;

	if (!m_map.count(uid) || !m_map[uid].count(ud))
	{
		error_log("get fruit error. uid=%u, id=%u", uid, ud);
		throw std::runtime_error("get_fruit_error");
	}

	//删除dbc中的数据
	DelItem(uid, ud);

	return 0;
}

void DataShippingManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	GetData(uid).FromMessage(&data.shipping());
}

void DataShippingboxManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataShippingbox & box = GetDataByIndex(indexs[i]);

		DelItem(box.uid, box.id);
	}

	//新增新数据
	for(int i = 0; i < data.shipboxes_size(); ++i)
	{
		unsigned id = data.shipboxes(i).boxid();

		DataShippingbox & box = GetData(uid, id);
		box.FromMessage(&data.shipboxes(i));
	}
}

void DataShippingboxManager::ResetBoxes(unsigned uid)
{
	if (!m_map.count(uid))
	{
		return ;
	}

	for(map<unsigned, unsigned>::iterator uiter = m_map[uid].begin(); uiter != m_map[uid].end(); ++uiter)
	{
		unsigned index = uiter->second;

		m_data->data[index].Reset();

		m_data->MarkChange(index);
	}
}

void DataShippingboxManager::FullSpecialMessage(unsigned uid, google::protobuf::RepeatedPtrField<ProtoShipping::ShippingBoxCPP> * msg)
{
	//设置基本信息
	vector<unsigned> indexs;

	GetIndexs(uid, indexs);

	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataShippingbox & box = GetDataByIndex(indexs[i]);

		if (0 == box.propsid)
		{
			break;
		}

		ProtoShipping::ShippingBoxCPP * boxmsg = msg->Add();
		//设置航运箱子的基本信息
		box.SetMessage(boxmsg);

		//判断是否有援助的人
		if (box.aid_uid > 0)
		{
			//设置该数据的头像和名称
			boxmsg->set_name(ResourceManager::Instance()->GetUserName(box.aid_uid));
			boxmsg->set_fig(ResourceManager::Instance()->GetHeadUrl(box.aid_uid));
		}
	}
}

void DataInvitedListManager::SetMessage(unsigned uid, unsigned alliance_id, ProtoAlliance::AllianceInvitedCPP * msg)
{
	if (m_map.count(uid) && m_map[uid].count(alliance_id))
	{
		unsigned index = m_map[uid][alliance_id];

		DataInvitedList & invited = GetDataByIndex(index);

		invited.SetMessage(msg);
	}
}

void DataInvitedListManager::FullMessage(unsigned uid, unsigned maxcount, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceInvitedCPP> * msg)
{
	//设置基本信息
	vector<unsigned> indexs;

	GetIndexs(uid, indexs);

	for(size_t i = 0; i < indexs.size() && i < maxcount; ++i)
	{
		DataInvitedList & invited = GetDataByIndex(indexs[i]);

		//设置邀请的的基本信息
		invited.SetMessage(msg->Add());
	}
}
