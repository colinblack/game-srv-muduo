#ifndef __DATA_LOG_H__
#define __DATA_LOG_H__

#include <string>
#include <vector>

#define DATA_LOG(action,ip,type,fmt,args...) CDataLog::DataLog(action,ip,type,fmt,##args)
#define RECV_LOG(action,ip,fmt,args...) CDataLog::DataLog(action,ip,"recv",fmt,##args)
#define SEND_LOG(action,ip,fmt,args...) CDataLog::DataLog(action,ip,"send",fmt,##args)
#define DATA_INFO_LOG(action, ...) CDataLog::DataLog(action, "", "info", __VA_ARGS__)

class CDataLog
{
public:
	//记录cgi的收发情况
	static void DataLog(const char *action, const char *ip, const char * type, const char* format, ...);

	static std::string& ParseIP( const std::string pyIp, std::string& realIp );

public:
	//static char log_dir[100];

private:
	static int fd;
	static int write_times;
	static int last_day;
	static bool init( struct tm **p );
};

#endif
