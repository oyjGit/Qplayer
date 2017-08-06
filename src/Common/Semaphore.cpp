#include "Semaphore.h"
#include <iostream>
#include <thread>

//namespace Qplayer
//{

CSemaphore::CSemaphore(int count) :mCount(count), mWaitPassed(false), mWaiting(false)
{

}

CSemaphore::~CSemaphore()
{
	cleanup();
}

int CSemaphore::signal(void)
{
	std::unique_lock<std::mutex> lock(mLock);
	mCount++;
	mCond.notify_one();
	return 0;
}

int CSemaphore::wait(size_t timeOut)
{
	std::unique_lock<std::mutex> lock(mLock);
	mWaiting = true;
	if (timeOut > 0)
	{
		std::cv_status status = std::cv_status::no_timeout;
		auto startTime = std::chrono::steady_clock::now();
		if (mCount == 0)
		{
			try
			{
				status = mCond.wait_for(lock, std::chrono::milliseconds(timeOut));
			}
			catch (std::system_error& e)
			{
				std::cout << "!error! cond wait_for exception,msg:" << e.what() << std::endl;
			}
		}
		mWaiting = false;
		mWaitPassed = true;
		if (mCount > 0 || std::cv_status::no_timeout == status)
		{
			mCount--;
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		while (mCount == 0)
		{
			try
			{
				mCond.wait(lock);
			}
			catch (std::system_error& e)
			{
				std::cout << "!error! cond wait_for exception,msg:" << e.what() << std::endl;
			}
		}
		mWaiting = false;
		mWaitPassed = true;
		mCount--;
		return 0;
	}
}

int CSemaphore::cleanup(void)
{
	if (mWaiting)
	{
		mWaitPassed = false;
		mCount = 0;
		signal();
		while (!mWaitPassed)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	return 0;
}

int CSemaphore::value()
{
	std::unique_lock<std::mutex> lock(mLock);
	return mCount;
}

#if 0
//¸ü¼ÓÍ¨ÓÃµÄÄ£°åÊµÏÖ
template <typename Mutex, typename CondVar>
class basic_semaphore {
public:
	using native_handle_type = typename CondVar::native_handle_type;

	explicit basic_semaphore(size_t count = 0);
	basic_semaphore(const basic_semaphore&) = delete;
	basic_semaphore(basic_semaphore&&) = delete;
	basic_semaphore& operator=(const basic_semaphore&) = delete;
	basic_semaphore& operator=(basic_semaphore&&) = delete;

	void notify();
	void wait();
	bool try_wait();
	template<class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period>& d);
	template<class Clock, class Duration>
	bool wait_until(const std::chrono::time_point<Clock, Duration>& t);

	native_handle_type native_handle();

private:
	Mutex   mMutex;
	CondVar mCv;
	size_t  mCount;
};

using semaphore = basic_semaphore<std::mutex, std::condition_variable>;

template <typename Mutex, typename CondVar>
basic_semaphore<Mutex, CondVar>::basic_semaphore(size_t count)
	: mCount{ count }
{}

template <typename Mutex, typename CondVar>
void basic_semaphore<Mutex, CondVar>::notify() {
	std::lock_guard<Mutex> lock{ mMutex };
	++mCount;
	mCv.notify_one();
}

template <typename Mutex, typename CondVar>
void basic_semaphore<Mutex, CondVar>::wait() {
	std::unique_lock<Mutex> lock{ mMutex };
	mCv.wait(lock, [&]{ return mCount > 0; });
	--mCount;
}

template <typename Mutex, typename CondVar>
bool basic_semaphore<Mutex, CondVar>::try_wait() {
	std::lock_guard<Mutex> lock{ mMutex };

	if (mCount > 0) {
		--mCount;
		return true;
	}

	return false;
}

template <typename Mutex, typename CondVar>
template<class Rep, class Period>
bool basic_semaphore<Mutex, CondVar>::wait_for(const std::chrono::duration<Rep, Period>& d) {
	std::unique_lock<Mutex> lock{ mMutex };
	auto finished = mCv.wait_for(lock, d, [&]{ return mCount > 0; });

	if (finished)
		--mCount;

	return finished;
}

template <typename Mutex, typename CondVar>
template<class Clock, class Duration>
bool basic_semaphore<Mutex, CondVar>::wait_until(const std::chrono::time_point<Clock, Duration>& t) {
	std::unique_lock<Mutex> lock{ mMutex };
	auto finished = mCv.wait_until(lock, t, [&]{ return mCount > 0; });

	if (finished)
		--mCount;

	return finished;
}

template <typename Mutex, typename CondVar>
typename basic_semaphore<Mutex, CondVar>::native_handle_type basic_semaphore<Mutex, CondVar>::native_handle() {
	return mCv.native_handle();
}
#endif

//}