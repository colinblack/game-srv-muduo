#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#include "lock.h"

/*
 * 可继承的单间模版类， 需要将基类作为友元
 * class Simple : public CSingleton<Simple>
 * {
 * private:
 * 	friend class CSingleton<Simple>;
 * 	virtual ~Simple(){}
 * 	Simple(){}
 * 	};
*/
template <class T>
class CSingleton
{
public:
    static T* Instance (void);
    static void Destroy (void);

protected:
    CSingleton (void){}
    CSingleton (const CSingleton&){}
    CSingleton& operator= (const CSingleton&){}

private: 
    static T*       _instance;
    static CBaseMutex   _mutex;
};


//implement
template <class T>
CBaseMutex CSingleton<T>::_mutex;

template <class T>
T* CSingleton<T>::_instance = 0;


template <class T>
T* CSingleton<T>::Instance (void)
{
    if (0 == _instance)
    {
        CScopedLock guard(_mutex);

        if (0 == _instance)
        {
            _instance = new T;
        }
    }

    return _instance;
}

template <class T>
void CSingleton<T>::Destroy (void)
{
	if(0 != _instance)
	{
		CScopedLock guard(_mutex);
		if(0 != _instance)
		{
			delete _instance;
			_instance = 0;
		}
	}

	return;
}

#endif //__SINGLETON_H__
