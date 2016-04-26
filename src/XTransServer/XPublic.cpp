#include "StdAfx.h"
#include "XClassFactory.h"
#include "XPublic.h"

XConfig::XAppConfigInfo g_xAppConfigInfo;
map<string, XNet::XListener *> g_mapListener;
map<string, XDataBase *> g_mapDataBase;
map<string, XTcpClientPool *> g_mapTcpClientPool;

int GetConfigValueByName(const string &strName, string &strValue)
{
	map<string, string>::const_iterator it;
	it = g_xAppConfigInfo.m_mapXConfigValue.find(strName);
	if(it == g_xAppConfigInfo.m_mapXConfigValue.end())
	{
		return X_FAILURE;
	}

	strValue = it->second;
	return X_SUCCESS;
}

int GetDataBase(const string &strDbName, XDataBase *&pXDataBase)
{
	map<string, XDataBase *>::const_iterator it = g_mapDataBase.find(strDbName);
	if(it == g_mapDataBase.end())
	{
		if (strDbName == "")
			return X_SUCCESS;//strDbName为空则认为该服务不需要数据库
		XLogClass::warn("Can Not Find DataBase Instance [%s]", strDbName.c_str());
		return X_NOTEXIST;
	}
	
	pXDataBase = it->second;
	return X_SUCCESS;
}

int GetNewTcpClientByName(const string &strName, XTcpClient *&pXTcpClient)
{
	map<string, XTcpClientPool *>::const_iterator it = g_mapTcpClientPool.find(strName);
	if (it == g_mapTcpClientPool.end())
	{
		XLogClass::warn("GetNewTcpClientByName Can Not Find XTcpClientPool [%s]", strName.c_str());
		return X_NOTEXIST;
	}

	return it->second->GetNewTcpClient(pXTcpClient);
}

int GetTcpClientByName(const string &strName, XTcpClient *&pXTcpClient)
{
	map<string, XTcpClientPool *>::const_iterator it = g_mapTcpClientPool.find(strName);
	if(it == g_mapTcpClientPool.end())
	{
		XLogClass::warn("GetTcpClientByName Can Not Find XTcpClientPool [%s]", strName.c_str());
		return X_NOTEXIST;
	}

	return it->second->GetTcpClient(pXTcpClient);
}

int GetTcpClientVectorByName(const string &strName, vector<XTcpClient * >&vTcpClient)
{
	map<string, XTcpClientPool *>::const_iterator it = g_mapTcpClientPool.find(strName);
	if(it == g_mapTcpClientPool.end())
	{
		XLogClass::warn("GetTcpClientVectorByName Can Not Find XTcpClientPool [%s]", strName.c_str());
		return X_NOTEXIST;
	}

	return it->second->GetTcpClientVector(vTcpClient);
}

int CreateServiceByName(const string &strServiceName, XService *&xService)
{
	map<string, XConfig::XServiceInfo *>::const_iterator it = g_xAppConfigInfo.m_mapPXServiceInfo.find(strServiceName);
	if (it == g_xAppConfigInfo.m_mapPXServiceInfo.end())
	{
		XLogClass::error("CreateServiceByName Service Class[%s] Not Exist", strServiceName.c_str());
		return X_NOTEXIST;
	}

	int iResult = XClassFactory::CreateServiceByName(strServiceName, xService);
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("CreateServiceByName Service Class[%s] Not Exist", strServiceName.c_str());
		return iResult;
	}

	GetDataBase(it->second->m_strDbName, xService->pXDataBase);

	return X_SUCCESS;
}

int CallServiceLoadByName(const string &strServiceName)
{
	XService *xService = NULL;
	int iResult = CreateServiceByName(strServiceName, xService);
	if(iResult != X_SUCCESS)
		return iResult;

	log4cpp::NDC::push(strServiceName);
	try
	{
		iResult = xService->DoInitializeEx();
	}
	catch(exception &ex)
	{
		XLogClass::error("CallServiceLoadByName Exception:[%s]", ex.what());
	}
	delete xService;
	log4cpp::NDC::pop();

	return iResult;
}

int CallServiceMainByName(const string &strServiceName, void *pVoid, unsigned char *pRequestBuffer, size_t szRequestLen, unsigned char *&pResponseBuffer, size_t &szResponseLen)
{
	XService *xService = NULL;
	int iResult = CreateServiceByName(strServiceName, xService);
	if (iResult != X_SUCCESS)
		return iResult;

	log4cpp::NDC::push(strServiceName);
	try
	{
		iResult = xService->DoMainEx(pVoid, pRequestBuffer, szRequestLen, pResponseBuffer, szResponseLen);
	}
	catch(exception &ex)
	{
		XLogClass::error("CallServiceMainByName Exception:[%s]", ex.what());
	}
	delete xService;
	log4cpp::NDC::pop();

	return iResult;
}

int CallServiceLoopByName(const string &strServiceName, void *pVoid, unsigned char *pRequestBuffer, size_t szRequestLen, unsigned char *&pResponseBuffer, size_t &szResponseLen)
{
	XService *xService = NULL;
	int iResult = CreateServiceByName(strServiceName, xService);
	if (iResult != X_SUCCESS)
		return iResult;

	log4cpp::NDC::push(strServiceName);
	try
	{
		iResult = xService->DoLoopEx(pVoid, pRequestBuffer, szRequestLen, pResponseBuffer, szResponseLen);
	}
	catch(exception &ex)
	{
		XLogClass::error("CallServiceLoopByName Exception:[%s]", ex.what());
	}
	delete xService;
	log4cpp::NDC::pop();

	return iResult;
}
