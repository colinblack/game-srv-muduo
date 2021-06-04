/*
 * ResourceManager.cpp
 *
 *  Created on: 2016-9-2
 *      Author: Ralf
 */

#include "ResourceManager.h"

int ResourceManager::OnInit() {
	for(unsigned i=0;i<MEMORY_PROPERTY_NUM;++i)
	{
		if(m_data->item[i].uid != 0)
			m_map[m_data->item[i].uid] = i;
		else
			m_freeIndex.insert(i);
	}
	return 0;
}
