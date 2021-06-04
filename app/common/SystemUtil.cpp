/*
 * SystemUtil.cpp
 *
 *  Created on: 2012-1-31
 *      Author: dada
 */

#include "SystemUtil.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

namespace System
{
	static SignalCallback s_sigusr1 = NULL;
	static SignalCallback s_sigusr2 = NULL;

	static void OnSigusr1(int n)
	{
		if(System::s_sigusr1 != NULL)
		{
			System::s_sigusr1();
			signal(SIGUSR1, System::OnSigusr1);
		}
	}

	static void OnSigusr2(int n)
	{
		if(System::s_sigusr2 != NULL)
		{
			System::s_sigusr2();
			signal(SIGUSR2, System::OnSigusr2);
		}
	}
};

bool System::InitDaemon()
{
	umask(0);

	int f = open("/dev/null", O_RDWR);
	if(f != -1)
	{
		dup2(f, STDIN_FILENO);
		dup2(f, STDOUT_FILENO);
		dup2(f, STDERR_FILENO);
	}

	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	IgnoreSignal(SIGPIPE);
	signal(SIGUSR1,  SIG_IGN);
	signal(SIGUSR2,  SIG_IGN);

	return daemon(1, 1) == 0;
}

void System::InitSig(SignalCallback sigusr1, SignalCallback sigusr2, OnSig onsig)
{
	if(sigusr1 != NULL)
	{
		System::s_sigusr1 = sigusr1;
		signal(SIGUSR1, System::OnSigusr1);
	}
	else
	{
		signal(SIGUSR1,  SIG_IGN);
	}
	if(sigusr2 != NULL)
	{
		System::s_sigusr2 = sigusr2;
		signal(SIGUSR2, System::OnSigusr2);
	}
	else
	{
		signal(SIGUSR2,  SIG_IGN);
	}
	if(onsig != NULL)
	{
		for(int i=SIGRTMIN;i<=SIGRTMAX;++i)
		{
			struct sigaction act;
			sigemptyset(&act.sa_mask);
			act.sa_sigaction = onsig;
			sigaction(i, &act, NULL);
		}
	}
}

bool System::IgnoreSignal(int signum)
{
	struct sigaction sig;
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);
	return sigaction(signum, &sig, NULL) == 0;
}

string System::GetModuleDirectory()
{
	string sPath;
	char path[300];
	ssize_t length = readlink("/proc/self/exe", path, sizeof(path));
	if(length < 0)
	{
		return sPath;
	}
	path[length] = '\0';
	char *pSep = strrchr(path, '/');
	if(pSep != NULL)
	{
		*pSep = '\0';
	}
	sPath = path;
	return sPath;
}

bool System::SetCurrentDirectory(const string &path)
{
	return chdir(path.c_str()) == 0;
}
