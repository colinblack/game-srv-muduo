/*
 * Base64.cpp
 *
 *  Created on: 2011-5-17
 *      Author: dada
 */

#include "Crypt.h"

char g_Base64EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

unsigned char g_Base64DecodeTable[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 255, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
     19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
     37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255
};

bool Crypt::Base64Encode(string &result, const string &text, const char *altChars)
{
	if(altChars != NULL)
	{
		g_Base64EncodeTable[62] = altChars[0];
		g_Base64EncodeTable[63] = altChars[1];
	}
	size_t textSize = text.size();
	size_t resultSize = ((textSize + 2) / 3 * 4);
	result.resize(resultSize);
	size_t ri = 0;
    size_t i;
    size_t textRoundSize = textSize / 3 * 3;
	for(i = 0; i < textRoundSize; i += 3)
	{
		result[ri++] = g_Base64EncodeTable[(text[i] >> 2) & 0x3f];
		result[ri++] = g_Base64EncodeTable[((text[i] << 4) & 0x30) | ((text[i + 1] >> 4) & 0xf)];
		result[ri++] = g_Base64EncodeTable[((text[i + 1] << 2) & 0x3D) | ((text[i + 2] >> 6) & 0x3)];
		result[ri++] = g_Base64EncodeTable[text[i + 2] & 0x3f];
	}
	if(i < textSize)
	{
        char a = text[i];
        char b = ((i + 1) < textSize) ? text[i + 1] : 0;
        result[ri++] = g_Base64EncodeTable[(a >> 2) & 0x3f];
        result[ri++] = g_Base64EncodeTable[((a << 4) & 0x30) | ((b >> 4) & 0xf)];
        result[ri++] = ((i + 1) < textSize) ? g_Base64EncodeTable[(b << 2) & 0x3D] : '=';
        result[ri++] = '=';
	}
	if(altChars != NULL)
	{
		g_Base64EncodeTable[62] = '+';
		g_Base64EncodeTable[63] = '/';
	}
	return true;
}

bool Crypt::Base64EncodeTrim(string &result, const string &text, const char *altChars)
{
	if(!Base64Encode(result, text, altChars))
	{
		return false;
	}
	size_t resultSize = result.size();
	if(resultSize > 0 && result[resultSize - 1] == '=')
	{
		if(resultSize > 1 && result[resultSize - 2] == '=')
		{
			result.resize(resultSize - 2);
		}
		else
		{
			result.resize(resultSize -1);
		}
	}
	return true;
}

bool Crypt::Base64Decode(string &result, const string &text, const char *altChars)
{
	size_t textSize = text.size();
	size_t resultSize = textSize / 4 * 3;
	if(textSize > 0)
	{
		if(textSize % 4 == 0)
		{
			if(text[textSize - 1] == '=')
			{
				resultSize--;
				if(text[textSize - 2] == '=')
				{
					resultSize--;
				}
			}
		}
		else
		{
			resultSize += textSize % 4 - 1;
		}
	}
	result.resize(resultSize);

	if(altChars != NULL)
	{
		g_Base64DecodeTable['+'] = 255;
		g_Base64DecodeTable['/'] = 255;
		g_Base64DecodeTable[(unsigned char)(altChars[0])] = 62;
		g_Base64DecodeTable[(unsigned char)(altChars[1])] = 63;
	}

	bool success = true;
	size_t ti = 0;
	unsigned char curr = 0;
	unsigned char next;
	for(size_t i = 0; i < resultSize; i++)
	{
		next = g_Base64DecodeTable[(unsigned char)(text[ti++])];
		if(next >= 64)
		{
			success = false;
			break;
		}
		size_t n = i % 3;
		if(n == 0)
		{
			curr = next;
			next = g_Base64DecodeTable[(unsigned char)(text[ti++])];
			if(next >= 64)
			{
				success = false;
				break;
			}
			result[i] = (curr << 2) | ((next >> 4) & 0x3);
			curr = next;
		}
		else if(n == 1)
		{
			result[i] = (curr << 4) | ((next >> 2) & 0xf);
			curr = next;
		}
		else
		{
			result[i] = (curr << 6) | next;
		}
	}

	if(altChars != NULL)
	{
		g_Base64DecodeTable[(unsigned char)(altChars[0])] = 255;
		g_Base64DecodeTable[(unsigned char)(altChars[1])] = 255;
		g_Base64DecodeTable['+'] = 62;
		g_Base64DecodeTable['/'] = 63;
	}

	return success;
}

bool Crypt::Base64UrlEncode(string &result, const string &text)
{
	return Base64Encode(result, text, "-_");
}

bool Crypt::Base64UrlEncodeTrim(string &result, const string &text)
{
	return Base64EncodeTrim(result, text, "-_");
}

bool Crypt::Base64UrlDecode(string &result, const string &text)
{
	return Base64Decode(result, text, "-_");
}

