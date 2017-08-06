#include "logcpp.h"
#include <iostream>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/RollingFileAppender.hh>
#if defined(WIN32)
#include <log4cpp/Win32DebugAppender.hh>
#endif

CLogWraper* CLogWraper::mInstance = new CLogWraper;

CLogWraper& CLogWraper::getInstance()
{
	return *mInstance;
}

void CLogWraper::destroyInstance()
{
	if (nullptr != mInstance)
	{
		mInstance->mCategoryRef->info("CLogWraper destroy");
		mInstance->mCategoryRef->shutdown();
		delete mInstance;
	}
}

CLogWraper::CLogWraper()
{
}

CLogWraper::~CLogWraper()
{
}

int CLogWraper::init(const std::string& name, bool append, size_t fileSize)
{
	if (name.empty())
	{
		return -1;
	}
	mInstance->mCategoryRef = &log4cpp::Category::getRoot();
	//设置日志格式
	log4cpp::PatternLayout *pattern_one = new log4cpp::PatternLayout;
    pattern_one->setConversionPattern("%d: %p %c %x:%m%n");

    //获取文件日志输出
	log4cpp::FileAppender *file_appender = nullptr;
	if (append)
	{
		file_appender = new log4cpp::FileAppender("fileAppender", name);
	}
	else
	{
		file_appender = new log4cpp::RollingFileAppender("fileAppender", name, fileSize);
	}
	file_appender->setLayout(pattern_one);

#ifdef LOG_LEVEL_DEBUG
	mInstance->mCategoryRef->setPriority(log4cpp::Priority::DEBUG);
#else
	mInstance->mCategoryRef->setPriority(log4cpp::Priority::INFO);
#endif
	
	mInstance->mCategoryRef->addAppender(file_appender);

	log4cpp::Appender *os_appender = nullptr;
#if defined(WIN32) && !defined(CONSOLE)
	os_appender = new log4cpp::Win32DebugAppender("OutputDebugString");
#else
	os_appender = new log4cpp::OstreamAppender("osAppender", &std::cout);
#endif
	//获取屏幕输出
	log4cpp::PatternLayout *pattern_two = new log4cpp::PatternLayout;
	pattern_two->setConversionPattern("%d: %p %c %x:%m%n");
	os_appender->setLayout(pattern_two);
	mInstance->mCategoryRef->addAppender(os_appender);
	return 0;
}

void CLogWraper::unInit()
{
}

void CLogWraper::setPriority(Priority priority)
{
	switch (priority)
	{
	case (ERROR) :
		mInstance->mCategoryRef->setPriority(log4cpp::Priority::ERROR);
		break;

	case (WARN) :
		mInstance->mCategoryRef->setPriority(log4cpp::Priority::WARN);
		break;

	case (INFO) :
		mInstance->mCategoryRef->setPriority(log4cpp::Priority::INFO);
		break;

	case (DEBUG) :
		mInstance->mCategoryRef->setPriority(log4cpp::Priority::DEBUG);
		break;

	default:
		mInstance->mCategoryRef->setPriority(log4cpp::Priority::DEBUG);
		break;
	}
}

log4cpp::Category& CLogWraper::getImp()
{
	return *mInstance->mCategoryRef;
}