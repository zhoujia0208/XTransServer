#include "StdAfx.h"
#include <fcntl.h>
#include "XTcpListener.h"
#include "XHttpListener.h"
#include "XHttpsListener.h"
#include "XTcpClient.h"
#include "XServer.h"

#define XVERSIONNO "3.2.2.4"

static uv_mutex_t *g_pMutex;
static void lock_callback(int mode, int type, const char *file, int line)
{
	if (mode & CRYPTO_LOCK) {
		uv_mutex_lock(&(g_pMutex[type]));
	}
	else {
		uv_mutex_unlock(&(g_pMutex[type]));
	}
}

#define OPENSSL_VERSION_1_0_0   0x10000000L
#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0
static void thread_id(CRYPTO_THREADID *pId)
{
	CRYPTO_THREADID_set_numeric(pId, (unsigned long)uv_thread_self());
}
#else  
static unsigned long thread_id()
{
	return (unsigned long)uv_thread_self();
}
#endif  


void InitOpensslLocks()
{
	int iLockNum = CRYPTO_num_locks();
	g_pMutex = new uv_mutex_t[iLockNum];

	for (int i = 0; i< iLockNum; i++)
		uv_mutex_init(&g_pMutex[i]);

#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0  
	CRYPTO_THREADID_set_callback(thread_id);
#else
	CRYPTO_set_id_callback((unsigned long(*)())thread_id);
#endif
	CRYPTO_set_locking_callback((void(*)(int, int, const char *, int))lock_callback);
}

void ReleaseOpensslLocks()
{
	int iLockNum = CRYPTO_num_locks();
#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0
	CRYPTO_THREADID_set_callback(NULL);
#else  
	CRYPTO_set_id_callback(NULL);
#endif  
	CRYPTO_set_locking_callback(NULL);

	for (int i = 0; i < iLockNum; i++)
		uv_mutex_destroy(&g_pMutex[i]);

	delete(g_pMutex);
}

int InitDataBase()
{
	XLogClass::info("Start InitDataBase");
	for (map<string, XConfig::XDataBaseInfo *>::iterator it = g_xAppConfigInfo.m_mapPXDataBaseInfo.begin(); it != g_xAppConfigInfo.m_mapPXDataBaseInfo.end(); it++)
	{
		XDataBase *pXDataBase = new XDataBase();
		int iResult = pXDataBase->Initialize(it->second->m_strType, it->second->m_strName,
			it->second->m_strConnectString, it->second->m_iConnectCount, it->second->m_iHeartBeatInterval);
		if (iResult != X_SUCCESS)
		{
			RELEASE(pXDataBase);
			return X_FAILURE;
		}
		g_mapDataBase[it->first] = pXDataBase;
	}
	XLogClass::info("InitDataBase Success");
	return X_SUCCESS;
}

int InitTcpClientPool()
{
	XLogClass::info("Start InitTcpClientPool");
	for (map<string, XConfig::XTcpClientPoolInfo *>::iterator it = g_xAppConfigInfo.m_mapPXTcpClientPoolInfo.begin(); it != g_xAppConfigInfo.m_mapPXTcpClientPoolInfo.end(); it++)
	{
		XTcpClientPool *pXTcpClientPool = new XTcpClientPool();
		int iConnectCount = 0;
		if (it->second->m_iKeep)//需要保持连接的TcpClient才去按配置建立连接
			iConnectCount = it->second->m_iConnectCount;

		int iResult = pXTcpClientPool->Initialize(it->second->m_strName, it->second->m_strRemoteIP, it->second->m_iRemotePort, iConnectCount);
		if (iResult != X_SUCCESS)
		{
			RELEASE(pXTcpClientPool);
			return X_FAILURE;
		}
		g_mapTcpClientPool[it->first] = pXTcpClientPool;
	}

	XLogClass::info("InitTcpClientPool Success");
	return X_SUCCESS;
}

void StartServiceLoop(void *pVoid)
{
	XConfig::XLoopServiceInfo *pXLoopServiceInfo = (XConfig::XLoopServiceInfo *)pVoid;
	while (pXLoopServiceInfo->iStatus)
	{
		byte *pResponseBuffer = NULL;
		size_t szResponseLen = 0;
		CallServiceLoopByName(pXLoopServiceInfo->m_strName, NULL, NULL, 0, pResponseBuffer, szResponseLen);
		RELEASE(pResponseBuffer);
		uv_sleep(pXLoopServiceInfo->m_iInterval);
	}
}

int InitService()
{
	int iResult = X_FAILURE;
	for (map<string, XConfig::XServiceInfo *>::iterator it = g_xAppConfigInfo.m_mapPXServiceInfo.begin(); it != g_xAppConfigInfo.m_mapPXServiceInfo.end(); it++)
	{
		string strServiceName = it->first;
		try
		{
			//执行服务初始化
			iResult = CallServiceLoadByName(strServiceName);
			if (iResult != X_SUCCESS)
			{
				XLogClass::debug("InitService CallServiceLoadByName [%s] Fail", strServiceName.c_str());
				return X_FAILURE;
			}

			//循环类服务，启动线程
			if (it->second->m_xType == XConfig::XSERVICETYPE_LOOP)
			{
				XConfig::XLoopServiceInfo *pXLoopServiceInfo = (XConfig::XLoopServiceInfo *)(it->second);
				uv_thread_t hThread;
				iResult = uv_thread_create(&hThread, StartServiceLoop, (void *)pXLoopServiceInfo);
				if (iResult != X_SUCCESS)
				{
					//it->second.bLoop = false;
					return X_FAILURE;
				}
				pXLoopServiceInfo->m_i64ThreadId = (int64_t)hThread;
			}
		}
		catch (exception &ex)
		{
			XLogClass::error("InitService [%s] Exception:[%s]", strServiceName.c_str(), ex.what());
			return X_FAILURE;
		}
	}

	return X_SUCCESS;
}

