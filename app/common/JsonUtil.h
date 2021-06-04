/*
 * JsonUtil.h
 *
 *  Created on: 2011-7-20
 *      Author: dada
 */

#ifndef JSONUTIL_H_
#define JSONUTIL_H_

#include "Common.h"

namespace Json
{
	string ToString(const Json::Value &value);
	bool ToString(const Json::Value &value, const char *key, string &data);
	bool FromString(Json::Value &value, const string &data);

	bool IsObject(const Json::Value &value, const char *key);
	bool IsArray(const Json::Value &value, const char *key);
	bool IsObjectOrArray(const Json::Value &value, const char *key);

	bool GetObject(const Json::Value &value, const char *key, Json::Value &data);
	bool GetArray(const Json::Value &value, const char *key, Json::Value &data);
	bool GetObjectOrArray(const Json::Value &value, const char *key, Json::Value &data);
	bool GetString(const Json::Value &value, const char *key, string &data);
	bool GetInt(const Json::Value &value, const char *key, int &data);
	bool GetUInt(const Json::Value &value, const char *key, unsigned &data);
	bool GetUInt64(const Json::Value &value, const char *key, uint64_t &data);

	bool GetString(const Json::Value &value, unsigned i, string &data);
	bool GetInt(const Json::Value &value, unsigned i, int &data);
	bool GetUInt(const Json::Value &value, unsigned i, unsigned &data);
	bool GetUInt64(const Json::Value &value, unsigned i, uint64_t &data);
}

#endif /* JSONUTIL_H_ */
