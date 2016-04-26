#pragma once
#ifndef __XCONFIG_H__
#define __XCONFIG_H__

#include <sstream>
#include <string>
#include <list>
#include <map>

namespace XConfig {

	enum XLengthType
	{
		XLENGTHTYPE_NONE = 0,
		XLENGTHTYPE_BCD,
		XLENGTHTYPE_BYTE,
		XLENGTHTYPE_STRING,
	};

	enum XTranCodeType
	{
		XTRANCODETYPE_BCD = 1,
		XTRANCODETYPE_BYTE,
		XTRANCODETYPE_STRING,
		XTRANCODETYPE_URI,
	};

	enum XListenerType
	{
		XLISTENERTYPE_TCP = 0,
		XLISTENERTYPE_HTTP,
		XLISTENERTYPE_HTTPS,
	};

	enum XServiceType
	{
		XSERVICETYPE_NORMAL = 0,
		XSERVICETYPE_LOOP,
	};

	class XLengthInfo
	{
	public:
		XLengthInfo();
		XLengthType m_xType;//长度类型
		int m_iOffset;//长度的起始位置在整个报文中的偏移
		int m_iLength;//长度所占的长度
		int m_iLengthOffset;//长度所表示的内容的起始偏移位置
	};

	class XTranCodeInfo
	{
	public:
		XTranCodeInfo();
		XTranCodeType m_xType;//交易代码类型
		int m_iOffset;//交易代码的起始位置在整个报文中的偏移
		int m_iLength;//交易代码所占的长度
	};

	class XTransactionInfo
	{
	public:
		XTransactionInfo();
		string m_strTranCode;//交易代码
		int m_iNeedResponse;//是否需要应答
		list<string> m_liCallSerivce;
	};

	class XListenerInfo
	{
	public:
		XListenerInfo();
		XListenerType m_xType;
		string m_strName;//监听名称
		int m_iPort;//监听端口
		int m_iMaxConnectCount;
		int m_iTimeOut;//接收超时时间(毫秒)
		XTranCodeInfo m_xTranCodeInfo;//交易代码信息
		map<string, XTransactionInfo> m_mapXTranList;//包含的交易列表
	};

	class XTcpListenerInfo :public XListenerInfo
	{
	public:
		XLengthInfo m_xLengthInfo;//报文长度信息
	};

	class XHttpListenerInfo :public XListenerInfo
	{
	public:
		string m_strLocalPath;//本地基准目录
		string m_strDefaultPage;//默认访问文件
	};

	class XHttpsListenerInfo :public XHttpListenerInfo
	{
	public:
		string m_strServerCert;//证书文件
		string m_strServerPrivateKey;//私钥文件
		int m_iVerifyPeer;//是否必须验证客户端
		string m_strTrustCert;//信任的证书文件
	};

	class XDataBaseInfo
	{
	public:
		XDataBaseInfo();
		string m_strName;
		string m_strType;
		string m_strConnectString;
		int m_iConnectCount;
		int m_iHeartBeatInterval;
	};

	class XServiceInfo
	{
	public:
		XServiceInfo();
		XServiceType m_xType;
		string m_strName;
		string m_strDbName;
		int iStatus;//0-停用，1-启用
	};

	class XNormalServiceInfo:public XServiceInfo
	{
	};

	class XLoopServiceInfo :public XServiceInfo
	{
	public:
		XLoopServiceInfo();
		int m_iInterval;
		int64_t m_i64ThreadId;
	};

	class XTcpClientPoolInfo
	{
	public:
		XTcpClientPoolInfo();
		string m_strName;
		string m_strRemoteIP;
		int m_iRemotePort;
		int m_iKeep;
		int m_iConnectCount;
	};

	class XThreadPoolInfo
	{
	public:
		XThreadPoolInfo();
		size_t m_szSize;//线程池大小
	};

	class XLoggerInfo
	{
	public:
		XLoggerInfo();
		int m_iLogPriority; //日志级别
		int m_iScreenPrint;
	};

	class XAppConfigInfo
	{
	public:
		string m_strFileName;
		string m_strPath;
		string m_strFile;
		XLoggerInfo m_xLoggerInfo;
		XThreadPoolInfo m_xThreadPoolInfo;
		map<string, XTcpClientPoolInfo *> m_mapPXTcpClientPoolInfo;
		map<string, XListenerInfo *> m_mapPXListenerInfo;
		map<string, XDataBaseInfo *> m_mapPXDataBaseInfo;
		map<string, XServiceInfo *> m_mapPXServiceInfo;
		map<string, string> m_mapXConfigValue;
	};

	int ReadConfig();
}
#endif //__XCONFIG_H__
