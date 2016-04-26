#include "StdAfx.h"
#include "XTcpClient.h"

int XTcpClientPool::Initialize(string strName, string strRemoteIP, int iRemotePort, int iConnectCount)
{
	XLogClass::info("XTcpClientPool::Initialize [%s],IP[%s],Port[%d],ConnectCount[%d]", 
		strName.c_str(), strRemoteIP.c_str(), iRemotePort, iConnectCount);
	m_strName = strName;
	m_strRemoteIP = strRemoteIP;
	m_iRemotePort = iRemotePort;
	m_iConnectCount = iConnectCount;
	m_iIndex = 0;
	for(int i = 0; i < iConnectCount; i++)
	{
		XTcpClient *pXTcpClient = new XTcpClient();
		//Tcp连接初始化错误，暂时不做处理，等后续长链接的心跳服务等恢复链接
		pXTcpClient->Connect(m_strRemoteIP, m_iRemotePort);
		m_vTcpClient.push_back(pXTcpClient);
	}

	XLogClass::info("XTcpClientPool::Initialize [%s] Success", strName.c_str());
	return X_SUCCESS;
}

XTcpClientPool::~XTcpClientPool()
{
	Release();
}

int XTcpClientPool::Release()
{
	for(int i = 0; i < (int)m_vTcpClient.size(); i++)
	{
		m_vTcpClient[i]->Close();
		RELEASE(m_vTcpClient[i]);
	}
	m_vTcpClient.clear();

	return X_SUCCESS;
}

int XTcpClientPool::GetTcpClient(XTcpClient *&pXTcpClient)
{
	if(m_iIndex >= m_iConnectCount)
	{
		XLogClass::error("XTcpClientPool::GetTcpClient m_iIndex >= m_iConnectCount");
		return X_FAILURE;
	}

	pXTcpClient = m_vTcpClient[m_iIndex++];
	if(m_iIndex >= m_iConnectCount)
		m_iIndex = 0;

	return X_SUCCESS;
}

int XTcpClientPool::GetTcpClientVector(vector<XTcpClient * >&vTcpClient)
{
	vTcpClient = m_vTcpClient;

	return X_SUCCESS;
}

int XTcpClientPool::GetNewTcpClient(XTcpClient *&pXTcpClient)
{
	pXTcpClient = new XTcpClient();
	XTcpResult xTcpResult = pXTcpClient->Connect(m_strRemoteIP, m_iRemotePort);
	if (xTcpResult != Success)
		return X_FAILURE;
	return X_SUCCESS;
}

XTcpClient::XTcpClient(void)
{
	m_pXLoop = uv_loop_new();
	m_pXLoop->data = NULL;
	xTcpResult = UnKnow;

	uv_tcp_init(m_pXLoop, &m_hTcp);
	uv_timer_init(m_pXLoop, &m_hTimer);
	m_hTimer.data = this;
	uv_mutex_init(&m_hMutex);
	pXUserData = NULL;
}

XTcpClient::~XTcpClient(void)
{
	this->Close();
	uv_mutex_destroy(&m_hMutex);
	if(m_pXLoop != NULL)
	{
		uv_loop_delete(m_pXLoop);
		m_pXLoop = NULL;
	}
	RELEASE(pXUserData);
}

int XTcpClient::Lock()
{
	uv_mutex_lock(&m_hMutex);

	return X_SUCCESS;
}

int XTcpClient::TryLock()
{
	if(!uv_mutex_trylock(&m_hMutex))
		return X_SUCCESS;
	return X_FAILURE;
}

int XTcpClient::UnLock()
{
	uv_mutex_unlock(&m_hMutex);

	return X_SUCCESS;
}

SOCKET XTcpClient::GetSocketId()
{
#ifdef _WIN32
	return m_hTcp.socket;
#else
	return m_hTcp.io_watcher.fd;
#endif
}

