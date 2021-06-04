#ifndef _SERVER_INC_H_
#define _SERVER_INC_H_

#define RMI ResourceManager::Instance()
#define SEQ_GET_RMI(seq) RMI->m_data->item[seq]
#define GET_RMI(uid) SEQ_GET_RMI(RMI->GetIndex(uid))
#define UMI UserManager::Instance()
#define LMI LogicManager::Instance()
#define BMI BattleServer::Instance()

#include "Kernel.h"
#include "DataInc.h"
#include "ProtoInc.h"
#include "LogicManager.h"
#include "ProtoManager.h"
#include "UserManager.h"
#include "LogicResourceManager.h"
#include "LogicUserManager.h"
#include "LogicNotifyManager.h"
#include "LogicGameActivityManager.h"
#include "LogicRoutineManager.h"
#include "LogicBuildingManager.h"
#include "LogicOrderManager.h"
#include "ParseManager.h"
#include "LogicPropsManager.h"
#include "LogicProductLineManager.h"
#include "LogicQueueManager.h"
#include "LogicGM.h"
#include "LogicShopManager.h"
#include "LogicAdvertiseManager.h"
#include "LogicFriendManager.h"
#include "LogicShippingManager.h"
#include "LogicIdCtrl.h"
#include "LogicAllianceManager.h"

#include "TruckManager.h"
#include "LogicTaskManager.h"
#include "LogicVIPManager.h"
#include "LogicNPCSellerMgr.h"
#include "LogicNPCShopManager.h"
#include "LogicMissionManager.h"
#include "LogicCustomerManager.h"
#include "LogicRechargeActivity.h"
#include "LogicRandomBoxManager.h"
#include "LogicDailyShareRewards.h"
#include "LogicSignInRewards.h"
#include "LogicCropsRewards.h"
#include "LogicOrderRewards.h"
#include "LogicMailDogManager.h"
#include "LogicRotaryTable.h"
#include "LogicFriendlyTreeManager.h"
#include "LogicActivityTencent.h"
#include "LogicAccessAdManager.h"

#include "LogicThemeManager.h"
#include "LogicFundActivityManager.h"
#include "LogicCardManager.h"

#include "LogicDynamicInfoManager.h"
#include "LogicLocationHelpManager.h"
#include "LogicFriendOrderManager.h"
#include "LogicMessageBoardManager.h"
#include "LogicShopSellCoinManager.h"

#include "LogicKeeperManager.h"
#include "LogicFriendWorkerManager.h"
#include "LogicSysMail.h"
#include "LogicPetManager.h"
#include "LogicNewShareActivity.h"
#include "LogicXsgReportManager.h"

#include "LogicRecharge4399Activity.h"
#include "LogicDaily4399Activity.h"
#endif
