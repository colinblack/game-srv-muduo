#ifndef LOGIC_PET_MANAGER_H
#define LOGIC_PET_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicPetManager :public BattleSingleton, public CSingleton<LogicPetManager>
{
private:
	friend class CSingleton<LogicPetManager>;
	LogicPetManager(){}

public:
	virtual void CallDestroy() { Destroy();}

	//解锁宠物住所
	int Process(unsigned uid, ProtoPet::UnlockPetResidenceReq* req, ProtoPet::UnlockPetResidenceResp* resp);

	//获取解锁宠物信息
	int Process(unsigned uid, ProtoPet::GetUnlockPetInfoReq* req, ProtoPet::GetUnlockPetInfoResp* resp);

	//解锁宠物
	int Process(unsigned uid, ProtoPet::UnlockPetReq* req, ProtoPet::UnlockPetResp* resp);

	//喂养宠物
	int Process(unsigned uid, ProtoPet::FeedPetReq* req, ProtoPet::FeedPetResp* resp);

	//逗养宠物
	int Process(unsigned uid, ProtoPet::TeasePetReq* req, ProtoPet::TeasePetResp* resp);

	//更改名字
	int Process(unsigned uid, ProtoPet::ChangePetNameReq* req, ProtoPet::ChangePetNameResp* resp);

	//分享解锁宠物
	int InviteUnlockPet(unsigned uid);
private:

};


#endif //LOGIC_PET_MANAGER_H
