/*
 * Crypt.h
 *
 *  Created on: 2011-5-17
 *      Author: dada
 */

#ifndef __CRYPT_H__
#define __CRYPT_H__

#include <string>
using std::string;

namespace Crypt
{
	//Base64
	bool Base64Encode(string &result, const string &text, const char *altChars = NULL);
	bool Base64EncodeTrim(string &result, const string &text, const char *altChars = NULL);
	bool Base64Decode(string &result, const string &text, const char *altChars = NULL);
	bool Base64UrlEncode(string &result, const string &text);
	bool Base64UrlEncodeTrim(string &result, const string &text);
	bool Base64UrlDecode(string &result, const string &text);

	//Url
	string UrlEncode(const std::string &sData);
	string UrlEncodeForTX(const std::string &sData);
	string EncodeForTXSig(const string &sData);
	string UrlDecode(const std::string &sData);

	//MD5
	string Md5Encode(const string &data);

	//HMAC-SHA
	string Sha1(const string &text);
	string Sha1Raw(const string &text);
	string HmacSha1(const string &text, const string &key);
	string HmacSha256(const string &text, const string &key);

	//RSA-SHA
	bool RsaSha1Signature(const string &text, const string &key, string &sign);
	bool RsaSha1Verify(const string &text, const string &key, const string &sign);

	//Dawx Crypt
	bool DawxEncode(string &result, const string &text, const string &key);
	bool DawxDecode(string &result, const string &text, const string &key);
}

#endif	//__CRYPT_H__
