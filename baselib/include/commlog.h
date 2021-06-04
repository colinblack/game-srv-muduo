//log类头文件
#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <string>
#include <map>
//log文件的最大序号
#define MAX_LOG_CNT 1000
//5层
#define APP_FATAL		0
#define APP_ERROR		1 
#define APP_WARNING	 	2
#define APP_INFO  		3
#define APP_DEBUG 		4
#define APP_BUSI		5

//传给BootLog，代表成功，或者失败
#define LOG_OK 0
#define LOG_FAIL -1

//写详细log信息的宏，代替WriteLog成员函数(包括文件名，行)
#define DETAIL(lvl, fmt, args...) \
	write_log (lvl, "[%s:%d][%s]" fmt "\n", __FILE__, __LINE__, __func__, ##args)

#define only_boot_log(ok, fmt, args...)          	mylog.write_boot_log(ok, 0, fmt, ##args);
//获取log对象声明
//log类初始
#define log_init(lvl,path, pre_name,rewin,size,record_size)\
	do {\
	CLog * p = CLog::GetInstance();\
	p->initalize(lvl,path, pre_name,rewin,size,record_size);\
	} while(0)

//#define fatal_log(fmt, args...)     CLog::GetInstance()->DETAIL(APP_FATAL, fmt, ##args)	//写fatal日志
//#define error_log(fmt, args...)     CLog::GetInstance()->DETAIL(APP_ERROR, fmt, ##args)    //写error日志
//#define warn_log(fmt, args...)      CLog::GetInstance()->DETAIL(APP_WARNING, fmt, ##args)  //写warn日志
//#define info_log(fmt, args...)      CLog::GetInstance()->DETAIL(APP_INFO, fmt, ##args)     //写info日志
//#define debug_log(fmt, args...)		  CLog::GetInstance()->DETAIL(APP_DEBUG, fmt, ##args)    //写debug日志
#define boot_log(ok, fmt, args...) { \
	mylog.write_boot_log(ok, 0, fmt, ##args); \
	if(ok == LOG_OK ) info_log(fmt, ##args); \
	else if(ok == LOG_FAIL)error_log(fmt, ##args); }

//add by bob 2011-03-15
#define fatal_log(fmt, args...) \
do\
{\
	std::string __file(__FILE__);\
	std::size_t pos = __file.rfind('/');\
	if( pos != std::string::npos )\
		__file.erase(0, pos + 1);\
	CLog::GetInstance()->write_log (APP_FATAL, "[%s:%d][%s]" fmt "\n", __file.c_str(), __LINE__, __func__, ##args);\
}while(0)

#define error_log(fmt, args...) \
do\
{\
	std::string __file(__FILE__);\
	std::size_t pos = __file.rfind('/');\
	if( pos != std::string::npos )\
		__file.erase(0, pos + 1);\
	CLog::GetInstance()->write_log (APP_ERROR, "[%s:%d][%s]" fmt "\n", __file.c_str(), __LINE__, __func__, ##args);\
}while(0)

#define warn_log(fmt, args...) \
do\
{\
	std::string __file(__FILE__);\
	std::size_t pos = __file.rfind('/');\
	if( pos != std::string::npos )\
		__file.erase(0, pos + 1);\
	CLog::GetInstance()->write_log (APP_WARNING, "[%s:%d][%s]" fmt "\n", __file.c_str(), __LINE__, __func__, ##args);\
}while(0)

#define info_log(fmt, args...) \
do\
{\
	std::string __file(__FILE__);\
	std::size_t pos = __file.rfind('/');\
	if( pos != std::string::npos )\
		__file.erase(0, pos + 1);\
	CLog::GetInstance()->write_log (APP_INFO, "[%s:%d][%s]" fmt "\n", __file.c_str(), __LINE__, __func__, ##args);\
}while(0)

#define debug_log(fmt, args...) \
do\
{\
	std::string __file(__FILE__);\
	std::size_t pos = __file.rfind('/');\
	if( pos != std::string::npos )\
		__file.erase(0, pos + 1);\
	CLog::GetInstance()->write_log (APP_DEBUG, "[%s:%d][%s]" fmt "\n", __file.c_str(), __LINE__, __func__, ##args);\
}while(0)

#define busi_log(logtype,fmt,args...)\
do\
{\
	CLog* pLog = CLog::GetInstance(logtype);\
	if(pLog != NULL) pLog->busi_write(fmt "\n",##args);\
}while(0)

#define BUSI_COIN(fmt,args...) busi_log("coin",fmt,##args)
#define BUSI_EQUIP(fmt,args...) busi_log("equip",fmt,##args)
#define BUSI_LOGIN(fmt,args...) busi_log("login",fmt,##args)
#define BUSI_TASK(fmt,args...) busi_log("task",fmt,##args)
#define BUSI_ONLINE(fmt,args...) busi_log("online",fmt,##args)
#define BUSI_BUY(fmt,args...) busi_log("buy",fmt,##args)
#define BUSI_TITLE(fmt, args...) busi_log("title", fmt, ##args)
#define BUSI_TOKEN(fmt, args...) busi_log("token", fmt, ##args)
#define BUSI_CHAT(fmt, args...) busi_log("chat", fmt, ##args)
#define BUSI_MISC(fmt, args...) busi_log("misc", fmt, ##args)

///////////////////////////////


//存放每层log文件的信息
typedef struct fds_t
{
    int seq;                      //序号
    int opfd;                     //log文件标识符
    unsigned short day;           //log文件的生成日期
    fds_t(){
		seq = 0;
		opfd = -1;
		day = 0;
	};
}  stfds;

class CLog
{
    private:
        char log_dir[256];
        char log_prefix[32];                       //log文件的存放目录及其前缀
        unsigned int log_size;                     //log文件的大小
		//unsigned int record_size;					//log记录大小
        //char * log_buffer;                         //写log文件时用到的buf
        stfds log_fd[APP_BUSI+1];                 //所有log文件的文件信息
        int log_level;                             //输出错误日志的最高层数

		static CLog * _pInstance;  
		static std::map<std::string,CLog*> _logMap;

		static std::string SRV_ID;
		static std::string PRO_ID;
		static std::string LOGGER_SERVER_IP;
		static short LOGGER_SERVER_PORT;

	int log_valid;							   //当前log对象是否可用

        inline void get_log_name(int lvl, int seq, char* file_name, time_t now);  //获得log文件的文件名
        int request_log_seq(int lvl);     //生成序列号，最大序列号由MAX_LOG_CNT决定
        int openfd(int lvl, time_t now);  //打开log文件
        int shiftfd(int lvl, time_t now,int len);                      //生成新的log文件fd信息
		int rewind;	//日志文件回绕标志 0：不回绕 ,1：回绕

    public:
        CLog();
        ~CLog();
        //dada@20120201: 线程安全改造，废弃record_size，固定为2000
        int initalize(int lvl,const char *logdir, const char* pre_name, int rewind,uint32_t size,uint32_t record_size);    //log类初始化函数，知道log文件的存放目录
        int write_log(int lvl, const char* fmt, ...);                                                                       //写日志
        int busi_write(const char* fmt,...);
        void write_boot_log(int result, int dummy, const char *fmt, ...);                                      //在屏幕上打印启动信息

        int get_level() {return log_level;}

		/*! 
		\brief 单件方法
		\return 指向实例的一个指针。该实例全程唯一。
		*/
		static CLog * GetInstance();
		static CLog	* GetInstance(const std::string& name);
		static int create_inst(const char* logtype,const char *logdir);
		static int initLoggerServer(const std::string& srvId, const std::string& proId, const std::string& ip, const short port);

		static std::map<int, CLog*> _logMapF;
		static void SetDomain(int domain);
		static int _domain;
};

extern volatile time_t 	now ;
#endif
