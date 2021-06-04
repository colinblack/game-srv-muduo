/*
 * FileUtil.h
 *
 *  Created on: 2011-6-1
 *      Author: dada
 */

#ifndef FILEUTIL_H_
#define FILEUTIL_H_

#include "Common.h"

class CFile
{

public:

	CFile();
	~CFile();

	operator int();	//get fd

	int Open(const char *path, int oflag);
	int Close();

	int Read(void *buf, size_t &count);
	int Write(const void *buf, size_t count);


private:
	int m_fd;
};

namespace File
{
	bool IsExist(const string &path);
	int Read(const string &path, string &content);
	int Write(const string &path, const string &content);
	int Append(const string &path, const string &content);
	int Clear(const string &path);
    int RecursiveMkdir(const char * pPath);
}

#endif /* FILEUTIL_H_ */
