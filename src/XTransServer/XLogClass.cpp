#include "StdAfx.h"
#include "XLogClass.h"
//#include <log4cpp/StringQueueAppender.hh>
//#include <log4cpp/RollingFileAppender.hh>
//

DailyFileAppender::DailyFileAppender(const std::string& name,
									 const std::string& fileName,
									 bool append,
									 mode_t mode) :
LayoutAppender(name),
	_fileName(fileName),
	_flags(O_CREAT | O_APPEND | O_WRONLY),
	_mode(mode),
	_iDate(_getDate())
{
	if (!append)
		_flags |= O_TRUNC;

	//创建目录
	int idx = _fileName.find_last_of('/');
	if(idx < 0)
	{
		idx = _fileName.find_last_of('\\');
	}
	if(idx >= 0)
	{
		string strDir = _fileName.substr(0, idx);
		cout << "LogDir: " + strDir << endl;
		CreateDir(strDir);
	}

	char cDate[10];
	sprintf(cDate, "%d", _iDate);
	string strFileName =  _fileName + "." + cDate + ".log";
	_fd = ::open(strFileName.c_str(), _flags, _mode);
}


DailyFileAppender::~DailyFileAppender()
{
	close();
}

void DailyFileAppender::close()
{
	if (_fd != -1)
	{
		::close(_fd);
		_fd = -1;
	}
}

void DailyFileAppender::setAppend(bool append)
{
	if (append)
	{
		_flags &= ~O_TRUNC;
	}
	else
	{
		_flags |= O_TRUNC;
	}
}

bool DailyFileAppender::getAppend() const
{
	return (_flags & O_TRUNC) == 0;
}

void DailyFileAppender::setMode(mode_t mode)
{
	_mode = mode;
}

mode_t DailyFileAppender::getMode() const
{
	return _mode;
}

void DailyFileAppender::_append(const LoggingEvent& event)
{
	if(this->reopen())
	{
		std::string message(_getLayout().format(event));
		if (!::write(_fd, message.data(), message.length()))
		{
			// XXX help! help!
		}
	}
}

bool DailyFileAppender::reopen()
{
	int iDate = _getDate();
	if (_iDate != iDate)
	{
		char cDate[10];
		sprintf(cDate, "%d", iDate);
		string strFileName =  _fileName + "." + cDate + ".log";
		int fd = ::open(strFileName.c_str(), _flags, _mode);
		if (fd < 0)
			return false;

		if (_fd != -1)
			::close(_fd);
		_fd = fd;
		_iDate = iDate;

		return true;
	}

	return true;
}

int DailyFileAppender::_getDate()
{
	time_t t;
	struct tm sTM;
	time(&t);//获取系统时间
#ifdef _WIN32
	localtime_s(&sTM, &t);//转换本地时间
#else
	localtime_r(&t, &sTM);
#endif

	return (sTM.tm_year + 1900) * 10000 + (sTM.tm_mon + 1) * 100 + sTM.tm_mday;
}



//log4cpp::OstreamAppender *XLogClass::osAppender = NULL;
//DailyFileAppender *XLogClass::rollfileAppender = NULL;

log4cpp::Category *XLogClass::m_pScreenCategory = NULL;
log4cpp::Category *XLogClass::m_pDailyFileCategory = NULL;

//log4cpp::StringQueueAppender *XLogClass::strQueueAppender = NULL;
//log4cpp::Category *XLogClass::m_pStrQueueCategory = NULL;
//size_t XLogClass::szMaxUnWriteRecord = 20000;
//uv_thread_t XLogClass::hLogThread;
//bool XLogClass::bLog = false;

int XLogClass::Initialize(string strPath, string strFile, log4cpp::Priority::Value Priority)
{
	//strQueueAppender = new log4cpp::StringQueueAppender("strQueueAppender");
	log4cpp::OstreamAppender *pOsAppender = new log4cpp::OstreamAppender("osAppender", &cout);
	DailyFileAppender *pDailyFileAppender = new DailyFileAppender("dailyFileAppender",strPath + "log/" + strFile);

	//log4cpp::PatternLayout* pStrQueueLayout = new log4cpp::PatternLayout();
	//pStrQueueLayout->setConversionPattern("%d: %p %x: %m%n");//%c
	//strQueueAppender->setLayout(pStrQueueLayout);

	log4cpp::PatternLayout* pOsLayout = new log4cpp::PatternLayout();
	//pOsLayout->setConversionPattern("%d: %p %x: %m%n");
	pOsLayout->setConversionPattern("%d [%t] %p %x: %m%n");
	pOsAppender->setLayout(pOsLayout);

	log4cpp::PatternLayout* pRollFileLayout = new log4cpp::PatternLayout();
	//pRollFileLayout->setConversionPattern("%d: %p %x: %m%n");
	pRollFileLayout->setConversionPattern("%d [%t] %p %x: %m%n");
	pDailyFileAppender->setLayout(pRollFileLayout);

	log4cpp::Category& root = log4cpp::Category::getRoot();

	//m_pStrQueueCategory = &root.getInstance("StringQueueCategory");
	//m_pStrQueueCategory->addAppender(strQueueAppender);
	//m_pStrQueueCategory->setPriority(Priority);

	m_pScreenCategory = &root.getInstance("ScreenCategory");
	m_pScreenCategory->addAppender(pOsAppender);
	m_pScreenCategory->setPriority(Priority);

	m_pDailyFileCategory = &root.getInstance("RollFileCategory");
	m_pDailyFileCategory->addAppender(pDailyFileAppender);
	m_pDailyFileCategory->setPriority(Priority);

	return X_SUCCESS;
}

