/*
 * StringFilter.cpp
 *
 *  Created on: 2011-9-8
 *      Author: dada
 */

//AC-BMH + SUNDAY算法

#include "StringFilter.h"

#define MAX_FILTER_SIZE 50000
#define TRIE_HASH_SIZE 7

#define NIL 0xffffffff

struct HashItem
{
	uint32_t Value;
	uint32_t Next;
};

struct TrieNode
{
	unsigned char Type;
	unsigned char Value;
	HashItem HashTable[TRIE_HASH_SIZE];
};

enum TrieNodeType
{
	TN_ROOT = 0,
	TN_BRANCH = 1,
	TN_MATCH = 2
};

struct FilterData
{
	int MinLength;	//匹配字符串的最小长度
	unsigned char CharShift[256];	//字符出现在Trie树的最小深度（根部为0），最大为MinLength
	uint32_t Root[256];	//首字符匹配表
	TrieNode Trie[MAX_FILTER_SIZE];
	HashItem HashPool[MAX_FILTER_SIZE];
};

bool StringFilter::GenerateData(const vector<string> &words, const string &dataPath)
{
	//create share memory
	CReadOnlyShareMemory shareMemory;
	if(!shareMemory.Create(dataPath.c_str(), sizeof(FilterData)))
	{
		error_log("[create_filter_data_fail][path=%s]", dataPath.c_str());
		return false;
	}
	FilterData *pData = (FilterData *)shareMemory.GetAddress();

	//gen data
	pData->MinLength = 0xff;
	for(int i = 0; i < 256; i++)
	{
		pData->CharShift[i] = 0xff;
		pData->Root[i] = NIL;
	}
	int trieIndex = 0;
	int hashIndex = 0;
	for(vector<string>::const_iterator itr = words.begin(); itr != words.end(); itr++)
	{
		int size = itr->size();
		if(size < pData->MinLength)
		{
			pData->MinLength = itr->size();
		}
		TrieNode *pCurrNode;
		for(int i = 0; i < size; i++)
		{
			unsigned char cCurr = (unsigned char)itr->at(i);
			if(pData->CharShift[cCurr] > i)
			{
				pData->CharShift[cCurr] = i;
			}
			if(i == 0)
			{
				if(pData->Root[cCurr] == NIL)
				{
					TrieNode *pNewNode = &pData->Trie[trieIndex];
					pNewNode->Type = TN_BRANCH;
					pNewNode->Value = cCurr;
					for(int i = 0; i < TRIE_HASH_SIZE; i++)
					{
						pNewNode->HashTable[i].Value = NIL;
						pNewNode->HashTable[i].Next = NIL;
					}
					pData->Root[cCurr] = trieIndex;
					pCurrNode = pNewNode;
					trieIndex++;
				}
				else
				{
					pCurrNode = &pData->Trie[pData->Root[cCurr]];
					if(pCurrNode->Type == TN_MATCH)
					{
						break;
					}
				}
			}
			else
			{
				bool match = false;
				bool nextHash = true;
				HashItem *pCurrHashItem = &pCurrNode->HashTable[cCurr % TRIE_HASH_SIZE];
				while(nextHash)
				{
					if(pCurrHashItem->Value == NIL)
					{
						nextHash = false;
					}
					else if(pData->Trie[pCurrHashItem->Value].Value == cCurr)
					{
						match = true;
						nextHash = false;
					}
					else if(pCurrHashItem->Next == NIL)
					{
						nextHash = false;
					}
					else
					{
						pCurrHashItem = &pData->HashPool[pCurrHashItem->Next];
					}
				}
				if(match)
				{
					pCurrNode = &pData->Trie[pCurrHashItem->Value];
					if(pCurrNode->Type == TN_MATCH)
					{
						break;
					}
				}
				else
				{
					TrieNode *pNewNode = &pData->Trie[trieIndex];
					pNewNode->Type = TN_BRANCH;
					pNewNode->Value = cCurr;
					for(int i = 0; i < TRIE_HASH_SIZE; i++)
					{
						pNewNode->HashTable[i].Value = NIL;
						pNewNode->HashTable[i].Next = NIL;
					}
					if(pCurrHashItem->Value == NIL)
					{
						pCurrHashItem->Value = trieIndex;
					}
					else
					{
						HashItem *pNewHashItem = &pData->HashPool[hashIndex];
						pNewHashItem->Value = trieIndex;
						pNewHashItem->Next = NIL;
						pCurrHashItem->Next = hashIndex;
						hashIndex++;
					}
					pCurrNode = pNewNode;
					trieIndex++;
				}
			}
		}
		pCurrNode->Type = TN_MATCH;
	}
	for(int i = 0; i < 256; i++)
	{
		if(pData->CharShift[i] > pData->MinLength)
		{
			pData->CharShift[i] = pData->MinLength;
		}
	}

	info_log("[gen_filter_data][result=success,min_len=%d,node_count=%d,hash_count=%d]",
			pData->MinLength, trieIndex, hashIndex);

	return true;
}

