#include "DataTaskManager.h"

void DataTaskManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataTask & task = GetDataByIndex(indexs[i]);

		DelItem(task.uid, task.id);
	}

	//新增新数据
	for(int i = 0; i < data.task_size(); ++i)
	{
		unsigned id = data.task(i).id();

		DataTask & task = GetData(uid, id);

		task.FromMessage(&data.task(i));
	}
}
