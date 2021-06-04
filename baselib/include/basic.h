#ifndef _BASIC_H_
#define _BASIC_H_

/*!
 * \file basic.h
 * \author FelitHuang
*/

#include <string>
#include <vector>
#include <time.h>
#include "csconv.h"

/*!
 * \brief 基础库类，实现一些公共函数，一般以静态成员方式提供
*/
using namespace std;
class CBasic
{
public:
	/*! <QQ号范围*/
	enum enQQRange
	{
		UIN_MIN		=	10000,
		UIN_MAX		=	1999999999,
		UIN_BEGIN	=	10000,
		UIN_END		=	2000000000
	};
	
    /*! <MD5报文长度*/
    enum enMd5LenRange
    {
        MD5_MIN_LEN = 4,
        MD5_MAX_LEN = 22,
        MD5_OLD_LEN = 32
    };   

    /*! <频率限制AppID 频率定义查询/etc/tbase_limit.conf */
    enum _QQSHOW_FREQUENCY_LIMIT_APPID
    {
        QFLA_ITEM_PAY = 0,          /*! <商城购买*/
        QFLA_SAVESHOW = 1,          /*! <保存形象 GIF & FLASH*/
        QFLA_CAMERA_APPREQ = 2,     /*! <照相馆合影，包括与他人和与宠物*/
        QFLA_CAMERA_SENDMAIL = 3,   /*! <发送邮件频率限制*/
        QFLA_CLIENT_ASK = 4,        /*! <客户端发起索要请求*/
        QFLA_CLIENT_ANSWER = 5,     /*! <客户端响应索要请求*/
        QFLA_SEND_ALL    = 7        /*! <赠送所有物品频率*/
    };

    /*!
     * \brief 字符串替换
     * \param[in,out] sData: 要进行替换的对象并对之进行替换
     * \param[in] sSrc: 要替换的子串
     * \param[in] sDst: 用于替换的子串
     * \return std::string 返回sData对象引用
    */
    static std::string& StringReplace(std::string& sData, const std::string& sSrc, const std::string& sDst);

    /*!
     * \brief 分解字符串
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<std::string> vItems的引用
    */
    static std::vector<std::string>& StringSplit(const std::string& sData, const std::string& sDelim, std::vector<std::string>& vItems);
	
	/*!
     * \brief 分解字符串(连续的分割符看作一个如 |||==|)
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<std::string> vItems的引用
    */
    static std::vector<std::string>& StringSplitTrim(const std::string& sData, const std::string& sDelim, std::vector<std::string>& vItems);

	/*!
     * \brief 去除字符串前后的空格
     * \param[in/out] sData: 要进行Trim的字符串
     * \param[in] sDelim: 分隔字符串,默认为空格
     * \return std::string sData
    */
    static std::string StringTrim(std::string& sData, const std::string& sDelim =" ");

	/*!
	 * \brief 把数组合成字符串
	 * \param[in] vstElem: 要合成的数组
	 * \param[in] sDelim:  分隔符
	 * \return std::string 合成的字符串
	*/
	static std::string StringJoin(const std::vector<std::string>& vstElem, const std::string& sDelim);
	/*!
	 * \brief 把数组合成字符串
	 * a
	 * a和b
	 * a,b和c
	 *
	 * \param[in] vstElem: 要合成的数组
	 * \param[in] lastDelim:  后分隔符
	 * \param[in] listDelim:  前分隔符
	 * \return std::string 合成的字符串
	*/
	static std::string StringHumanJoin(const std::vector<std::string>& vstElem,
			const std::string& lastDelim=" and ", const std::string& listDelim=", ");

	/*!
	 * \brief 把数组合成字符串
	 * \param[in] vstElem: 要合成的数组
	 * \param[in] sDelim:  分隔符
	 * \return std::string 合成的字符串
	*/
	static std::string StringJoin(const std::vector<int>& vstElem, const std::string& sDelim);
    
    /*!
     * \brief 分解字符串, 返回数据转换为整型
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<int> vItems的引用
    */
    static std::vector<int>& StringSplit(const std::string& sData, const std::string& sDelim, std::vector<int>& vItems);
    
    /*!
     * \brief 分解字符串, 返回数据转换为整型
     * \param[in] sData: 要进行分解的字符串
     * \param[in] sDelim: 分隔字符串
     * \param[out] vItems: 返回字符串列表包含空串
     * \return std::vector<long long> vItems的引用
    */
    static std::vector<long long>& StringSplit(const std::string& sData, const std::string& sDelim, std::vector<long long>& vItems);

    /*! \brief 对字符串编译C->%XX*/
    static std::string StringEscape(const std::string& sData);
    
    /*! \brief 对字符串编码进行解码%XX->C*/
    static std::string StringUnEscape(const std::string& sData);

    /*! \brief 对字符串编码进行XML编码功能 */
	static std::string XMLEncode(const std::string& sData);

    /*! \brief 对字符串编码进行XML部分编码功能 */
	static std::string XMLMiniEncode(const std::string& sData);

	/*! \brief 对字符串编译功能与JavaScript.escape()类似 */
	static std::string escape(const std::string& sData);

    /*! \brief 对字符串编码进行解码功能与JavaScript.unescape()类似 */
	static std::string unescape(const std::string& sData);

