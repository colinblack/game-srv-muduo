#include <map>
#include <list>
#include <fstream>
#include <thread>
#include <mutex> 

#include "model.h"
#include "cJSON.h"

using namespace std;

class FileLogger
{
public:
	FileLogger();
	~FileLogger() {};
	//FileLogger(string appId);	
	void Start();
	void Quit();
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
	void SetAppId(string appId);
	void SetRootDir(string rootDir);
	void Flush();
private:
	bool mQuit = false;
	bool mComplete = false;
	int mTimeInterval = 15 * 60;
	int mFlushInterval = 3;
	long mLastFileTime = 0;
	long mLastFlushTime = 0;
	string mLastFileDate;
	list<LogItem> mLogList;
	ofstream mLogFile;
	string mAppId;
	string mRootDir;
	mutex mMutex;
	string mLocalIp;	

	bool WriteToFile();
	cJSON* OnRoleInfo(RoleInfo *roleInfo);
	void OnMissionInfo(MissionInfo *missionInfo);
	void AddStringToObject(cJSON* root, string key, string &value);
	bool AddLog(cJSON *root);
	bool OpenFile(long timeStamp);
	int CreateDir(char* path);
	void Run();
	int strcmp(const char *str1, const char *str2);
	const char* getLocalIp();
	char* RandomUUID(char buf[37]);
};
