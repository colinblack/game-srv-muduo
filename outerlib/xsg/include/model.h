#include <string>
#include <map>
using namespace std;

#ifndef MODEL__H
#define MODEL__H

struct BaseMsg
{
	/**
	* 消息协议的版本号
	*/
	string msgVersion = "3.0.0";

	/*
	*SDK语言类型
	*/
	string dataSdkLanguage = "C++";

	/*
	*数据上报SDK版本号
	*/
	string dataSdkVersion = "1.0.0";

	/**
	*数据来源， 固定值
	*/
	string datasource = "server";

	/**
	*消息时间戳
	*/
	long stime = 0;
	string timestamp;
	/**
	*游戏的AppID, 用于区分游戏
	*/
	string appId;
	/**
	*数据类型
	*/
	string msgType;
	/**
	*设备ID
	*/
	string deviceId;
	/**
	*服务器IP
	*/
	string serverIp;
	/**
	*扩展属性
	*/
	map<string, string> ext;
};



struct RoleInfo : public BaseMsg
{
	/**
	*账号ID
	*/
	string accountId;
	/**
	*角色ID
	*/
	string roleId;
	/**
	*角色名
	*/
	string roleName;
	/**
	*角色类型,如法师，道士，战士
	*/
	string roleType;
	/**
	*角色等级
	*/
	int roleLevel = 0;
	/**
	*角色vip等级
	*/
	int roleVipLevel = 0;
	/**
	*游戏区ID
	*/
	string zone;
	/**
	*游戏区名称
	*/
	string zoneName;
	/**
	*游戏服ID,示例:s1,s2
	*/
	string server;
	/**
	*游戏服名称,示例:风云争霸
	*/
	string serverName;
	/**
	*公会ID
	*/
	string partyId;
	/**
	*角色性别
	*/
	string gender;
	/**
	*战斗力
	*/
	uint64_t battleScore = 0;
	/**
	*渠道
	*/
	string channel;
	/*
	*西瓜channel
	*/
	string pxgksChannel;
	/**
	*渠道描述
	*/
	string channelDesc;
};


struct CustomEventInfo : RoleInfo
{
	/**
	* 事件ID
	*/
	string eventId;
	/**
	* 事件描述
	*/
	string eventDesc;
};

struct LoginInfo : RoleInfo
{
	/**
	* 操作系统:android/iso等
	*/
	string os;
	/**
	* 网络类型
	*/
	string network;
	/**
	* 应用自身版本号，如1.3.1,对应andriod:versionName
	*/
	string appVersion;
	/**
	* 应用自身版本号，如1,对应andriod:versionCode
	*/
	string appVersionCode;
	/**
	* 操作系统版本号：ios 6.1.3、android 4.3等
	*/
	string osVersion;
	/**
	* 渠道sdk的版本
	*/
	string sdkVersion;
	/**
	* 设备提供商： 小米、华为、三星、苹果
	*/
	string deviceBrand;
	/**
	* 设备型号：小米note、华为meta7 iphone 6 plus等
	*/
	string deviceModel;
	/**
	* 设备屏幕大小: 1024*920
	*/
	string deviceScreen;
	/**
	* mac地址
	*/
	string mac;
	/**
	* Android设备编号（android独有
	*/
	string imei;
	/**
	* UUID,用于唯一标示某台设备（android独有）
	*/
	string uuid;
	/**
	* 应用的包名（android独有）
	*/
	string packageName;
	/**
	* BuildNumber（android独有）
	*/
	string buildNumber;
	/**
	* 运营商：中国移动，中国联通等
	*/
	string carrier;
	/**
	* ICCID：Integrate circuit card identity 集成电路卡识别码即SIM卡卡号，相当于手机号码的身份证。
	*/
	string imsi;
	/**
	* 广告标示符（iOS版本在进行AppStore商店推广时才需要采集）
	*/
	string idfa;
	/**
	* 客户端IP地址
	*/
	string clientIp;
};

struct LogoutInfo : RoleInfo
{};

struct MissionInfo : RoleInfo
{
	/**
	* missionID
	*/
	string missionId;
	/**
	* mission名称
	*/
	string missionName;
	/**
	* mission类型：如活动，关卡，任务等
	*/
	string missionType;
	/**
	* mission标识：enter,success,failed等
	*/
	string missionFlag;
};


