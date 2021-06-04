
#ifndef __EMAIL__
#define __EMAIL__

#include <string>

struct QQMail
{
	/*
	 * 发送系统邮件
	 * param[in] sNameSend: 显示的发送名称[eg. QQ秀项目组]
	 * param[in] sMailSend: 显示的发送邮箱[eg. show@tencent.com]
	 * param[in] sMailRecv: 接收的邮箱[eg. 10001@qq.com]
	 * param[in] sTitle: 标题
	 * param[in] sContent: 内容
	 * return int [0--succ !0--fail]
	*/
	int send(const std::string& sNameSend, const std::string& sMailSend, const std::string& sMailRecv, const std::string& sTitle, const std::string& sContent, int iMailTime, const std::string& sHost, int iPort, int nTimeOut, const std::string& sHelo="qq.com");
};

#endif