XTcpResult XTcpClient::Connect(string strRemoteIP, int iRemotePort)
{
	int iResult = 0;
	this->Close();
	m_strRemoteIP = strRemoteIP;
	m_iRemotePort = iRemotePort;

	uv_tcp_init(m_pXLoop, &m_hTcp);
	m_hTcp.data = this;
	uv_timer_init(m_pXLoop, &m_hTimer);

	struct sockaddr_in addr;
	iResult  = uv_ip4_addr(m_strRemoteIP.c_str(), m_iRemotePort, &addr);
	if (iResult != X_SUCCESS)
	{
		XLogClass::error("XTcpClient::Connect uv_ip4_addr 调用失败");
		return Failure;
	}

	uv_connect_t *pConnHandle = new uv_connect_t();
	if(pConnHandle == NULL)
	{
		XLogClass::error("XTcpClient::Connect 动态创建 uv_connect_t 失败");
		return Failure;
	}
	pConnHandle->data = this;

	iResult =  uv_tcp_connect(pConnHandle, &m_hTcp, (const sockaddr *)&addr, onConnect);
	if(iResult != X_SUCCESS)
	{
		XLogClass::error("XTcpClient::Connect uv_tcp_connect 调用失败");
		return Failure;
	}

	xTcpResult = UnKnow;
	iResult = uv_run(m_pXLoop, UV_RUN_DEFAULT);
	if(iResult != X_SUCCESS)
	{
		XLogClass::error("XTcpClient::Connect uv_run 调用失败");
		return Failure;
	}

	return xTcpResult;
}

int XTcpClient::Close()
{
	if (!uv_is_closing((uv_handle_t*)&m_hTimer))
		uv_close((uv_handle_t*)&m_hTimer, NULL);
	if (!uv_is_closing((uv_handle_t*)&m_hTcp))
		uv_close((uv_handle_t*)&m_hTcp, NULL);
	uv_run(m_pXLoop, UV_RUN_DEFAULT);
	return X_SUCCESS;
}

XTcpResult XTcpClient::WriteSync(unsigned char *pData, size_t szLength, int iTimeOut)
{
	if (uv_is_closing((uv_handle_t*)&m_hTcp))
	{
		XLogClass::error("XTcpClient::WriteSync m_hTcp 已关闭");
		return Failure;
	}
	uv_write_t *pWriteHandle = (uv_write_t *)new uv_write_t();
	if(pWriteHandle == NULL)
	{
		XLogClass::error("XTcpClient::WriteSync 动态创建 uv_write_t 失败");
		return Failure;
	}

	pWriteHandle->data = this;

	uv_buf_t write_buffer;
	write_buffer.base = (char *)pData;
	write_buffer.len = szLength;

	int iResult = uv_write(pWriteHandle, (uv_stream_t *)&m_hTcp, &write_buffer, 1, onWrite);
	if(iResult != 0)
	{
		XLogClass::error("XTcpClient::WriteSync uv_write 调用失败: %s", uv_strerror(iResult));

		delete pWriteHandle;
		return Failure;
	}

	xTcpResult = UnKnow;
	iResult = uv_run(m_pXLoop, UV_RUN_DEFAULT);
	if(iResult != X_SUCCESS)
	{
		XLogClass::error("XTcpClient::WriteSync uv_run 调用失败");
		return Failure;
	}

	return xTcpResult;
}

XTcpResult XTcpClient::ReadSync(unsigned char *&pData, size_t &szLength, int iTimeOut)
{
	if (uv_is_closing((uv_handle_t*)&m_hTcp))
	{
		XLogClass::error("XTcpClient::ReadSync m_hTcp 已关闭");
		return Failure;
	}

	if(iTimeOut > 0)
	{
		uv_timer_start(&m_hTimer, onReadTimeOut, iTimeOut, 0);
	}
	
	int iResult = uv_read_start((uv_stream_t*)&m_hTcp, onAlloc, onRead);
	if(iResult != 0)
	{
		XLogClass::error("XTcpClient::ReadSync uv_read_start 调用失败");
		return Failure;
	}

	xTcpResult = UnKnow;
	iResult = uv_run(m_pXLoop, UV_RUN_DEFAULT);
	if(iResult != X_SUCCESS)
	{
		XLogClass::error("XTcpClient::ReadSync uv_run 调用失败");
		return Failure;
	}
	if(xTcpResult == Success)
	{
		if(pData == NULL)
		{
			szLength = szReadLength;
			pData = new BYTE[szLength];
		}
		memcpy(pData, pReadBuffer.get(), szReadLength);
	}

	return xTcpResult;
}

