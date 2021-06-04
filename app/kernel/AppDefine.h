//AppDefine.h
//20110525	dada	create

#ifndef __APPDEFINE_H__
#define __APPDEFINE_H__

//config
#define APP_CONFIG_PATH						"../conf/app_config.xml"
#define APP_DOMAIN_CONFIG_PATH  			"../../pi/conf/domai_config.xml"

#define DEFAULT_APP_PATH					"/data/release/sg17/"
#define CONFIG_ALL_DATA_LOG_PATH			"datalog/"
#define CONFIG_DBC_SERVER					"pi/conf/dbc_server.xml"
#define CONFIG_STRING_FILTER_DATA 			"data/stringfilter.dat"
#define CONFIG_JSON_PATH		 			"pi/conf/json/"
#define CONFIG_MAP_PATH						"pi/conf/json/data.map"

#define DEFAULT_APP_CONFIG_PATH				"conf/app_config.xml"
#define CONFIG_DATA_LOG_PATH				"datalog/"
#define CONFIG_BUSINESS_LOG_DIR 			"log/business/"
#define CONFIG_LOG_PATH						"log/"
#define CONFIG_DATA_PATH 					"data/"
#define CONFIG_SHM_PATH 					"shm/"
#define CONFIG_ROOM_PATH 					"room/"
#define CONFIG_SAVE_PATH 					"save/"

#define CONFIG_LOG_LEVEL		"log_level"
#define CONFIG_LOG_NAME			"log_name"
#define CONFIG_LOG_REWIND		"log_rewind"
#define CONFIG_LOG_SIZE			"log_size"
#define CONFIG_LOG_RECORD_SIZE	"log_record_size"

#define CONFIG_WHITE_LIST		"white_list"
#define CONFIG_ADMIN_LIST		"admin_list"
#define CONFIG_SRVID "server_id"
#define CONFIG_OPENTIME "open_ts"
#define CONFIG_BASE "base"

//商会名称缓存文件
#define ALLIANCE_NAME_CACHE_FILE "alliance_name_cache.csv"
#define ALLIANCE_SEARCH_SHELL "pi/tools/FuzzyQueryAlliance.sh"

#define BASE_FORBID_REASON_LEN 32
#define BASE_NAME_LEN 32
#define BASE_FIG_LEN 128

//卡片id
#define MONTH_CARD_ID 8 //月卡
#define LIFE_CARD_ID 10 //终生卡


//延迟下线时间(秒)
#define OFFLINE_DELAY 60
#define TUTOR_MAX 55555

enum PlatformType
{
	PT_UNKNOW 		= -1,	//未知
	PT_TEST 		= 0,	//测试平台
	PT_FACEBOOK 	= 1,	//Facebook平台中文版
	PT_PENGYOU		= 2, 	//朋友平台
	PT_RENREN 		= 3,	//人人平台
	PT_QZONE 		= 4,	//QZONE平台
	PT_SINAWB 		= 5,	//新浪微博
	PT_WEIYOUXI 	= 6,	//新浪微游戏
	PT_KUAIWAN 		= 7,	//快玩平台
	PT_MIXI 		= 8,	//Mixi平台
	PT_TXWEIBOAPP 	= 9,	//腾讯微博应用平台
	PT_MOBAGE 		= 10,	//日本Yahoo!Mobage平台
	PT_TXWEIBO 		= 11,	//腾讯微博
	PT_KAIXIN 		= 12,	//开心网
	PT_FETION 		= 13,	//飞信
	PT_VN 			= 14,	//英文版自建平台
	PT_3366 		= 15,	//3366
	PT_qqgame		= 16,	//qq游戏大厅
	PT_TW			= 17, 	//台湾BSG
	PT_EN			= 18,	//FB英文版
	PT_DO			= 19,	//英文版联运
	PT_4399			= 20,	//4399联运
	PT_C9			= 21,	//C9
	PT_SOGOU		= 22,	//搜狗游戏联运平台
	PT_7477			= 23,   //7477联运
	PT_TX_C9		= 24,   //tx-c9
	PT_KW			= 25,  	//mix for PT_7477
	PT_7K			= 26,	//mix for PT_7477
	PT_360UU		= 27,	//mix for PT_7477
	PT_QD			= 28,	//mix for PT_7477
	PT_V1			= 29,	//mix for PT_7477
	PT_66YOU		= 30,	//mix for PT_7477
	PT_51IGAME		= 31,	//mix for PT_7477
	PT_HUANLEYUAN	= 32,	//mix for PT_7477
	PT_BAIDU	    = 33,	//baidu联运
	PT_51WAN	    = 34,	//
	PT_YUNQU	    = 35,	//
	PT_XUNLEI	    = 36,	//
	PT_TIEXUE	    = 37,	//
	PT_FENGHUANG    = 38,	//
	PT_7k7k		    = 39,	//
	PT_360		    = 40,	//
	PT_YLT		    = 41,	//
	PT_CYXC		    = 42,	//
	PT_WB		    = 43,	//手q玩吧
	PT_LIMI         = 44,   //厘米
	PT_WX           = 45,//微信
	PT_U9           = 46,   //u9
	PT_FB           = 47, //facebook
	PT_Mi2			= 48, //小米2
	PT_XMFOUR       = 49,   //小米4
	PT_XMZZ           = 50,   //小米赚赚
	PT_VIVO          = 51,  //vivo
	PT_OPPO          = 52,//oppo

