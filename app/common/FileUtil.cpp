/*
 * FileUtil.cpp
 *
 *  Created on: 2011-6-1
 *      Author: dada
 */

#include "FileUtil.h"

CFile::CFile()
{
	m_fd = -1;
}

CFile::~CFile()
{
	Close();
}

CFile::operator int()
{
	return m_fd;
}

int CFile::Open(const char *path, int oflag)
{
	m_fd = open(path, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if(m_fd == -1)
	{
		return errno;
	}
	return 0;
}

int CFile::Close()
{
	if(m_fd != -1)
	{
		int result = close(m_fd);
		m_fd = -1;
		if(result != 0)
		{
			return errno;
		}
	}
	return 0;
}

int CFile::Read(void *buf, size_t &count)
{
	ssize_t readCount = read(m_fd, buf, count);
	if(readCount == -1)
	{
		return errno;
	}
	count = size_t(readCount);
	return 0;
}

int CFile::Write(const void *buf, size_t count)
{
	ssize_t n = write(m_fd, buf, count);
	if(n == -1)
	{
		return errno;
	}
	if(n != (ssize_t)count)
	{
		return -1;
	}
	return 0;
}

bool File::IsExist(const string &path)
{
//	CFile file;
//	int result;
//	result = file.Open(path.c_str(), O_RDONLY);
//	return result == 0;
	return access(path.c_str(), F_OK) == 0;
}

int File::Read(const string &path, string &content)
{
	CFile file;
	int result;
	result = file.Open(path.c_str(), O_RDONLY);
	if(result != 0)
	{
		return result;
	}

	char buffer[2000];
	size_t length = sizeof(buffer);
	size_t readLength = length;
	content.clear();
	while(readLength == length)
	{
		readLength = length;
		result = file.Read(buffer, readLength);
		if(result != 0)
		{
			return result;
		}
		content.append(buffer, readLength);
		if(readLength != length && buffer[readLength - 1] != '\0')
		{
			content.append("\0");
		}
	}
	return 0;
}

int File::Write(const string &path, const string &content)
{
	CFile file;
	int result;
	result = file.Open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC );
	if(result != 0)
	{
		return result;
	}
	result = file.Write(content.c_str(), content.size());
	return result;
}

int File::Append(const string &path, const string &content)
{
	CFile file;
	int result;
	result = file.Open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND);
	if(result != 0)
	{
		return result;
	}
	result = file.Write(content.c_str(), content.size());
	return result;
}

int File::Clear(const string &path)
{
	CFile file;
	int result;
	result = file.Open(path.c_str(), O_TRUNC);
	return result;
}

int File::RecursiveMkdir(const char * pPath)
{
    if (NULL == pPath)
    {
        return -1;
    }

    if (0 == access(pPath, F_OK))
    {
       return 0;
    }

    int nRet = mkdir(pPath, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH|S_IWGRP|S_IWOTH);   //需要头文件<sys/stat.h>的支持

    if (0 == nRet)
    {
        return 0;
    }
    else  //创建目录失败，多级目录。递归创建
    {
        char szLocalPath[100] = {0};
        char * pstr = strrchr((char *)pPath, '/');   //需要头文件<string.h>支持
        int len = pstr - pPath;

        if (len >= sizeof(szLocalPath))
        {
            len = sizeof(szLocalPath) -1;  //留1个位给最终的\0结束字符
        }

        strncpy(szLocalPath, pPath, len);  // 需要头文件<string.h>支持
        szLocalPath[len+1] = '\0';
        cout<<"递归创建的路径是"<<szLocalPath<<endl;
        RecursiveMkdir(szLocalPath);
        return RecursiveMkdir(pPath);
    }
}

