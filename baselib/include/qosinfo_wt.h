#ifndef _QOS_WT_H_
#define _QOS_WT_H_
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <sys/time.h>
#include <vector>
#include <map>
#include <ext/hash_map>
#include <stdio.h>
using namespace std;
using namespace __gnu_cxx;

//extern int Log(const char* fmt, ...);
//#define _SYS_LOG_	Log("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);

typedef struct HOSTINFOtag
{
	unsigned	_host_ip;		//
	short		_host_port;		//
	int			_sock_fd;		//
	float		_sock_recv_timeout;	//接收超时
	float		_sock_send_timeout;	//发送超时
	bool		_sock_keepcnt;	//
	bool		_sock_reuse;	//
	
	HOSTINFOtag()
	{
		_host_ip = 0;
		_host_port = 0;
		_sock_fd = -1;
		_sock_recv_timeout = 2.0;
		_sock_send_timeout = 1.0;
		_sock_keepcnt = true;
		_sock_reuse = false;
	};
	
	static unsigned HostStr2Int(char* host_addr)
	{
		return (unsigned)inet_addr(host_addr);
	};
	
	static char* HostInt2Str(unsigned host_addr)
	{
		struct in_addr in;
		in.s_addr = host_addr;
		return (char*)inet_ntoa(in);
	};
	
	static void Host2Str(unsigned host_ip,short host_port,string& host_str)
	{
		host_str.reserve(64);host_str = "";
		int _ret = snprintf((char*)host_str.c_str(),63,"%u%d",host_ip,host_port);		
		if ( _ret > 0 )*((char*)host_str.c_str() + _ret) = '\0';
		
		return ;
	};
	
}HOSTINFO;

typedef struct QOSHOSTINFOtag
{
	HOSTINFO		_host_info;			//主机信息
	float			_cfg_wt;			//配置的权重
	float 			_cur_wt;			//当前实际权重
	int 			_req_count;			//请求数
	int				_rsp_error;			//请求错误数
	int				_rsp_time;			//
	
	QOSHOSTINFOtag()
	{
		_cfg_wt = 0.0;
		_cur_wt = 0.0;
		_req_count = 0;
		_rsp_error = 0;
		_rsp_time = 0;
	};
	
}QOSHOSTINFO;

#define QOS_INFO_SUCC 		0
#define QOS_INFO_FAIL 		-1
typedef enum ENUM_QOS_TYPE
{
	QOS_TYPE_NULL,			//未启用QOS功能
	QOS_TYPE_RSP_CODE,		//按成功失败率调权重
	QOS_TYPE_RSP_TIME		//按响应时间调整权重(connect失败按超时算)
};

struct str_hash
{
	size_t operator()(const string& str) const
	{
		return __stl_hash_string(str.c_str());
	}
};

class CQosInfoWt
{
public:
	CQosInfoWt();
	~CQosInfoWt();
	
	int Init(int qos_rebuild_sec, float qos_extend_rate,int req_min,ENUM_QOS_TYPE qos_type);
	int AddHost(HOSTINFO host_info,float wt);
	int UpdateHost(HOSTINFO host_info,int rsp_code,int rsp_time_usec);
	int AllocServer(HOSTINFO& host_info);
	int AllocServer(HOSTINFO& host_info,int host_idx);
	int List();
	char* GetErrMsg() { return (char*)_err_msg.c_str(); };
	int HostNum() { return _vec_host.size(); };
	int HostQos(char* host_ip,short host_port,float& host_wt);	
	int GetHostInfo(vector<QOSHOSTINFO>& vec_host_info);
	
	static CQosInfoWt* Instance();
private:
	static CQosInfoWt* _ins;
	
	int  RebuildQos();
	int Gcd(int a, int b);
	int Gcd_n();
	
	int 	_gcd_wt;			//权重的最大公约数
	float 	_max_wt;			//最大的权重
	int 	_cur_svr_id;		//当前机器序号
	float 	_cur_wt;			//当前权重
	
	vector<QOSHOSTINFO>				_vec_host;			//SERVER的QOS信息
	hash_map<string,int,str_hash>	_map_vec_idx;	//SERVER的QOS信息在VEC中的位置
	
	int						_ret;
	string					_err_msg;						//
	ENUM_QOS_TYPE			_qos_type;						//QOS类型
	int						_qos_rebuild_internal;			//重建QOS信息的时间间隔
	int						_qos_rebuild_max;				//重建QOS信息的最大时间
	float					_qos_extend_rate;				//QOS扩充的比例
	struct timeval 			_qos_build_tm;					//建立实际
	struct timeval 			_qos_active_tm;					//更新时间
	
	int						_req_max;						//单个HOST访问量最大的值[实际统计的数据]
	int						_req_min;						//当个HOST访问量最小值[配置的参数]
};

#endif
