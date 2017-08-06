#ifndef __QPLAYER_THREADDELEGATE_H__
#define __QPLAYER_THREADDELEGATE_H__

#if 0 //接口类暂时没有用到
#include <typeinfo>

class IDelegate
{
public:
	virtual ~IDelegate() { };
	virtual void invoke() = 0;
#if 0
	virtual bool isType(const std::type_info& type) = 0;
	virtual bool compare(const IDelegate* delegate) const = 0;
#endif
};
#endif

#include "Thread.h"
#include <queue>

template<class T>
class CThreadDelegate : public CThread
{
	typedef int (T::*Func)();
	typedef int (T::*FuncCallBack)(int eid,int ret,int currentFunc);
	typedef std::queue<Func>	ListFunc;
	typedef typename ListFunc::iterator FuncIter;
public:
	CThreadDelegate(T* obj, FuncCallBack fun,int deid) :CThread
	{
		mFuncs.clear();
		mObj = obj;
		mCallBackFunc = fun;
		mDelegateEventId = deid;
	}
	~CThreadDelegate()
	{
		stopTask(); 
		mFuncs.clear();
	}
	void invoke()
	{
		startTask();
	}
	void addFunc(Func fun)
	{
		if (fun)
		{
			mFuncs.push_back(fun);
		}
	}

#if 0
	bool isType(const std::type_info& type)
	{
		return typeid(CThreadDelegate) == type;
	}
	bool compare(const IDelegate* delegateObj) const
	{
		if (delegateObj && delegateObj->isType(typeid(CThreadDelegate)))
		{
			CThreadDelegate* cast = static_cast<CThreadDelegate*>(delegateObj);
			if (cast->mObj == mObj)
			{
				//TODO:迭代比较
				return true;
			}
			return false;
		}
		return false;
	}
#endif
protected:
	void taskProc()
	{
		int ret = -1;
		int currentFunc = 0;
		if (mObj != NULL)
		{
			for (auto iter = mFuncs.begin(); iter != mFuncs.end(); ++iter)
			{
				if ((*iter))
				{
					ret = (mObj->*(*iter))();
					if (ret != EOK)
					{
						(mObj->*mCallBackFunc)(mDelegateEventId, ret, currentFunc);
						break;
					}
				}
				currentFunc++;
			}
		}
		if (currentFunc == mFuncs.size())
		{
			(mObj->*mCallBackFunc)(mDelegateEventId, ret, currentFunc);
		}
		mFuncs.clear();
	}
private:
	T*					mObj;
	ListFunc			mFuncs;
	FuncIter			mIter;
	FuncCallBack		mCallBackFunc;
	int					mDelegateEventId;
};


template<class T1,class T2>
class CThreadDelegateSignleParams : public CThread
{
	typedef int (T1::*Func)();
	typedef int (T1::*FuncCallBack)(int eid, int ret, int currentFunc);
	typedef std::queue<Func>	ListFunc;
	typedef typename ListFunc::iterator FuncIter;
public:
	CThreadDelegateSignleParams(T1* obj, T2 args, FuncCallBack fun, int deid) :CThread
	{
		mFuncs.clear();
		mObj = obj;
		mCallBackFunc = fun;
		mDelegateEventId = deid;
		mParams = args;
	}
	~CThreadDelegateSignleParams()
	{
		stopTask(); 
		mFuncs.clear();
	}
	void invoke()
	{
		startTask();
	}
	void addFunc(Func fun)
	{
		if (fun)
		{
			mFuncs.push_back(fun);
		}
	}
protected:
	void taskProc()
	{
		int ret = -1;
		int currentFunc = 0;
		if (mObj != NULL)
		{
			for (FuncIter iter = mFuncs.begin(); iter != mFuncs.end(); ++iter)
			{
				if ((*iter))
				{
					ret = (mObj->*(*iter))(mParams);
					if (ret != EOK)
					{
						(mObj->*mCallBackFunc)(mDelegateEventId, ret, currentFunc);
						break;
					}
				}
				currentFunc++;
			}
		}
		if (currentFunc == mFuncs.size())
		{
			(mObj->*mCallBackFunc)(mDelegateEventId, EOK, currentFunc);
		}
		mFuncs.clear();
	}
private:
	T1*					mObj;
	T2					mParams;
	ListFunc			mFuncs;
	FuncIter			mIter;
	FuncCallBack		mCallBackFunc;
	int					mDelegateEventId;
};

#endif