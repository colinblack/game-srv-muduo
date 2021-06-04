#ifndef __STR_OPT_SHAWXIAO_2008_07_10__
#define __STR_OPT_SHAWXIAO_2008_07_10__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <iconv.h>

using namespace std;

void StrReplace( std::string & strBig, const std::string & strsrc, const std::string &strdst );
int StrSeparate( const char* sSrc, const char* sSep, int* n, int iMaxNum );
int StrSeparate( char* sSrc, const char* sSep, char** sStart, int iMaxNum );
int StrReplace( const char* sSrc, const char* sFind, const char* sSet, char* sDst );
int StrReplace( const char* sSrc, const char* sFind, const int iSet, char* sDst );
int StrReplaceN(  const char* sSrc, char** ppFind, char** ppSet, int iNum, char* sDst  );
char* StrTrim( char* sSrc, const char* sFilter );

int FileToStream(const char* sPath, std::string &data, int iMoreSize, int iHeadMoreSize);
int StreamToFile(const char* sPath, const std::string &data);

string gbk2utf8( const string& src );
string utf82gbk( const string& src );
string int2str(int n);

#endif
