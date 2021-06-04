/*
 * ParseManager.cpp
 *
 *  Created on: 2015-11-12
 *      Author: Administrator
 */

#include "ParseManager.h"

ParseManager::ParseManager():
	m_parsers(NULL)
{

}

ParseManager::~ParseManager()
{
	delete m_parsers;
}

bool ParseManager::Init()
{
	string path = MainConfig::GetAllServerPath(CONFIG_MAP_PATH);

	if (NULL != m_parsers)
	{
		delete m_parsers;
	}

	m_parsers = new Parser();

	if(!m_parsers->parse(path))
	{
		return false;
	}

	return true;
}
