#ifndef _BASE_COMM_DEFINE_H_
#define _BASE_COMM_DEFINE_H_

#define PATH_XML_OPTFLAG			"/data/release/auto/conf/bmp_optflag.xml"
#define PATH_XML_WORKSHOP			"/data/release/auto/conf/workshop.xml"
#define PATH_XML_PRODUCT			"/data/release/auto/conf/pasture_product.xml"
#define PATH_XML_TOOL_PASTURE		"/data/release/auto/conf/pasture_tool.xml"
#define PATH_XML_TOOL                   "/data/release/auto/conf/tool.xml"
#define PATH_XML_ITEM                   "/data/release/auto/conf/item.xml"
#define PATH_XML_MEMCACHE               "/data/release/auto/conf/memcache.xml"
#define PATH_XML_FEEDS                  "/data/release/auto/conf/feeds.xml"
#define PATH_XML_FEEDS_NEW              "/data/release/auto/conf/newfeeds.xml"

#define PATH_XML_BOT_PASTURE            "/data/release/auto/conf/cgi/bot.xml"
#define PATH_XML_BOT                    "/data/release/auto/conf/cgi/bot.xml"
#define PATH_XML_FARMLOGICSRV           "/data/release/auto/conf/cgi/logicsrv.xml"
#define PATH_XML_FARM_HPAGE_LOGICSRV    "/data/release/auto/conf/cgi/hpage_logicserver.xml"
#define PATH_XML_FARM_OPT_LOGICSRV      "/data/release/auto/conf/cgi/opt_logicserver.xml"
#define PATH_XML_ONLINECOUNT            "/data/release/auto/conf/cgi/onlinecount.xml"
#define PATH_XML_ONLINECOUNT_PASTURE    "/data/release/auto/conf/onlinecount.xml"
#define PATH_XML_FARM_PLANT_LOGICSRV    "/data/release/auto/conf/cgi/plant_logicserver.xml"

#define PATH_XML_PASTURE_CGIENTER_LOGICSRV "/data/release/auto/conf/cgienter_logicserver.xml"
#define PATH_XML_PASTURE_CGIHELP_LOGICSRV "/data/release/auto/conf/cgihelp_logicserver.xml"
#define PATH_XML_PASTURE_CGISTEAL_LOGICSRV "/data/release/auto/conf/cgisteal_logicserver.xml"

#define PATH_XML_PASTURE_CGIGETEXP_BATCHDBC	"/data/release/auto/conf/cgigetexp_batchdbc.xml"

#define PATH_HTMLTPL_XIAOYOU_INVITE	"/data/release/auto/conf/cgi/invite_page.htm"
#define PATH_HTMLTPL_XIAOYOU_INVITELIMIT	"/data/release/auto/conf/cgi/invite_limit_page.htm"
#define PATH_HTMLTPL_XIAOYOU_MCINVITE "/data/release/auto/conf/cgi/mc_invite_page.htm"
#define PATH_HTMLTPL_XIAOYOU_MCINVITELIMIT	"/data/release/auto/conf/cgi/mc_invite_limit_page.htm"

#define PATH_XML_PASTURE_FLASHVERSION "/data/release/auto/conf/flashversion.xml"

#define PATH_XML_FARM_DBC_MEMUSER "/data/release/auto/conf/server/dbc_mem_user.xml"
#define PATH_XML_FLOWER "/data/release/auto/conf/cgi/flower.xml"
#define PATH_XML_STR4I_TIPS "/data/release/auto/conf/server/str4i_tips.xml"

#define PATH_XML_PASTURE_DEFLOCK	"/data/release/auto/conf/default_lock.xml"
#define LOG_CONF_PATH_NEW "/data/release/auto/conf/log_config.xml"
#define PATH_XML_RED_LAND	"/data/release/auto/conf/server/redland.xml"

#define PATH_XML_FARMCONF4I "/data/release/auto/conf/server/conf4i.xml"
#define PATH_XML_MODULEID4I "/data/release/auto/conf/server/moduleid4i.xml"
#define PATH_XML_CROP_FOR_SERVER "/data/release/auto/conf/server/crop_info.conf"


//1:空间 2:校友 3 手机空间 4 手机校友 0:未知
enum emDomain
{
	DOMAIN_UNKNOW=0,
	DOMAIN_DAWX=1

};

#endif

