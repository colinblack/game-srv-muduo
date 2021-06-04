#include "Kernel.h"
#include <fstream>
#include <sstream>
#include "Parser.h"

#define MAGIC_HEAD 20180125
#define MAGIC_VERSION 20181111

Parser::Parser()
{
	m_width = 0;
	m_height = 0;
}

Parser::~Parser()
{

}

bool Parser::parse(const string& path)
{
	string content;
	if(File::Read(path,content) != 0)
	{
		error_log("open file %s failed",path.c_str());
		return false;
	}

	const char* pData = content.data();
	uint32_t magicHead;
	uint32_t magicVer;

	magicHead = *((uint32_t*)pData);
	pData += sizeof(uint32_t);
	magicVer = *((uint32_t*)pData);
	pData += sizeof(uint32_t);

	if(magicHead != MAGIC_HEAD || magicVer != MAGIC_VERSION)
	{
		error_log("invalid head or version:[%u,%u]",magicHead,magicVer);
		return false;
	}

	m_width = *((uint32_t*)pData);
	pData += sizeof(uint32_t);
	m_height = *((uint32_t*)pData);
	pData += sizeof(uint32_t);

	info_log("%s width height[%u %u]", path.c_str(), m_width, m_height);

	string rawData;
	string cmpCnt;

	cmpCnt.append(pData,content.size() - 16);
/*
	if(!Compress::GzipUncompress(rawData,cmpCnt)){
		error_log("uncompress failed") ;
		return false;
	}
*/
	rawData = cmpCnt;
	if(rawData.size() < m_width * m_height){
		error_log("wwm content length error,expect %u but give %u ,[%u,%u]",m_width * m_height,rawData.size(),m_width,m_height);
		return false;
	}

	//File::Write("/data/lipman/fire_srv/server/app/conf/zip.wwm",rawData);
	m_cells.resize(GRID_LENGTH, 0);
	pData = rawData.data();

	/*
	byte flag;//1:陆地 2:水面 3:山峰
	int pos = 0, left = 0;

	for(uint32_t i = 0; i < m_width * m_height; ++i)
	{
		flag = *((uint16_t*)pData);
		pData += 1;
		//修改下，让m_cells中的数据只存位标志
		if (flag > 0)
		{
			pos = (i)/INT_BITS;
			left = (i) % INT_BITS;
			m_cells[pos] |= 1 <<left;
		}
	}
	*/

	char tmp[MAP_MAX_GRID]={0};
	for(uint32_t i = 0; i < m_height; ++i)
	{
		for(uint32_t j = 0; j < m_width; ++j)
		{
			for(uint32_t t=0;t<4;++t)
				tmp[i*m_width*4 + m_width*t + j] = *pData;
			++pData;
		}
	}

	byte flag;//1:陆地 2:水面 3:山峰
	int pos = 0, left = 0;

	for(uint32_t i = 0; i < MAP_MAX_GRID; ++i)
	{
		flag = tmp[i];
		//修改下，让m_cells中的数据只存位标志
		if (flag > 0)
		{
			pos = (i)/INT_BITS;
			left = (i) % INT_BITS;
			m_cells[pos] |= 1 <<left;
		}
	}

	//debug_log("21469:%d 21470:%d 21471:%d",m_cells[21469],m_cells[21470],m_cells[21471]);
	return true;
}

bool Parser::isMovable(uint32_t x, uint32_t y, uint32_t type)
{
	if(x >= m_width || y >= m_height)
	{
		return false;
	}
	if(m_cells[y*m_width+x] == type)
	{
		return true;
	}
	else
	{
		return false;
	}
}
