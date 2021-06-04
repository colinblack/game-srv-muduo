/*
 * AsynManager.cpp
 *
 *  Created on: 2016-10-9
 *      Author: Ralf
 */


#include "AsynManager.h"


int AsynManager::OnInit() {
	for(unsigned i=0;i<MEMORY_PROPERTY_NUM*MEMORY_ASYN_NUM;++i)
	{
		if(m_data->item[i].uid != 0)
			m_map[m_data->item[i].uid][m_data->item[i].id] = i;
		else
			m_freeIndex.insert(i);
	}
	return 0;
}