int InitListener()
{
	for (map<string, XConfig::XListenerInfo *>::iterator it = g_xAppConfigInfo.m_mapPXListenerInfo.begin(); it != g_xAppConfigInfo.m_mapPXListenerInfo.end(); it++)
	{
		XNet::XListener *pXListener = NULL;
		switch (it->second->m_xType)
		{
		case XConfig::XLISTENERTYPE_TCP:
			pXListener = new XNet::XTcpListener();
			break;
		case XConfig::XLISTENERTYPE_HTTP:
			pXListener = new XNet::XHttpListener();
			break;
		case XConfig::XLISTENERTYPE_HTTPS:
			pXListener = new XNet::XHttpsListener();
			break;
		default:
			return X_FAILURE;
		}

		pXListener->Initialize(it->second, uv_default_loop());
		g_mapListener[it->first] = pXListener;
	}

	return X_SUCCESS;
}

int DoInitialize()
{
	cout << "Version:" << XVERSIONNO << endl;

	int iThreadPoolSize = 0;
	char *pThreadPoolSize = NULL;
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
	signal(SIGCLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, exit);
	signal(SIGKILL, exit);
	signal(SIGTERM, exit);
	signal(SIGQUIT, exit);
#endif

	char szFileName[256 + 1] = { 0 };
#ifdef _WIN32
	DWORD dwResult = GetModuleFileName(NULL, szFileName, 256);
	if (dwResult <= 0 || dwResult > 256)
	{
		cout << "Windows GetModuleFileName Fail" << endl;
		return X_FAILURE;
	}
	g_xAppConfigInfo.m_strFileName = szFileName;
	int iTemp = g_xAppConfigInfo.m_strFileName.find_last_of("\\") + 1;
	g_xAppConfigInfo.m_strPath = g_xAppConfigInfo.m_strFileName.substr(0, iTemp);
	g_xAppConfigInfo.m_strFile = g_xAppConfigInfo.m_strFileName.substr(iTemp);
#else
	int iPathLen = readlink("/proc/self/exe", szFileName, 256);
	if (iPathLen <= 0 || iPathLen > 256)
	{
		cout << "Linux readlink Fail" << endl;
		return X_FAILURE;
	}
	g_xAppConfigInfo.m_strFileName = szFileName;
	int iTemp = g_xAppConfigInfo.m_strFileName.find_last_of("/") + 1;
	g_xAppConfigInfo.m_strPath = g_xAppConfigInfo.m_strFileName.substr(0, iTemp);
	g_xAppConfigInfo.m_strFile = g_xAppConfigInfo.m_strFileName.substr(iTemp);
#endif
	cout << "FileName:" + g_xAppConfigInfo.m_strFileName << endl;
	cout << "Path:" + g_xAppConfigInfo.m_strPath << endl;
	cout << "File:" + g_xAppConfigInfo.m_strFile << endl;

	int iResult = X_SUCCESS;
	//初始化日志
	iResult = XLogClass::Initialize(g_xAppConfigInfo.m_strPath, g_xAppConfigInfo.m_strFile, log4cpp::Priority::DEBUG);
	if (iResult != X_SUCCESS)
	{
		cout << "XLogClass::Initialize Fail" << endl;
		return X_FAILURE;
	}

	//读取配置
	iResult = XConfig::ReadConfig();
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("XConfig::ReadConfig Fail");
		return X_FAILURE;
	}

	//设置配置中的日志级别
	XLogClass::SetPriority(g_xAppConfigInfo.m_xLoggerInfo.m_iLogPriority);

	//初始化线程池,要环境变量增加 UV_THREADPOOL_SIZE
	pThreadPoolSize = getenv("UV_THREADPOOL_SIZE");
	if (pThreadPoolSize != NULL)
		iThreadPoolSize = atoi(pThreadPoolSize);
	else
		iThreadPoolSize = 0;
	cout << "Before Set ThreadPoolSize:" << iThreadPoolSize << endl;
	if (g_xAppConfigInfo.m_xThreadPoolInfo.m_szSize > 0 && g_xAppConfigInfo.m_xThreadPoolInfo.m_szSize != (size_t)iThreadPoolSize)
	{
		string strSize = "UV_THREADPOOL_SIZE=" + convert<string>(g_xAppConfigInfo.m_xThreadPoolInfo.m_szSize);
		iResult = putenv((char *)strSize.c_str());
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("ThreadPool Initialize Fail");
		}
	}

	pThreadPoolSize = getenv("UV_THREADPOOL_SIZE");
	if (pThreadPoolSize != NULL)
		iThreadPoolSize = atoi(pThreadPoolSize);
	else
		iThreadPoolSize = 0;
	cout << "After Set ThreadPoolSize:" << iThreadPoolSize << endl;

	InitOpensslLocks();
	cout << "InitOpensslLocks OK" << endl;

	//_CrtDumpMemoryLeaks();
	//初始化TcpClient连接
	iResult = InitTcpClientPool();
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("InitTcpClientPool Fail");
	}

	//初始化数据库连接
	iResult = InitDataBase();
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("InitDataBase Fail");
		return X_FAILURE;
	}

	iResult = InitService();
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("InitService Fail");
		return X_FAILURE;
	}

	iResult = InitListener();
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("InitListener Fail");
		return X_FAILURE;
	}

	//ReleaseOpensslLocks();
	return X_SUCCESS;
}
