#ifndef __LOGFORCPP_LOG_WRAPER_H__
#define __LOGFORCPP_LOG_WRAPER_H__

#define LOG4CPP_FIX_ERROR_COLLISION 1
#include<log4cpp/Category.hh>

class CLogWraper
{
public:
	enum Priority
	{
		ERROR,
		WARN,
		INFO,
		DEBUG
	};

	static CLogWraper& getInstance();
	static void destroyInstance();

	int init(const std::string& name, bool append = true, size_t fileSize = 10 * 1024 * 1024);
	void unInit();

	void setPriority(Priority priority);
	log4cpp::Category& getImp();

private:
	CLogWraper();
	~CLogWraper();

private:
	static CLogWraper*	mInstance;
	log4cpp::Category*	mCategoryRef;
};

inline std::string int2string(int line) 
{
	std::ostringstream oss;
	oss << line;
	return oss.str();
}

#ifdef _LOG4CPP_
CLogWraper& logIns = CLogWraper::getInstance();
#else
extern CLogWraper& logIns;
#endif

#define logError(...) CLogWraper::getInstance().getImp().error(__VA_ARGS__)
#define logWarn(...) CLogWraper::getInstance().getImp().warn(__VA_ARGS__)
#define logInfo(...) CLogWraper::getInstance().getImp().info(__VA_ARGS__)
#define logDebug(...) CLogWraper::getInstance().getImp().debug(__VA_ARGS__)
#define logFatal(...) CLogWraper::getInstance().getImp().fatal(__VA_ARGS__)

#endif