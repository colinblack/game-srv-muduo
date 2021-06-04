/*
 * CommonUtil.h
 *
 *  Created on: 2011-6-9
 *      Author: dada
 */

#ifndef COMMONUTIL_H_
#define COMMONUTIL_H_

#include <time.h>

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

enum Bool
{
	False = 0,
	True = 1
};

bool isSameMin(time_t time1, time_t time2);
bool isSameFiveMin(time_t time1, time_t time2);
bool isSameDay(time_t time1,time_t time2);
bool isSameWeek(time_t time1,time_t time2);

typedef unsigned char byte;

#ifndef UINT8_MAX
#define UINT8_MAX 0xff
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffff
#endif

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

#endif /* COMMONUTIL_H_ */
