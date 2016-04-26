#pragma once
#ifndef __XLOGCLASS_H__
#define __XLOGCLASS_H__

#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/NDC.hh>

using namespace log4cpp;

class XLogClass
{
public:
	static int Initialize(string strPath, string strFile, log4cpp::Priority::Value Priority);

	static int Release();

	static void SetPriority(log4cpp::Priority::Value Priority);

	static void debug(const std::string& strLogMessage);
	static void debug(const char *stringFormat, ...);

	static void warn(const std::string& strLogMessage);
	static void warn(const char *stringFormat, ...);

	static void error(const std::string& strLogMessage);
	static void error(const char *stringFormat, ...);

	static void alert(const std::string& strLogMessage);
	static void alert(const char *stringFormat, ...);

	static void crit(const std::string& strLogMessage);
	static void crit(const char *stringFormat, ...);

	static void emerg(const std::string& strLogMessage);
	static void emerg(const char *stringFormat, ...);

	static void fatal(const std::string& strLogMessage);
	static void fatal(const char *stringFormat, ...);

	static void info(const std::string& strLogMessage);
	static void info(const char *stringFormat, ...);

	static void notice(const std::string& strLogMessage);
	static void notice(const char *stringFormat, ...);

private:

	static log4cpp::Category *m_pScreenCategory;
	static log4cpp::Category *m_pDailyFileCategory;
	
};

class DailyFileAppender : public LayoutAppender
{
public:
	DailyFileAppender(const std::string& name, const std::string& fileName,
		bool append = true, mode_t mode = 00644);

	virtual ~DailyFileAppender();

	virtual bool reopen();
	virtual void close();
	virtual void setAppend(bool append);
	virtual bool getAppend() const;
	virtual void setMode(mode_t mode);
	virtual mode_t getMode() const;

protected:
	virtual void _append(const LoggingEvent& event);
	int _getDate();

	const std::string _fileName;
	int _flags;
	mode_t _mode;
	int _iDate;
	int _fd;
};

#endif //__XLOGCLASS_H__
