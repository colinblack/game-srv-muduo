/*
 * CommonUtil.cpp
 *
 *  Created on: 2011-6-9
 *      Author: dada
 */

#include "CommonUtil.h"

bool isSameDay(time_t time1,time_t time2){
	int year1,day1,year2,day2;
	struct tm *tblock;

	tblock = localtime(&time1);

	year1 = tblock->tm_year;
	day1 = tblock->tm_yday;

	tblock = localtime(&time2);
	year2 = tblock->tm_year;
	day2 = tblock->tm_yday;

	return year1 == year2 && day1 == day2;
}

bool isSameMin(time_t time1, time_t time2) {
	int year1, day1, hour1, min1, year2, day2, hour2, min2;
	struct tm *tblock;

	tblock = localtime(&time1);
	year1 = tblock->tm_year;
	day1 = tblock->tm_yday;
	hour1 = tblock->tm_hour;
	min1 = tblock->tm_min;

	tblock = localtime(&time2);
	year2 = tblock->tm_year;
	day2 = tblock->tm_yday;
	hour2 = tblock->tm_hour;
	min2 = tblock->tm_min;
	return year1 == year2 && day1 == day2 && hour1 == hour2 && min1 == min2;
}

bool isSameFiveMin(time_t time1, time_t time2){
	int year1, day1, hour1, min1, year2, day2, hour2, min2;
		struct tm *tblock;

		tblock = localtime(&time1);
		year1 = tblock->tm_year;
		day1 = tblock->tm_yday;
		hour1 = tblock->tm_hour;
		min1 = tblock->tm_min;

		tblock = localtime(&time2);
		year2 = tblock->tm_year;
		day2 = tblock->tm_yday;
		hour2 = tblock->tm_hour;
		min2 = tblock->tm_min;
		return year1 == year2 && day1 == day2 && hour1 == hour2 && (min1/5 == min2/5);
}



bool isSameWeek(time_t time1,time_t time2)
{
	time_t v1 = time1 > time2 ? time1 : time2;
	time_t v2 = time1 > time2 ? time2 : time1;

	if(v1 - v2 > 7 * 24 * 3600){
		return false;
	}

	int day1,day2;
	struct tm *tblock;

	tblock = localtime(&time1);

	day1 = tblock->tm_wday;

	tblock = localtime(&time2);
	day2 = tblock->tm_wday;

	return day2 <= day1;
}
