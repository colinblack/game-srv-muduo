#ifndef _NET_CLIENT_EX_H_
#define _NET_CLIENT_EX_H_
#include <sys/time.h>
#include "qzone_protocol.h"
#include "qosinfo_wt.h"

class CNetClientEx
{
public:
	CNetClientEx();
	~CNetClientEx();
	
	//采用负载均衡的方式路由
	int StartQos(int qos_rebuild_sec = 300, float qos_extend_rate = 0.05,int req_min = 3,ENUM_QOS_TYPE qos_type = QOS_TYPE_RSP_CODE);	
	
	//采用指定路由的方式路由
	int GetHost(int host_idx);
	int HostNum();
	int HostQos(char* host_ip,short host_port,float& host_wt);
	//
	
	int Open(const char* ip,unsigned short port,
		float recvtimeout_usec=2,float sendtimeout_usec=2);
	int Open(const char* ip,unsigned short port,
		float recvtimeout_usec,float sendtimeout_usec,bool keep_cnt,bool reuse);
	void Close();
	//
	
	int Recv(QzoneProtocolPacket& pack);
	int Send(QzoneProtocolPacket& pack);
	int Recv(char* pbuf,int buf_size,int& buf_len);
	int Send(char* pbuf,int buf_len);
	int RecvFromWire(char* pbuf,int buf_size,int& buf_len);
	int SetSendTime(float sendtimeout_usec);
	int SetRecvTime(float recvtimeout_usec);
	int SendAndRecv(QzoneProtocolPacket& send_pack,QzoneProtocolPacket& recv_pack,float time_out,bool& net_err_flag);
	
	int GetHostInfo(vector<QOSHOSTINFO>& vec_host_info);
	char* GetErrMsg() { return (char*)_err_msg.c_str(); };
protected:
	int AsyncConnect(unsigned int ip, unsigned short port);
	int AsyncConnect(unsigned int ip, unsigned short port,float time_out);
	int SelectSingleRead(int fd, float timeout);
protected:
	int  _ret;
	int SetNonBlock(int fd);
	int SelectSingleReadEx(int fd, float timeout, fd_set* r_set);
	int AllocHost();
	int UpdateHost(int ret,bool net_err_flag);
	
	/////////////////////////////////////////////////////////////
	HOSTINFO				_host_info;
	CQosInfoWt				_qos_info;
	ENUM_QOS_TYPE			_qos_type;
	string					_err_msg;
};

#endif