static string g_dataPath;
static FilterData *g_pData = NULL;
static int g_minLength;
static unsigned char *g_pCharShift;
static uint32_t *g_pRoot;
static TrieNode *g_pTrie;
static HashItem *g_pHashPool;

bool LoadData()
{
	static bool s_init = false;
	static CReadOnlyShareMemory s_shareMemory;
	if(!s_init)
	{
		if(g_dataPath.empty())
		{
			error_log("[filter_data_path_empty][data_path=%s]", g_dataPath.c_str());
		}
		if(!s_shareMemory.Open(g_dataPath.c_str(), sizeof(FilterData)))
		{
			error_log("[open_filter_data_fail][path=%s]", g_dataPath.c_str());
		}
		g_pData = (FilterData *)s_shareMemory.GetAddress();
		if(g_pData != NULL)
		{
			g_minLength = 2;//g_pData->MinLength;
			g_pCharShift = g_pData->CharShift;
			g_pRoot = g_pData->Root;
			g_pTrie = g_pData->Trie;
			g_pHashPool = g_pData->HashPool;
		}
		s_init = true;
	}
	return g_pData != NULL;
}

bool StringFilter::Init(const string &dataPath)
{
	if(dataPath.empty())
	{
		error_log("[filter_data_path_empty][data_path=%s]", dataPath.c_str());
	}
	g_dataPath = dataPath;
	return true;
}

bool StringFilter::Check(const string &s)
{
	if(g_pData == NULL)
	{
		if(!LoadData())
		{
			return false;
		}
	}

	int size = s.size();
	if(size < g_minLength)
	{
		return true;
	}

	const unsigned char *pHead = (const unsigned char *)s.c_str();
	const unsigned char *pCurrHead = pHead + size - g_minLength;

	while(pCurrHead >= pHead)
	{
		const unsigned char *pCurr = pCurrHead;
		unsigned char cCurr = *pCurr;
		TrieNode *pCurrNode;
		bool compareNext = false;
		if(g_pRoot[cCurr] != NIL)
		{
			pCurrNode = &g_pTrie[g_pRoot[cCurr]];
			if(pCurrNode->Type == TN_MATCH)
			{
				return false;
			}
			cCurr = *(++pCurr);
			compareNext = true;
		}
		while(compareNext)
		{
			HashItem *pCurrHashItem = &pCurrNode->HashTable[cCurr % TRIE_HASH_SIZE];
			bool match = false;
			bool nextHash = true;
			while(nextHash)
			{
				if(pCurrHashItem->Value == NIL)
				{
					nextHash = false;
				}
				else if(g_pTrie[pCurrHashItem->Value].Value == cCurr)
				{
					match = true;
					nextHash = false;
				}
				else if(pCurrHashItem->Next == NIL)
				{
					nextHash = false;
				}
				else
				{
					pCurrHashItem = &g_pHashPool[pCurrHashItem->Next];
				}
			}
			if(match)
			{
				pCurrNode = &g_pTrie[pCurrHashItem->Value];
				if(pCurrNode->Type == TN_MATCH)
				{
					return false;
				}
				cCurr = *(++pCurr);
			}
			else
			{
				compareNext = false;
			}
		}

		int matchShift = pCurr - pCurrHead;
		int bcShift = 1;
		if(matchShift + 1 < g_minLength)
		{
			bcShift = g_pCharShift[cCurr] - matchShift;
		}
		int sundayShift = 1;
		if(pCurrHead > pHead)
		{
			sundayShift = g_pCharShift[*(pCurrHead - 1)] + 1;
			if(sundayShift > bcShift)
			{
				bcShift = sundayShift;
			}
		}
		if(bcShift <= 0)
		{
			bcShift = 1;
		}
		if(pHead + bcShift <= pCurrHead)
		{
			pCurrHead -= bcShift;
		}
		else
		{
			return true;
		}
	}

	return true;
}

