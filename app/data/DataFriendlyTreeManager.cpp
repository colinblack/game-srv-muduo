#include "DataFriendlyTreeManager.h"

void DataFriendlyTreeManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataFriendlyTree & tree = GetDataByIndex(indexs[i]);

		DelItem(tree.uid, tree.id);
	}

	//新增新数据
	for(int i = 0; i < data.friendlytree_size(); ++i)
	{
		unsigned id = data.friendlytree(i).id();

		DataFriendlyTree & tree = GetData(uid, id);

		tree.FromMessage(&data.friendlytree(i));
	}
}

unsigned DataFriendlyTreeManager::GetNextWaterUd(unsigned uid)
{
	vector<unsigned> indexs;
	indexs.clear();
	GetIndexs(uid, indexs);

	unsigned maxud = 0;
	for(int i = 0; i < indexs.size(); i++)
	{
		DataFriendlyTree & tree = GetDataByIndex(indexs[i]);
		maxud = (maxud > tree.id) ? maxud : tree.id;
	}
	return maxud + 1;
}
