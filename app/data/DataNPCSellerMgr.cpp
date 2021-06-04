#include "DataNPCSellerMgr.h"

void DataNPCSellerManager::FullMessage(unsigned uid,ProtoNPCSeller::NPCSellerCPP *msg)
{
	DataNPCSeller &npcSeller = DataNPCSellerManager::Instance()->GetData(uid);
	npcSeller.SetMessage(msg);
}