	/*! \brief 对GB2312编译转换为UTF-8编译 */
	static std::string UTF8(const std::string& sData);

	/*! \brief 检查进行中文完整截断保存最多nlen字节 */
	static std::string& StringChnCut(std::string& sData, int nSize);
    
    /*!
     * \brief 检查双字节字符的截断位置,保证数据的正确性
     * \param[in] pcData: 字符指针首地址
     * \param[in] nSize:  最大字符数(注:应保证不能大于字符串长度)
     * \return int <= nSize 应该保留的字节数 [ =nSize OR =nSize-1 ]
    */
    static int CheckChn(const char* pcData, int nSize);

	/*!
	 * \brief 检查词数(中文词=2字节)
	 * \param[in] sData: 要检查的字符串
	 * \return int 词数
	*/
	static int Wordlen(const std::string& sData);

	/*!
	 * \brief 
	 * \param[in] sData: 源字符串
	 * \return std::string 返回目的串
	*/
	static std::string Wordreserve(const std::string& sData, int nSize);
    
    /*!
     * \brief 根据MD5算法生成签名
     * \param[in] sData: 待签名数据
     * \param[in] iDataLen: 待签名数据长度 
     * \param[in] sKey: 签名公钥
     * \param[in] iKeyLen: 公钥长度 
     * \param[in] iMD5Len: 签名结果长度,不包括结尾所需的'\0'
     * \param[out] sMD5: 签名结果
     * \return int [ 0--签名成功  !0--签名失败，或者结果长度小于4 ]
    */
    static int MD5Make(const std::string& sData, int iDataLen, const std::string& sKey, int iKeyLen, std::string& sMD5, int iMD5Len);
    
    /*!
     * \brief 验证MD5算法生成的签名
     * \param[in] sData: 待签名数据
     * \param[in] iDataLen: 待签名数据长度 
     * \param[in] sKey: 签名公钥
     * \param[in] iKeyLen: 公钥长度
     * \param[out] sMD5: 签名
     * \param[in] iMD5Len: 签名结果长度,不包括结尾所需的'\0'
     * \return int [ 0--签名成功  !0--签名失败 ]
    */
    static int MD5NCmp(const std::string& sData, int iDataLen,  const std::string& sKey, int iKeyLen, const std::string& sMD5, int iMD5Len);

	/*!
	 * \brief 对MD5Make的封装
	*/
	static std::string MD5Make(const std::string& sData, const std::string& sKey);

	/*!
	 * \brief 对MD5NCmp的封装
	*/
	static int MD5NCmp(const std::string& sData, const std::string& sKey, const std::string& sMD5);
 
   /*!
     * \brief 生成通用MD5
     * \param[in] sData: 
     * \param[out] sMD5:
     * \return int [ 0--succ !0--fail ]
    */
    static int MD5Comm(const std::string& sData, std::string& sMD5);

    /*!
     * \brief 对ID的访问次数做限制，在 /etc/tbase_limit.conf 中配置appid
     * \param[in] iAppID    配置ID号
     * \param[in] szID        独立ID， 仅前14位(不包括结尾'\0')有效
     * \return int [ 0--限制成功  !0--限制失败 ]
    */
    static int CheckLimit(int iAppID, const std::string& sID);

public:

    /*!
     * \brief 测试UIN正确性
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int IsUinValid(int iUin);

     /*!
     * \brief 测试UIN正确性
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int IsQQShowItemnoValid(int iItemNo);

     /*!
     * \brief 设置当前调用者身份
     * \parma[in] sCallerID 调用者身份
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int SetCaller(const std::string &sCallerID);

     /*!
     * \brief 获得当前调用者身份
     * \return 调用者身份
    */
    static const std::string GetCaller(void);

      /*!
     * \brief 获得当前调用者身份针对原子服务器进行了删减
     * \return 调用者身份
    */
    static const std::string GetAtomCaller(void);

     /*!
     * \brief 记录WEB服务器IP
     * \parma[in] sCallerID 调用者身份
     * \return int [ 0--OK  !0--FAIL ]
    */
    static int RecWebServerIP(const std::string &sIP);

     /*!
     * \brief 获得WEB服务器IP
     * \return 服务器IP
    */
    static const std::string GetWebServerIP(void);

	/*!
	 * \brief 生成QQ成内部序列号(以portal序列号方式)
	*/
	static std::string QQShowInternalSerialID(const std::string& sCode="-QQSID");
    
    /*
    *压缩. 以V开头的形象串 输入限制<6k 输出限制<8k
    *sQQShowCode 压缩字符串 
    *sZipType 压缩类型 Zl：zlib库 其他：失败
    *成功：返回解压串 失败返回 空串
    */
    static std::string Compress(const std::string& sQQShowCode, const std::string& sZipType);
    
    /*
    *解压缩 ，可读明码，以Z1 或Z2开头 输入限制<8k 输出限制<6k（针对形象串）
    *in 压缩字符串 //
    *成功：返回解压串 失败：返回空
    */
    static std::string DeCompress(const std::string &sEnode);

    /*! \brief 获取本地ip地址*/
    static std::string GetLocalIp();
private:

    /*! 调用者身份 */
    static std::string sCallerID;

    /*! 记录发送请求的WEB服务器IP*/
    static std::string sWebServerIP;
     
};

#endif

