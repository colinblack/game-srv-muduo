/*!
 * \file cgiin.h
 * \author FelitHuang
 * \version 1.0.0.1
 * \date    2005-12-20
*/


#ifndef    __CGIIN__
#define    __CGIIN__

#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include "basetype.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define    QQSHOW_CHSET_MD5             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./"
#define    QQSHOW_CHSET_SEX             "MFU"
#define    QQSHOW_CHSET_AV              "-0123456789ABCDEFabcdef|_.#VvMFUmfu%"

#define    QQSHOW_CHSET_UPPERCASE       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define    QQSHOW_CHSET_LOWERCASE       "abcdefghijklmnopqrstuvwxyz"
#define    QQSHOW_CHSET_NUMBER          "0123456789"
#define    QQSHOW_CHSET_WORD            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"    // 即正则表达式中的\w
#define    QQSHOW_CHSET_SPACE           " \f\n\r\t\v"    // 即正则表达式中的\b
#define    QQSHOW_CHSET_URLSET          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-=:./?&!#$% *+|"

struct QQShowClientInfo
{
    std::string sRemoteAddress;
    std::string sRemotePort;
};

/*!
 * \class CCGIIn
 * \brief 本类实现CGI参数的输入并进行输入安全检查
*/
class CCGIIn
{
public:
    enum LIMIT_TYPE
    {
        CGI_INT_MIN    = -0x7FFFFFFF,        /*!< 整型默认最小值 */
        CGI_INT_MAX    = +0x7FFFFFFF,        /*!< 整型默认最大值 */
        CGI_INT_ERR    = CGI_INT_MIN-0x01,   /*!< 整型默认错误值 */
        CGI_CHR_ERR    = 0                  /*!< 字符默认错误值 */
    };
    enum ERROR_MASK
    {
        CGI_EM_SUC    = 0x00,            /*!< 成功值码 */
        CGI_EM_NUL    = 0x01,            /*!< 空值掩码 */
        CGI_EM_INV    = 0x02,            /*!< 非法值码 */
        CGI_EM_SET    = 0x04,            /*!< 字符集码 */
        CGI_EM_DIR    = 0x08,            /*!< 脏话掩码 */
		CGI_EM_SPC    = 0x10             /*!< 全空格值 */
    };


	static int CheckPost()
	{
		char* pcData = getenv("REQUEST_METHOD");
		return (NULL == pcData || std::string("POST") != pcData) ? -1 : 0;
	}

	static int CheckGet()
	{
		char* pcData = getenv("REQUEST_METHOD");
		return (NULL == pcData || std::string("GET") != pcData) ? -1 : 0;
	}

    /*!
     * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] nMin : 允许的最小值
	 * \param[in] nMax : 允许的最大值
	 * \param[in] nInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] nNull: 当参数值不存在或者为空时返回此值
	 * \return int [ nMin<=RET<=nMax OR REG=nInvalidate OR RET=nNull ]
    */
    static int       GetCGIInt(const std::string& sName, int nMin=CGI_INT_MIN, int nMax=CGI_INT_MAX, int nInvalidate=CGI_INT_ERR, int nNull=CGI_INT_ERR);

	/*!
     * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] dMin : 允许的最小值
	 * \param[in] dMax : 允许的最大值
	 * \param[in] dInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] dNull: 当参数值不存在或者为空时返回此值
	 * \return double [ dMin<=RET<=dMax OR REG=dInvalidate OR RET=dNull ]
	*/
    static double    GetCGIDbl(const std::string& sName, double dMin, double dMax, double dInvalidate, double dNull);

	/*!
	 * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \param[in] cInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] dNull: 当参数值不存在或者为空时返回此值
	 * \return char [ RET in sChSet OR REG=dInvalidate OR RET=dNull ]
	*/
    static char      GetCGIChr(const std::string& sName, const std::string& sChSet=std::string(""), char cInvalidate=CGI_CHR_ERR, char cNull=CGI_CHR_ERR);

	/*!
	 * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] bChkDirty: 是否进行脏话检查
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \param[in] bAllSpaceValid: 是否允许输入值全为空字符(QQSHOW_CHSET_SPACE)
	 * \return std::string [ 符合需要检查的内容时返回参数值,否则返回空串 ]
	*/	
    static const std::string    GetCGIStr(const std::string& sName, int nSize=0, bool bChkDirty=true, const std::string& sChSet=std::string(""), bool bAllSpaceValid=false);

