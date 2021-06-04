/*
 * ErrorUtil.cpp
 *
 *  Created on: 2011-8-8
 *      Author: dada
 */

#include "ErrorUtil.h"

static int s_lastError = 0;
static string s_lastErrorMessage;

void ResetError()
{
	s_lastError = R_SUCCESS;
	s_lastErrorMessage.clear();
}

void SetError(int error)
{
	s_lastError = error;
	s_lastErrorMessage.clear();
}

void SetError(int error, const char *errorMessage)
{
	s_lastError = error;
	s_lastErrorMessage = errorMessage;
}

void SetError(int error, const string &errorMessage)
{
	s_lastError = error;
	s_lastErrorMessage = errorMessage;
}

int GetError()
{
	return s_lastError;
}

const char *GetErrorMessage()
{
	static char s_error[][20] = {
			"success",
			"error",
			"server_refuse",
			"session_error",
			"auth_error",
			"param_error",
			"db_error",
			"data_error",
			"no_data",
			"data_limit",
			"logic_error",
			"platform_error",
			"unknow_error"
	};
	if(!s_lastErrorMessage.empty())
	{
		return s_lastErrorMessage.c_str();
	}
	if(s_lastError >= 0 && s_lastError < (int)COUNT_OF(s_error) - 1)
	{
		return s_error[s_lastError];
	}
	else
	{
		return s_error[COUNT_OF(s_error) - 1];
	}
}
