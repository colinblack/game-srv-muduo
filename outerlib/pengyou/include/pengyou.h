/**
 * C++ SDK for pengyou Open API
 *
 * @version 1.0.0
 * @author dev.opensns@qq.com
 * @copyright ? 2010, Tencent Corporation. All rights reserved.
 */
#ifndef __QZONG_PENGYOU_OPENAPI__
#define __QZONG_PENGYOU_OPENAPI__

#include <string>
#include <vector>

/**
 * API(cgi)响应消息体中的响应码，其值分为3个区间:
 * 0:正确返回
 * (0,1000):部分数据获取错误，相当于容错的返回
 * [1000,~):完全错误，数据不可用
 */
#define API_RETURE_ERROR_CODE_BASE       1000

using namespace std;

/**
 * T_UserInfo - 用户登录信息
 */
typedef struct __UserInfo
{
    string         strNickName;      /* 昵称 */
    string         strGender;        /* 性别 */
    string         strProvince;      /* 省份 */
    string         strCity;          /* 城市 */
    string         strFigureUrl;     /* 头像url */
    bool           bIsVip;           /* 是否为黄钻用户(true表示是，下面类同) */
    bool           bIsYearVip;       /* 是否为年度黄钻用户(bIsVip为true时有效) */
    int            nVipLevel;        /* 黄钻等级(bIsVip为true时有效) */
} T_UserInfo;

/**
 * T_RelationStat - 好友关系状态
 */
typedef struct __RelationStat
{
    int            nIsFriend;        /* 是否为好友(是:true; 否:false) */
} T_RelationStat;

/**
 * T_RelationInfo - 好友关系链信息
 */
typedef struct __RelationInfo
{
    typedef struct RelationItemInfo
    {
        string     strOpenId;        /* 好友QQ号码转化得到的id */
        string     strNickName;      /* 昵称 */
        string     strGender;        /* 性别 */
        string     strFigureUrl;     /* 头像url */
    } T_RelationItemInfo;
    typedef vector<RelationItemInfo> RIIVector;
    RIIVector      vecItem;          /* 好友信息集 */
} T_RelationInfo;

/**
 * T_MultiInfoInfo - 好友的详细信息
 */
typedef struct __MultiInfoInfo
{
    typedef struct FriendItemInfo
    {
        string     strOpenId;        /* 与APP通信的用户key，它和QQ号码一一对应 */
        string     strNickName;      /* 昵称 */
        string     strGender;        /* 性别 */
        string     strFigureUrl;     /* 好友头像url */
        bool       bIsVip;           /* 该好友是否为黄金用户 */
        bool       bIsYearVip;       /* 该好友是否为年度黄钻用户 */
        int        nVipLevel;        /* 黄钻等级 */
    } T_FriendItemInfo;
    typedef vector<FriendItemInfo> FIIVector;
    FIIVector      vecItem;          /* 好友信息集 */
} T_MultiInfoInfo;

/**
 * T_AppSetupStatus - 指定应用的安装情况
 */
typedef struct __AppSetupStatus
{
    int            nSetupStat;       /* 是否安装(0:没有安装；1:安装) */
} T_AppSetupStatus;

/**
 * T_VipStatInfo - 用户的黄钻身份状态
 */
typedef struct __VipStatInfo
{
    bool           bIsVip;           /* true为黄钻; false为否 */
} T_VipStatInfo;

/**
 * T_EmotionInfo - 好友签名信息
 */
typedef struct __EmotionInfo
{
    struct EmotionItemInfo
    {
        string     strContent;       /* 内容 */
        string     strOpenId;        /* 好友的openid */
    };
    typedef vector<EmotionItemInfo> EIIVector;
    EIIVector      vecItem;          /* 签名信息集 */
} T_EmotionInfo;

class CPengyou
{
public:
	enum QQPlatformType
	{
		QPT_PENGYOU = 0,
		QPT_QZONE = 1,
		QPT_WEIBO = 2,
	};

private:
    QQPlatformType m_type;
    string          m_ErrMsg;
    string          m_Appid;
    string          m_Appkey;
    string          m_Appname;
    vector<string>  m_IpVector;

private:
   /******************************************************************************
    * request - 执行Http请求
    *
    * 输入参数: url - 执行请求的URL
    *           querystr - 表单参数
    * 输出参数: apires - 接收服务端响应数据
    * 返 回 值: 0 成功; 否则失败
    ******************************************************************************/
    int request(string & apires, const string & url, const string & querystr);

   /******************************************************************************
    * getApiUrl - 通过API(cgi)名字，获取请求的URL
    *
    * 输入参数: method - API名字
    * 输出参数: url - 执行请求的URL
    * 返 回 值: 0 成功; 否则失败
    ******************************************************************************/
    int getApiUrl(string & url, const string & method);

    inline string & buildHttpQuery(string & querystr,
                                   const string & openid, const string & openkey);

   /******************************************************************************
    * responesWriteCallback - 用于接收服务端响应数据的回调接口(由cURL负责调用)
    *
    * 输入参数: ptr - 数据buf
    *           size - 数据块大小
    *           nmemb - 数据块个数(数据总量为size*nmemb)
    *           userdata - 外层透传过来的参数，通常设置为外部接收buf
    * 输出参数: 无
    * 返 回 值: >=0 实际接收的数据量
    ******************************************************************************/
    static size_t responesWriteCallback(void *ptr, size_t size,
                                        size_t nmemb, void *userdata);

