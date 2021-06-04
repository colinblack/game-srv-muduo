#ifndef _TSC_H_
#define _TSC_H_
#if 0
#include <asm/msr.h>
#endif

//以秒为单位取当前时间
#define sectime(t) {\
	long long time_now; \
	rdtscll(time_now); \
	t = (long)(time_now/tscsec);}

//以毫秒为单位取当前时间
#define mtime(t) {\
	long long time_now; \
	rdtscll(time_now); \
	t = time_now/tscmsec;}
	
//以秒为单位获取日历时间
#define unixtime(t) {\
	long long time_now; \
	rdtscll(time_now); \
	t = (long)(time_now/tscsec) + diffsec;}
	
extern int tscmsec;	
extern int diffsec;
extern long long tscsec; 
extern long long current_tsc;

/* read tsc interface */
#ifdef __x86_64__
# define rdtscll(val) do { \
	     unsigned int __a,__d; \
	     asm volatile("rdtsc" : "=a" (__a), "=d" (__d)); \
	     (val) = ((unsigned long)__a) | (((unsigned long)__d)<<32); \
} while(0)

#else
# define rdtscll(val)  do {\
	     __asm__ __volatile__("rdtsc" : "=A" (val));\
}while(0)
#endif

extern void tsc_init ();
#endif
