#ifndef __DBC_LOCK_H__
#define __DBC_LOCK_H__

#include <pthread.h>

/*
 * 线程锁 其中trylock是非阻塞的，获取不到锁返回非0值
*/
class CBaseMutex
{
    friend class condition;

public:
    inline CBaseMutex (void)
    {
        ::pthread_mutex_init (&_mutex, 0);
    }

    inline void lock (void) 
    {
        ::pthread_mutex_lock (&_mutex);
    }

    inline void unlock (void) 
    {
        ::pthread_mutex_unlock (&_mutex);
    }

    inline int trylock (void)
    {
        return ::pthread_mutex_trylock (&_mutex);
    }

    inline ~CBaseMutex (void)
    {
        ::pthread_mutex_destroy (&_mutex);
    }

    inline pthread_mutex_t* GetMutex()
    {
    	return &_mutex;
    }

private:
    CBaseMutex (const CBaseMutex& m);
    CBaseMutex& operator= (const CBaseMutex &m);

private:
    pthread_mutex_t _mutex;
};

/*
 * 自动线程锁，在生命周期内生效
*/
class CScopedLock 
{
    friend class condition;

public:
    inline CScopedLock (CBaseMutex& mutex) : _mutex (mutex)
    {
        _mutex.lock ();
    }

    inline ~CScopedLock (void) 
    {
        _mutex.unlock ();
    }

private:
    CBaseMutex& _mutex;
};

/*
 * 非阻塞自动线程锁，在生命周期内生效，需要调用GetTry判断是否生效
*/
class CScopedTryLock
{
    friend class condition;

public:
    inline CScopedTryLock (CBaseMutex& mutex) : _mutex (mutex), m_try(false)
    {
    	m_try = (_mutex.trylock () == 0);
    }

    inline ~CScopedTryLock (void)
    {
    	if(m_try)
    		_mutex.unlock ();
    }

    inline bool GetTry()
    {
    	return m_try;
    }

private:
    CBaseMutex& _mutex;
    bool m_try;
};

#endif //__DBC_LOCK_H__
