#include "DataMissionManager.h"

void DataMissionManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataMission & mission = GetDataByIndex(indexs[i]);

		DelItem(mission.uid, mission.id);
	}

	//新增新数据
	for(int i = 0; i < data.mission_size(); ++i)
	{
		unsigned id = data.mission(i).id();

		DataMission & mission = GetData(uid, id);

		mission.FromMessage(&data.mission(i));
	}
}
