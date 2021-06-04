#ifndef TIME_H_
#define TIME_H_

#include <ctime>
#include <string>
#include <sys/time.h>

class CTime
{
public:

    /*!
     * \brief 定义常用常量
    */
    enum
    {
        SECONDS_OF_DAY = 24*60*60 ,
        SECONDS_OF_WEEK = 7*24*60*60
    };

public:

    /*!
     * \brief 获取某时间点之间的天数: 如 2005-01-10 12:00:00 -> 2005-01-11: 06:00:00 = 1天
     * \param[in] tFrom: 时间起点
     * \param[in] tTo:   时间终点
     * \return int [ 时间天数差, 分正负 ]
    * \remark 以本地时间为准进行处理
    */
    static int GetDayInterval(time_t tFrom, time_t tTo);
    
    static int GetWeekInterval(time_t tFrom, time_t tTo);

    /*!
     * \brief 获取当前年份
     * \return 年份
    * \remark 以本地时间为准进行处理
     */
    static int GetCurrentYear( void );
	
		/*!
     * \brief 获取系统微秒数
     * \return int [ 当前微妙 ]
    */
	static int GetCurrentuTime();

	/*!
     * \brief 获取系统毫秒数
     * \return int [ 当前毫秒 ]
    */
	static int GetCurrentMSTime();

    /*!
     * \brief 返回时间点所以日开始时间点: 如 2005-01-10 12:10:29 -> 2005-01-10 00:00:00
     * \param[in] tTIme: 时间点
     * \return int [ 返回一天开始时间 ]
    * \remark 以本地时间为准进行处理
    */
    static time_t GetDayBeginTime(time_t tTime);


    static time_t GetWeekBeginTime(time_t tTime);

	/*!
	* \brief 返回时间点所以日开始时间点: 如 2005-01-10 12:10:29 -> 2005-01-01 00:00:00
	* \param[in] tTIme: 时间点
	* \return int [ 返回一个月的始时间 ]
	* \remark 以本地时间为准进行处理	*/

	static time_t GetMonthBeginTime(time_t tTime) ; 

    
    /*!
     * \breif 格式化时间
    */
    static const std::string FormatTime(const std::string& sFmt, const tm& stTime);


    /*!
     * \breif 格式化时间
    * \remark 以本地时间为准进行处理
    */
    static const std::string FormatTime(const std::string& sFmt, time_t tTime);


    /*!
     * \breif 时间结构转换
    */
    static time_t MakeTime(tm& stTime);


    /*!
     * \breif 时间结构转换
    */
    static time_t MakeTime(int iYear, int iMon, int iDay, int iHour=0, int iMin=0, int iSec=0);


    /*!
     * \brief 时间解析函数把 YYYY-MM-DD 格式分析成日期
     * \param[in]  sDate:  源日期数据 YYYY-MM-DD | YYYY/MM/DD
     * \param[out] iYear:  年
     * \param[out] iMon:   月
     * \param[out] iDay:   日
     * \param[in]  sDelim: 分隔符
     * \return int [ 0--succ !0--fail ]
    * \remark 本函数亦可作 HH:MM:SS 时间的分析
    */
    static int ParseDate(const std::string& sDate, int& iYear, int& iMon, int& iDay, const std::string& sDelim="-");

	/*!
	 * \brief 把YYYY-mm-dd HH:ii:ss 转换成int
	 * \param[in] data: 源日期
	 * \return time_t, 0 fail, success
	 */
	static time_t ParseDate(const std::string& data);

    /*!
     * \brief 返回时间代表的年月日
     * \param[out] iYear:  年
     * \param[out] iMon:   月
     * \param[out] iDay:   日
     * \param[in]  iTime:  时间, <0 时为当前时间
     * \return int [ 返回iTime, 如 iTime<0, 返回当前时间值 ]
    * \remark 以本地时间为准进行处理
    */
    static int GetDate(int& iYear, int& iMon, int& iDay, int iTime=-1);

    /*!
     * \brief 检查日期合法性并作修正
     * \return int [ 0--合法 !0--不合法并作修正 ]
    */
    static int CheckDate(int& iYear, int& iMonth, int& iDay);

    /*!
     * \brief 时间格式化为UTC格式: Tue, 21 Feb 2006 02:20:04 UTC
     * \param[in] 时间
     * \remark 返回全球标准时间而非本地时间
    */
    static std::string UTCTime(time_t tTime);

	/*!
	 * \brief 取当前时间
	*/
	static timeval GetTimeOfDay();

	/*!
	 * \brief 计算两时间之间微秒差
	 * \return time_t[tvto-tvfrom]
	*/
	static time_t GetUSInterval(const timeval& tvfrom, const timeval& tvto);

	/*!
	 * \brief 计算两时间之间毫秒差
	 * \return time_t[tvto-tvfrom]
	*/
	static time_t GetMSInterval(const timeval& tvfrom, const timeval& tvto);

	static bool IsDiffDay( time_t t1, time_t t2 );
	static bool IsDiffHour(time_t t1, time_t t2);
	static bool IsDiffWeek(time_t t1, time_t t2);
};
#endif

