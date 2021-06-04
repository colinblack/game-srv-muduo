#ifndef _INIFILE_H
#define _INIFILE_H
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
typedef std::map<std::string, std::string, std::less<std::string> > strMap;
typedef strMap::iterator strMapIt;

const std::string MIDDLESTRING = "_____***_______";

class IniFile
{
public:
    IniFile( ){};
    ~IniFile( ){};
    int open(std::string pinipath);
    std::string read(std::string psect, std::string pkey);
protected:
    int do_open(std::string pinipath);
    strMap c_inimap;
};
#endif
