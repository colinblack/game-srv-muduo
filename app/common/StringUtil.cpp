/*
 * StringUtil.cpp
 *
 *  Created on: 2011-5-19
 *      Author: dada
 */

#include "StringUtil.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <algorithm>

bool String::Format(string &out, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	size_t n = vsnprintf(NULL, 0, format, args);
	char *buffer;
	buffer = new char[n + 2];
	va_start(args, format);
	size_t r = vsnprintf(buffer, n + 1, format, args);
	out = buffer;
	delete buffer;
	va_end(args);
	return r == n;
}

unsigned int String::GetHash(const char *str)
{
	int l = (strlen(str) + 1) / 2;
	unsigned short *s = (unsigned short *)str;
	unsigned long hash = 0;
	for(int i = 0; i < l; i++)
	{
		hash ^= (s[i] << (i & 0x0f));
	}
	return hash;
}

bool String::EqualNoCase(const string &s1, const string &s2)
{
	return strcasecmp(s1.c_str(), s2.c_str()) == 0;
}

bool String::StartWith(const string &str, const string &start)
{
	return str.compare(0, start.size(), start) == 0;
}

void String::ToUpper(string &s)
{
	transform(s.begin(), s.end(), s.begin(), toupper);
}

void String::ToLower(string &s)
{
	transform(s.begin(), s.end(), s.begin(), tolower);
}

void String::Trim(string &s)
{
	size_t length = s.size();
	if(length <= 0)
	{
		return;
	}
	size_t index = length;
	while(index > 0)
	{
		char ch = s[index - 1];
		if(ch >= 0 && ch <= ' ')
		{
			index--;
		}
		else
		{
			break;
		}
	}
	if(index < length)
	{
		s.erase(index);
		length = s.size();
		if(length <= 0)
		{
			return;
		}
	}
	index = 0;
	while(index < length)
	{
		char ch = s[index];
		if(ch >= 0 && ch <= ' ')
		{
			index++;
		}
		else
		{
			break;
		}
	}
	s.erase(0, index);
}

bool String::FindMatch(string &result, const string &text, const string &before, const string &after, bool allowEnd)
{
	string::size_type start;
	string::size_type end;
	start = text.find(before);
	if(start != string::npos)
	{
		start += before.size();
		end = text.find(after, start);
		if(end != string::npos)
		{
			result = text.substr(start, end - start);
			return true;
		}
		else if(allowEnd)
		{
			result = text.substr(start);
			return true;
		}
	}
	return false;
}

int String::Utf8GetLength(string &s)
{
	int length = 0;
	const unsigned char *ps = (const unsigned char *)s.c_str();
	const unsigned char *pEnd = ps + s.size();
	while(ps < pEnd)
	{
		unsigned char ch = *ps;
		if(ch < 0xC0)
		{
			ps++;
		}
		else if(ch < 0xE0)
		{
			ps += 2;
		}
		else if(ch < 0xF0)
		{
			ps += 3;
		}
		else if(ch < 0xF8)
		{
			ps += 4;
		}
		else
		{
			ps++;
		}
		length++;
	}
	return length;
}

void String::Utf8Regularize(string &s)
{
	size_t index = 0;
	size_t length = s.size();
	while(index < length)
	{
		unsigned char ch = s[index];
		size_t skip;
		if(ch < 0xC0)
		{
			skip = 1;
		}
		else if(ch < 0xE0)
		{
			skip = 2;
		}
		else if(ch < 0xF0)
		{
			skip = 3;
		}
		else if(ch < 0xF8)
		{
			skip = 4;
		}
		else
		{
			skip = 1;
		}
		if(index + skip > length)
		{
			s.erase(index);
			return ;
		}
		index += skip;
	}
}

void String::Split(const string& str,char delim,vector<string>& rlt){
	rlt.clear();
	if(str.length() == 0){
		return;
	}
	
	unsigned  lastPos = 0;
	unsigned  pos = 0;
	while((lastPos = str.find_first_of(delim,pos)) != 4294967295){
		rlt.push_back(str.substr(pos,lastPos - pos));
		pos = lastPos + 1;
	}

	rlt.push_back(str.substr(pos,str.length()-pos));
}

string String::b2s(const char *ptr, size_t nbytes,int32_t line_len){
	std::string hex_string;
	for(unsigned int i = 0; i < nbytes; i++)
	{
		if(i > 0 && i % line_len == 0)
		{
			hex_string += '[';
			for(unsigned int j=i-line_len; j < i; j++)
			{
				char chr = *((char*)ptr+j);
				if(isprint(chr))
					hex_string += chr;
				else
					hex_string += '.';
			}

			hex_string += "]\n";

		}

		char chr[3] = {0};

		sprintf(chr, "%02x ", (unsigned char)(*((char*)ptr+i)));
		hex_string += string(chr);
	}


	int left = nbytes % line_len;
	for(int i=line_len-left;i>0;i--)
	{
                hex_string += "   ";
	}

	if(left < line_len)
	{
		//hex_string += "    ";
		hex_string += "[";


		for(unsigned int j=nbytes-left; j < nbytes; j++)
                {
                     char chr = *((char*)ptr+j);
                     if(isprint(chr))
               		      hex_string += chr;
                     else
                        hex_string += '.';
                }
		hex_string += ']';
	}

	hex_string += '\n';
	return hex_string;
}

int String::s2i(const string& s, int defalutValue)
{
    errno = 0;
    int i = strtol(s.c_str(), NULL, 10);
    if(errno != 0)
    {
        return defalutValue;
    }

    return i;
}

string String::i2s(int i)
{
    char sTmp[64] = {0};
    sprintf(sTmp, "%d", i);
    return string(sTmp);
}

string String::u2s(uint32_t i)
{
	char sTmp[64] = {0};
	sprintf(sTmp, "%u", i);
	return string(sTmp);
}

