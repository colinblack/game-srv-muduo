/*
 * DataExchangeCode.h
 *
 *  Created on: 2012-12-5
 *      Author: Administrator
 */

#ifndef DATAEXCHANGECODE_H_
#define DATAEXCHANGECODE_H_
#include "Kernel.h"
struct DataExchangeCode{
	string code;
	int type;
	unsigned uid;
	unsigned gentime;
	unsigned deadline;
	unsigned  usetime;
};
class CDataExchangeCode :public CDataBaseDBC
{
public:
	CDataExchangeCode(int table = DB_EXCHANGE_CODE) : CDataBaseDBC(table) {}
	virtual ~CDataExchangeCode();
	int AddExchageCode( const DataExchangeCode &data );
	int GetExchageCode( DataExchangeCode &data );
	int SetExchageCode( const DataExchangeCode &data );

};

#endif /* DATAEXCHANGECODE_H_ */
