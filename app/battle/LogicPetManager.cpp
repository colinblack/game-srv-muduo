#include "ServerInc.h"


int LogicPetManager::Process(unsigned uid, ProtoPet::UnlockPetResidenceReq* req, ProtoPet::UnlockPetResidenceResp* resp)
{
	const ConfigPet::PetResidenceCPP petResidenceCfg = ConfigManager::Instance()->pet.m_config.pet_residence();
	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;
	unsigned unlock_level = petResidenceCfg.unlock_level();

	//校验解锁等级
	if(level < unlock_level)
	{
		throw std::runtime_error("level_unlock");
	}

	//校验是否已解锁
	if(userwrap.Obj().isUnlockPetResidence == 1)
	{
		throw std::runtime_error("already_unlock");
	}

	//校验通过、做解锁处理
	LogicUserManager::Instance()->CommonProcess(uid,petResidenceCfg.unlock_cost(),"pet_residence_unlock",resp->mutable_common());

	userwrap.Obj().isUnlockPetResidence = 1;
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);

	return 0;
}


int LogicPetManager::Process(unsigned uid, ProtoPet::GetUnlockPetInfoReq* req, ProtoPet::GetUnlockPetInfoResp* resp)
{
	vector<unsigned>petIndexs;
	petIndexs.clear();
	DataPetManager::Instance()->GetIndexs(uid,petIndexs);

	for(int i = 0; i < petIndexs.size(); i++)
	{
		DataPet & pet = DataPetManager::Instance()->GetDataByIndex(petIndexs[i]);
		pet.SetMessage(resp->add_arraypet());
	}
	return 0;
}

int LogicPetManager::Process(unsigned uid, ProtoPet::UnlockPetReq* req, ProtoPet::UnlockPetResp* resp)
{
	unsigned petid = req->petid();
	const ConfigPet::PetCPP & petcfg = PetCfgWrap().GetPetCfgByPetId(petid);
	DBCUserBaseWrap userwrap(uid);

	//校验是否已解锁
	if(userwrap.Obj().level < petcfg.unlock_level())
	{
		throw std::runtime_error("level_unlock");
	}

	//校验是否已存在此宠物
	bool is_exsit = DataPetManager::Instance()->IsExistItem(uid,petid);
	if(is_exsit)
	{
		throw std::runtime_error("pet_already_exsit");
	}

	//校验通过，进行资源扣除
	LogicUserManager::Instance()->CommonProcess(uid,petcfg.unlock_cost(),"pet_unlock_cost",resp->mutable_common());

	//添加新宠物
	DataPet & pet = DataPetManager::Instance()->GetData(uid,petid);
	pet.normalEndts = Time::GetGlobalTime() + petcfg.normal_cd();
	memset(pet.name, 0, BASE_NAME_LEN);
	strncpy(pet.name, petcfg.comment().c_str(), BASE_NAME_LEN-1);
	DataPetManager::Instance()->UpdateItem(pet);
	pet.SetMessage(resp->mutable_pet());

	return 0;
}

int LogicPetManager::InviteUnlockPet(unsigned uid)
{
	//校验玩家是否有解锁过动物
	vector<unsigned>petlist;
	petlist.clear();
	DataPetManager::Instance()->GetIndexs(uid,petlist);
	unsigned petid = 1001;
	for(int i = 0; i < petlist.size(); i++)
	{
		DataPet & pet = DataPetManager::Instance()->GetDataByIndex(petlist[i]);
		if(pet.id == petid)
			return 0;
	}

	//添加新宠物
	const ConfigPet::PetCPP & petcfg = PetCfgWrap().GetPetCfgByPetId(petid);
	DataPet & pet = DataPetManager::Instance()->GetData(uid,petid);
	pet.normalEndts = Time::GetGlobalTime() + petcfg.normal_cd();
	memset(pet.name, 0, BASE_NAME_LEN);
	strncpy(pet.name, petcfg.comment().c_str(), BASE_NAME_LEN-1);
	DataPetManager::Instance()->UpdateItem(pet);

	if(UserManager::Instance()->IsOnline(uid))
	{
		ProtoPet::PushPetMsg *msg = new ProtoPet::PushPetMsg;
		pet.SetMessage(msg->mutable_pet());
		LMI->sendMsg(uid,msg);
	}
	return 0;
}

