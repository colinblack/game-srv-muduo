#include "Math.h"
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

bool g_bInitRandom = false;

bool Math::InitRandom()
{
	if(g_bInitRandom)
	{
		return true;
	}
	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_sec ^ tv.tv_usec);
	srandom(tv.tv_sec ^ tv.tv_usec);
	g_bInitRandom = true;
	return true;
}

int Math::GetRandomInt(int max)
{
	return rand() % max;
}

int Math::GetRandomInt()
{
	return rand();
}

unsigned Math::GetRandomUInt()
{
	return (((unsigned)rand()) << 16) | (((unsigned)rand()) & 0xffff);
}

int Math::Abs(int n)
{
	return n >= 0 ? n : -n;
}

