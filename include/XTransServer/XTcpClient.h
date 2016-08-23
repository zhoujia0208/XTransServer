#pragma once
#include "XSocket.h"

enum XTcpResult
{
	UnKnow = -1,
	Success,
	Failure,
	TimeOut,
};

class XTcpClient
{
public:
	XTcpClient();
	~XTcpClient();

	SOCKET GetSocketId();
	XTcpResult Connect(string strRemoteIP, int iRemotePort);
	XTcpResult WriteSync(unsigned char *pData, size_t szLength, int iTimeOut);
	XTcpResult ReadSync(unsigned char *&pData, size_t &szLength, int iTimeOut);
	int Close();

	int Lock();
	int TryLock();
	int UnLock();

	void *pXUserData;
	string m_strRemoteIP;
	int m_iRemotePort;

private:
	XTcpResult xTcpResult;
	uv_loop_t *m_pXLoop;
	uv_tcp_t m_hTcp;
	uv_timer_t m_hTimer;
	//uv_buf_t bReadBuffer;
	uv_mutex_t m_hMutex;
	auto_ptr<BYTE> pReadBuffer;
	size_t szReadLength;

	static void onConnect(uv_connect_t *pConnHandle, int iStatus);
	static void onAlloc(uv_handle_t *pHandle, size_t suggested_size, uv_buf_t* buf);
	static void onRead(uv_stream_t *pStreamHandle, ssize_t nread, const uv_buf_t *buf);
	static void onReadTimeOut(uv_timer_t *pTimerHandle);

	static void onWrite(uv_write_t *pWriteHandle, int iStatus);
};

class XTcpClientPool
{
public:
	XTcpClientPool();
	~XTcpClientPool();

	int Initialize(XConfig::XTcpClientPoolInfo *pInfo);
	int Release();

	//获取一个可用的连接，暂时简单点，循环获取
	int GetTcpClient(XTcpClient *&pXTcpClient);
	int GetTcpClientVector(vector<XTcpClient * >&vTcpClient);

	//获取一个新的连接，需要调用方释放
	int GetNewTcpClient(XTcpClient *&pXTcpClient);
private:
	vector<XTcpClient *> m_vTcpClient;
	XConfig::XTcpClientPoolInfo *m_pInfo;
	vector<string> m_vRemoteIPs;
	int m_iIndex;
	int m_iNewIndex;
};

extern map<string, XTcpClientPool *> g_mapTcpClientPool;

