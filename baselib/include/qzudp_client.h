#ifndef _QZUDP_CLIENT_H_
#define _QZUDP_CLIENT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <string>
#include <sys/types.h>

#include "qzone_protocol.h"

using namespace std;

namespace QZONE
{

typedef struct HOSTADDRtag
{
	string	_host_ip;
	u_short	_host_port;
	float	_time_out;
	
	HOSTADDRtag()
	{
		_host_ip = "";
		_host_port = 0;
		_time_out = 0.0;
	}

    HOSTADDRtag(string sIP, u_short unPort, float fTimeOut)
    {
        _host_ip = sIP;
        _host_port = unPort;
        _time_out = fTimeOut;
    }
}HOSTADDR;

class CUdpClient
{
public:
	CUdpClient();
	~CUdpClient();
	
	int Init(HOSTADDR _host_addr);
	int SendBuf(char* buf,int buf_len);
	int RecvBuf(char* buf,int buf_size,int& buf_len,float time_out);
	int SendPack(QzoneProtocolPacket& send_pack);
	int RecvPack(QzoneProtocolPacket& recv_pack,float time_out);
	
	int BeginTrans(char* buf,int buf_size,int& buf_len,float time_out);
	void GetHostAddr(HOSTADDR& host_addr)
	{ host_addr = _host_addr; };
	char* GetErrMsg() { return (char*)_err_msg.c_str(); };
private:
	int				_ret;
	string			_err_msg;
	HOSTADDR		_host_addr;
	int				_sock_fd;
};


};


#endif
