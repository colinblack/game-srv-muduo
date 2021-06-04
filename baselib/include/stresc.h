/*!
 * \file stresc.h
 * \author FelitHuang
 * \version 1.0.0.1
 * \date    2005/12/20
*/


#ifndef    __TSTRINGT__
#define    __TSTRINGT__


#include <string>


/*!
 * \class TSTRBase
 * \brief 过滤类字符串基类,提供一些接口和公共操作
*/
class CStrBase
{
protected:
    
    /*! \brief 构造函数:本类不能直接生成对象*/
    CStrBase();

    /*! \brief 析构函数*/
    virtual ~CStrBase();


public:

    /*! \brief 返回数据
     \return const std::string
     */
    virtual const std::string& Data() const;

protected:

    /*! 
    \brief 字符串多替换函数. 注:前面替换好的内容可能被后面内容替换
    \param [in] sData: 源目数据,内容将被修改
    \param [in] sSrc:  将被替换的子串数组
    \param [in] sDst:  用于替换子串的字符串数组
    \param [in] nlen:  数组元数个数
    */
    void ReplaceString(std::string& sData, const std::string* sSrc, const std::string* sDst, size_t nlen);

protected:

    /*! \brief 成员数据*/
    std::string m_Data;
};


/*!
 * \class CStrDirect
 * \brief 本类为直接字符串,请慎用.输出到页面中的内容非特殊情况不能使用本类型
*/
class CStrDirect : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrDirect(const char cInit);

    /*! \brief 构造函数*/
    CStrDirect(const std::string& sInit);
};


/*!
 * \class CStrHtml
 * \brief 本类为实现HTML转义设计
*/
class CStrHtml : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrHtml(const char cInit);

    /*! \brief 构造函数*/
    CStrHtml(const std::string& sInit);

protected:

    /*! \brief 转义函数*/
    void Translate();
};


/*!
 * \class CStrInput
 * \brief 本类为实现输出到INPUT输入框内容过滤设计
*/
class CStrInput : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrInput(const char cInit);

    /*! \brief 构造函数*/
    CStrInput(const std::string& sInit);

protected:

    /*! \brief 转义函数*/
    void Translate();
};


/*!
 * \class CStrScript
 * \brief 本类为实现SCRIPT脚本过滤设计
*/
class CStrScript : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrScript(const char cInit);

    /*! \brief 构造函数*/
    CStrScript(const std::string& sInit);

protected:

    /*! \brief 转义函数*/
    void Translate();
};


/*!
 * \class CStrTArea
 * \brief 本类为实现输出到TEXTAREA输入框内容过滤设计
*/
class CStrTArea : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrTArea(const char cInit);

    /*! \brief 构造函数*/
    CStrTArea(const std::string& sInit);

protected:

    /*! \brief 转义函数*/
    void Translate();
};

/*!
 * \class CStrXml
 * \brief 本类为实现输出到XML文档过滤设计
*/
class CStrXml : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrXml(const char cInit);

    /*! \brief 构造函数*/
    CStrXml(const std::string& sInit);

protected:

    /*! \brief 转义函数*/
    void Translate();
};

/*!
 * \class CStrUrl
 * \brief 本类为实现输出到HREF-SRC类数据内容过滤设计
*/
class CStrUrl : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrUrl(const char cInit);

    /*! \brief 构造函数*/
    CStrUrl(const std::string& sInit, const std::string& sProtocol="http://", const std::string& sDomain="show.qq.com");

protected:

    /*!
	 * \brief URL合法性检查函数
	 * \param[in] sInit: 待检查的字符串URL
	 * \param[in] sProtocol: 协议检查串 为空不检查 多个协议可以'|'分开 eg: http://|ftp://|file://
	 * \param[in] sDomain: 域名检查串 为空不检查 子域名也可通过检查
	*/
    int CheckUrl(const std::string& sInit, const std::string& sProtocol, const std::string& sDomain);

    /*! \brief 转义函数*/
    //void Translate();    //URL字符转义还需要知道具体环境,因些无法进行独立的转义
};


/*!
 * \class CStrSqlValue
 * \brief 本类为实现DB数据转义设计
*/
class CStrSqlValue : public CStrBase
{
public:

    /*! \brief 构造函数*/
    CStrSqlValue(const char cInit);

    /*! \brief 构造函数*/
    CStrSqlValue(const char* sInit, size_t nMaxlen=0);

    /*! \brief 构造函数*/
    CStrSqlValue(const std::string& sInit, size_t nMaxlen=0);

    /*! \brief 构造函数*/
    CStrSqlValue(int nInit);

    /*! \brief 构造函数*/
    CStrSqlValue(long nInit);

    /*! \brief 构造函数*/
    CStrSqlValue(size_t nInit);

    /*! \brief 构造函数*/
    CStrSqlValue(double dInit);

protected:

    /*! \brief 转义函数*/
    void Translate();
};


/*!
 * \class CCgiOutValue
 * \brief 本类为实现页面填充统一接口设计
 *
 * 本类用于实现页面输出参数的统一接口
 * 所有的CGI输出参数类型都应该是本类类型数据 
 * 本类提供CGI输出时需要的安全数据的转换接口
 * 这些接口都在重载的构造函数中实现
*/
class CCgiOutValue
{
public:

    /*! \brief 构造函数*/
    CCgiOutValue(int _init);

    /*! \brief 构造函数*/
    CCgiOutValue(double _init);

    /*! \brief 构造函数*/
    CCgiOutValue(const CStrBase& _init);

    /*! \brief 获取数据*/
    const std::string& Data() const;

protected:

    /*! \brief 数据成员*/
    std::string m_Data;
};


#endif    //***  __CSTRINGT__