	PT_MAX,
};

#define Is_QQ_Platform(Platform) (Platform == PT_PENGYOU || Platform == PT_QZONE || Platform == PT_3366 || Platform == PT_qqgame || Platform == PT_TX_C9)
#define ORDER_LENGTH 50

//农场地图
#define INT_BITS ((1<<3) * sizeof(int))
#define CHAR_BITS ((1<<3) * sizeof(char))
#define MAP_NOT_EXPAND_LENGTH (35)
#define MAP_EXPAND_LENGTH (105)
#define MAP_LENGTH (140)
#define MAP_WIDTH (35)
#define MAP_BASIC_GRID (MAP_WIDTH * MAP_NOT_EXPAND_LENGTH)
#define MAP_MAX_GRID (MAP_LENGTH * MAP_WIDTH)
#define EXPAND_MAP_MAX_GRID (MAP_EXPAND_LENGTH * MAP_WIDTH)

//障碍物长度，向上取整
#define BARRIER_LENGTH ((MAP_BASIC_GRID + CHAR_BITS - 1) / CHAR_BITS)
#define EXPAND_MAPGRID_LENGTH ((MAP_BASIC_GRID + CHAR_BITS - 1) / CHAR_BITS)
#define GRID_LENGTH ((MAP_MAX_GRID + INT_BITS - 1) / INT_BITS)
#define IsValidGridId(id) ( (id >= 1) && (id <= MAP_MAX_GRID))
#define EXPAND_MAPLAND_LENGTH ((EXPAND_MAP_MAX_GRID + CHAR_BITS - 1) / CHAR_BITS)

//DB
//对应DBC端口号和共享内存号为18000+ID
//注意，DB_ID_CTRL值应与cgiapp/kernel/appdefine.h中的宏值一致
#define DB_ID_CTRL			2
#define DB_EXCHANGE_CODE    4
#define DB_NAME				7
//全区联盟
#define DB_ALLIANCE_MAPPING	8
#define KEY_ALLIANCE_ID_CTRL 5

#define DB_BASE             101
#define DB_BASE_BUFF        5000
#define DB_BASE_FULL        1

#define DB_ITEM             110
#define DB_ITEM_FULL        200
#define DB_SHOP             127
#define DB_SHOP_FULL        100

#define DB_MAILDOG          144
#define DB_MAILDOG_FULL      20

#define DB_SHOPSELLCOIN          148
#define DB_SHOPSELLCOIN_FULL      31

#define DB_FRIENDWORKER          150
#define DB_FRIENDWORKER_FULL      500


#define DB_FRIENDLYTREE          145
#define DB_FRIENDLYTREE_FULL      50

#define DB_SYSMAIL 149  //系统邮件
#define DB_SYSMAIL_FULL 100

#define DB_FEEDBACK 151	//用户反馈
#define DB_FEEDBACK_FULL 500

#define DB_TASK             131
#define DB_TASK_FULL        500

#define DB_MISSION          143
#define DB_MISSION_FULL     500

#define DB_PET          152
#define DB_PET_FULL     100

#define DB_ORDER            111
#define DB_ORDER_FULL       10

#define DB_TRUCK            112
#define DB_TRUCK_FULL       1
//通用活动
#define DB_GAME_ACTIVITY 102
#define DB_GAME_ACTIVITY_FULL 50
//通用活动中活动数据的个数
#define DB_GAME_DATA_NUM 15
//充值历史
#define DB_CHARGE_HISTORY 103
#define DB_CHARGE_HISTORY_FULL 15

