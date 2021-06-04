#include "DataPetManager.h"


void DataPetManager::FromMessage(unsigned uid, const ProtoArchive::UserData & data)
{
	//删除旧数据
	vector<unsigned> indexs;
	GetIndexs(uid, indexs);

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataPet & pet = GetDataByIndex(indexs[i]);

		DelItem(pet.uid, pet.id);
	}

	//新增新数据
	for(int i = 0; i < data.pet_size(); ++i)
	{
		unsigned id = data.pet(i).petid();

		DataPet & pet = GetData(uid, id);

		pet.FromMessage(&data.pet(i));
	}
}

