/*!
 * \file sqltpl.h
 * \author FelitHuang
 * \version 1.0.0.1
 * \date    2005/12/20
*/


#ifndef    __CSQLTPL__
#define    __CSQLTPL__

#include <string>
#include <vector>

#include "stresc.h"



/*!
 * \class CSqlTpl
 * \brief 本类为保证数据库操作安全设计
 *
 * 本类为保证数据库操作安全设计
 * 主要目的是抛弃无类型检查的s  printf函数
 * 而是使用可进行类型检查与处理的类实现数据安全的保证
 * 利用模板填充生成SQL语句
 * 使用中用户需要操作一个SQL语句并把相关的填充值输入
 * 然后调用获取数据函数即可得到安全的SQL语句
*/
class CSqlTpl
{
public:

    /*!  \brief  定义一些返回码*/
    enum enCSqlTplErrorCode { ECODE_SUCC=0x00, ECODE_FAIL };

public:
    
    /*!  
    \brief  构造函数,进行模板的读入初始化工作
       注:如果SQL该位置确实是标识符内容,可输入同内容数据进行替换
    \param [in] sSQL:   SQL语句
    \param [in] sToken: 替换内容标识
    */
    CSqlTpl(const std::string& sSQL="", const std::string& sToken="[%]");

    /*!  \brief  析构函数*/
    virtual ~CSqlTpl();

public:

    /*!  
    \brief  用设置好的内容对模板块进行填充
     注:如果模板内容或填充内容已经改改,前次获取之内容失效
    \return  std:string=替换好的内容
    */
    const std::string& GetSQL();

    /*!  
    \brief  设置新的SQL源语句
    \param [in] sSQL:   SQL语句
    \param [in] sToken: 替换内容标识
    \return  返回本对象引用
    */
    CSqlTpl& SetSQL(const std::string& sSQL, const std::string& sToken="[%]");

    /*! 
    \brief  在已有的SQL语句末追加SQL语句
    \param [in] sSQL: 要追加的SQL语句
    \return  返回本对象引用
    */
    CSqlTpl& AppSQL(const std::string& sSQL);

    /*! 
    \brief  设置代替内容,把标识符换成实际内容
    \param [in] itSQL: 用于替换标识符的实际内容
    \return  返回本对象引用
    */
    CSqlTpl& NewValue(const CStrSqlValue& stSQL);

    /*! 
    \brief  删除所有设置了的替换内容,
     注:已经填充过的内容不会自动清除且每次填充时从已输入的第一个数据开始
     因此若旧的内容不再需要,必须调用本函数清除
    \return  返回本对象引用
    */
    CSqlTpl& ClnValue();

    /*!  
    \brief  设置代替内容,把标识符换成实际内容,功能同newValue()函数
    \param [in] itSQL: 用于替换标识符的实际内容
    \return  返回本对象引用
    */
    CSqlTpl& operator << (const CStrSqlValue& stSQL);

protected:

    /*!  \brief  源SQL语句*/
    std::string m_sSQLSrc;

    /*!  \brief  目标SQL语句*/
    std::string m_sSQLDst;

    /*!  \brief  替换标识*/
    std::string m_sToken;

    /*!  \brief  标识符替换内容*/
    std::vector<std::string> m_vValue;
};

#endif    //*** __CSQLTPL__
