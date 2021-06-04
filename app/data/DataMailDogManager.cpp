#include "DataMailDogManager.h"

void DataMailDogManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataMailDog & maildog = GetDataByIndex(indexs[i]);

		DelItem(maildog.uid, maildog.id);
	}

	//新增新数据
	for(int i = 0; i < data.maildog_size(); ++i)
	{
		unsigned id = data.maildog(i).id();

		DataMailDog & maildog = GetData(uid, id);

		maildog.FromMessage(&data.maildog(i));
	}
}
