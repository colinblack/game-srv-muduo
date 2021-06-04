/*
 * DataBaseDBC.cpp
 *
 *  Created on: 2011-1-4
 *      Author: LARRY
 */

#include "DataBaseDBC.h"


CDataBaseDBC::CDataBaseDBC( const int table ) : m_iTableId( table )
{
	m_dbcsrv = NULL;
	m_pRouteTable = NULL;
}

CDataBaseDBC::~CDataBaseDBC()
{
}

int CDataBaseDBC::Init()
{
	return 0;
}

int CDataBaseDBC::SetTableId( int tableid )
{
	m_iTableId = tableid;
	return 0 ;
}

DBC::Server* CDataBaseDBC::LoadHandle( unsigned uId )
{
	if(!m_pRouteTable)
	{
		char temp[250] = {0};
		strcpy(temp, MainConfig::GetAllServerPath(CONFIG_DBC_SERVER).c_str());
		m_pRouteTable = new CDBCRouteTable(temp);
	}
	m_dbcsrv = NULL;
	int iRet = m_pRouteTable->get_route( m_iTableId, uId, &m_dbcsrv, false, CDBCKeepAlive::_ka.GetMaxHandle(), NULL );
	if(iRet!=0)
	{
		error_log( "dbc get_route failed:%s", m_pRouteTable->get_msg().c_str());
	}
	return m_dbcsrv;
}

DBC::Server* CDataBaseDBC::GetHandle( unsigned uId )
{
	return m_dbcsrv;
}

void CDataBaseDBC::PrintfError(  )
{
	printf( "<ResultCode>%d</ResultCode>\n"
		"<ErrorMessage>%s</ErrorMessage>\n"
		"<ErrorFrom>%s</ErrorFrom>\n",
		m_dbcret.ResultCode(),
		m_dbcret.ErrorMessage(),
		m_dbcret.ErrorFrom() );
}
