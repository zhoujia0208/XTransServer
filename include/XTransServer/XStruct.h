#pragma once
#ifndef __STRUCT_H__
#define __STRUCT_H__

#include <sstream>
#include <string>
#include <list>
#include <map>
#include <uv.h>

using namespace std;

typedef unsigned char BYTE;
typedef unsigned char byte;

typedef struct LENGTH_INFO
{
	string strType;//长度类型；bcd/byte/string/none(表示没有长度标识)

	int iOffset;//长度的起始位置在整个报文中的偏移

	int iLength;//长度所占的长度

	int iLengthOffset;//长度所表示的内容的起始偏移位置
} _LENGTH_INFO;

typedef struct TRANCODE_INFO
{
	string strType;//交易代码类型；bcd/byte/string/none(表示没有交易代码标识)

	int iOffset;//交易代码的起始位置在整个报文中的偏移

	int iLength;//交易代码所占的长度
} _TRANCODE_INFO;

typedef struct TRANSACTION_INFO
{
	string strTranCode;//交易代码

	bool bNeedResponse;//是否需要应答

	list<string> liSerivceCallOrder;
} _TRANSACTION_INFO;


typedef struct LISTENER_INFO
{
	string strName;//监听名称
	int iPort;//监听端口
	int iMaxConnectCount;
	int iTimeOut;//接收超时时间(毫秒)
	string strType;//监听类型

	string strLocalPath;//本地基准目录(仅对Http类型的监听有效)
	string strDefaultPage;//默认文件(仅对Http类型的监听有效)

	LENGTH_INFO lengthInfo;//报文长度信息

	TRANCODE_INFO trancodeInfo;//交易代码信息

	map<string, TRANSACTION_INFO> mapTranList;//包含的交易列表
	map<int64_t, void*> mapXTcpConn;
	int64_t g_i64SessionIdInner;
	//OnConnectionCall pFuncOnConnectionCall;
	void *pFuncOnConnectionCall;

	int iConnectedCount;
} _LISTENER_INFO;

typedef struct DATABASE_INFO
{
	string strName;
	string strType;
	string strConnectString;
	int iConnectCount;
	int iHeartBeatInterval;
}_DATABASE_INFO;

typedef struct SERVICE_INFO
{
	string strName;
	string strType;
	string strDbName;
	bool bLoop;
	int iInterval;
	uv_thread_t hThread;
	int iStatus;//0-停用，1-启用
}_SERVICE_INFO;

typedef struct TCPCLIENT_INFO
{
	string strName;
	string strRemoteIP;
	int iRemotePort;
	string strType;
	int iConnectCount;
}_TCPCLIENT_INFO;

typedef struct _THREADPOOL_INFO
{
	size_t szSize;//线程池大小
}THREADPOOL_INFO;

typedef struct _LOG_CONFIG
{
	int logPriority; //日志级别
	bool bScreenPrint;
}LOG_CONFIG;

class ApplicationConfig
{
public:
	string strFileName;
	string strPath;
	string strFile;

	LOG_CONFIG logConfig;

	map<string, LISTENER_INFO> mapListenerInfo;

	map<string, DATABASE_INFO> mapDataBaseInfo;

	map<string, SERVICE_INFO> mapServiceInfo;

	map<string, TCPCLIENT_INFO> mapTcpClientInfo;

	THREADPOOL_INFO threadPoolInfo;

	map<string, string> mapConfigValue;
};

class XUserData
{
public:
	XUserData(){};
	virtual ~XUserData(){};
};

#endif //__STRUCT_H__
