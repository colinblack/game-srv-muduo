
/*!
 * \file staticdata.h
 * \author FelitHuang
 * \date 2008-08-26
*/

#ifndef	STATICDATA_H
#define	STATICDATA_H

struct STATICDATA
{
	STATICDATA()
	{
	}
	~STATICDATA()
	{
		m_iCGIValueState = 0;
		m_iCookieState = 0;
	}
	static int m_iCGIValueState;
	static int m_iCookieState;
};

#endif
