#include"DataFriendWorkerManager.h"

void DataFriendWorkerManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataFriendWorker & friendWorker = GetDataByIndex(indexs[i]);

		DelItem(friendWorker.uid, friendWorker.id);
	}

	//新增新数据
	for(int i = 0; i < data.friendworker_size(); ++i)
	{
		unsigned id = data.friendworker(i).workeruid();

		DataFriendWorker & friendWorker = GetData(uid, id);
		friendWorker.FromMessage(&data.friendworker(i));
	}
}
