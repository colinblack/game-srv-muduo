/*
 * Convert.h
 *
 *  Created on: 2011-6-23
 *      Author: dada
 */

#ifndef CONVERT_H_
#define CONVERT_H_

#include "Common.h"

#define htonb(val) val
#define ntohb(val) val
#define hton64(val)	\
	(((val) >> 56) |\
	(((val) & 0x00ff000000000000ll) >> 40) |\
	(((val) & 0x0000ff0000000000ll) >> 24) |\
	(((val) & 0x000000ff00000000ll) >> 8)  |\
	(((val) & 0x00000000ff000000ll) << 8)  |\
	(((val) & 0x0000000000ff0000ll) << 24) |\
	(((val) & 0x000000000000ff00ll) << 40) |\
	(((val) << 56)))
#define ntoh64(val) hton64(val)

namespace Convert
{
	bool StringToInt(int &n, const string &s);
	int StringToInt(const string &s, int defaultValue = 0);
	bool StringToUInt(unsigned int &n, const string &s);
	unsigned int StringToUInt(const string &s, unsigned int defaultValue = 0);
	bool StringToInt64(int64_t &n, const string &s);
	int64_t StringToInt64(const string &s, int64_t defaultValue = 0);
	bool StringToUInt64(uint64_t &n, const string &s);
	uint64_t StringToUInt64(const string &s, uint64_t defaultValue = 0);

	string IntToString(int n);
	string UIntToString(unsigned int n);
	string Int64ToString(int64_t n);
	string UInt64ToString(uint64_t n);

	Message* CreateMessage(const string& typeName);
};

#endif /* CONVERT_H_ */
