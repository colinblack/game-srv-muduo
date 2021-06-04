#include "BusinessLog.h"
#include "Kernel.h"

static char s_logbuf[4096] = {0};

CBusinessLog::CBusinessLog(const std::string &name)
	: m_name(name), m_fd(-1), m_day(0)
{

}

void CBusinessLog::Log(const char* format, ...)
{
	time_t now;
	time( &now );
	struct tm *ptm = localtime(&now);
	if (m_fd < 0 || ptm->tm_mday != m_day)
	{
		if (m_fd >= 0)
		{
			close(m_fd);
			m_fd = -1;
		}
		string dir = Config::GetPath(CONFIG_BUSINESS_LOG_DIR);
		if (dir.empty())
			return;
		if (dir[dir.length() - 1] != '/')
			dir += "/";
		snprintf(s_logbuf, sizeof(s_logbuf), "%s%s_%04d%02d%02d.log", dir.c_str(), m_name.c_str(),
				1900+ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday);
		m_fd = open(s_logbuf, O_CREAT | O_WRONLY | O_APPEND, 0644);
		m_day = ptm->tm_mday;
	}
	if (m_fd < 0)
		return;
	snprintf(s_logbuf, sizeof(s_logbuf), "%04d-%02d-%02d %02d:%02d:%02d|",
			1900+ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	int len = strlen(s_logbuf);
	va_list args;
	va_start(args, format);
	vsnprintf(s_logbuf+len, sizeof(s_logbuf)-len-2, format, args);
	va_end(args);
	strcat(s_logbuf, "\r\n");

	write(m_fd, s_logbuf, strlen(s_logbuf));
}

CBusinessLog * CBusinessLogHelper::GetInstance(const std::string &name)
{
	static map<string, CBusinessLog *> s_logInstance;
	map<string, CBusinessLog *>::iterator it = s_logInstance.find(name);
	if (it == s_logInstance.end())
	{
		s_logInstance[name] = new CBusinessLog(name);
	}
	return s_logInstance[name];
}
