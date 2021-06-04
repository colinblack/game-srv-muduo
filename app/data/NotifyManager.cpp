#include "NotifyManager.h"

int NotifyManager::OnInit()
{
	for(unsigned i=0;i< MAX_SIZE; ++i)
	{
		if(m_data->item[i].uid != 0)
			m_map[m_data->item[i].uid][m_data->item[i].id] = i;
		else
			m_freeIndex.insert(i);
	}

	return 0;
}

int NotifyManager::ClearAllItemById(unsigned id)
{
	for(NotifyMap::iterator niter = m_map.begin(); niter != m_map.end(); ++niter)
	{
		unsigned uid = niter->first;

		for(map<unsigned, unsigned>::iterator uiter = m_map[uid].begin(); uiter != m_map[uid].end(); )
		{
			if (uiter->first == id)
			{
				//删除
				unsigned index = uiter->second;
				m_data->item[index].Clear();
				m_freeIndex.insert(index);

				m_map[uid].erase(uiter++);
			}
			else
			{
				uiter++;
			}
		}
	}

	return 0;
}