//建筑
#define DB_BUILD 121
#define DB_BUILD_FULL  1000
//地块生产
#define DB_CROPLAND  122
#define DB_CROPLAND_FULL 200
//生产设备
#define DB_PRODUCEEQUIP 123
#define DB_PRODUCEEQUIP_FULL 13
//动物生产
#define DB_ANIMAL 124
#define DB_ANIMAL_FULL 63
//设备星级
#define DB_EQUIPMENT_STAR 125
#define DB_EQUIPMENT_STAR_FULL 20
//果树生产
#define DB_FRUIT 126
#define DB_FRUIT_FULL 400
//关注
#define DB_CONCERN 128
#define DB_CONCERN_FULL 100
//粉丝
#define DB_FANS 129
#define DB_FANS_FULL 100
//援助记录
#define DB_AID_RECORD 130
//保存7天援助数据
#define MAX_AID_DAYS (7)
//每日最多保存10个援助用户
#define MAX_AID_USERS (10)
#define DB_AID_RECORD_FULL (MAX_AID_DAYS*MAX_AID_USERS)
//航运
#define DB_SHIPPING 132
//箱子
#define DB_SHIPPINGBOX 133
#define DB_SHIPPINGBOX_FULL 12
//农场助手
#define DB_KEEPER 146
#define DB_KEEPER_FULL 5
//农场助手任务
#define DB_KEEPER_TASK 147
#define DB_KEEPER_TASK_FULL 400
//商会
#define DB_ALLIANCE 134
#define DataAlliance_name_LENG 32
#define DataAlliance_description_LENG 210
#define DataAlliance_race_order_id_LENG 16
#define DataAlliance_race_rank_point_LENG 4
#define DataAlliance_race_week_point_LENG 4
#define DataAllianceMember_race_grade_reward_LENG 4
#define DataAllianceMember_race_stage_reward_LENG 32
#define DataAllianceMember_race_stage_reward_USED 27	// 有效长度
#define DataAllianceMember_race_order_log_LENG 16
#define DataAllianceMember_race_order_progress_LENG 4
#define DataAllianceMember_race_order_id_LENG 3
//商会成员
#define DB_ALLIANCE_MEMBER 135
#define DB_ALLIANCE_MEMBER_FULL 100
//商会申请列表
#define DB_ALLIANCE_APPLY 136
#define DB_ALLIANCE_APPLY_FULL 30
#define DataAlliance_apply_reason_LENG 64
//用户被邀请列表
#define DB_INVITED_LIST 137
#define DB_INVITED_LIST_FULL 10
//商会捐收
#define DB_ALLIANCE_DONATION 138
#define DB_ALLIANCE_DONATION_FULL 50
//商会通知
#define DB_ALLIANCE_NOTIFY 139
#define DB_ALLIANCE_NOTIFY_FULL 10
#define DataAllianceNotify_content_LENG 64

//npc商人
#define DB_NPCSELLER  140
#define DB_NPCSHOP  141
#define DB_NPCSHOP_FULL 6

#define DB_VIPGIFT  142
#define DB_VIPGIFT_FULL 6

#define MEM_SESSION			91
#define SESSION_DATA		"data"

#define MEMORY_RESOURCE       301
#define MEMORY_PROPERTY_NUM DB_BASE_BUFF*10

#define MEMORY_ASYN       302
#define MEMORY_ASYN_NUM   5

#define SPCIAL_NEW_USER 0

enum ASYN_TYPE
{
	e_asyn_cash				= 0,
	e_asyn_coin				= 1,
	e_asyn_accrecharge		= 2,
	e_asyn_exp				= 3,
	e_asyn_r1				= 4,
	e_asyn_r2				= 5,
	e_asyn_r3				= 6,
	e_asyn_r4				= 7,
	e_asyn_month_card       = 8,
	e_asyn_life_card       = 9,

	e_asyn_max				= 10000,
};

//通知系统
#define MEMORY_NOTIFY	303
#define NOTIFY_CONTENT_LEN 128
#define PER_USER_MAX_NOTIFY	 20

//广告系统
#define MEMORY_ADVERTISE	304
#define PER_USER_MAX_ADVERTISE 30

//商会
#define MEMORY_ALLIANCE	305
#define MEMORY_ALLIANCE_FULL DB_BASE_BUFF*10



