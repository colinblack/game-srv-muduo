/*!
 * \file cookie.h
 * \author FelitHuang
 * \date 2006-01-13
*/


#ifndef    COOKIE_H_
#define    COOKIE_H_

#include <string>
#include <vector>
#include "fcgi_stdio.h"

class CHeader
{
public:

	/*!
	 * \brief 输出HTML头
	 * \remark
	 -------------------
	 HTML页面头以"\n\n"作为所有头输出结束的标志
	 在此之后输出的所有头信息都将作为正常页面内容输出
	 -------------------
	*/
	static int ScriptHeader(const std::string& chSet="gb2312", int iExpire=0);

    static int HtmlHeader(const std::string& chSet="gb2312", int iExpire=0);

    static int XMLHeader(const std::string& chSet="gb2312", int iExpire=0);

	static int LocationHeader(const std::string& sLocation, const std::string& chSet="gb2312", int iExpire=0);

    /*!
    * \brief Pragma: sText
    */
    static int Pragma(const std::string& sText);
};

class CCookie
{
public:

    /*!
    * \brief 设置Cookie
	* \param[in] sName:  Cookie名称
	* \param[in] sValue: Cookie值
	* \param[in] iExpireSec: 过期时间[==0时不进行设置]
	* \param[in] sDomain: 所属域
	* \param[in] sPath: 所属路径
	* \param[in] bSecure: 安全性
	* \return int [ 返回0 ]
    */
    static int SetCookie(const std::string& sName, const std::string& sValue, int iExpireSec, const std::string& sDomain, const std::string& sPath, bool bSecure);

    /*!
    * \brief 获取Cookie
	* \param[in] sName:  Cookie名称
	* \return std::string [ 返回相应的值,如果该值不存在则返回空串 ]
    */
    static const std::string& GetCookie(const std::string& sName);

    /*!
    * \brief 检查是否存在(一般不需要检查)
	* \param[in] sName:  Cookie名称
    * \return int [ 0--存在 !0--不存在 ]
    */
    static int CheckExists(const std::string& sName);

    /*!
    * \brief 清除Cookie
	* \param[in] sName:  Cookie名称
	* \param[in] sDomain: 所属域
	* \param[in] sPath: 所属路径
	* \param[in] bSecure: 安全性
    */
    static int ClearCookie(const std::string& sName, const std::string& sDomain, const std::string& sPath, bool bSecure);


protected:

	/*!
	 * \class CData CCookie数据类,对数据进行封装与管理
	*/
    class CData
    {
    public:
		/*!
		 * \class CItem CCookie数据结构
		*/
        struct CItem
        {
            std::string sName;	//!< 名称
            std::string sData;	//!< 值
        };
    public:
        CData();
        std::vector<CItem> m_vData;	//!< 值对列表
    };
protected:
	/*!
	 * \brief 通过此函数进行数值列表的管理
	*/
    static CData& GetData();
};


#endif
