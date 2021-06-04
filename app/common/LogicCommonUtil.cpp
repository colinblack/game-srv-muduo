#include "LogicCommonUtil.h"

int LogicCommonUtil::TurnLuckTable(vector<unsigned> & prates, int len, int & target)
{
	//获得概率总和
	int max = 0, last = 0;

	for(int i = 0 ; i < len; ++i)
	{
		max += prates[i];
	}

	int random = 0;

	//产生随机值
	random = Math::GetRandomInt(max);

	int j = 0;

	for (; j < len; ++j )
	{
		if (random < (last + prates[j]))
		{
			break;
		}

		last += prates[j];
	}

	target = j;

	return 0;
}


int LogicCommonUtil::GetRandomBetweenAB(int min, int max)
{
	int randmax = max - min + 1;   //n-m+1
	int randval =  Math::GetRandomInt(randmax); //rand()%(n-m+1)

	int val = randval + min;  // rand()%(n-m+1) + m

	return val;
}

int LogicCommonUtil::SetBitCurrent(unsigned & current, int pos)
{
	//方法？用一个1000的值与current进行或运算即可。而1000中1的位置，就是pos的值.即将1直接左移pos个位置即可
	unsigned target = 1;
	target = target <<pos;

	current = current | target;

	return 0;
}

int LogicCommonUtil::SetBitCurrent(unsigned char & current, int pos)
{
	//方法？用一个1000的值与current进行或运算即可。而1000中1的位置，就是pos的值.即将1直接左移pos个位置即可
	unsigned char target = 1;
	target = target <<pos;

	current = current | target;

	return 0;
}

int LogicCommonUtil::SetZeroRange(unsigned char & current, int first, int last)
{
	//通过循环调用即可
	for(int i = first; i <= last; ++i)
	{
		SetZeroCurrent(current, i);
	}

	return 0;
}

int LogicCommonUtil::SetZeroCurrent(unsigned char & current, int pos)
{
	//将指定位设置为0，则必须进行交集运算. 值是1101, 0的位置，就是pos的位置，可以通过取反获得，即先产生0010,即将1左移pos位
	unsigned char target = ~(1 << pos);

	current = current & target;

	return 0;
}

int LogicCommonUtil::GetTargetBitValue(int maxsize, unsigned &target)
{
	target = 1;

	//将一个1的数，先左移1位，再和1进行或运算，然后循环即可
	for(int i = 1; i < maxsize; ++i)   //计算满足条件时的值,前面n-size为0，低位size全为1
	{
		target = (target << 1) | 1;
	}

	return 0;
}

bool LogicCommonUtil::CheckPosIsZero(unsigned value, unsigned pos)
{
	//第一个1，左移pos，然后与value进行&运算，如果该位为1，则值为1，反之，为0
	unsigned target = 1;

	target = target <<pos;

	int result = value & target;

	return result ? false : true;
}

int LogicCommonUtil::GetHourTimes(unsigned start, unsigned end)
{
	if (start >= end)
	{
		return 0;
	}

	time_t tStart = (time_t)start;
	time_t tEnd = (time_t)end;

	struct tm tmStartTime;   //定义tm类型
	struct tm tmEndTime;

	localtime_r(&tStart, &tmStartTime);   //从time_t转换成tm时，获得的就是距离1900的差值
	localtime_r(&tEnd, &tmEndTime);

	//将时间转换成整点时间
	tmStartTime.tm_isdst = 0;
	tmStartTime.tm_min = 0;
	tmStartTime.tm_sec = 0;

	//结束时间处理
	tmEndTime.tm_isdst = 0;
	tmEndTime.tm_min = 0;
	tmEndTime.tm_sec = 0;

	unsigned uTimeStart = (unsigned)mktime(&tmStartTime);
	unsigned uTimeEnd = (unsigned)mktime(&tmEndTime);

	unsigned diff = uTimeEnd - uTimeStart;

	int hour = diff/3600;

	return hour;
}

unsigned LogicCommonUtil::GetHourTime(unsigned times)
{
	time_t tTimes = (time_t)times;

	struct tm tmNow;   //定义tm类型
	localtime_r(&tTimes, &tmNow);

	//将时间转换成整点时间
	tmNow.tm_isdst = 0;
	tmNow.tm_min = 0;
	tmNow.tm_sec = 0;

	//根据pos，往前推算
	return (unsigned)mktime(&tmNow);
}