// 商会竞赛分组
#define MEMORY_ALLIANCE_RACE_GROUP	306
#define MEMORY_ALLIANCE_RACE_GROUP_FULL (DB_BASE_BUFF*2)
#define MEMORY_ALLIANCE_RACE_GROUP_SIZE (15)

//活动共享内存大小
#define MEMORY_ACTIVITY	307
#define MEMORY_ACTIVITY_FULL 64

//动态消息
#define MEMORY_DYNAMIC_INFO 308
#define PER_USER_MAX_DYNAMIC_INFO 30	//每个玩家最多动态
#define MAX_PLAYER_SIZE_DY 50000		//单服承载人数
#define MAX_INTERNAL_DAYS 3				//共享内存动态消息淘汰算法最大间隔天数
#define DELETE_COUNT_PER_TIME 3000		//共享内存满，单次淘汰动态人数

//好友订单
#define MEMORY_FRIEND_ORDER 309
#define MAX_FRIEND_ORDER_NUM 100000
#define MAX_FO_BASKET_SIZE 3
#define FO_CD_OVER_TIME 4*3600-60
#define PER_USER_MAX_FRIEND_ORDER 30
#define SOURCE_FO_DELETE_TIME 10*86400

//cdKey
#define MEMORY_CD_KEY 312
#define MEMORY_CD_KEY_MAX 50000

//留言板
#define MEMORY_MESSAGE_BOARD 310
#define PER_USER_MAX_LEAVE_MSG 30
#define MAX_PLAYER_SIZE_MSG 20000
#define MAX_INTERNAL_DAYS_MSG 7
#define DELETE_COUNT_PER_TIME_MSG 1500

//矿洞
#define MEMERY_MINE 311
#define M_ROW_SIZE 5
#define M_LINE_SIZE 8
#define MAX_PLAYER_SIZE_MINE 40000


#define ALLIANCE_RACE_DEFAULT_TASK_STORAGE_ID 1	// 商会竞赛默认任务库ID
#define ALLIANCE_RACE_STAGE_REWARD_GROUP_SIZE 3	// 商会竞赛阶段奖励每组数量

#define ALLIANCE_RACE_GAME_TIME 518400	// 商会竞赛持续时长6天

#define MEMORY_SYSMAIL 324
//@end

#define ADMIN_UID			10000000
#define UID_MIN				10000000
#define UID_MAX				2000000000
#define UID_ZONE			500000
#define IsValidUid(uid)		(uid > UID_MIN && uid < UID_MAX)
#define ALLIANCE_ID_NULL	0
#define ALLIANCE_ID_MIN		2000000000
#define ALLIANCE_ID_START	2010000000
#define AID_ZONE			500000
#define IsAllianceId(id)	(id >= ALLIANCE_ID_MIN)


//error
#define KEY_RESULT			"error"
#define KEY_ERROR_MESSAGE	"errMsg"
#define KEY_PARAMS			"params"


#define BUFF_SIZE 2048


#define U64_LOW(val) (val & 0X00000000FFFFFFFF)
#define U64_HIGH(val) ((val >> 32) & 0X00000000FFFFFFFF)
#define U32_LOW(val) (val & 0X0000FFFF)
#define U32_HIGH(val) ((val >> 16) & 0X0000FFFF)
#define LOW_8(val) (val & 0X000000FF)
#define MAKE64(a,b,c) 	(*((uint64_t*)(&(a)))) = ((uint64_t)(b) << 32) | (c)

#define AddtoVct(filed)  if (0 != filed) {vctsubs.push_back(filed);}

//内部通讯命令字
#define PROTOCOL_EVENT_SECOND_TIMER 101
#define PROTOCOL_EVENT_BATTLE_CONNECT 102 //battle间通信
#define PROTOCOL_EVENT_BATTLE_FORWARD 103 //battle间转发给用户