	/*!
	 * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \return std::string [ 符合需要检查的内容时返回参数值,否则返回空串 ]
	*/	
    static const std::string    GetCGIFmtStr(const std::string& sName, int nSize=0, const std::string& sChSet=std::string(""));




    /*!
     * \brief 多值返回
	 * \param[in] sName: 参数名称
	 * \param[out] vStr: 返回值集合
	 * \param[in] nMin : 允许的最小值
	 * \param[in] nMax : 允许的最大值
	 * \param[in] nInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] nNull: 当参数值不存在或者为空时返回此值
    */
    static void        GetCGIIntMul(const std::string& sName, std::vector<int>& vInt, int nMin=CGI_INT_MIN, int nMax=CGI_INT_MAX, int nInvalidate=CGI_INT_ERR, int nNull=CGI_INT_ERR);

	/*!
	 * \brief 多值返回
	 * \param[in] sName: 参数名称
	 * \param[out] vInt: 返回值集合
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	*/	
    static void        GetCGIStrMul(const std::string& sName, std::vector<std::string>& vStr, int nSize=0, bool bChkDirty=true, const std::string& sChSet=std::string(""), bool bAllSpaceValid=false);

	/*!
	 * \brief 多值返回
	 * \param[in] sName: 参数名称
	 * \param[out] vStr: 返回值集合
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	*/	
    static void        GetCGIFmtStrMul(const std::string& sName, std::vector<std::string>& vStr, int nSize=0, const std::string& sChSet=std::string(""));

	/*!
	 * \brief 获取UIN
	 * \param[in] sName: 参数名称
	 * \return[int] (UIN OR (<0)=ERROR)
	*/
	static int GetCGIUin(const std::string& sName);
	
	/** 
	 * @brief （通过环境变量HTTP_QVIA或REMOTE_ADDR，取得来源IP）
	 *
	 * @return  成功返回IP(主机字节序的int)，否则返回-1
	 */
	static unsigned GetRemoteIP();
	static char *GetStrRemoteIP();

public:
	
    /*!
     * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] nMin : 允许的最小值
	 * \param[in] nMax : 允许的最大值
	 * \param[in] nInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] nNull: 当参数值不存在或者为空时返回此值
	 * \return int [ nMin<=RET<=nMax OR REG=nInvalidate OR RET=nNull ]
    */
    static int        GetCookieInt(const std::string& sName, int nMin=CGI_INT_MIN, int nMax=CGI_INT_MAX, int nInvalidate=CGI_INT_ERR, int nNull=CGI_INT_ERR);

	/*!
     * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] dMin : 允许的最小值
	 * \param[in] dMax : 允许的最大值
	 * \param[in] dInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] dNull: 当参数值不存在或者为空时返回此值
	 * \return double [ dMin<=RET<=dMax OR REG=dInvalidate OR RET=dNull ]
	*/
    static double    GetCookieDbl(const std::string& sName, double dMin, double dMax, double dInvalidate, double dNull);

	/*!
	 * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \param[in] cInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] dNull: 当参数值不存在或者为空时返回此值
	 * \return char [ RET in sChSet OR REG=dInvalidate OR RET=dNull ]
	*/
    static char        GetCookieChr(const std::string& sName, const std::string& sChSet=std::string(""), char cInvalidate=CGI_CHR_ERR, char cNull=CGI_CHR_ERR);

	/*!
	 * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] bChkDirty: 是否进行脏话检查
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \param[in] bAllSpaceValid: 是否允许输入值全为空字符(QQSHOW_CHSET_SPACE)
	 * \return std::string [ 符合需要检查的内容时返回参数值,否则返回空串 ]
	*/	
    static const std::string    GetCookieStr(const std::string& sName, int nSize=0, bool bChkDirty=true, const std::string& sChSet=std::string(""), bool bAllSpaceValid=false);
	
	/*!
	 * \brief 单值返回
	 * \param[in] sName: 参数名称
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \return std::string [ 符合需要检查的内容时返回参数值,否则返回空串 ]
	*/	
    static const std::string    GetCookieFmtStr(const std::string& sName, int nSize=0, const std::string& sChSet=std::string(""));


public:
    /*!
     * \brief 客户端信息获取
     * \param[out] 客户端信息结构
     * \return int [ 0--succ  !0--fail ]
    */
    static int GetClientInfo(QQShowClientInfo& stClientInfo);

