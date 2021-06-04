/*
 * DataFeedback.h
 *
 *  Created on: 2018-07-29
 *      Author: que
 */

#ifndef DATA_USER_FEEDBACK_H_
#define DATA_USER_FEEDBACK_H_

#include "Kernel.h"
#include "ProtoMessageBoard.pb.h"

#define DATA_FEEDBACK_CONTENTS_COUNT 35*4


struct DataFeedback {
	unsigned uid;
	unsigned id;
	char contents[DATA_FEEDBACK_CONTENTS_COUNT];
	DataFeedback() {
		uid = 0;
		id = 0;
		memset(contents, 0, sizeof(contents));
	}
	void SetMessage(ProtoMessageBoard::FeedbackInfo* msg)
	{
		msg->set_ts(id);
		msg->set_id(id);
		msg->set_words(string(contents));
	}
};
class CDataFeedback: public DBCBase<DataFeedback, DB_FEEDBACK> {
public:
	virtual int Get(DataFeedback &data) {
		DBCREQ_DECLARE(DBC::GetRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(contents);
		DBCREQ_EXEC;
		DBCREQ_IFNULLROW;
		DBCREQ_IFFETCHROW;
		DBCREQ_GET_BEGIN();
		DBCREQ_GET_INT(data, uid);
		DBCREQ_GET_INT(data, id);
		DBCREQ_GET_CHAR(data, contents, DATA_FEEDBACK_CONTENTS_COUNT);
		return 0;
	}
	virtual int Get(vector<DataFeedback> &data) {
		if (data.empty())
			return R_ERROR;
		DBCREQ_DECLARE(DBC::GetRequest, data[0].uid);
		DBCREQ_SET_KEY(data[0].uid);
		data.clear();
		DBCREQ_NEED_BEGIN();
		DBCREQ_NEED(uid);
		DBCREQ_NEED(id);
		DBCREQ_NEED(contents);
		DBCREQ_EXEC;
		DBCREQ_ARRAY_GET_BEGIN(data);
		DBCREQ_ARRAY_GET_INT(data, uid);
		DBCREQ_ARRAY_GET_INT(data, id);
		DBCREQ_ARRAY_GET_CHAR(data, contents, DATA_FEEDBACK_CONTENTS_COUNT);
		DBCREQ_ARRAY_GET_END();
		return 0;
	}
	virtual int Add(DataFeedback &data) {
		DBCREQ_DECLARE(DBC::InsertRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_CHAR(data, contents, DATA_FEEDBACK_CONTENTS_COUNT);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Set(DataFeedback &data) {
		DBCREQ_DECLARE(DBC::UpdateRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_SET_INT(data, id);
		DBCREQ_SET_CHAR(data, contents, DATA_FEEDBACK_CONTENTS_COUNT);
		DBCREQ_EXEC;
		return 0;
	}
	virtual int Del(DataFeedback &data) {
		DBCREQ_DECLARE(DBC::DeleteRequest, data.uid);
		DBCREQ_SET_KEY(data.uid);
		DBCREQ_SET_CONDITION(EQ, id, data.id);
		DBCREQ_EXEC;
		return 0;
	}
};

#endif
