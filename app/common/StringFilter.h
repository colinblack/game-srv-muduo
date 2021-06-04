/*
 * StringFilter.h
 *
 *  Created on: 2011-9-8
 *      Author: dada
 */

#ifndef STRINGFILTER_H_
#define STRINGFILTER_H_

#include "Common.h"

namespace StringFilter
{
	bool GenerateData(const vector<string> &words, const string &dataPath);

	bool Init(const string &dataPath);

	bool Check(const string &s);
	bool Replace(string &s);
}

#endif /* STRINGFILTER_H_ */