enum ActivityID
{
	e_Activity_Recharge 	= 1, //首充活动
	e_Activity_DailyShare	= 2, //每日分享
	e_Activity_Crops		= 3, //农作物双倍
	e_Activity_Order       	= 4, //订单双倍
	e_Activity_SignIn		= 5, //签到
	e_Activity_RotaryTable  = 6, //转盘
	e_Activity_FriendlyTree = 7, //为避免base表过大,用户有些个人数据存档在这里(注:现阶段只用到15个字段中的前15个字段)
	e_Activity_Tencent  = 8, 	//qqgame活动
	e_Activity_Fund  = 9, //基金
	e_Activity_Theme  = 10, 	//付费主题
	e_Activity_UserData_1  = 11,//为避免base表过大,用户有些个人数据存档在这里(注:现阶段只用到此15个字段中的中的前13个)
	e_Activity_New_Share   = 12,//新的分享活动
	e_Activity_4399_Recharge = 13,//4399平台首冲翻倍活动
	e_Activity_4399_Daily = 15,//4399平台每日充值活动
	e_Activity_max
};
enum SigNum
{
	e_Sig_Save			= 64,
	e_Sig_data_log		= 63,
	e_Sig_try_clear		= 62,
	e_Sig_print_info	= 61, //打印resource_manager的信息
};
enum CurCMD
{
	e_CMD_none			= 0,
	e_CMD_new			= 1,
	e_CMD_login			= 2,

	e_CMD_max
};

enum NotifyID
{
	e_Notify_max
};

enum Build_Direct
{
	direct_right = 0,  //朝右
	direct_down  = 1,  //朝下
};

enum Truck_State
{
	Truck_Idle 	= 0,  		//空闲
	Truck_Transport  = 1,  	//运输中
	Truck_Arrival  = 2,  	//已经达到
};

enum BuildType
{
	build_type_corpland = 0,   //农地
	build_type_animal_residence = 1,  //动物住所
	build_type_animal			= 2,  //动物
	build_type_produce_equipment = 3,  //生产设备
	build_type_fruit_tree = 4,  //果树
	build_type_decorate = 5,   //装饰
	build_type_storage = 10,  //仓库
	build_type_house = 11,  //房子
	build_point_decorate = 12,//点券装饰物
};

enum StorageType
{
	type_granary = 1, //粮仓
	type_warehouse = 2, //货仓
};

//玩家属性类型
enum PropertyType
{
	type_coin			= 1,
	type_cash			= 2, //钻石
	type_exp			= 3,  //如果用户升级了会额外推送升级协议
	type_level			= 4,  //等级
	type_alliance_id 	= 5,  //商会id
	type_friend_value   = 6, //友情值
};

//生产线状态
enum ProduceStatus
{
	status_free 		= 0,  //空闲
	status_hungry 		= 0,  //饥饿
	status_growup 		= 1,  //成长
	status_procing		= 1, //生产中
	status_harvest 		= 2,  //收获
	status_suspend		= 2, //暂停生产
	status_full			= 2, //饱腹
	status_withered		= 3, //枯萎
	status_seek_help	= 4, //求助中
	status_already_help = 5,  //已援助
};

enum EquipmentStarProperty
{
	property_type_coin = 1,  //金币加成
	property_type_exp = 2,  //经验加成
	property_type_time = 3,  //生产时间缩短
};

//任务类型
enum TaskType
{
	task_of_min                   = 0,
	task_of_harvest_crops         = 1,//收获农作物
	task_of_harvest_cropsproduct  = 2,//收获农产品
	task_of_harvest_product       = 3,//收获产品
	task_of_harvest_fuit          = 4,//收集x果树果实y个
	task_of_build_building        = 5,//建造x建筑y个
	task_of_Focus_friend          = 6,//关注好友x个
	task_of_alive_fuit            = 7,//帮助救活果树x个
	task_of_start_cowcar          = 8,//发出牛车x次
	task_of_start_ship            = 9,//发出货船x次
	task_of_shop_coin             = 10,//商店赚取x金币
	task_of_shop_shelf            = 11,//商店开启货架x个
	task_of_create_ad             = 12,//发布广告x次
	task_of_product_device_cnt    = 13,//生产设备数量达x个
	task_of_product_device_star   = 14,//生产设备星级总数达x个
	task_of_plant_crops           = 15,//种植x农作物y个
	task_of_reward_order_coin     = 16,//领取订单金币奖励x个
	task_of_build_product         = 17,//建造x产品y个
	task_of_adopt_animal          = 18,//领养x动物y个

	task_of_max
};
enum AllianceRaceTaskType
{
	alliance_race_task_of_plant_crops           = 1,//种植x农作物y个
	alliance_race_task_of_cropsproduct        = 2,//收获任意动物生产产品y个
	alliance_race_task_of_harvest_product       = 3,//收获x生产产品y个
	alliance_race_task_of_start_cowcar          = 4,//卡车发车y次
	alliance_race_task_of_start_ship            = 5,//航运完成y趟
	alliance_race_task_of_feed_animal          = 6,//喂养任意动物y个
	alliance_race_task_of_help                  = 7,//帮助任务
};

