/*
 * logserver.h
 *
 *  Created on: 2014年1月24日
 *      Author: Administrator
 */

#ifndef LOGSERVER_H_
#define LOGSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include<string>

#define MAXSIZE          4096

using namespace std;

struct LogMsg {
	string srvId;
	string proId;
	string logFileName;
	string logStr;
};

class Logger {
public:
	static Logger *getInstance(){
		static Logger sender;
		if(!sender.initFlag) {
			sender.init();
			sender.initFlag = true;
		}
		return &sender;
	}
	~Logger(){};

private:
	Logger(){initFlag = false; sockfd = -1;};

public:
	bool sendData(const LogMsg &logMsg, string ip, short port);

private:
	bool init();
	bool createSocket();
	string enCode(const LogMsg &logMsg);

private:
	bool initFlag;
	int sockfd;

};

#endif /* LOGSERVER_H_ */