unsigned LogicCommonUtil::GetHourByTime(unsigned time)
{
	time_t tTimes = (time_t)time;

	struct tm tmNow;   //定义tm类型
	localtime_r(&tTimes, &tmNow);

	return tmNow.tm_hour;
}

int LogicCommonUtil::GetDate(unsigned time, int & year, int & month, int &day)
{
	time_t tTimes = (time_t)time;

	struct tm tmNow;   //定义tm类型
	localtime_r(&tTimes, &tmNow);

	year	= tmNow.tm_year+1900;
	month	= tmNow.tm_mon+1;
	day	= tmNow.tm_mday;

	return 0;
}

void LogicCommonUtil::GetRandoms(int start, int end, int cnt, std::vector<int>& result)
{
	result.clear();
	if (start > end) return ;

	if (end - start <= cnt)
	{
		for (int i = start; i <= end; i++)
		{
			result.push_back(i);
		}
	}
	else
	{
		std::vector<int> vals;
		for (int i = start; i <= end; i++)
		{
			vals.push_back(i);
		}

		while(cnt > 0 && vals.size() > 0)
		{
			int nRandomVal = Math::GetRandomInt(vals.size());
			result.push_back(vals[nRandomVal]);

			vals.erase(vals.begin() + nRandomVal);

			--cnt;
		}
	}
}

int LogicCommonUtil::GetRandomIndex(const std::vector<unsigned>& vProb)
{
	unsigned nTotal = 0;
	for (size_t i = 0; i < vProb.size(); ++i)
	{
		nTotal += vProb[i];
	}

	if (nTotal == 0)
	{
		throw std::runtime_error("prob_weight_is_null");
	}

	int nRandom = Math::GetRandomInt(nTotal);

	for (size_t i = 0; i < vProb.size(); ++i)
	{
		if (nRandom < vProb[i])
		{
			return i;
		}

		nRandom -= vProb[i];
	}

	throw std::runtime_error("product_random_value_error");
}

bool LogicCommonUtil::IsCrossTime(unsigned start, unsigned end, int hour)
{
	if (start >= end)
	{
		return false;
	}

	time_t tStart = (time_t)start;
	time_t tEnd = (time_t)end;

	struct tm tmStartTime;   //定义tm类型
	struct tm tmEndTime;

	localtime_r(&tStart, &tmStartTime);
	localtime_r(&tEnd, &tmEndTime);

	//只关心start是否在hour之前.因为某动作在hour时处理，如果start的小时大于hour，说明该动作已发生，则不必再处理一次
	if (tmStartTime.tm_hour >= hour)  //在时间之后，不必处理
	{
		return false;
	}
	else
	{
		//在此之前，要判断是否跨天，如果跨天，则必然已跨过指定小时,之所以这里说必定跨过这个小时，是因为之前已判断过start的小时小于hour.如果跨天，当然要走过hour
		if (CTime::GetDayInterval(start, end))
		{
			return true;
		}
		else   //没有跨天，则判断end的小时是否大于hour
		{
			if (tmEndTime.tm_hour >= hour)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

int LogicCommonUtil::ExecCommand(const char * pCmd, string & errmsg, string & output)
{
	if (NULL == pCmd)
	{
	    return 5;
	}

	char *pBuf = 0;
	char *pFilter = 0;
	FILE *pipe_fp = 0;
	int r = 0;
	int nCount = 0;

	if(0 == (pBuf = (char *)calloc(sizeof(char), 1024)))
	{
	    return 7;
	}

	pipe_fp = popen(pCmd, "r");

	if (NULL == pipe_fp)
	{
		errmsg = strerror(errno);
		return 1;
	}

	for (;;)
	{
	    r = fread(pBuf + r, 1, 1024 - nCount, pipe_fp);

	    if (r <= 0)
	    	break;

	    nCount += r;
	}

	int ret = pclose(pipe_fp);

	if (-1 == ret)
	{
		//关闭失败?
		errmsg = strerror(errno);
		return ret;
	}

	// child process exit status.
	int status_child = WEXITSTATUS(ret);

	if (0 == status_child)
	{
		//去掉末尾的换行符
        pFilter = strrchr(pBuf, '\n');

        if(pFilter)
        	*pFilter = 0;
		output = pBuf;
	}

	return status_child;
}