//单向任务类型
enum MissionType
{
	mission_of_have_build            = 5,//拥有x建筑y个
	mission_of_start_cowcar          = 8,//发出牛车x次
	mission_of_create_ad             = 12,//发布广告x次
	mission_of_plant_product         = 15,//种植x农作物y个
	mission_of_product               = 17,//生产x产品y个
	mission_of_have_animal           = 18,//拥有x动物y个
	mission_of_level                 = 101,//达到x等级
};

//邮件狗更新信息
enum{
	update_trunk_cnt_daily = 1,//记录卡车发出次数
	update_visited_cnt_daily = 2,//记录被访问次数
	update_shop_income_daily = 3,//记录商店收入
	update_ship_cnt_daily   = 4,//记录航运发出次数
};

//定时任务类型
enum RoutineType
{
	routine_type_build 	= 1,  //建筑和各生产线
	routine_type_order	= 2,  //订单
	routine_type_truck 	= 3,  //卡车
	routine_type_ship 	= 4,  //航运
};
//加速消耗类型
enum RoutineSpeedUpType
{
	routine_speed_cash = 0,
	routine_speed_item = 1,
};

//物品类型
enum ItemType
{
	item_type_cropland  = 1,  //地块生产
	item_type_fruit		= 2,  //果树生产
	item_type_animal	= 3,  //动物生产
	item_type_produce	= 4, //设备生产
	item_type_other		= 5,  //其他
};

//成员职位类型
enum MemberPositionType
{
	pos_type_chief 		= 1, //会长
	pos_type_director 	= 2, //理事
	pos_type_committee  = 3, //委员
	pos_type_member 	= 4, //成员
	pos_type_none		= 5, //平民
};

//商会权限类型
enum PrivilegeType
{
	privilege_join_competition 		= 0x1, //参与竞赛
	privilege_donate		   		= 0x2, //捐收物品
	privilege_invite		   		= 0x4, //邀请入会
	privilege_approve		   		= 0x8, //批准入会
	privilege_edit_competition 		= 0x10, //竞赛编辑
	privilege_manipulate_committee  = 0x20, //升降委员
	privilege_broadcast_notice		= 0x40, //群发公告
	privilege_kick_out		 		= 0x80, //踢出成员
	privilege_manipulate_director	= 0x100, //升降理事
	privilege_transfer_chief		= 0x200, //转任会长
	privilege_edit_alliance 		= 0x400, //编辑商会
	privilege_del_race_order 		= 0x800, //撕毁商会竞赛订单
};

//商会捐收状态
enum DonationStatus
{
	donation_processing = 0,  //捐收中
	donation_finish = 1,  //捐收已完成
};

// 共享内存中的活动ID
enum MemoryActivityId
{
	memory_activity_id_alliance_race	= 1,	// 商会竞赛
	memory_activity_id_max	= MEMORY_ACTIVITY_FULL,
};
// 玩家标志位
enum BaseFlagId
{
	base_flag_id_follow_public_reward	= 1,	// 关注公众号奖励
	base_falg_id_is_have_month_card     = 2,    //是否拥有月卡
	base_falg_id_is_reward_month_card   = 3,    //当天是否已领过月卡奖励
	base_falg_id_is_have_life_card      = 4,    //是否拥有终身卡
	base_falg_id_is_reward_life_card    = 5,    //当天是否已领过终身卡奖励

	base_falg_id_fairy_speed_up_open    = 6,    //仙人系统开启
	base_falg_id_fairy_speed_up_crop    = 7,    //仙人农作物加速
	base_falg_id_fairy_speed_up_equip   = 8,    //仙人设备加速
	base_falg_id_fairy_speed_up_farm    = 9,    //仙人农场加速
	base_flag_id_is_send_promot_mail    = 10,   //玩家登录后是否收到过首次提醒邮件
	base_flag_id_is_send_recharge_mail    = 11,   //玩家登录后是否收到过首次充值提醒邮件
	base_flag_id_is_send_reward_mail_over = 12,   //小米奖励邮件是否已发送完毕
	base_flag_id_is_send_first_reward_mail = 13,   //小米第一天奖励邮件
	base_flag_id_is_send_second_mail = 14,   //小米第二天奖励邮件
	base_flag_id_is_send_third_mail = 15,   //小米第三天奖励邮件
};