void XTcpClient::onConnect(uv_connect_t *pConnHandle, int iStatus)
{
	XTcpClient *pXTcpClient = (XTcpClient *)pConnHandle->data;

	if(iStatus != X_SUCCESS)
	{
		XLogClass::error("TcpClient::onConnection 失败: %s", uv_strerror(iStatus));
		pXTcpClient->xTcpResult = Failure;
		if (!uv_is_closing((uv_handle_t*)&pXTcpClient->m_hTimer))
			uv_close((uv_handle_t*)&pXTcpClient->m_hTimer, NULL);
		if (!uv_is_closing((uv_handle_t*)&pXTcpClient->m_hTcp))
			uv_close((uv_handle_t*)&pXTcpClient->m_hTcp, NULL);
	}
	else
	{
		pXTcpClient->xTcpResult = Success;
	}
	delete pConnHandle;
	return;
}

void XTcpClient::onWrite(uv_write_t *pWriteHandle, int iStatus)
{
	XTcpClient *pXTcpClient = (XTcpClient *)pWriteHandle->data;
	if (iStatus == X_SUCCESS)
	{
		pXTcpClient->xTcpResult = Success;
	}
	else
	{
		XLogClass::error("XTcpClient::onWrite 失败: %s", uv_strerror(iStatus));
		pXTcpClient->xTcpResult = Failure;
	}

	delete pWriteHandle;
	return;
}

void XTcpClient::onAlloc(uv_handle_t *pHandle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char *)new char[suggested_size];
	if (buf->base == NULL)
	{
		XLogClass::error("XTcpClient::onAlloc 分配内存失败");
		buf->len = 0;
	}
	else
		buf->len = suggested_size;
}

void XTcpClient::onRead(uv_stream_t *pStreamHandle, ssize_t nread, const uv_buf_t *buf)
{
	XTcpClient *pTcpClient = (XTcpClient *)pStreamHandle->data;
	uv_timer_stop(&pTcpClient->m_hTimer);
	uv_read_stop((uv_stream_t*)&pTcpClient->m_hTcp);
	//错误处理，包括连接断开
	if (nread < 0) 
	{
		if(nread == UV_EOF)
		{
			XLogClass::debug("XTcpClient::onRead UV_EOF: %ld", pTcpClient->GetSocketId());
		}
		else
		{
			XLogClass::error("XTcpClient::onRead ERROR: %ld %s", pTcpClient->GetSocketId(), uv_strerror(nread));
		}
		pTcpClient->xTcpResult = Failure;
	}

	//没有数据收到，什么都不做，继续收
	else if(nread == 0)
	{
	}

	//收到数据，存下数据
	else
	{
		pTcpClient->szReadLength = nread;
		pTcpClient->pReadBuffer.reset(new BYTE[nread]);
		memcpy(pTcpClient->pReadBuffer.get(), buf->base, nread);
		pTcpClient->xTcpResult = Success;
	}

	delete(buf->base);
	return;
}

void XTcpClient::onReadTimeOut(uv_timer_t *pTimerHandle)
{
	XLogClass::debug("XTcpClient::onReadTimeOut");
	XTcpClient *pTcpClient =(XTcpClient *)pTimerHandle->data;

	uv_timer_stop(&pTcpClient->m_hTimer);
	uv_read_stop((uv_stream_t*)&pTcpClient->m_hTcp);

	pTcpClient->xTcpResult = TimeOut;
	return;
}
