#ifndef __BASE_PTLOGIN_H__
#define __BASE_PTLOGIN_H__

#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string>

#define FARMLAND_PTLOGIN_CONF "/data/release/farmland/conf/cgi/farmland_ptlogin.xml"

using namespace std;
class CPtLogin
{
	public:
		CPtLogin(){Init();};
		~CPtLogin(){};
		static int CheckLogin(unsigned int & iUin, bool bNotCkPtlogin = false);
	private:
		static int GetRemoteAddress(string& sRemoteAddress);
		static int PickUin(const char* szUin);
		static int Init(const char * config = FARMLAND_PTLOGIN_CONF);
	private:	
		static int mn_Flag;
		static int mn_Appid;
};


#endif