int LogicPetManager::Process(unsigned uid, ProtoPet::FeedPetReq* req, ProtoPet::FeedPetResp* resp)
{
	unsigned petid = req->petid();
	const ConfigPet::PetCPP & petcfg = PetCfgWrap().GetPetCfgByPetId(petid);
	DBCUserBaseWrap userwrap(uid);

	//是否存在此宠物
	bool is_exsit = DataPetManager::Instance()->IsExistItem(uid,petid);
	if(!is_exsit)
	{
		throw std::runtime_error("pet_is_not_exsit");
	}

	//判定宠物是否处于饥饿状态
	DataPet & pet = DataPetManager::Instance()->GetData(uid,petid);
	if(pet.teaseEndts < Time::GetGlobalTime())
	{
		//表明宠物未处于逗养状态
		if(pet.normalEndts > Time::GetGlobalTime())
		{
			//宠物处于正常状态
			throw std::runtime_error("pet_is_not_hungry");
		}
	}
	else
	{
		//宠物处于逗养状态
		throw std::runtime_error("pet_is_not_hungry");
	}

	//校验通过,进行资源扣除
	LogicUserManager::Instance()->CommonProcess(uid,petcfg.feed_cost(),"feed_pet_cost",resp->mutable_common());

	//更新宠物信息
	pet.teaseFlag = 0;
	pet.teaseEndts = Time::GetGlobalTime() + petcfg.tease_cd();
	pet.normalEndts = Time::GetGlobalTime() + petcfg.tease_cd() + petcfg.normal_cd();
	DataPetManager::Instance()->UpdateItem(pet);
	pet.SetMessage(resp->mutable_pet());

	//随机奖励
	vector<unsigned>weights;
	weights.clear();
	const ConfigPet::PetGardenCfg & petgardencfg = ConfigManager::Instance()->pet.m_config;
	for(int i = 0; i < petgardencfg.pet_feed_reward_size();  i++)
	{
		weights.push_back(petgardencfg.pet_feed_reward(i).weight());
	}
	int target = 0;
	LogicCommonUtil::TurnLuckTable(weights,weights.size(),target);
	if(target != 0 && target < petgardencfg.pet_feed_reward_size())
	{
		LogicUserManager::Instance()->CommonProcess(uid,petgardencfg.pet_feed_reward(target).reward(),"pet_feed_reward",resp->mutable_reward());
	}

	return 0;
}


int LogicPetManager::Process(unsigned uid, ProtoPet::TeasePetReq* req, ProtoPet::TeasePetResp* resp)
{
	unsigned petid = req->petid();
	const ConfigPet::PetCPP & petcfg = PetCfgWrap().GetPetCfgByPetId(petid);
	DBCUserBaseWrap userwrap(uid);

	//是否存在此宠物
	bool is_exsit = DataPetManager::Instance()->IsExistItem(uid,petid);
	if(!is_exsit)
	{
		throw std::runtime_error("pet_is_not_exsit");
	}

	//判定宠物是否处于挑逗状态
	DataPet & pet = DataPetManager::Instance()->GetData(uid,petid);
	if(pet.teaseEndts < Time::GetGlobalTime())
	{
		//表明宠物未处于逗养状态
		throw std::runtime_error("pet_is_not_tease_status");
	}

	//校验通过,进行资源扣除
	LogicUserManager::Instance()->CommonProcess(uid,petcfg.tease_cost(),"tease_pet_cost",resp->mutable_common());

	//更新宠物信息
	pet.teaseFlag = 1;
	pet.SetMessage(resp->mutable_pet());

	//随机奖励
	vector<unsigned>weights;
	weights.clear();
	const ConfigPet::PetGardenCfg & petgardencfg = ConfigManager::Instance()->pet.m_config;
	for(int i = 0; i < petgardencfg.pet_tease_reward_size();  i++)
	{
		weights.push_back(petgardencfg.pet_tease_reward(i).weight());
	}
	int target = 0;
	LogicCommonUtil::TurnLuckTable(weights,weights.size(),target);
	if(target != 0 && target < petgardencfg.pet_tease_reward_size())
	{
		LogicUserManager::Instance()->CommonProcess(uid,petgardencfg.pet_tease_reward(target).reward(),"pet_tease_reward",resp->mutable_reward());
	}
	return 0;
}


int LogicPetManager::Process(unsigned uid, ProtoPet::ChangePetNameReq* req, ProtoPet::ChangePetNameResp* resp)
{
	unsigned petid = req->petid();
	bool is_exsit = DataPetManager::Instance()->IsExistItem(uid,petid);
	if(!is_exsit)
	{
		throw std::runtime_error("animal_is_not_exsit");
	}

	string name = req->name();
	String::Trim(name);
	if(name.empty() || !StringFilter::Check(name))
	{
		throw std::runtime_error("name_fobbid");
	}

	DataPet & pet = DataPetManager::Instance()->GetData(uid,petid);
	memset(pet.name, 0, BASE_NAME_LEN);
	strncpy(pet.name, name.c_str(), BASE_NAME_LEN-1);
	DataPetManager::Instance()->UpdateItem(pet);
	pet.SetMessage(resp->mutable_pet());

	return 0;
}
