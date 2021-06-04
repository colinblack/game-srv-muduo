/*
 * Convert.cpp
 *
 *  Created on: 2011-6-23
 *      Author: dada
 */

#include "Convert.h"

bool StringIsInt(const string &s)
{
	const char *pc = s.c_str();
	bool hasNumber = false;
	bool hasSign = false;
	while(*pc != '\0')
	{
		if(*pc < '0' || *pc > '9')
		{
			if(hasNumber || hasSign)
			{
				return false;
			}
			else
			{
				if(*pc == '+' || *pc == '-')
				{
					hasSign = true;
				}
				else if(*pc != ' ' && *pc != '\t')
				{
					return false;
				}
			}
		}
		else
		{
			hasNumber = true;
		}
		pc++;
	}
	return hasNumber;
}

bool StringIsUInt(const string &s)
{
	const char *pc = s.c_str();
	bool hasNumber = false;
	while(*pc != '\0')
	{
		if(*pc < '0' || *pc > '9')
		{
			if(hasNumber || (*pc != ' ' && *pc != '\t'))
			{
				return false;
			}
		}
		else
		{
			hasNumber = true;
		}
		pc++;
	}
	return hasNumber;
}

bool Convert::StringToInt(int &n, const string &s)
{
	if(!StringIsInt(s))
	{
		return false;
	}
	errno = 0;
	n = strtol(s.c_str(), NULL, 10);
	return errno == 0;
}

int Convert::StringToInt(const string &s, int defaultValue)
{
	if(!StringIsInt(s))
	{
		return defaultValue;
	}
	errno = 0;
	int n = strtol(s.c_str(), NULL, 10);
	if(errno != 0)
	{
		return defaultValue;
	}
	return n;
}

bool Convert::StringToUInt(unsigned int &n, const string &s)
{
	if(!StringIsUInt(s))
	{
		return false;
	}
	errno = 0;
	n = strtoul(s.c_str(), NULL, 10);
	return errno == 0;
}

unsigned int Convert::StringToUInt(const string &s, unsigned int defaultValue)
{
	if(!StringIsUInt(s))
	{
		return defaultValue;
	}
	errno = 0;
	unsigned int n = strtoul(s.c_str(), NULL, 10);
	if(errno != 0)
	{
		return defaultValue;
	}
	return n;
}
;
bool Convert::StringToInt64(int64_t &n, const string &s)
{
	if(!StringIsInt(s))
	{
		return false;
	}
	errno = 0;
	n = strtoll(s.c_str(), NULL, 10);
	return errno == 0;
}

int64_t Convert::StringToInt64(const string &s, int64_t defaultValue)
{
	if(!StringIsInt(s))
	{
		return defaultValue;
	}
	errno = 0;
	int64_t n = strtoll(s.c_str(), NULL, 10);
	if(errno != 0)
	{
		return defaultValue;
	}
	return n;
}

bool Convert::StringToUInt64(uint64_t &n, const string &s)
{
	if(!StringIsUInt(s))
	{
		return false;
	}
	errno = 0;
	n = strtoull(s.c_str(), NULL, 10);
	return errno == 0;
}

uint64_t Convert::StringToUInt64(const string &s, uint64_t defaultValue)
{
	if(!StringIsUInt(s))
	{
		return defaultValue;
	}
	errno = 0;
	uint64_t n = strtoull(s.c_str(), NULL, 10);
	if(errno != 0)
	{
		return defaultValue;
	}
	return n;
}

string Convert::IntToString(int n)
{
	string s;
	String::Format(s, "%d", n);
	return s;
}

string Convert::UIntToString(unsigned int n)
{
	string s;
	String::Format(s, "%u", n);
	return s;
}

string Convert::Int64ToString(int64_t n)
{
	string s;
	String::Format(s, "%lld", n);
	return s;
}

string Convert::UInt64ToString(uint64_t n)
{
	string s;
	String::Format(s, "%llu", n);
	return s;
}

Message* Convert::CreateMessage(const string& typeName)
{
	Message* message = NULL;
	const Descriptor* descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		const Message* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype)
			message = prototype->New();
	}
	return message;
}
