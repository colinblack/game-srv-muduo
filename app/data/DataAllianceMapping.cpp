#include "DataAllianceMapping.h"

int CDataAllianceMapping::Get(DataAllianceMapping &data)
{
	DBCREQ_DECLARE(DBC::GetRequest, CCRC::GetCrc32(data.alliance_name));
	DBCREQ_SET_KEY(data.alliance_name);

	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(alliance_id);

	DBCREQ_EXEC;

	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;
	DBCREQ_GET_BEGIN();

	DBCREQ_GET_INT(data, alliance_id);

	return 0;
}

int CDataAllianceMapping::Add(DataAllianceMapping &data)
{
	DBCREQ_DECLARE(DBC::InsertRequest, CCRC::GetCrc32(data.alliance_name));

	DBCREQ_SET_KEY(data.alliance_name);
	DBCREQ_SET_INT(data, alliance_id);

	DBCREQ_EXEC;

	return 0;
}

int CDataAllianceMapping::Del(const char * name)
{
	DBCREQ_DECLARE(DBC::DeleteRequest, CCRC::GetCrc32(name));
	DBCREQ_SET_KEY(name);

	DBCREQ_EXEC;

	return 0;
}
