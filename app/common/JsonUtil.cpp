/*
 * JsonUtil.cpp
 *
 *  Created on: 2011-7-20
 *      Author: dada
 */

#include "JsonUtil.h"

string Json::ToString(const Json::Value &value)
{
	return Json::FastWriter().write(value);
}

bool Json::ToString(const Json::Value &value, const char *key, string &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	data = Json::ToString(value[key]);
	return true;
}

bool Json::FromString(Json::Value &value, const string &data)
{
	return Json::Reader().parse(data, value);
}

bool Json::IsObject(const Json::Value &value, const char *key)
{
	if(!value.isMember(key))
	{
		return false;
	}
	if(!value[key].isObject())
	{
		return false;
	}
	return true;
}

bool Json::IsArray(const Json::Value &value, const char *key)
{
	if(!value.isMember(key))
	{
		return false;
	}
	if(!value[key].isArray())
	{
		return false;
	}
	return true;
}

bool Json::IsObjectOrArray(const Json::Value &value, const char *key)
{
	if(!value.isMember(key))
	{
		return false;
	}
	if(!value[key].isArray() && !value[key].isObject())
	{
		return false;
	}
	return true;
}

bool Json::GetObject(const Json::Value &value, const char *key, Json::Value &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	if(!value[key].isObject())
	{
		return false;
	}
	data = value[key];
	return true;
}

bool Json::GetArray(const Json::Value &value, const char *key, Json::Value &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	if(!value[key].isArray())
	{
		return false;
	}
	data = value[key];
	return true;
}

bool Json::GetObjectOrArray(const Json::Value &value, const char *key, Json::Value &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	if(!value[key].isArray() && !value[key].isObject())
	{
		return false;
	}
	data = value[key];
	return true;
}

bool Json::GetString(const Json::Value &value, const char *key, string &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	ValueType type = value[key].type();
	if(type == Json::stringValue)
	{
		data = value[key].asString();
	}
	else if(type == Json::uintValue)
	{
		unsigned u = value[key].asUInt();
		data = Convert::UIntToString(u);
	}
	else if(type == Json::intValue)
	{
		int n = value[key].asInt();
		data = Convert::IntToString(n);
	}
	else if(type == Json::nullValue)
	{
		data.clear();
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetInt(const Json::Value &value, const char *key, int &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	ValueType type = value[key].type();
	if(type == Json::intValue)
	{
		data = value[key].asInt();
	}
	else if(type == Json::uintValue)
	{
		unsigned u = value[key].asUInt();
		if(u > 0x7fffffff)
		{
			return false;
		}
		data = u;
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetUInt(const Json::Value &value, const char *key, unsigned &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	ValueType type = value[key].type();
	if(type == Json::intValue)
	{
		int n = value[key].asInt();
		if(n < 0)
		{
			return false;
		}
		data = n;
	}
	else if(type == Json::uintValue)
	{
		data = value[key].asUInt();
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetUInt64(const Json::Value &value, const char *key, uint64_t &data)
{
	if(!value.isMember(key))
	{
		return false;
	}
	ValueType type = value[key].type();
	if(type == Json::intValue)
	{
		int n = value[key].asInt();
		if(n < 0)
		{
			return false;
		}
		data = n;
	}
	else if(type == Json::uintValue)
	{
		data = value[key].asUInt();
	}
	else if(type == Json::realValue)
	{
		double d = value[key].asDouble();
		if(d < 0)
		{
			return false;
		}
		data = (uint64_t)d;
	}
	else if(type == Json::stringValue)
	{
		if(!Convert::StringToUInt64(data, value[key].asString()))
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetString(const Json::Value &value, unsigned i, string &data)
{
	ValueType type = value[i].type();
	if(type == Json::stringValue)
	{
		data = value[i].asString();
	}
	else if(type == Json::uintValue)
	{
		unsigned u = value[i].asUInt();
		data = Convert::UIntToString(u);
	}
	else if(type == Json::intValue)
	{
		int n = value[i].asInt();
		data = Convert::IntToString(n);
	}
	else if(type == Json::nullValue)
	{
		data.clear();
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetInt(const Json::Value &value, unsigned i, int &data)
{
	ValueType type = value[i].type();
	if(type == Json::intValue)
	{
		data = value[i].asInt();
	}
	else if(type == Json::uintValue)
	{
		unsigned u = value[i].asUInt();
		if(u > 0x7fffffff)
		{
			return false;
		}
		data = u;
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetUInt(const Json::Value &value, unsigned i, unsigned &data)
{
	ValueType type = value[i].type();
	if(type == Json::intValue)
	{
		int n = value[i].asInt();
		if(n < 0)
		{
			return false;
		}
		data = n;
	}
	else if(type == Json::uintValue)
	{
		data = value[i].asUInt();
	}
	else
	{
		return false;
	}
	return true;
}

bool Json::GetUInt64(const Json::Value &value, unsigned i, uint64_t &data)
{
	ValueType type = value[i].type();
	if(type == Json::intValue)
	{
		int n = value[i].asInt();
		if(n < 0)
		{
			return false;
		}
		data = n;
	}
	else if(type == Json::uintValue)
	{
		data = value[i].asUInt();
	}
	else if(type == Json::realValue)
	{
		double d = value[i].asDouble();
		if(d < 0)
		{
			return false;
		}
		data = (uint64_t)d;
	}
	else if(type == Json::stringValue)
	{
		if(!Convert::StringToUInt64(data, value[i].asString()))
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}
