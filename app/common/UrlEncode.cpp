/*
 * UrlEncode.cpp
 *
 *  Created on: 2011-5-17
 *      Author: dada
 */

#include "Crypt.h"
#include "trans.h"

string Crypt::UrlEncode(const string &sData)
{
	static char s_itox[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string sValue;
	const char* epos = sData.data() + sData.size();
	for(const char* bpos = sData.data(); bpos < epos; bpos++)
	{
		const char ch = *bpos;
		if((ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z'))
		{
			sValue += ch;
		}
		else
		{
			sValue += "%";
			sValue += s_itox[((unsigned char)ch) >> 4];
			sValue += s_itox[((unsigned char)ch) & 0x0F];
		}
	}
	return sValue;
}

string Crypt::UrlEncodeForTX(const string &sData)
{
	static char s_itox[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string sValue;
	const char* epos = sData.data() + sData.size();
	for(const char* bpos = sData.data(); bpos < epos; bpos++)
	{
		const char ch = *bpos;
		if((ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			ch == '-' || ch == '_' || ch == '.')
		{
			sValue += ch;
		}
		else
		{
			sValue += "%";
			sValue += s_itox[((unsigned char)ch) >> 4];
			sValue += s_itox[((unsigned char)ch) & 0x0F];
		}
	}
	return sValue;
}

string Crypt::EncodeForTXSig(const string &sData)
{
	static char s_itox[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string sValue;
	const char* epos = sData.data() + sData.size();
	for(const char* bpos = sData.data(); bpos < epos; bpos++)
	{
		const char ch = *bpos;
		if((ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			ch == '!' || ch == '*' || ch == '(' || ch == ')')
		{
			sValue += ch;
		}
		else
		{
			sValue += "%";
			sValue += s_itox[((unsigned char)ch) >> 4];
			sValue += s_itox[((unsigned char)ch) & 0x0F];
		}
	}
	return sValue;
}

string Crypt::UrlDecode(const string &sData)
{
	string result;
	result.resize(sData.size());
	char *pcResult = const_cast<char *>(result.c_str());
	const char *pcData = sData.c_str();
	const char *pcDataEnd = pcData + sData.size();
	char vc1[2] = { 0, 0 };
	while(pcData < pcDataEnd)
	{
		if(*pcData == '%' && pcData + 2 < pcDataEnd && isxdigit(pcData[1]) && isxdigit(pcData[2]))
		{
			CTrans::X2TOC(pcData + 1, vc1);
			*pcResult++ = vc1[0];
			pcData += 3;
		}
		else
		{
			*pcResult++ = *pcData++;
		}
	}
	result.resize(pcResult - result.c_str());
	return result;
}

