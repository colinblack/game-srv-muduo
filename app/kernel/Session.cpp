/*
 * Session.cpp
 *
 *  Created on: 2011-1-10
 *      Author: LARRY
 */

#include "Session.h"

class CDataSession : public CDataBaseDBC
{
public:
	CDataSession(int table = MEM_SESSION) : CDataBaseDBC(table) {}
	virtual ~CDataSession() {}

public:
	const string CreateNewSessionID(unsigned uid);
	int GetSessionAttribute(unsigned uid,const string &key,string & value);
	string GetSessionAttribute(unsigned uid,const string &key);
	int SetSessionAttribute(unsigned uid,const string &key,const string &value);
	int RemoveSessionAttribute(unsigned uid,const string &key);
	int DestorySession(unsigned uid);
	int init(unsigned uid);
};

const string CDataSession::CreateNewSessionID(unsigned uid)
{
	string skey = "";
	//srand(time(NULL) + uid);	//not need
	int range = 36;
	for (int i = 0; i < 8 ; i++ )
	{
		int c = rand() % range;
		if(c >= 26)
		{
			c -= 26;
			//skey += CTrans::ITOS(c) ;
			skey += (char)('0' + c);
		}
		else
		{
			skey += (char)('a' + c);
		}
	}
	return skey;
}

string CDataSession::GetSessionAttribute(unsigned uid,const string &key)
{
	string value;
	GetSessionAttribute(uid,key,value);
	return value;
}

int CDataSession::GetSessionAttribute(unsigned uid,const string &key,string &value)
{
	DBCREQ_DECLARE( DBC::GetRequest, uid );

	req.SetKey( uid);
	req.Need( "svalue", 1 );
	req.Need( "lastcmod", 2 );
	req.EQ("skey",key.c_str());

	DBCREQ_EXEC;
	DBCREQ_IFNULLROW;
	DBCREQ_IFFETCHROW;

	value = m_dbcret.StringValue( 1 );
	//long t = m_dbcret.IntValue( 2 );

	return 0;
}

int CDataSession::SetSessionAttribute(unsigned uid,const string &key,const string &value)
{
	DBCREQ_DECLARE( DBC::ReplaceRequest,uid );

	long now = 0;
	mtime(now);

	req.SetKey( uid );
	req.Set("skey",key.c_str() );
	req.Set( "svalue", value.c_str() );
	req.Set( "lastcmod", now );

	//req.EQ("skey",key.c_str());

	DBCREQ_EXEC;

	return 0;
}

int CDataSession::RemoveSessionAttribute(unsigned uid,const string &key)
{
	DBCREQ_DECLARE( DBC::DeleteRequest, uid );

	long now = 0;
	mtime(now);

	req.SetKey( uid  );
	req.EQ("skey",key.c_str() );

	DBCREQ_EXEC;

	return 0;
}

int CDataSession::DestorySession(unsigned uid)
{

	DBCREQ_DECLARE( DBC::DeleteRequest, uid );

	long now = 0;
	mtime(now);

	req.SetKey( uid  );

	DBCREQ_EXEC;
	return 0;
}

namespace Session
{
	static CDataSession g_session;
	string CreateSessionKey(unsigned uid)
	{
		return g_session.CreateNewSessionID(uid);
	}
	string GetValue(unsigned uid,const string &key)
	{
		return g_session.GetSessionAttribute(uid, key);
	}
	int GetValue(unsigned uid,const string &key,string &value)
	{
		return g_session.GetSessionAttribute(uid, key, value);
	}
	int SetValue(unsigned uid,const string &key,const string &value)
	{
		return g_session.SetSessionAttribute(uid, key, value);
	}
	int RemoveValue(unsigned uid,const string &key)
	{
		return g_session.RemoveSessionAttribute(uid, key);
	}
	int RemoveSession(unsigned uid)
	{
		return g_session.DestorySession(uid);
	}
}
