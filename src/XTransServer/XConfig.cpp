#include "StdAfx.h"
#include "XConfig.h"

#define CONFIG_FILE "config.xml"
namespace XConfig {

	XLengthInfo::XLengthInfo()
	{
		m_xType = XLENGTHTYPE_NONE;
		m_iOffset = 0;
		m_iLength = 0;
		m_iLengthOffset = 0;
	}

	XTranCodeInfo::XTranCodeInfo()
	{
		m_xType = XTRANCODETYPE_BCD;//默认BCD
		m_iOffset = 0;
		m_iLength = 0;
	}

	XTransactionInfo::XTransactionInfo()
	{
		m_iNeedResponse = 1;
	}

	XListenerInfo::XListenerInfo()
	{
		m_xType = XLISTENERTYPE_TCP;
		m_iPort = 0;
		m_iMaxConnectCount = 0;
		m_iTimeOut = 0;
	}

	XDataBaseInfo::XDataBaseInfo()
	{
		m_iConnectCount = 0;
		m_iHeartBeatInterval = 0;
	}

	XServiceInfo::XServiceInfo()
	{
		m_xType = XSERVICETYPE_NORMAL;
		iStatus = 1;
	}

	XLoopServiceInfo::XLoopServiceInfo()
	{
		m_iInterval = 0;
		m_i64ThreadId = 0;
	}

	XTcpClientPoolInfo::XTcpClientPoolInfo()
	{
		m_xType = XHOSTTYPE_IP;
		m_iRemotePort = 0;
		m_iKeep = 0;
		m_iConnectCount = 0;
	}

	XThreadPoolInfo::XThreadPoolInfo()
	{
		m_szSize = 0;
	}

	XLoggerInfo::XLoggerInfo()
	{
		m_iLogPriority = log4cpp::Priority::DEBUG;
		m_iScreenPrint = 1;
	}

	const map<string, int>::value_type g_mapLoggerPriorityValues[] =
	{
		map<string, int>::value_type("debug", log4cpp::Priority::DEBUG),
		map<string, int>::value_type("info", log4cpp::Priority::INFO),
		map<string, int>::value_type("notice", log4cpp::Priority::NOTICE),
		map<string, int>::value_type("warn", log4cpp::Priority::WARN),
		map<string, int>::value_type("error", log4cpp::Priority::ERROR),
		map<string, int>::value_type("crit", log4cpp::Priority::CRIT),
		map<string, int>::value_type("alert", log4cpp::Priority::ALERT),
		map<string, int>::value_type("fatal", log4cpp::Priority::FATAL),
		map<string, int>::value_type("emerg", log4cpp::Priority::EMERG),
	};
	size_t szLoggerPriority = sizeof(g_mapLoggerPriorityValues) / sizeof(map<string, int>::value_type);
	const map<string, int> g_mapLoggerPriority(g_mapLoggerPriorityValues, g_mapLoggerPriorityValues + szLoggerPriority);

	const map<string, XLengthType>::value_type g_mapLengthTypeValues[] =
	{
		map<string, XLengthType>::value_type("none", XLENGTHTYPE_NONE),
		map<string, XLengthType>::value_type("bcd", XLENGTHTYPE_BCD),
		map<string, XLengthType>::value_type("byte", XLENGTHTYPE_BYTE),
		map<string, XLengthType>::value_type("string", XLENGTHTYPE_STRING),
	};
	size_t szLengthType = sizeof(g_mapLengthTypeValues) / sizeof(map<string, XLengthType>::value_type);
	const map<string, XLengthType> g_mapLengthType(g_mapLengthTypeValues, g_mapLengthTypeValues + szLengthType);

	const map<string, XTranCodeType>::value_type g_mapTranCodeTypeValues[] =
	{
		map<string, XTranCodeType>::value_type("bcd", XTRANCODETYPE_BCD),
		map<string, XTranCodeType>::value_type("byte", XTRANCODETYPE_BYTE),
		map<string, XTranCodeType>::value_type("string", XTRANCODETYPE_STRING),
		map<string, XTranCodeType>::value_type("uri", XTRANCODETYPE_URI),
	};
	size_t szTranCodeType = sizeof(g_mapTranCodeTypeValues) / sizeof(map<string, XTranCodeType>::value_type);
	const map<string, XTranCodeType> g_mapTranCodeType(g_mapTranCodeTypeValues, g_mapTranCodeTypeValues + szTranCodeType);

