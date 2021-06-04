#ifndef __SOCK_OPT_SHAWBAI_2008_06_17__
#define __SOCK_OPT_SHAWBAI_2008_06_17__

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

void SockAddr( const char* sAddr, const int iPort, sockaddr_in* pAddr );
int Listen( const char* sAddr, const int iPort, int backlog );
int Bind( int iSock, const char* sAddr, const int iPort );
int Select( int fd, struct timeval *timeout );
int ConnectNoBlock( int sock_fd, struct sockaddr_in& stSvraddr, struct timeval tout );
int SendN( int iSock, const char* data, int size );
int RecvN( int iSock, char* data, int size, struct timeval *timeout );
int recv_until( int fd, char* data, int size, const char* match, timeval* tout );

class CSockUdp
{
public:
	CSockUdp();
	virtual ~CSockUdp();

public:
	int Create();
	int Close();	
	int Bind(const char* sAddr, int port);
	int Send(const char* data, int size, sockaddr_in* pAddr, socklen_t iAddrLen);
	int Recv(char* data, int size, sockaddr_in* pAddr, socklen_t* pAddrLen, struct timeval *timeout);
	int RecvUntil( char* data, int size, struct timeval *timeout );

public:
	int m_iSock;
};

class CSockTcp
{
public:
	CSockTcp();
	virtual ~CSockTcp();

public:
	int Create();
	int Close();
	int Bind( const char* sAddr, int port );
	int Connect( const char* sAddr, int port );
	int Connect( sockaddr_in* pAddr );
	int ReConnect( sockaddr_in* pAddr );
	int SendN( const char* data, int size );
	int RecvN( char* data, int size, struct timeval *timeout );

public:
	int m_iSock;
	struct timeval m_connTout;
};

class CSockTcpEx : public CSockTcp
{
public:
	CSockTcpEx();
	CSockTcpEx( const char* sAddr, int port );
	CSockTcpEx( sockaddr_in* pAddr );
	virtual ~CSockTcpEx();

public:
	int Init( const char* sAddr, int port );
	int Init( sockaddr_in* pAddr );
	int Send( const char* data, int size );
	int Recv( char* data, int size, struct timeval *timeout );

public:
	sockaddr_in m_addr;
	bool m_bConn;
};

#endif


