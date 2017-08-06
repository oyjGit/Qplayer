#ifndef __QPLAYER_SINGLETON_H__
#define __QPLAYER_SINGLETON_H__


template <class T>
class CSingleton
{
public:
	static T& instance()
	{
		static T _instance;
		return _instance;
	} 
protected: 
	CSingleton(void) {}
	virtual ~CSingleton(void) {}
	CSingleton(const CSingleton<T>&) = delete;
	CSingleton<T>& operator= (const CSingleton<T> &) = delete;
};

#endif