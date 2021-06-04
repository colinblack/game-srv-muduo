/*
 * ErrorUtil.h
 *
 *  Created on: 2011-8-8
 *      Author: dada
 */

#ifndef ERRORUTIL_H_
#define ERRORUTIL_H_

#include "Common.h"

enum ERESULT
{
	R_SUCCESS			= 0,	//成功
	R_ERROR				= 1,	//失败
	R_ERR_REFUSE		= 2,	//拒绝服务
	R_ERR_SESSION		= 3,	//会话错误
	R_ERR_AUTH			= 4,	//权限错误
	R_ERR_PARAM			= 5,	//参数错误
	R_ERR_DB			= 6,	//数据库错误
	R_ERR_DATA			= 7,	//数据错误
	R_ERR_NO_DATA		= 8,	//无数据
	R_ERR_DATA_LIMIT	= 9,	//数据限制
	R_ERR_LOGIC			= 10,	//逻辑错误
	R_ERR_PLATFORM		= 11,	//平台错误
	R_ERR_DULP			= 12,	//重复插入
};

void ResetError();
void SetError(int error);
void SetError(int error, const char *errorMessage);
void SetError(int error, const string &errorMessage);
int GetError();
const char *GetErrorMessage();

#define ERROR_RETURN(ErrorType)	\
	::SetError(ErrorType);	\
	return ErrorType	\

#define SUCCESS_RETURN()	ERROR_RETURN(R_SUCCESS)
#define PARAM_ERROR_RETURN()	ERROR_RETURN(R_ERR_PARAM)
#define DB_ERROR_RETURN()	ERROR_RETURN(R_ERR_DB)
#define DATA_ERROR_RETURN()	ERROR_RETURN(R_ERR_DATA)

#define ERROR_RETURN_MSG(ErrorType, msg)	\
	::SetError( ErrorType, msg );	\
	return ErrorType	\

#define PARAM_ERROR_RETURN_MSG(msg)	ERROR_RETURN_MSG(R_ERR_PARAM, msg)
#define DB_ERROR_RETURN_MSG(msg)	ERROR_RETURN_MSG(R_ERR_DB, msg)
#define NO_DATA_RETURN_MSG(msg)		ERROR_RETURN_MSG(R_ERR_NO_DATA, msg)
#define DATA_ERROR_RETURN_MSG(msg)	ERROR_RETURN_MSG(R_ERR_DATA, msg)
#define LOGIC_ERROR_RETURN_MSG(msg)	ERROR_RETURN_MSG(R_ERR_LOGIC, msg)
#define PT_ERROR_RETURN_MSG(msg)	ERROR_RETURN_MSG(R_ERR_PLATFORM, msg)
#define SESS_ERROR_RETURN_MSG(msg)	ERROR_RETURN_MSG(R_ERR_SESSION, msg)
#define REFUSE_RETURN_MSG(msg)		ERROR_RETURN_MSG(R_ERR_REFUSE, msg)

#endif /* ERRORUTIL_H_ */
