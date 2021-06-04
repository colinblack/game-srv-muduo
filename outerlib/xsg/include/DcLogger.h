#include <map>
#include <list>
#include <fstream>
#include <thread>
#include <mutex> 

#include "model.h"
#include "FileLogger.h"

using namespace std;

class DcLogger
{
public:
	DcLogger() {};
	DcLogger(string appId);
	DcLogger(string appId, string rootDir);
	~DcLogger();
	void OnRoleLogin(LoginInfo *loginInfo);
	void OnRoleLogout(LogoutInfo *logoutInfo);
	void OnRoleRecharge(RechargeInfo *rechargeInfo);
	void OnVirtualCurrencyPurchase(VirtualCurrencyPurchaseInfo  *virtualCurrencyPurchaseInfo);
	void OnVirtualCurrencyReward(VirtualCurrencyRewardInfo *virtualCurrencyRewardInfo);
	void OnVirtualCurrencyConsume(VirtualCurrencyConsumeInfo *virtualCurrencyConsumeInfo);
	void OnTokenPurchase(TokenPurchaseInfo *tokenPurchaseInfo);
	void OnTokenReward(TokenRewardInfo *tokenRewardInfo);
	void OnTokenConsume(TokenConsumeInfo *tokenConsumeInfo);
	void OnRoleLeveUp(RoleLevelUpInfo *roleLevelUpInfo);
	void OnMissionBegin(MissionInfo *missionInfo);
	void OnMissionStartOrFinish(MissionInfo *missionInfo);
	void OnMissionFail(MissionInfo *missionInfo);
	void OnMissionSuccess(MissionInfo *missionInfo);
	void OnMissionReward(MissionRewardInfo *missionRewardInfo);
	void OnRoleTrade(TradeInfo *tradeInfo);
	void OnPowerConsume(PowerConsumeInfo *powerConsumeInfo);
	void OnCustomEvent(CustomEventInfo *customEventInfo);
	void OnMissionInfo(MissionInfo *missionInfo);
	void Start();
	void Quit();
	void Flush();
private:
	FileLogger fileLogger;
};
