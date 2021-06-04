/*
 * TimeUtil.cpp
 *
 *  Created on: 2011-5-27
 *      Author: dada
 */

#include "TimeUtil.h"
#include "Common.h"

extern long timezone;

namespace Time
{
	Timestamp g_nGlobalTime;
}

bool Time::Init()
{
	tsc_init();
	UpdateGlobalTime();
	return true;
}

bool Time::UpdateGlobalTime()
{
	g_nGlobalTime = Timestamp::now();;
	return true;
}

unsigned Time::GetGlobalTime()
{
	UpdateGlobalTime();
	return g_nGlobalTime.microSecondsSinceEpoch() / Timestamp::kMicroSecondsPerSecond;
}

bool Time::IsToday(unsigned ts)
{
	return ((Time::GetGlobalTime() - timezone) / 86400) == ((ts - timezone) / 86400);
}

int Time::ThroughDay(unsigned beg, unsigned end)
{
	return ((end - timezone) / 86400 - (beg - timezone) / 86400);
}
bool Time::IsThisWeek(unsigned ts)
{
	uint32_t weekBeginTs = GetWeekBeginTs(Time::GetGlobalTime());
	if(weekBeginTs > 0)
	{
		return ts >= weekBeginTs && ts < weekBeginTs + 7 * 86400;
	}
	return false;
}

uint32_t Time::GetWeekBeginTs(unsigned ts)
{
	time_t nts = ts;

	struct tm ptm;
    if(localtime_r(&nts, &ptm) != NULL)
    {
		unsigned int wday = ptm.tm_wday;
		unsigned int frontDays = (wday == 0) ? 6 : (wday - 1);
		unsigned int todaySecond = ptm.tm_hour * 3600 + ptm.tm_min * 60 + ptm.tm_sec;
		return ts - frontDays * 86400 - todaySecond;
    }
    else
    {
    	return 0;
    }
}

unsigned Time::GetTodayBeginTs()
{
	return Time::GetGlobalTime() - GetTodaySecond();
}

unsigned Time::GetTodaySecond()
{
	return GetDaySecond(Time::GetGlobalTime());
}
unsigned Time::GetDayBeginTs(unsigned ts)
{
	return ts - GetDaySecond(ts);
}

unsigned Time::GetDaySecond(unsigned ts)
{
	time_t nts = ts;

	struct tm ptm;
	return (localtime_r(&nts, &ptm) != NULL) ? (ptm.tm_hour * 3600 + ptm.tm_min * 60 + ptm.tm_sec) : 0;
}
unsigned Time::GetNextMonthTs(unsigned ts, unsigned day, unsigned hour)
{
	time_t nts = ts;

	struct tm ptm;
    if(localtime_r(&nts, &ptm) != NULL)
    {
    	if(ptm.tm_mon >= 11)
    	{
    		ptm.tm_year++;
    		ptm.tm_mon = 0;
    	}
    	else
    	{
    		ptm.tm_mon++;
    	}
    	ptm.tm_mday = day;
    	ptm.tm_hour = hour;
    	ptm.tm_min = 0;
    	ptm.tm_sec = 0;
		return mktime(&ptm);
    }
    else
    {
    	return 0;
    }
}
const string Time::getTimeStr(unsigned ts)
{
	time_t nts = ts;
	struct tm ptm;
	localtime_r(&nts, &ptm);
	char str[128];
	snprintf(str, sizeof(str), "%u-%u-%u %u:%u:%u", ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec);
	return str;
}

const string Time::getTimeStrNew(unsigned ts)
{
	time_t nts = ts;

	struct tm ptm;
	localtime_r(&nts, &ptm);
	char str[128];
	snprintf(str, sizeof(str), "%u_%u_%u_%u_%u_%u", ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec);
	return str;
}
const string Time::getDayStr(unsigned ts)
{
	time_t nts = ts;

	struct tm ptm;
	localtime_r(&nts, &ptm);
	char str[128];
	snprintf(str, sizeof(str), "%u_%u_%u", ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday);
	return str;
}
