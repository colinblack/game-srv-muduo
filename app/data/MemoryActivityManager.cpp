#include "MemoryActivityManager.h"
int MemoryActivityManager::OnInit()
{
	return 0;
}

bool MemoryActivityManager::IsOn(uint32_t actId)
{
	return (actId < MEMORY_ACTIVITY_FULL) ? (m_data->item[actId].isOn > 0) : false;
}

void MemoryActivityManager::SetOn(uint32_t actId, uint8_t isOn)
{
	if(actId < MEMORY_ACTIVITY_FULL)
	{
		if(isOn > 0)	//打开
		{
			m_data->item[actId].isOn = 1;
		}
		else if(m_data->item[actId].isOn == 1)	// 关闭(防止重复关闭)
		{
			m_data->item[actId].isOn = 0;
			m_data->item[actId].isSettle = 0;
		}
		info_log("activity_status_change isOn=%u", isOn);
	}
	else
	{
		error_log("actId_out_of_limit actId=%u", actId);
		throw runtime_error("actId_out_of_limit");
	}
}

bool MemoryActivityManager::Settle(uint32_t actId)
{
	if(actId < MEMORY_ACTIVITY_FULL && m_data->item[actId].isOn == 0 && m_data->item[actId].isSettle == 0)//活动已关闭,且未结算过
	{
		info_log("settle_activity actId=%u", actId);
		m_data->item[actId].isSettle = 1;
		return true;
	}
	else
	{
		error_log("Settle_fail actId=%u", actId);
		return false;
	}
}