struct MissionRewardInfo : RoleInfo
{
	/**
	* missionID
	*/
	string missionId;
	/**
	* mission名称
	*/
	string missionName;
	/**
	* mission类型：如活动，关卡，任务等
	*/
	string missionType;
	/**
	* 虚拟货币金额
	*/
	uint64_t gold = 0;
	/**
	* 虚拟货币类型：如金币，银币，元宝等
	*/
	string virtualCurrencyType;
	/**
	* 玩家当前虚拟货币总量
	*/
	uint64_t virtualCurrencyTotal = 0;
	/**
	* 物品ID
	*/
	string itemId;
	/**
	* 物品名称
	*/
	string itemName;
	/**
	* 物品类型
	*/
	string itemType;
	/**
	* 物品数量
	*/
	int itemNum = 0;
};


struct PowerConsumeInfo : RoleInfo
{
	/**
	* missionID
	*/
	string missionId;
	/**
	* mission名称
	*/
	string missionName;
	/**
	* mission类型：如活动，关卡，任务等
	*/
	string missionType;
	/**
	* mission标识：enter,success,failed等
	*/
	string missionFlag;
	/**
	* 消耗活动力数量
	*/
	int consumeNum = 0;
	/**
	* 剩余活动力数量
	*/
	int remainNum = 0;
	/**
	* 活动力类型，如体力，精力等
	*/
	string powerType;
};

struct RechargeInfo : RoleInfo
{
	/**
	* "CNY", "KER", "HKD", "USD", "VND", "THB", "PHP"等币种标识
	*/
	string currency;
	/**
	* 充值金额，浮点数，入库去尾法保留2位小数,(单位:元)
	*/
	double money = 0;
	/**
	* 游戏充值定单号，服务端排重使用，避免重复定单
	*/
	string tradeNo;
	/**
	* 渠道充值定单号，服务端排重使用，避免重复定单
	*/
	string channelTradeNo;
};


struct RoleLevelUpInfo : RoleInfo
{};

struct TokenConsumeInfo : RoleInfo
{
	/**
	* 虚拟货币金额
	*/
	uint64_t gold = 0;
	/**
	* 虚拟货币类型：如金币，银币，元宝等
	*/
	string virtualCurrencyType;
	/**
	* 玩家当前虚拟货币总量
	*/
	uint64_t virtualCurrencyTotal = 0;
	/**
	* 是否绑定
	*/
	bool isBinding = 0;
	/**
	* 物品ID
	*/
	string itemId;
	/**
	* 物品名称
	*/
	string itemName;
	/**
	* 物品类型
	*/
	string itemType;
	/**
	* 物品数量
	*/
	int itemNum;
};

struct TokenPurchaseInfo : RoleInfo
{
	/**
	* 虚拟货币金额
	*/
	uint64_t gold;
	/**
	* 虚拟货币类型：如金币，银币，元宝等
	*/
	string virtualCurrencyType;
	/**
	* 玩家当前虚拟货币总量
	*/
	uint64_t virtualCurrencyTotal;
};


struct TokenRewardInfo : RoleInfo
{
	/**
	* 获得渠道
	*/
	string gainChannel;
	/**
	* 获得渠道类型
	*/
	string gainChannelType;
	/**
	* 虚拟货币金额
	*/
	uint64_t gold;
	/**
	* 虚拟货币类型：如金币，银币，元宝等
	*/
	string virtualCurrencyType;
	/**
	* 玩家当前虚拟货币总量
	*/
	uint64_t virtualCurrencyTotal;
	/**
	* 是否绑定
	*/
	bool isBinding;
};


struct TradeInfo : RoleInfo
{
	/**
	* 交易ID
	*/
	string tradeId;
	/**
	* 交易类型，如玩家间直接交易，拍卖行交易，摆摊交易等
	*/
	string tradeType;
	/**
	* 虚拟货币金额
	*/
	uint64_t gold;
	/**
	* 虚拟货币类型：如金币，银币，元宝等
	*/
	string virtualCurrencyType;  
	/**
	* 玩家当前虚拟货币总量
	*/
	uint64_t virtualCurrencyTotal;
	/**
	* 物品ID
	*/
	string itemId;
	/**
	* 物品名称
	*/
	string itemName;
	/**
	* 物品类型
	*/
	string itemType;
	/**
	* 物品数量
	*/
	int itemNum;
};


struct VirtualCurrencyConsumeInfo : TokenConsumeInfo
{};


struct VirtualCurrencyPurchaseInfo : TokenPurchaseInfo
{};

struct VirtualCurrencyRewardInfo : TokenRewardInfo
{};


struct LogItem
{
	long timeStamp;
	string logTime;
	char* logJson;
};

#endif