    /*!
     * \brief 获取错误状态值
     * \return int [ 0--succ 其它值可能由各种错误掩码合成 可与ERROR_MASK码测试 ]
    */
    inline static unsigned int GetErrorStatus()
    {
		return m_uiStatus;
    }

    /*!
     * \brief 检查空值状态
     * \return int [ 0--succ ]
    */
    inline static int CheckStatusNull()
    {
		return (m_uiStatus & CGI_EM_NUL) ? (-1) : (0);
    }

    /*!
     * \brief 检查值有效状态
     * \return int [ 0--succ ]
    */
    inline static int CheckStatusInvalid()
    {
		return (m_uiStatus & CGI_EM_INV) ? (-1) : (0);
    }

    /*!
     * \brief 检查字符集检查状态
     * \return int [ 0--succ ]
    */
    inline static int CheckStatusSetChk()
    {
		return (m_uiStatus & CGI_EM_SET) ? (-1) : (0);
    }

    /*!
     * \brief 检查脏话过滤状态
     * \return int [ 0--succ ]
    */
    inline static int CheckStatusDirty()
    {
		return (m_uiStatus & CGI_EM_DIR) ? (-1) : (0);
    }

    /*!
     * \brief 检查全空格状态
     * \return int [ 0--succ ]
    */
    inline static int CheckStatusSpace()
    {
		return (m_uiStatus & CGI_EM_SPC) ? (-1) : (0);
    }

protected:
	
    /*!
	 * \brief 整型值检查
	 * \param[in] sValue: 值
	 * \param[in] nMin : 允许的最小值
	 * \param[in] nMax : 允许的最大值
	 * \param[in] nInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] nNull: 当参数值不存在或者为空时返回此值
	 * \return int [ nMin<=RET<=nMax OR REG=nInvalidate OR RET=nNull ]
    */
    static int CheckInt(const std::string& sValue, int nMin, int nMax, int nInvalidate, int nNull);

	/*!
	 * \brief 浮点值检查
	 * \param[in] sValue: 值
	 * \param[in] dMin : 允许的最小值
	 * \param[in] dMax : 允许的最大值
	 * \param[in] dInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] dNull: 当参数值不存在或者为空时返回此值
	 * \return double [ dMin<=RET<=dMax OR REG=dInvalidate OR RET=dNull ]
	*/
    static double CheckDbl(const std::string& sValue, double dMin, double dMax, double dInvalidate, double dNull);

	/*!
	 * \brief 字符值检查
	 * \param[in] sValue: 值
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \param[in] cInvalidate: 当参数值不适合要求时返回此值
	 * \param[in] dNull: 当参数值不存在或者为空时返回此值
	 * \return char [ RET in sChSet OR REG=dInvalidate OR RET=dNull ]
	*/
    static char    CheckChr(const std::string& sValue, const std::string& sChSet, char cInvalidate, char cNull);

	/*!
	 * \brief 字符串检查
	 * \param[in] sValue: 值
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] bChkDirty: 是否进行脏话检查
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \param[in] bAllSpaceValid: 是否允许输入值全为空字符(QQSHOW_CHSET_SPACE)
	 * \return std::string [ 符合需要检查的内容时返回参数值,否则返回空串 ]
	*/	
    static const std::string CheckStr(const std::string& sValue, int nSize, const std::string& sChSet, bool bChkDirty, bool bAllSpaceValid);
	
	/*!
	 * \brief 格式字符串检查
	 * \param[in] sValue: 值
	 * \param[in] nSize: 限制返回字符串最大长度,为零则不限制.(注:由于进行了双字节字检查,返回长度可能比限制值小一)
	 * \param[in] sChSet: 检查参数值是否在此字符集内,为空时不进行检查
	 * \return std::string [ 符合需要检查的内容时返回参数值,否则返回空串 ]
	*/	
    static const std::string CheckFmtStr(const std::string& sValue, int nSize, const std::string& sChSet);

public:

    /*!
     * \brief 检查数据是否在指定字符集内
     * \param[in] sValue: 待检查的数据
     * \param[in] sChSet: 字符集
	 * \return int [ 0--在字符集内 !0--不在字符集内 ]
    */
    static int CheckInSet(const std::string& sValue, const std::string& sChSet);
public:

    /*!
     * \brief 脏话过滤
     * \return int [ 0--succ  !0--fail ]
    */
    static int CheckDirty(const std::string& sValue);


protected:

    /*!
     * \brief 最后操作错误码
    */
    static unsigned int m_uiStatus;
};

#endif    //*** __CGIIN__
