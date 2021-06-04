/*
 * DataName.cpp
 *
 *  Created on: 2015-5-12
 *      Author: Cream
 */

#include "DataName.h"
#include "crc.h"

int CDataName::AddName(const string &name, const string &open_id, unsigned uid)
{
	unsigned crc = CCRC::GetCrc32(name);
	if (crc > UID_MAX) {
		crc %= UID_MAX;
		if (crc < UID_MIN) {
			crc += UID_MIN;
		}
	}
	DBCREQ_DECLARE(DBC::InsertRequest, crc);
	DBCREQ_SET_KEY(name.c_str());
	DBCREQ_SET_STR_S(open_id);
	DBCREQ_SET_INT_S(uid);
	DBCREQ_EXEC;

	return 0;
}

int CDataName::GetName(const string &name, string &open_id, unsigned &uid)
{
	unsigned crc = CCRC::GetCrc32(name);
	if (crc > UID_MAX) {
		crc %= UID_MAX;
		if (crc < UID_MIN) {
			crc += UID_MIN;
		}
	}
	DBCREQ_DECLARE(DBC::GetRequest, crc);
	DBCREQ_SET_KEY(name.c_str());
	DBCREQ_NEED_BEGIN();
	DBCREQ_NEED(open_id);
	DBCREQ_NEED(uid);
	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;
	DBCREQ_GET_BEGIN();
	DBCREQ_GET_STR_S(open_id);
	DBCREQ_GET_INT_S(uid);

	return 0;
}

int CDataName::DelName(const string &name)
{
	unsigned crc = CCRC::GetCrc32(name);
	if (crc > UID_MAX) {
		crc %= UID_MAX;
		if (crc < UID_MIN) {
			crc += UID_MIN;
		}
	}
	DBCREQ_DECLARE(DBC::DeleteRequest, crc);
	DBCREQ_SET_KEY(name.c_str());
	DBCREQ_EXEC;

	return 0;
}