int XLogClass::Release()
{
	log4cpp::Category::shutdown();
	return X_SUCCESS;
}

void XLogClass::SetPriority(log4cpp::Priority::Value Priority)
{
	if(m_pScreenCategory != NULL)
		m_pScreenCategory->setPriority(Priority);
	if(m_pDailyFileCategory != NULL)
		m_pDailyFileCategory->setPriority(Priority);
};

void XLogClass::debug(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->debug(strLogMessage);
	}
	m_pDailyFileCategory->debug(strLogMessage);
	return;
};

void XLogClass::debug(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::DEBUG, stringFormat, va);
		va_end(va);
	}

	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::DEBUG, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::warn(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->warn(strLogMessage);
	}
	m_pDailyFileCategory->warn(strLogMessage);
	return;
};

void XLogClass::warn(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::WARN, stringFormat, va);
		va_end(va);
	}

	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::WARN, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::error(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->error(strLogMessage);
	}
	m_pDailyFileCategory->error(strLogMessage);
	return;
};

void XLogClass::error(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::ERROR, stringFormat, va);
		va_end(va);
	}

	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::ERROR, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::alert(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->alert(strLogMessage);
	}
	m_pDailyFileCategory->alert(strLogMessage);
	return;
};

void XLogClass::alert(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::ALERT, stringFormat, va);
		va_end(va);
	}

	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::ALERT, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::crit(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->crit(strLogMessage);
	}
	m_pDailyFileCategory->crit(strLogMessage);
	return;
};

void XLogClass::crit(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::CRIT, stringFormat, va);
		va_end(va);
	}

	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::CRIT, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::emerg(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->emerg(strLogMessage);
	}
	m_pDailyFileCategory->emerg(strLogMessage);
	return;
};

void XLogClass::emerg(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::EMERG, stringFormat, va);
		va_end(va);
	}

	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::EMERG, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::fatal(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->fatal(strLogMessage);
	}
	m_pDailyFileCategory->fatal(strLogMessage);
	return;
};

void XLogClass::fatal(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va, stringFormat);
		m_pScreenCategory->logva(Priority::FATAL, stringFormat, va);
		va_end(va);
	}

	va_start(va, stringFormat);
	m_pDailyFileCategory->logva(Priority::FATAL, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::info(const std::string& strLogMessage)
{
	//debug和info日志过多则自动忽略
	//if(strQueueAppender->queueSize() > szMaxUnWriteRecord)
	//	return;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->info(strLogMessage);
	}
	m_pDailyFileCategory->info(strLogMessage);
	return;
};

void XLogClass::info(const char *stringFormat, ...)
{
	//debug和info日志过多则自动忽略
	//if(strQueueAppender->queueSize() > szMaxUnWriteRecord)
	//	return;
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va, stringFormat);
		m_pScreenCategory->logva(Priority::INFO, stringFormat, va);
		va_end(va);
	}

	va_start(va, stringFormat);
	m_pDailyFileCategory->logva(Priority::INFO, stringFormat, va);
	va_end(va);
	return;
};

void XLogClass::notice(const std::string& strLogMessage)
{
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		m_pScreenCategory->notice(strLogMessage);
	}
	m_pDailyFileCategory->notice(strLogMessage);
	return;
};

void XLogClass::notice(const char *stringFormat, ...)
{
	va_list va;
	if(g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint)
	{
		va_start(va,stringFormat);
		m_pScreenCategory->logva(Priority::NOTICE, stringFormat, va);
		va_end(va);
	}
	va_start(va,stringFormat);
	m_pDailyFileCategory->logva(Priority::NOTICE, stringFormat, va);
	va_end(va);
	return;
};

//void XLogClass::screenLog(string strLogMessage)
//{
//	m_pScreenCategory->log(log4cpp::Priority::DEBUG, strLogMessage);
//}
//
//void XLogClass::fileLog(string strLogMessage)
//{
//	m_pDailyFileCategory->log(log4cpp::Priority::DEBUG, strLogMessage);
//}
//
//void XLogClass::StartLogger(void *pVoid)
//{
//	while(bLog)
//	{
//		while(strQueueAppender->queueSize() > 0)
//		{
//			try
//			{
//				string strLog = strQueueAppender->popMessage();
//				//if(m_pScreenCategory->getPriority() >= Priority::DEBUG)
//				screenLog(strLog);
//				fileLog(strLog);
//				if(!bLog)
//					break;
//			}
//			catch(...)
//			{
//			}
//		}
//		uv_sleep(50);
//	}
//}