bool StringFilter::Replace(string &s)
{
	if(g_pData == NULL)
	{
		if(!LoadData())
		{
			return false;
		}
	}
	// 审核用
	string tmpStr = s;
	for(string::iterator it = s.begin(); it != s.end();)
	{
		char c=*it;
		if(c == ' '
				|| c == '#'
				|| c == '!'
				|| c == '&'
				|| c == '$'
				|| c == '@'
				|| c == '^'
				|| c == ':'
				|| c == '\\'
				|| c == '/'
				|| c == '>'
				|| c == '<'
				|| c == '.'
				|| c == '='
				|| c == '}'
				|| c == '{'
				|| c == ']'
				|| c == '[')
		{
			it = s.erase(it);
		}else{
			++it;
		}
	}

	int size = s.size();
	if(size < g_minLength)
	{
		return true;
	}

	int currHeadIndex = size - g_minLength;
	bool isReplace = false;
	while(currHeadIndex >= 0)
	{
		int currIndex = currHeadIndex;
		unsigned char cCurr = (unsigned char)s[currIndex];
		bool match = false;
		TrieNode *pCurrNode;
		bool compareNext = false;
		if(g_pRoot[cCurr] != NIL)
		{
			pCurrNode = &g_pTrie[g_pRoot[cCurr]];
			if(pCurrNode->Type == TN_MATCH)
			{
				s.replace(currHeadIndex, 1, 1, '*');
				match = true;
				isReplace = true;
			}
			else
			{
				currIndex++;
				cCurr = (unsigned char)s[currIndex];
				compareNext = true;
			}
		}
		while(compareNext)
		{
			HashItem *pCurrHashItem = &pCurrNode->HashTable[cCurr % TRIE_HASH_SIZE];
			match = false;
			bool nextHash = true;
			while(nextHash)
			{
				if(pCurrHashItem->Value == NIL)
				{
					nextHash = false;
				}
				else if(g_pTrie[pCurrHashItem->Value].Value == cCurr)
				{
					match = true;
					nextHash = false;
				}
				else if(pCurrHashItem->Next == NIL)
				{
					nextHash = false;
				}
				else
				{
					pCurrHashItem = &g_pHashPool[pCurrHashItem->Next];
				}
			}
			if(match)
			{
				pCurrNode = &g_pTrie[pCurrHashItem->Value];
				if(pCurrNode->Type == TN_MATCH)
				{
					int length = currIndex - currHeadIndex + 1;
					string sToReplace;
					sToReplace.append(s.c_str() + currHeadIndex, length);
					int utf8Length = String::Utf8GetLength(sToReplace);
					s.replace(currHeadIndex, length, utf8Length, '*');
					compareNext = false;
					isReplace = true;

				}
				else
				{
					currIndex++;
					cCurr = (unsigned char)s[currIndex];
				}
			}
			else
			{
				compareNext = false;
			}
		}

		int matchShift = currIndex - currHeadIndex;
		int bcShift = 1;
		if(!match)
		{
			if(matchShift + 1 < g_minLength)
			{
				bcShift = g_pCharShift[cCurr] - matchShift;
			}
		}
		int sundayShift = 1;
		if(currHeadIndex > 0)
		{
			sundayShift = g_pCharShift[(unsigned char)s[currHeadIndex - 1]] + 1;
			if(sundayShift > bcShift)
			{
				bcShift = sundayShift;
			}
		}
		if(bcShift <= 0)
		{
			bcShift = 1;
		}
		if(bcShift <= currHeadIndex)
		{
			currHeadIndex -= bcShift;
		}
		else
		{
			if(!isReplace)
			{
				s = tmpStr;
			}
			return true;
		}
	}

	if(!isReplace)
	{
		s = tmpStr;
	}

	return true;
}

