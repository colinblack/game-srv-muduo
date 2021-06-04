#ifndef LOGIC_COMMON_UTIL_H
#define LOGIC_COMMON_UTIL_H

#include "Common.h"

class LogicCommonUtil
{
public:
	LogicCommonUtil(){}
	virtual ~LogicCommonUtil(){}

	/*
	 * 在概率数组中随机抽取一个
	 * param  prates(in),len(in),target(out)
	 * return 0-成功
	 */
	static int TurnLuckTable(vector<unsigned> & prates, int len, int & target);

	/*
	 * 在两个值之间产生一个随机值
	 * param  min(in),max(in)
	 * return 随机值
	 */
	static int GetRandomBetweenAB(int min, int max);

	/*
	 * 设置指定位值为1
	 * param  current(in&out),pos(in)
	 * return 0-成功
	 */
	static int SetBitCurrent(unsigned & current, int pos);

	/*
	* 设置指定位值为1
	* param  current(in&out),pos(in)
	* return 0-成功
	*/
	static int SetBitCurrent(unsigned char & current, int pos);

	/*
	* 设置指定范围内位的值为0
	* param  current(in&out),first(in),last(in)，范围区间是[first, last]
	* return 0-成功
	*/
	static int SetZeroRange(unsigned char & current, int first, int last);

	/*
	* 设置指定位的值为0
	* param  current(in&out),pos(in)
	* return 0-成功
	*/
	static int SetZeroCurrent(unsigned char & current, int pos);

	/*
	 * 设置低位maxsize个全为1的值
	 * param  maxsize(in),target(out)
	 * return 0-成功
	 */
	static int GetTargetBitValue(int maxsize, unsigned &target);

	/*
	 * 判断某个值的指定pos位是否为0
	 * param  value(in),pos(in)-下标
	 * return true-是
	 */
	static bool CheckPosIsZero(unsigned value, unsigned pos);

	/*
	 * 获取两个时间点之间的整点次数
	 * param  value(in),pos(in)
	 * return 整点次数
	 */
	static int GetHourTimes(unsigned start, unsigned end);

	//获取整点时间
	static unsigned GetHourTime(unsigned times);

	//获取给定时间的小时值
	static unsigned GetHourByTime(unsigned time);

	//获取给定时间的年月日
	static int GetDate(unsigned time, int & year, int & month, int &day);

	//[start, end]产生cnt个不重复的随机数， 数量不够返回全部
	//适合小区间, 大区间效率不高
	static void GetRandoms(int start, int end, int cnt, std::vector<int>& result);

	//根据概率表产生一个随机的对应偏移值
	static int GetRandomIndex(const std::vector<unsigned>& vProb);

	//判断是否在两个时间范围内，跨越了指定的小时
	static bool IsCrossTime(unsigned start, unsigned end, int hour);

	//执行命令，并获取输出
	static int ExecCommand(const char * cmd, string & errmsg, string & output);
};

#endif