enum BlueType
{
	TYPE_BLUE = 1, // 蓝钻贵族
	TYPE_LUXURY_BLUE = 2, // 豪华蓝钻
	TYPE_YEAR_BLUE = 3 // 年费蓝钻
};

enum DynamicInfoType
{
	TYPE_DY_VISIT  = 101,	//被其他玩家访问
	TYPE_DY_BUY    = 102,	//他人购买了你的商品
	TYPE_DY_SHIP   = 103,	//帮助装船
	TYPE_DY_TREE   = 104,	//帮助救树
	TYPE_DY_FULL   = 105,	//捐助已捐满
	TYPE_DY_MSG	   = 106,	//在他人庄园里留言了
	TYPE_DY_ANSWER = 107,	//他人回复了你的留言
	TYPE_DY_CONCERN= 108,	//他人关注了我
	TYPE_DY_WATER  = 109,	//帮好友浇水

	TYPE_DY_HELP_TREE = 201,
	TYPE_DY_HELP_SHIP = 202,
	TYPE_DY_HELP_WATER = 203,
	TYPE_DY_HELP_BUY = 204,
	TYPE_DY_INVITE_ALLIANCE = 205,

	TYPE_DY_HELP_TREE_DONE = 301,
	TYPE_DY_HELP_SHIP_DONE = 302,
	TYPE_DY_HELP_WATER_DONE = 303,
	TYPE_DY_HELP_BUY_DONE = 304,
	TYPE_DY_INVITE_ALLIANCE_DONE = 305
};

enum FriendOrderStatus
{
	STATUS_FO_FREE_BASKET = 0,
	STATUS_FO_WAIT_BUY = 1,
	STATUS_FO_OVER_DATE = 2,
	STATUS_FO_SELL_OUT = 3,
	STATUS_FO_FREEZE = 4,
	STATUS_FO_CAN_BUY = 5,
	STATUS_FO_OTHER_BOUGHT = 6,
	STATUS_FO_TIME_OUT = 7,
	STATUS_FO_UNDEFINED = 8,
	STATUS_FO_BUY_SUCCESS =9

};

enum LeaveMessageType
{
	TYPE_MSG_SECRET_SEND = 101,
	TYPE_MSG_SECRET_ANSWER = 102,

	TYPE_MSG_PUBLIC_SEND = 201,
	TYPE_MSG_PUBLIC_ANSWER = 202
};

#define IS_PUBLIC_MSG(type) \
	((type) == TYPE_MSG_PUBLIC_SEND \
	|| (type) == TYPE_MSG_PUBLIC_ANSWER)

#define IS_SECRET_MSG(type) \
	((type) == TYPE_MSG_SECRET_SEND \
	|| (type) == TYPE_MSG_SECRET_ANSWER)

#define IS_ANSWER_MSG(type) \
	((type) == TYPE_MSG_SECRET_ANSWER \
	|| (type) == TYPE_MSG_PUBLIC_ANSWER)

#define IS_UP_TOP_TYPE(type) \
	((type) >= TYPE_DY_HELP_TREE \
	&& (type) <= TYPE_DY_INVITE_ALLIANCE)

enum KeeperId
{
	KEEPER_ID_ANIMAL = 1,	// 动物助手
	KEEPER_ID_PRODUCT = 2,	// 产品助手
};
enum ProduceType
{
	PRODUCE_TYPE_MAN = 0,		// 手动生产
	PRODUCE_TYPE_KEEPER = 1,	// 助手生产
};
enum QueueBuildType	// 队列中的建筑类型
{
	QUEUE_BUILD_TYPE_UNKNOW = 0,	// 未知类型
	QUEUE_BUILD_TYPE_CROP = 1,	// 农地
	QUEUE_BUILD_TYPE_ANIMAL = 2,	// 农场
	QUEUE_BUILD_TYPE_PRODUCT_LINE = 3,	// 生产线
};
enum NewMsgType	// 新消息提醒类型
{
	NEW_MSG_TYPE_SYS_MAIL = 1,	// 系统邮件
};
//判断是否拥有某类权限
#define IsPrivilege(authority, type) ((authority & type) ? true : false)

//exchange code
#define CODE_SIZE 12
#define CODE_TYPE 32//0-31

#define CMI ConfigManager::Instance()

#endif	//__APPDEFINE_H__