	const map<string, XHostType>::value_type g_mapHostTypeValues[] =
	{
		map<string, XHostType>::value_type("ip", XHOSTTYPE_IP),
		map<string, XHostType>::value_type("hostname", XHOSTTYPE_HOSTNAME),
	};
	size_t szHostType = sizeof(g_mapHostTypeValues) / sizeof(map<string, XHostType>::value_type);
	const map<string, XHostType> g_mapHostType(g_mapHostTypeValues, g_mapHostTypeValues + szHostType);

	void ReadListenerConfig(xmlNodePtr pNode, XListenerInfo *pXListenerInfo)
	{
		pXListenerInfo->m_strName = (char *)xmlGetProp(pNode, BAD_CAST"Name");
		pXListenerInfo->m_iPort = atoi((char *)xmlGetProp(pNode, BAD_CAST"Port"));
		pXListenerInfo->m_iMaxConnectCount = atoi((char *)xmlGetProp(pNode, BAD_CAST"MaxConnectCount"));
		pXListenerInfo->m_iTimeOut = atoi((char *)xmlGetProp(pNode, BAD_CAST"TimeOut"));

		//string strPathListener = (char *)xmlGetNodePath(pNode);
		XXmlNodeList xTranCode = XXmlDocument::searchNodesByPath((xmlDocPtr)pNode, "/TranCode");
		pXListenerInfo->m_xTranCodeInfo.m_xType = g_mapTranCodeType.find((char *)xmlGetProp(xTranCode[0], BAD_CAST"Type"))->second;
		if (pXListenerInfo->m_xTranCodeInfo.m_xType != XTRANCODETYPE_URI)
		{
			pXListenerInfo->m_xTranCodeInfo.m_iOffset = atoi((char *)xmlGetProp(xTranCode[0], BAD_CAST"Offset"));
			pXListenerInfo->m_xTranCodeInfo.m_iLength = atoi((char *)xmlGetProp(xTranCode[0], BAD_CAST"Length"));
		}
		XXmlNodeList xTranList = XXmlDocument::searchNodesByPath((xmlDocPtr)pNode, "/TransactionList/Transaction");
		for (int j = 0; j < (int)xTranList.size(); j++)
		{
			XTransactionInfo xTransactionInfo;
			xTransactionInfo.m_strTranCode = (char *)xmlGetProp(xTranList[j], BAD_CAST"TranCode");
			xTransactionInfo.m_iNeedResponse = atoi((char *)xmlGetProp(xTranList[j], BAD_CAST"NeedResponse"));
			string strCallSerivce = (char *)xmlGetProp(xTranList[j], BAD_CAST"CallSerivce");
			xTransactionInfo.m_liCallSerivce = split(strCallSerivce, ",", true);
			pXListenerInfo->m_mapXTranList[xTransactionInfo.m_strTranCode] = xTransactionInfo;
		}
	}

