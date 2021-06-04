/*
 * DawxCrypt.cpp
 *
 *  Created on: 2011-7-25
 *      Author: dada
 */

#include "Common.h"

char g_Base64EncodeTableMess[] = "SUjpnmyM0fY6CGe_wt2aIghHLR8VxODF3T4zKkvocE1Wi9-ZQ7d5PlqNrsAXuBJb";
char g_DawxEncodeTable[64];
unsigned char g_DawxDecodeTable[256];

void GenEncodeTable(const string &key)
{
	memcpy(g_DawxEncodeTable, g_Base64EncodeTableMess, 64);
	int size = key.size();
	unsigned char ch = 0;
	for(int i = 63; i > 0 ; i--)
	{
		ch ^= (unsigned char)key[(63 - i) % size];
		int swap = ch % (i + 1);
		char temp = g_DawxEncodeTable[i];
		g_DawxEncodeTable[i] = g_DawxEncodeTable[swap];
		g_DawxEncodeTable[swap] = temp;
	}
}

void GenDecodeTable(const string &key)
{
	GenEncodeTable(key);
	memset(g_DawxDecodeTable, -1, sizeof(g_DawxDecodeTable));
	for(unsigned i = 0; i < 64; i++)
	{
		g_DawxDecodeTable[(unsigned char)g_DawxEncodeTable[i]] = i;
	}
}

bool Crypt::DawxEncode(string &result, const string &text, const string &key)
{
	GenEncodeTable(key);
	size_t textSize = text.size();
	size_t resultSize = ((textSize + 2) / 3 * 4);
	result.resize(resultSize);
	size_t ri = 0;
    size_t i;
    size_t textRoundSize = textSize / 3 * 3;
	for(i = 0; i < textRoundSize; i += 3)
	{
		result[ri++] = g_DawxEncodeTable[(text[i] >> 2) & 0x3f];
		result[ri++] = g_DawxEncodeTable[((text[i] << 4) & 0x30) | ((text[i + 1] >> 4) & 0xf)];
		result[ri++] = g_DawxEncodeTable[((text[i + 1] << 2) & 0x3D) | ((text[i + 2] >> 6) & 0x3)];
		result[ri++] = g_DawxEncodeTable[text[i + 2] & 0x3f];
	}
	if(i < textSize)
	{
        char a = text[i];
        char b = ((i + 1) < textSize) ? text[i + 1] : 0;
        result[ri++] = g_DawxEncodeTable[(a >> 2) & 0x3f];
        result[ri++] = g_DawxEncodeTable[((a << 4) & 0x30) | ((b >> 4) & 0xf)];
        result[ri++] = ((i + 1) < textSize) ? g_DawxEncodeTable[(b << 2) & 0x3D] : '=';
        result[ri++] = '=';
	}
	return true;
}

bool Crypt::DawxDecode(string &result, const string &text, const string &key)
{
	GenDecodeTable(key);
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

	size_t ti = 0;
	unsigned char curr = 0;
	unsigned char next;
	for(size_t i = 0; i < resultSize; i++)
	{
		next = g_DawxDecodeTable[(unsigned char)(text[ti++])];
		if(next >= 64)
		{
			return false;
		}
		size_t n = i % 3;
		if(n == 0)
		{
			curr = next;
			next = g_DawxDecodeTable[(unsigned char)(text[ti++])];
			if(next >= 64)
			{
				return false;
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

	return true;
}