   /******************************************************************************
    * jsonDecode - Json串解析接口
    *
    * 输入参数: jsonstr - 待解析的Json串
    * 输出参数: value - 存放解析结果
    * 返 回 值: 0 成功; 否则失败
    ******************************************************************************/
    template<typename T>
    int jsonDecode(T & value, const string & jsonstr);

    typedef enum __PY_ErrCode
    {
        E_PY_SUC               = 0,
        E_PY_PARAM_INVALID     = 1,         /* 参数错误 */
        E_PY_GET_URL_FAIL      = 2,         /* 获取api的url失败 */
        E_PY_CURL_INIT_FAIL    = 3,         /* cURL初始化失败 */
        E_PY_CURL_PERFORM_FAIL = 4,         /* 执行http请求失败 */
        E_PY_SRV_NO_RESPONSE   = 5,         /* API无响应 */
        E_PY_JSON_DECODE_FAIL  = 6,         /* JSON解析错误 */
        E_PY_API_RESPONSE_ERR  = 7,         /* API返回出错 */
        E_PY_UNKOWN_ERROR      = 8,         /* 其他 */
        E_PY_CODE_COUNT
    } E_PY_ErrCode;

    static const char * m_ErrMsgArray[];

    inline void setErrorMsg(E_PY_ErrCode code);
    inline void setErrorMsg(const char * msg);
    inline void setErrorMsg(const string & msg);
public:
    CPengyou(QQPlatformType type,
    		const string & appid,
            const string & appkey,
            const string & appname,
            const vector<string> & domainvec);

    CPengyou(QQPlatformType type,
    		const string & appid,
            const string & appkey,
            const string & appname,
            const string & domain);

    ~CPengyou(){};

    const string & GetErrorMessage();

    const string & GetAppId() const {return m_Appid;}
    const string & GetAppKey() const {return m_Appkey;}
    const string & GetAppName() const {return m_Appname;}

    void AddDomain(const string & domain)
    {
        if (!domain.empty()) {m_IpVector.push_back(domain);}
    }

    void ClearDomainSets() { m_IpVector.clear(); }

   /******************************************************************************
    * GetUserInfo - 返回当前登录用户信息
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    * 输出参数: tUserinfo - 存放用户信息
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int GetUserInfo(T_UserInfo & tUserinfo,
                    const string & openid, const string & openkey);

   /******************************************************************************
    * IsFriend - 验证是否好友(验证 fopenid 是否是 openid 的好友)
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    *           fopenid - 好友的openid
    * 输出参数: tRelationSt - 查询结果
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int IsFriend(T_RelationStat & tRelationSt,
                 const string & openid, const string & openkey,
                 const string & fopenid);

   /******************************************************************************
    * GetFriendList - 获取好友列表
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    *           nInfoDetails - 是否需要好友的详细信息(0:不需要;1:需要)
    *           nAppSetupStat - 好友应用安装类型(-1 表示返回没有安装此应用的好友;
    *                           1 表示返回安装了此应用的好友; 0 表示返回所有好友)
    *           nPage - 获取对应页码的好友列表，从1开始算起，每页是100个好友。(不传或者0：返回所有好友;>=1，返回对应页码的好友信息)
    * 输出参数: tRelationInfo - 存放用户好友关系信息
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int GetFriendList(T_RelationInfo & tRelationInfo,
                      const string & openid, const string & openkey,
                      int nInfoDetails = 0, int nAppSetupStat = 0, int nPage = 1);

   /******************************************************************************
    * GetMultiInfo - 获取好友详细信息
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    *           fopenids - 好友的openid列表, 多个需以"_"(下划线)分隔
    * 输出参数: tMultiInfoInfo - 存放用户好友的信息
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int GetMultiInfo(T_MultiInfoInfo & tMultiInfoInfo,
                         const string & openid, const string & openkey,
                         const string & fopenids);

   /******************************************************************************
    * IsSetup - 验证登录用户是否安装了应用
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    * 输出参数: tAppSetupStat - 查询结果，存放应用安装状态
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int IsSetup(T_AppSetupStatus & tAppSetupStat,
                const string & openid, const string & openkey);

   /******************************************************************************
    * IsVip - 判断用户是否为黄钻
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    * 输出参数: tVipStat - 查询结果，是否为黄钻用户
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int IsVip(T_VipStatInfo & tVipStat,
              const string & openid, const string & openkey);

   /******************************************************************************
    * GetFriendEmotion - 获取好友的签名信息
    *
    * 输入参数: openid - 用户标识ID
    *           openkey - 登录态标识
    *           fopenids - 需要获取数据的openid列表(一次最多20个),多个openid中间以_隔开，最多100个openid
    * 输出参数: tEmotionInfo - 存放好友的签名信息
    * 返 回 值: 0 成功; 否则失败，错误信息见GetErrorMessage();
    ******************************************************************************/
    int GetFriendEmotion(T_EmotionInfo & tEmotionInfo,
                         const string & openid, const string & openkey,
                         const string & fopenids);

};

#endif // End for __QZONG_PENGYOU_OPENAPI__

