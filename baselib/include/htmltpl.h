/*!
 * \file htmltpl.h
 * \author FelitHuang
 * \version 1.0.0.1
 * \date    2005/12/20
*/


#ifndef    __HTMLTPL__
#define    __HTMLTPL__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "stresc.h"


/*!
 * \class CHtmlTpl
 * \brief 本类实现模板的基本操作实现页面的填充操作
 *
 * 本类实现模板的基本操作实现页面的填充操作
 * 其中提供了页面模板填充的基本操作
 * 有些数据由CGI参数中得到并需要输出到页面
 * 对于这些数据都必须是TCGIOutValue类型(安全CGI类型数据)
*/
class CHtmlTpl
{
public:

    /*! 
    \brief 定义一些返回码
    */
    enum ECODE { ECODE_SUCC=0x00, ECODE_FAIL=-1, ECODE_NOTPL=-2, ECODE_NOTOKEN=-3 };

public:

    /*! 
    \brief 构造函数,进行模板的读入初始化工作
    \param [in] sFileName: 模板文件名
    \param [in] sTokenHead: 替换数据标识前公共部分
    \param [in] sTokenTail: 替换数据标识后公共部分
    */
    CHtmlTpl(const std::string& sFileName="", const std::string& sTokenHead="<%$", const std::string& sTokenTail="$%>");

    /*! \brief 析构函数*/
    virtual ~CHtmlTpl();

public:

    /*! 
    \brief 获取初始化情况
    \return int [ 0--初始化成功  !0--初始化失败]
    */
    int CheckInit();
    
    /*! 
    \brief 设置新的模板文件
    \param [in] sFileName: 模板文件名
    \param [in] sTokenHead: 替换数据标识前公共部分
    \param [in] sTokenTail: 替换数据标识后公共部分
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int SetFile(const std::string& sFileName="", const std::string& sTokenHead="<%$", const std::string& sTokenTail="$%>");

protected:

    /*!
	\brief 获取文件模板,不对外使用
	*/
    int NewHtml();

    /*!
	\brief 释放文件模板,不对外使用
	*/
    int DelHtml();

public:

    /*! 
    \brief 获取新的模板块,模板块内容为开始标记到结束标记之间内容,返回模板块索引
    \param [in] sBegin: 新模板块开始标识(不包含该标识)
    \param [in] sEnd:   新模板块结束标识(不包含该标识)
    \param [in] hTpl:   从此索引的模板块中获取
    \return  int=[>0:新模板块索引][<0:获取新模板块失败]
    */
    int NewTpl(const std::string& sBegin, const std::string& sEnd, int hTpl=0);

    /*! \brief 删除模板块并释放其资源
    \param [in] hTpl: 模板块索引[>0]
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int DelTpl(int hTpl);

    /*! 
    \brief 删除所有模板块
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int ClnTpl();

public:

    /*! 
    \brief 直译模板,直接输出模板块内容,不进行填充
    \param [in] hTpl:   要输出的模板块索引[>=0]
    \param [in] ostm:   输出到的流对象
    \param [in] sBegin: 输出的开始标识(不包含该标识)
    \param [in] sEnd:   输出的结束标识(不包含该标识)
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int FillTpl(int hTpl, std::ostream& ostm=std::cout, const std::string& sBegin=std::string(), const std::string& sEnd=std::string());

    /*! 
    \brief 填充模板,用设置好的内容对模板块进行填充
    \param [in] hTpl: 要输出的模板块索引[>=0]
    \param [in] ostm: 要输出到的流对象
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int FillTplWithValue(int hTpl, std::ostream& ostm=std::cout);

public:

    /*! \brief 设置新的代替内容,把标识符换成实际内容
    \param [in] sToken: 标识符
    \param [in] oHtml:  内容
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int NewValue(const std::string& sToken, const CCgiOutValue& stHtml);

    /*! 
    \brief 删除所有设置了的替换内容
    如果输入的数据不再需要,请务必调用此函数删除
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int ClnValue();


protected:

    /*! 
    \brief 读入文件,不对外使用
    \return  int=[ECODE_SUCC:成功][...:失败]
    */
    int ReadFile();

protected:

    /*! \brief 文件名*/
    std::string m_sfName;

    /*! \brief 模板块列表,0块为整个文件模板块*/
    std::vector<std::string> m_vtpl;

    /*! \brief 标识符替换列表*/
    std::vector<std::string> m_vToken;

    /*! \brief 标识符替换内容*/
    std::vector<std::string> m_vValue;

protected:

    /*! \brief 标识符的公共开头,在原始页面中通常是<%$*/
    std::string m_sTokenHead;

    /*! \brief 标识符的公共结束,在原始页面中通常是$%>*/
    std::string m_sTokenTail;
};


/*!
 * \class CHtmlTplEx
 * \brief 本类是为了CHTMLTPL实现流操作符<<进行设计,可根据实际选择使用

 * 本类实现CHTMLTPL替换数据的流输入
 * 由于输入过程中需要进行类型的交替控制,流的实现不能用一般重载方式实现
 * 而是使用了一个内置类来实现
*/
class CHtmlTplEx : public CHtmlTpl
{
public:

    /*! \brief 构造函数*/
    CHtmlTplEx (const std::string& sFileName="", const std::string& sTokenHead="<%$", const std::string& sTokenTail="$%>");

    /*! \brief 拷贝构造函数*/
    CHtmlTplEx (const CHtmlTplEx& tplInit);


protected:

    /*! \brief 内置类用于类型限制*/
    class ISTRM
    {
    public:
        
        /*! \brief 流操作符用于输入内容*/
        CHtmlTplEx& operator << (const CCgiOutValue& _stValue);
        
        /*! \brief 赋值操作符-本对象一旦跟模板对象绑定就不能再更改*/
        ISTRM& operator = (const ISTRM&);
        
    public:
        /*! \brief 标识符*/
        const std::string* m_pToken;
        
        /*! \brief 模板对象指针,将由此对象执行操作*/
        CHtmlTplEx* m_ptpl;
    };

public:

    /*! \brief 流操作符用于输入标识符*/
    ISTRM& operator << (const std::string& sToken);

protected:

    /*! \brief 内置类对象*/
    ISTRM m_strm;
};

#endif    //*** __HTMLTPL__