	int ReadConfig()
	{
		XXmlDocument xConfigXml;
		int iResult = xConfigXml.OpenFile(g_xAppConfigInfo.m_strFileName + "." + CONFIG_FILE);
		if (iResult != X_SUCCESS)
			return X_FAILURE;
		iResult = xConfigXml.XsdValidate(g_xAppConfigInfo.m_strPath + "config.xsd");
		if (iResult != X_SUCCESS)
			return X_FAILURE;
		try
		{
			//读取日志相关配置
			XXmlNodeList xLogger = xConfigXml.searchNodesByPath("/Config/Logger");
			string strPriority = (char *)xmlGetProp(xLogger[0], BAD_CAST"Priority");
			g_xAppConfigInfo.m_xLoggerInfo.m_iLogPriority = g_mapLoggerPriority.find(strPriority)->second;
			g_xAppConfigInfo.m_xLoggerInfo.m_iScreenPrint = atoi((char *)xmlGetProp(xLogger[0], BAD_CAST"ScreenPrint"));

			//读取线程池大小配置
			XXmlNodeList xThreadPool = xConfigXml.searchNodesByPath("/Config/ThreadPool");
			g_xAppConfigInfo.m_xThreadPoolInfo.m_szSize = atoi((char *)xmlGetProp(xThreadPool[0], BAD_CAST"Size"));

			//读取链接客户端配置
			XXmlNodeList xTcpClient = xConfigXml.searchNodesByPath("/Config/TcpClientPools/TcpClientPool");
			for (int i = 0; i < (int)xTcpClient.size(); i++)
			{
				char *p = NULL;
				XTcpClientPoolInfo *pXTcpClientPoolInfo = new XTcpClientPoolInfo();
				pXTcpClientPoolInfo->m_strName = (char *)xmlGetProp(xTcpClient[i], BAD_CAST"Name");
				p = (char *)xmlGetProp(xTcpClient[i], BAD_CAST"Type");
				if (p != NULL)
					pXTcpClientPoolInfo->m_xType = g_mapHostType.find(p)->second;
				pXTcpClientPoolInfo->m_strRemoteHost = (char *)xmlGetProp(xTcpClient[i], BAD_CAST"RemoteHost");
				pXTcpClientPoolInfo->m_iRemotePort = atoi((char *)xmlGetProp(xTcpClient[i], BAD_CAST"RemotePort"));
				pXTcpClientPoolInfo->m_iKeep = atoi((char *)xmlGetProp(xTcpClient[i], BAD_CAST"Keep"));
				p = (char *)xmlGetProp(xTcpClient[i], BAD_CAST"ConnectCount");
				if(p != NULL)
					pXTcpClientPoolInfo->m_iConnectCount = atoi(p);
				g_xAppConfigInfo.m_mapPXTcpClientPoolInfo[pXTcpClientPoolInfo->m_strName] = pXTcpClientPoolInfo;
			}

			//读取数据库配置
			XXmlNodeList xDataBase = xConfigXml.searchNodesByPath("/Config/DataBase");
			for (int i = 0; i < (int)xDataBase.size(); i++)
			{
				XDataBaseInfo *pXDataBaseInfo = new XDataBaseInfo();
				pXDataBaseInfo->m_strName = (char *)xmlGetProp(xDataBase[i], BAD_CAST"Name");
				pXDataBaseInfo->m_strType = (char *)xmlGetProp(xDataBase[i], BAD_CAST"Type");
				pXDataBaseInfo->m_strConnectString = (char *)xmlGetProp(xDataBase[i], BAD_CAST"ConnectString");
				pXDataBaseInfo->m_iConnectCount = atoi((char *)xmlGetProp(xDataBase[i], BAD_CAST"ConnectCount"));
				pXDataBaseInfo->m_iHeartBeatInterval = atoi((char *)xmlGetProp(xDataBase[i], BAD_CAST"HeartBeatInterval"));
				g_xAppConfigInfo.m_mapPXDataBaseInfo[pXDataBaseInfo->m_strName] = pXDataBaseInfo;
			}

			//读取Tcp监听配置
			XXmlNodeList xTcpListener = xConfigXml.searchNodesByPath("/Config/TcpListener");
			for (int i = 0; i < (int)xTcpListener.size(); i++)
			{
				XTcpListenerInfo *pXTcpListenerInfo = new XTcpListenerInfo();
				pXTcpListenerInfo->m_xType = XLISTENERTYPE_TCP;
				ReadListenerConfig(xTcpListener[i], pXTcpListenerInfo);

				string strPathListener = (char *)xmlGetNodePath(xTcpListener[i]);
				XXmlNodeList xPackageLength = xConfigXml.searchNodesByPath(strPathListener + "/PackageLength");
				pXTcpListenerInfo->m_xLengthInfo.m_xType = g_mapLengthType.find((char *)xmlGetProp(xPackageLength[0], BAD_CAST"Type"))->second;
				pXTcpListenerInfo->m_xLengthInfo.m_iOffset = atoi((char *)xmlGetProp(xPackageLength[0], BAD_CAST"Offset"));
				pXTcpListenerInfo->m_xLengthInfo.m_iLength = atoi((char *)xmlGetProp(xPackageLength[0], BAD_CAST"Length"));
				pXTcpListenerInfo->m_xLengthInfo.m_iLengthOffset = atoi((char *)xmlGetProp(xPackageLength[0], BAD_CAST"LengthOffset"));
				g_xAppConfigInfo.m_mapPXListenerInfo[pXTcpListenerInfo->m_strName] = pXTcpListenerInfo;
			}

			//读取Http监听配置
			XXmlNodeList xHttpListener = xConfigXml.searchNodesByPath("/Config/HttpListener");
			for (int i = 0; i < (int)xHttpListener.size(); i++)
			{
				XHttpListenerInfo *pXHttpListenerInfo = new XHttpListenerInfo();
				pXHttpListenerInfo->m_xType = XLISTENERTYPE_HTTP;
				ReadListenerConfig(xHttpListener[i], pXHttpListenerInfo);
				pXHttpListenerInfo->m_strLocalPath = (char *)xmlGetProp(xHttpListener[i], BAD_CAST"LocalPath");
				pXHttpListenerInfo->m_strDefaultPage = (char *)xmlGetProp(xHttpListener[i], BAD_CAST"DefaultPage");
				g_xAppConfigInfo.m_mapPXListenerInfo[pXHttpListenerInfo->m_strName] = pXHttpListenerInfo;
			}

			//读取Https监听配置
			XXmlNodeList xHttpsListener = xConfigXml.searchNodesByPath("/Config/HttpsListener");
			for (int i = 0; i < (int)xHttpsListener.size(); i++)
			{
				XHttpsListenerInfo *pXHttpsListenerInfo = new XHttpsListenerInfo();
				pXHttpsListenerInfo->m_xType = XLISTENERTYPE_HTTPS;
				ReadListenerConfig(xHttpsListener[i], pXHttpsListenerInfo);
				pXHttpsListenerInfo->m_strLocalPath = (char *)xmlGetProp(xHttpsListener[i], BAD_CAST"LocalPath");
				pXHttpsListenerInfo->m_strDefaultPage = (char *)xmlGetProp(xHttpsListener[i], BAD_CAST"DefaultPage");
				pXHttpsListenerInfo->m_strServerCert = (char *)xmlGetProp(xHttpsListener[i], BAD_CAST"ServerCert");
				pXHttpsListenerInfo->m_strServerPrivateKey = (char *)xmlGetProp(xHttpsListener[i], BAD_CAST"ServerPrivateKey");
				char *pTemp = (char *)xmlGetProp(xHttpsListener[i], BAD_CAST"VerifyPeer");
				if (pTemp != NULL)
					pXHttpsListenerInfo->m_iVerifyPeer = atoi(pTemp);
				pTemp = (char *)xmlGetProp(xHttpsListener[i], BAD_CAST"TrustCert");
				if (pTemp != NULL)
					pXHttpsListenerInfo->m_strTrustCert = pTemp;
				g_xAppConfigInfo.m_mapPXListenerInfo[pXHttpsListenerInfo->m_strName] = pXHttpsListenerInfo;
			}

			//读取服务配置
			XXmlNodeList xService = xConfigXml.searchNodesByPath("/Config/NormalService");
			for (int i = 0; i < (int)xService.size(); i++)
			{
				XNormalServiceInfo *pXNormalServiceInfo = new XNormalServiceInfo();
				pXNormalServiceInfo->m_xType = XSERVICETYPE_NORMAL;
				pXNormalServiceInfo->m_strName = (char *)xmlGetProp(xService[i], BAD_CAST"Name");
				char *p = (char *)xmlGetProp(xService[i], BAD_CAST"DataBase");
				if (p != NULL)
					pXNormalServiceInfo->m_strDbName = p;
				g_xAppConfigInfo.m_mapPXServiceInfo[pXNormalServiceInfo->m_strName] = pXNormalServiceInfo;
			}

			xService.Dispose();
			xService = xConfigXml.searchNodesByPath("/Config/LoopService");
			for (int i = 0; i < (int)xService.size(); i++)
			{
				XLoopServiceInfo *pXLoopServiceInfo = new XLoopServiceInfo();
				pXLoopServiceInfo->m_xType = XSERVICETYPE_LOOP;
				pXLoopServiceInfo->m_strName = (char *)xmlGetProp(xService[i], BAD_CAST"Name");
				char *p = (char *)xmlGetProp(xService[i], BAD_CAST"DataBase");
				if (p != NULL)
					pXLoopServiceInfo->m_strDbName = p;
				pXLoopServiceInfo->m_iInterval = atoi((char *)xmlGetProp(xService[i], BAD_CAST"Interval"));
				g_xAppConfigInfo.m_mapPXServiceInfo[pXLoopServiceInfo->m_strName] = pXLoopServiceInfo;
			}

			//读取全局变量配置
			XXmlNodeList xConfigValue = xConfigXml.searchNodesByPath("/Config/ConfigValue");
			for (int i = 0; i < (int)xConfigValue.size(); i++)
			{
				string strKey = (char *)xmlGetProp(xConfigValue[i], BAD_CAST"Key");
				string strValue = (char *)xmlGetProp(xConfigValue[i], BAD_CAST"Value");
				g_xAppConfigInfo.m_mapXConfigValue[strKey] = strValue;
			}

			return X_SUCCESS;
		}
		catch (exception &ex)
		{
			XLogClass::error("ReadConfig Exception: Name[%s] Message[%s]", typeid(ex).name(), ex.what());
			return X_FAILURE;
		}

		return X_SUCCESS;
	}

}
