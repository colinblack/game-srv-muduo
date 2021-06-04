/*
 * StringUtil.h
 *
 *  Created on: 2011-5-19
 *      Author: dada
 */

#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <string>
#include <vector>
#include <stdint.h>
using std::string;
using std::vector;

namespace String
{
	bool Format(string &out, const char *format, ...);

	unsigned int GetHash(const char *str);

	bool EqualNoCase(const string &s1, const string &s2);
	bool StartWith(const string &str, const string &start);

	void ToUpper(string &s);
	void ToLower(string &s);
	void Trim(string &s);
	void Split(const string& str,char delim,vector<string>& rlt);

	//在text中查找出现在before后，after前的第一个子串，不含before和after
	//allowEnd为true时，若找不到after，则返回before后到字符串结尾的子串
	bool FindMatch(string &result, const string &text, const string &before, const string &after, bool allowEnd = false);

	int Utf8GetLength(string &s);
	void Utf8Regularize(string &s);

	string b2s(const char *ptr, size_t nbytes,int32_t line_len=10);
	int s2i(const string& s, int defaultValue);
	string i2s(int i);
	string u2s(uint32_t i);
}

#endif /* STRINGUTIL_H_ */
