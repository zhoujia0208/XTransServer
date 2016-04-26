#pragma once
#ifndef __XPUBLIC_H__
#define __XPUBLIC_H__

#include "XXmlDocument.h"
#include "XTcpClient.h"
using namespace XData;

extern XConfig::XAppConfigInfo g_xAppConfigInfo;
extern map<string, XNet::XListener *> g_mapListener;
extern map<string, XDataBase *> g_mapDataBase;
extern map<string, XTcpClientPool *> g_mapTcpClientPool;

int GetConfigValueByName(const string &strName, string &strValue);

int GetDataBase(const string &strDbName, XDataBase *&pXDataBase);
int GetNewTcpClientByName(const string &strName, XTcpClient *&pXTcpClient);
int GetTcpClientByName(const string &strName, XTcpClient *&pXTcpClient);
int GetTcpClientVectorByName(const string &strName, vector<XTcpClient * >&vTcpClient);

int CallServiceMainByName(const string &strServiceName, void *pVoid, unsigned char *pRequestBuffer, size_t szRequestLen, unsigned char *&pResponseBuffer, size_t &iResponseLen);
int CallServiceLoopByName(const string &strServiceName, void *pVoid, unsigned char *pRequestBuffer, size_t szRequestLen, unsigned char *&pResponseBuffer, size_t &iResponseLen);
int CallServiceLoadByName(const string &strServiceName);

#endif // __XPUBLIC_H__
