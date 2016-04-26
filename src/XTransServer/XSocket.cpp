#include "StdAfx.h"
#include "XSocket.h"
namespace XNet {

	XAsyncResult::XAsyncResult()
	{
		m_isCompleted = false;
		m_iStatus = 0;
		m_iInnerStatus = 0;
		m_pAsyncData = NULL;
		m_i64AsyncId = 0;
		m_iAsyncFlag = 0;

		m_pInnerClass = NULL;
		m_pRequestHandle = NULL;
		m_iTimeOut = 0;
		m_pTimerHandle = NULL;

		m_xAsyncCallback = NULL;
	}

	XAsyncResult::~XAsyncResult()
	{
		//防止错误是否需要先 uv_cancel ?
		RELEASE(m_pRequestHandle);
		//RELEASE(m_pTimerHandle);
		if(m_pTimerHandle != NULL)
			uv_close((uv_handle_t *)m_pTimerHandle, onClose);
	}

	void XAsyncResult::onClose(uv_handle_t *pHandle)
	{
		delete pHandle;
	}

	XBuffer::XBuffer()
	{
		m_pBuffer = NULL;
		m_szTotalLen = 0;
		m_szUsedLen = 0;
		m_iCopyFlag = 0;
	}

	XBuffer::XBuffer(BYTE *pBuffer, size_t szTotalLen, size_t szUsedLen)
	{
		m_pBuffer = pBuffer;
		m_szTotalLen = szTotalLen;
		m_szUsedLen = szUsedLen;
		m_iCopyFlag = 0;
	}

	XBuffer::~XBuffer()
	{
		_Dispose();
	}

	void XBuffer::Dispose()
	{
		_Dispose();
		m_szTotalLen = 0;
		m_szUsedLen = 0;

	}

	void XBuffer::_Dispose()
	{
		if (m_iCopyFlag)
		{
			delete m_pBuffer;
			m_iCopyFlag = 0;
		}
		m_pBuffer = NULL;
	}

	int XBuffer::Malloc(size_t szTotalLen)
	{
		_Dispose();
		m_pBuffer = new BYTE[szTotalLen];
		if (m_pBuffer == NULL)
		{
			XLogClass::error("XBuffer::Malloc new BYTE Fail");
			return X_FAILURE;
		}

		m_szTotalLen = szTotalLen;
		m_szUsedLen = 0;
		m_iCopyFlag = 1;

		return X_SUCCESS;
	}

	int XBuffer::CopyData(BYTE *pBuffer, size_t szLen)
	{
		if (m_szUsedLen + szLen > m_szTotalLen)
		{
			XLogClass::error("XBuffer::CopyData m_szUsedLen + szLen > m_szTotalLen");
			return X_FAILURE;
		}

		memcpy(m_pBuffer + m_szUsedLen, pBuffer, szLen);
		m_szUsedLen += szLen;
		return X_SUCCESS;
	}

	int XBuffer::SetData(BYTE *pBuffer, size_t szLen, int iCopyFlag)
	{
		_Dispose();
		if (iCopyFlag == 0)
		{
			//不需要复制内存的，直接指针赋值
			m_pBuffer = pBuffer;
			m_szTotalLen = szLen;
			m_szUsedLen = szLen;
			m_iCopyFlag = iCopyFlag;
			return X_SUCCESS;
		}

		if (Malloc(szLen) != X_SUCCESS)
			return X_FAILURE;

		if(CopyData(pBuffer, szLen) != X_SUCCESS)
			return X_FAILURE;

		return X_SUCCESS;
	}

	void XBuffer::SetCopyFlag(int iCopyFlag)
	{
		m_iCopyFlag = iCopyFlag;
	}

	XAsyncWriteRequest::XAsyncWriteRequest()
	{
		pBuffers = uvBuffers;
		szBuffer = 0;
		iFreeFlag = 0;
	}

	XAsyncWriteRequest::~XAsyncWriteRequest()
	{
		if (iFreeFlag)
		{
			for (size_t i = 0;i < szBuffer; i++)
				delete pBuffers[i].base;
		}
		if (pBuffers != uvBuffers)
			delete pBuffers;
	}

	XSocket::XSocket(XProtocolType xProtocolType)
	{
		m_pUserData = NULL;
		m_pLoopHandle = NULL;
		m_pTcpHandle = NULL;
		m_xProtocolType = xProtocolType;
		m_iReceiveBufferSize = DEFAULT_BUFFER_SIZE;
		m_i64SocketId = -1;
	}

	int XSocket::Initialize(uv_loop_t *pLoopHandle, void *pUserData)
	{
		m_pLoopHandle = pLoopHandle;
		m_pUserData = pUserData;
		if (m_pTcpHandle == NULL)
			m_pTcpHandle = new uv_tcp_t();

		if (m_pTcpHandle == NULL)
		{
			XLogClass::error("XSocket::Initialize new uv_tcp_t Fail");
			return X_FAILURE;
		}
		uv_tcp_init(pLoopHandle, m_pTcpHandle);
		//m_pTcpHandle->data = this;

		return X_SUCCESS;
	}

	XSocket::~XSocket()
	{
		_Dispose();
	}

	void XSocket::_Dispose()
	{
		CloseAsync(NULL, NULL);
	}

	inline SOCKET XSocket::_GetSocketId()
	{
#ifdef _WIN32
		return m_pTcpHandle->socket;
#else
		return m_pTcpHandle->io_watcher.fd;
#endif
	}

	int XSocket::Available()
	{
		return m_xReceiveBuffer.m_szUsedLen;
	}

	bool XSocket::IsBlocking()
	{
		return false;
	}

	bool XSocket::IsConnected()
	{
		if (m_pTcpHandle == NULL)
			return false;
		if (uv_is_closing((uv_handle_t*)m_pTcpHandle))
			return false;
		if(uv_is_readable((uv_stream_t*)m_pTcpHandle))
			return true;
		return false;
	}

	int XSocket::ConnectAsync(string strRemoteIP, int iRemotePort, int iTimeOut, XAsyncResult *pXAsyncResult, XAsyncCallback onConnectCallBack)
	{
		int iResult = X_SUCCESS;
		if (pXAsyncResult == NULL)
			return X_FAILURE;

		pXAsyncResult->m_xAsyncCallback = onConnectCallBack;
		pXAsyncResult->m_pInnerClass = this;
		pXAsyncResult->m_isCompleted = false;
		//if (iTimeOut > 0)
		//{
		//	if (pXAsyncResult->m_pTimerHandle == NULL)
		//	{
		//		pXAsyncResult->m_pTimerHandle = new uv_timer_t();
		//		if (pXAsyncResult->m_pTimerHandle == NULL)
		//		{
		//			XLogClass::error("XSocket::ConnectAsync new uv_timer_t Fail");
		//			return X_FAILURE;
		//		}
		//		uv_timer_init(m_pLoopHandle, (uv_timer_t *)pXAsyncResult->m_pTimerHandle);
		//	}
		//
		//	pXAsyncResult->m_iTimeOut = iTimeOut;
		//	uv_timer_start((uv_timer_t *)pXAsyncResult->m_pTimerHandle, onConnectAsyncTimeOut, iTimeOut, 0);
		//	((uv_timer_t *)(pXAsyncResult->m_pTimerHandle))->data = pXAsyncResult;
		//}

		//RELEASE(pXAsyncResult->m_pRequestHandle);
		pXAsyncResult->m_pRequestHandle = new uv_connect_t();
		if (pXAsyncResult->m_pRequestHandle == NULL)
		{
			XLogClass::error("XSocket::ConnectAsync new uv_connect_t Fail");
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			return X_FAILURE;
		}
		((uv_connect_t *)pXAsyncResult->m_pRequestHandle)->data = pXAsyncResult;

		struct sockaddr_in addr;
		iResult = uv_ip4_addr(strRemoteIP.c_str(), iRemotePort, &addr);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::ConnectAsync uv_ip4_addr Fail [%s]", uv_strerror(iResult));
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			return X_FAILURE;
		}

		iResult = uv_tcp_connect((uv_connect_t *)pXAsyncResult->m_pRequestHandle, m_pTcpHandle, (const sockaddr *)&addr, onConnectAsync);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::ConnectAsync uv_tcp_connect Fail [%s]", uv_strerror(iResult));
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			return X_FAILURE;
		}
		
		m_i64SocketId = _GetSocketId();
		return X_SUCCESS;
	}

	void XSocket::onConnectAsync(uv_connect_t *pConnHandle, int iStatus)
	{
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pConnHandle->data;
		if(pXAsyncResult->m_pTimerHandle != NULL)
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);

		pXAsyncResult->m_isCompleted = true;
		pXAsyncResult->m_iStatus = iStatus == 0 ? X_SUCCESS : X_FAILURE;
		pXAsyncResult->m_iInnerStatus = iStatus;

		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}
		//有设置回调，由调用方负责构造以及释放 XAsyncResult
		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}

	void XSocket::onConnectAsyncTimeOut(uv_timer_t *pTimerHandle)
	{
		XLogClass::debug("XSocket::onConnectAsyncTimeOut");
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pTimerHandle->data;
		uv_timer_stop(pTimerHandle);
		uv_cancel((uv_req_t *)pXAsyncResult->m_pRequestHandle);

		pXAsyncResult->m_isCompleted = true;
		pXAsyncResult->m_iStatus = X_TIMEOUT;
		pXAsyncResult->m_iInnerStatus = 0;
		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}

	void XSocket::CancelConnectAsync(XAsyncResult *pXAsyncResult)
	{
		return;
		//if (pXAsyncResult == NULL)
		//	return;
		//if (pXAsyncResult->m_isCompleted)
		//	return;
		//if(pXAsyncResult->m_pTimerHandle != NULL)
		//	uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
		//uv_cancel((uv_req_t *)pXAsyncResult->m_pRequestHandle);
		//pXAsyncResult->m_isCompleted = false;
		//pXAsyncResult->m_iStatus = X_CANCEL;
		//return;
	}

	int XSocket::CloseAsync(XAsyncResult *pXAsyncResult, XAsyncCallback onCloseCallBack)
	{
		if (m_pTcpHandle == NULL)
			return X_FAILURE;
		if (uv_is_closing((uv_handle_t *)m_pTcpHandle))
			return X_FAILURE;

		if (pXAsyncResult != NULL)
		{
			pXAsyncResult->m_xAsyncCallback = onCloseCallBack;
			pXAsyncResult->m_pInnerClass = this;
			//pXAsyncResult->m_pRequestHandle = m_pTcpHandle;//避免外部释放 m_pTcpHandle
		}

		m_pTcpHandle->data = pXAsyncResult;
		uv_close((uv_handle_t *)m_pTcpHandle, onCloseAsync);
		return X_SUCCESS;
	}

	void XSocket::onCloseAsync(uv_handle_t *pHandle)
	{
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pHandle->data;
		delete pHandle;

		if (pXAsyncResult == NULL)
			return;

		pXAsyncResult->m_isCompleted = true;
		pXAsyncResult->m_iStatus = X_SUCCESS;

		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}

	void XSocket::CancelCloseAsync(XAsyncResult *pXAsyncResult)
	{
		return;
		//if (pXAsyncResult == NULL)
		//	return;
		//if (pXAsyncResult->m_isCompleted)
		//	return;

		//pXAsyncResult->m_isCompleted = false;
		//pXAsyncResult->m_iStatus = X_CANCEL;
		//return;
	}

	int XSocket::ReadAsync(int iTimeOut, XAsyncResult *pXAsyncResult, XAsyncCallback onReadCallBack)
	{
		int iResult = X_SUCCESS;
		if (pXAsyncResult == NULL)
			return X_FAILURE;

		pXAsyncResult->m_xAsyncCallback = onReadCallBack;
		pXAsyncResult->m_pInnerClass = this;
		pXAsyncResult->m_isCompleted = false;
		if (iTimeOut > 0)
		{
			if (pXAsyncResult->m_pTimerHandle == NULL)
			{
				pXAsyncResult->m_pTimerHandle = new uv_timer_t();
				if (pXAsyncResult->m_pTimerHandle == NULL)
				{
					XLogClass::error("XSocket::ReadAsync new uv_timer_t Fail");
					return X_FAILURE;
				}
				uv_timer_init(m_pLoopHandle, (uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			}

			pXAsyncResult->m_iTimeOut = iTimeOut;
			uv_timer_start((uv_timer_t *)pXAsyncResult->m_pTimerHandle, onReadAsyncTimeOut, iTimeOut, 0);
			((uv_timer_t *)(pXAsyncResult->m_pTimerHandle))->data = pXAsyncResult;
		}

		//pXAsyncResult->m_pRequestHandle = m_pTcpHandle;
		//避免被外部释放
		pXAsyncResult->m_pRequestHandle = NULL;
		m_pTcpHandle->data = pXAsyncResult;

		iResult = uv_read_start((uv_stream_t *)m_pTcpHandle, onAlloc, onReadAsync);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::ReadAsync uv_read_start Fail [%s]", uv_strerror(iResult));
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			return X_FAILURE;
		}

		return X_SUCCESS;
	}

	//优化接收缓冲区逻辑，默认64K空间
	void XSocket::onAlloc(uv_handle_t *pHandle, size_t szSuggested, uv_buf_t *pBuffer)
	{
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pHandle->data;
		XSocket *pXSocket = (XSocket *)pXAsyncResult->m_pInnerClass;
		if (pXSocket->m_xReceiveBuffer.m_pBuffer == NULL)
		{
			if(pXSocket->m_xReceiveBuffer.Malloc(pXSocket->m_iReceiveBufferSize) != X_SUCCESS)//szSuggested
			{
				pBuffer->base = NULL;
				pBuffer->len = 0;
				return;
			}
		}

		pBuffer->base = (char *)(pXSocket->m_xReceiveBuffer.m_pBuffer + pXSocket->m_xReceiveBuffer.m_szUsedLen);
		//空余一个字节，不使用
		pBuffer->len = pXSocket->m_xReceiveBuffer.m_szTotalLen - pXSocket->m_xReceiveBuffer.m_szUsedLen - 1;
		return;
	}

	void XSocket::onReadAsync(uv_stream_t *pStreamHandle, ssize_t szRead, const uv_buf_t *pBuffer)
	{
		//int iResult = X_SUCCESS;
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pStreamHandle->data;
		XSocket *pXSocket = (XSocket *)pXAsyncResult->m_pInnerClass;
		if(pXAsyncResult->m_pTimerHandle != NULL)
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);

		//错误或者Socket关闭
		if (szRead < 0)
		{
			pXAsyncResult->m_iStatus = X_FAILURE;
		}
		else
		{
			//收到数据，已存下，在 m_xReceiveBuffer 中
			pXSocket->m_xReceiveBuffer.m_szUsedLen += szRead;
			pXAsyncResult->m_iStatus = X_SUCCESS;
		}

		//delete(pBuffer->base);//不需要释放，已在缓冲区中
		if (pXAsyncResult->m_iTimeOut > 0 && pXAsyncResult->m_pTimerHandle != NULL)
			uv_timer_start((uv_timer_t *)pXAsyncResult->m_pTimerHandle, onReadAsyncTimeOut, pXAsyncResult->m_iTimeOut, 0);

		pXAsyncResult->m_isCompleted = false;//需要继续接收
		pXAsyncResult->m_iInnerStatus = szRead;
		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);

		return;
	}

	void XSocket::onReadAsyncTimeOut(uv_timer_t *pTimerHandle)
	{
		XLogClass::debug("XSocket::onReadAsyncTimeOut");
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pTimerHandle->data;
		uv_timer_stop(pTimerHandle);
		uv_read_stop((uv_stream_t *)((XSocket *)pXAsyncResult->m_pInnerClass)->m_pTcpHandle);

		pXAsyncResult->m_isCompleted = true;
		pXAsyncResult->m_iStatus = X_TIMEOUT;
		pXAsyncResult->m_iInnerStatus = 0;
		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}

	void XSocket::CancelReadAsync(XAsyncResult *pXAsyncResult)
	{
		if (pXAsyncResult == NULL)
			return;
		if (pXAsyncResult->m_isCompleted)
			return;
		if(pXAsyncResult->m_pTimerHandle != NULL)
			uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
		if (pXAsyncResult->m_pInnerClass != NULL)
		{
			uv_read_stop((uv_stream_t *)((XSocket *)pXAsyncResult->m_pInnerClass)->m_pTcpHandle);
			pXAsyncResult->m_pInnerClass = NULL;
		}
			
		pXAsyncResult->m_isCompleted = false;
		pXAsyncResult->m_iStatus = X_CANCEL;
		return;
	}

	int XSocket::WriteAsync(vector<XBuffer *> &vBuffers, int iTimeOut, int iFreeFlag, XAsyncResult *pXAsyncResult, XAsyncCallback onWriteCallBack)
	{
		int iResult = X_SUCCESS;
		if (pXAsyncResult == NULL)
			return X_FAILURE;

		pXAsyncResult->m_xAsyncCallback = onWriteCallBack;
		pXAsyncResult->m_pInnerClass = this;
		pXAsyncResult->m_isCompleted = false;
		//if (iTimeOut > 0)
		//{
		//	if (pXAsyncResult->m_pTimerHandle == NULL)
		//	{
		//		pXAsyncResult->m_pTimerHandle = new uv_timer_t();
		//		if (pXAsyncResult->m_pTimerHandle == NULL)
		//		{
		//			XLogClass::error("XSocket::WriteAsync new uv_timer_t Fail");
		//			return X_FAILURE;
		//		}
		//		uv_timer_init(m_pLoopHandle, (uv_timer_t *)pXAsyncResult->m_pTimerHandle);
		//	}
		//
		//	pXAsyncResult->m_iTimeOut = iTimeOut;
		//	uv_timer_start((uv_timer_t *)pXAsyncResult->m_pTimerHandle, onWriteAsyncTimeOut, iTimeOut, 0);
		//	((uv_timer_t *)(pXAsyncResult->m_pTimerHandle))->data = pXAsyncResult;
		//}

		//RELEASE(pXAsyncResult->m_pRequestHandle);
		pXAsyncResult->m_pRequestHandle = new XAsyncWriteRequest();
		if (pXAsyncResult->m_pRequestHandle == NULL)
		{
			XLogClass::error("XSocket::WriteAsync new XAsyncWriteRequest Fail");
			if(pXAsyncResult->m_pTimerHandle != NULL)
				uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			return X_FAILURE;
		}

		XAsyncWriteRequest *pXWriteRequest = (XAsyncWriteRequest *)(pXAsyncResult->m_pRequestHandle);
		pXWriteRequest->uvRequest.data = pXAsyncResult;
		pXWriteRequest->iFreeFlag = iFreeFlag;
		pXWriteRequest->szBuffer = vBuffers.size();
		if (pXWriteRequest->szBuffer > 4)
		{
			pXWriteRequest->pBuffers = new uv_buf_t[pXWriteRequest->szBuffer];
			if (pXWriteRequest->pBuffers == NULL)
			{
				XLogClass::error("XSocket::WriteAsync new uv_buf_t Fail");
				return X_FAILURE;
			}
		}

		for (size_t i = 0; i < vBuffers.size(); i++)
		{
			pXWriteRequest->pBuffers[i].base = (char *)vBuffers[i]->m_pBuffer;
			pXWriteRequest->pBuffers[i].len = vBuffers[i]->m_szUsedLen;
		}

		iResult = uv_write(&pXWriteRequest->uvRequest, (uv_stream_t *)m_pTcpHandle, pXWriteRequest->pBuffers, pXWriteRequest->szBuffer, onWriteAsync);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::WriteAsync uv_write Fail [%s]", uv_strerror(iResult));
			//if (pXAsyncResult->m_pTimerHandle != NULL)
			//	uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
			return X_FAILURE;
		}

		return X_SUCCESS;
	}

	void XSocket::onWriteAsync(uv_write_t *pWriteHandle, int iStatus)
	{
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pWriteHandle->data;
		XAsyncWriteRequest *pXWriteRequest = (XAsyncWriteRequest *)pXAsyncResult->m_pRequestHandle;

		//停止写计时器
		//if (pXAsyncResult->m_pTimerHandle != NULL)
		//	uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);

		//释放发送的内容
		delete pXWriteRequest;
		pXAsyncResult->m_pRequestHandle = NULL;

		pXAsyncResult->m_isCompleted = true;
		pXAsyncResult->m_iStatus = iStatus == 0 ? X_SUCCESS : X_FAILURE;
		pXAsyncResult->m_iInnerStatus = iStatus;
		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}

	void XSocket::onWriteAsyncTimeOut(uv_timer_t *pTimerHandle)
	{
		XLogClass::debug("XSocket::onWriteAsyncTimeOut");
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pTimerHandle->data;
		XAsyncWriteRequest *pXWriteRequest = (XAsyncWriteRequest *)pXAsyncResult->m_pRequestHandle;

		if(pTimerHandle != NULL)
			uv_timer_stop(pTimerHandle);
		//uv_cancel((uv_req_t *)&pXAsyncWriteRequest->uvRequest);

		//释放发送的内容
		delete pXWriteRequest;
		pXAsyncResult->m_pRequestHandle = NULL;

		pXAsyncResult->m_isCompleted = true;
		pXAsyncResult->m_iStatus = X_TIMEOUT;
		pXAsyncResult->m_iInnerStatus = 0;
		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}

	void XSocket::CancelWriteAsync(XAsyncResult *pXAsyncResult)
	{
		return;
		//if (pXAsyncResult == NULL)
		//	return;
		//if (pXAsyncResult->m_isCompleted)
		//	return;

		//XAsyncWriteRequest *pXAsyncWriteRequest = (XAsyncWriteRequest *)pXAsyncResult->m_pRequestHandle;
		//if(pXAsyncResult->m_pTimerHandle != NULL)
		//	uv_timer_stop((uv_timer_t *)pXAsyncResult->m_pTimerHandle);
		//uv_cancel((uv_req_t *)&pXAsyncWriteRequest->uvRequest);

		////释放发送内容
		//delete pXAsyncWriteRequest->uvBuffer.base;

		//pXAsyncResult->m_isCompleted = false;
		//pXAsyncResult->m_iStatus = X_CANCEL;
		//return;
	}

	int XSocket::Accept(XSocket *pXSocket)
	{
		int iResult = X_SUCCESS;
		if (pXSocket == NULL)
		{
			XLogClass::error("XSocket::Accept pXSocket == NULL");
			return X_FAILURE;
		}

		iResult = pXSocket->Initialize(m_pLoopHandle, m_pUserData);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::Accept pXSocket->Initialize Fail");
			delete pXSocket;
			pXSocket = NULL;
			return X_FAILURE;
		}

		iResult = uv_accept((uv_stream_t *)m_pTcpHandle, (uv_stream_t *)pXSocket->m_pTcpHandle);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::Accept uv_accept Fail [%s]", uv_strerror(iResult));
			delete pXSocket;
			pXSocket = NULL;
			return X_FAILURE;
		}

		pXSocket->m_i64SocketId = pXSocket->_GetSocketId();
		return X_SUCCESS;
	}

	int XSocket::Bind(int iPort)
	{
		int iResult = X_SUCCESS;

		struct sockaddr_in addr;
		iResult = uv_ip4_addr("0.0.0.0", iPort, &addr);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XSocket::Bind uv_ip4_addr Fail [%s]", uv_strerror(iResult));
			return X_FAILURE;
		}

		iResult = uv_tcp_bind(m_pTcpHandle, (const sockaddr*)&addr, 0);
		if (iResult != 0)
		{
			XLogClass::error("XSocket::Bind uv_tcp_bind Fail [%s]", uv_strerror(iResult));
			return X_FAILURE;
		}

		m_i64SocketId = _GetSocketId();
		return X_SUCCESS;
	}

	int XSocket::ListenAsync(int iBackLog, XAsyncResult *pXAsyncResult, XAsyncCallback onConnectionCallBack)
	{
		int iResult = X_SUCCESS;
		if (pXAsyncResult == NULL)
			return X_FAILURE;

		pXAsyncResult->m_xAsyncCallback = onConnectionCallBack;
		pXAsyncResult->m_pInnerClass = this;
		m_pTcpHandle->data = pXAsyncResult;
		iResult = uv_listen((uv_stream_t *)m_pTcpHandle, iBackLog, onConnectionAsync);
		if (iResult != 0)
		{
			XLogClass::error("XSocket::ListenAsync uv_listen Fail [%s]", uv_strerror(iResult));
			return X_FAILURE;
		}

		return X_SUCCESS;
	}

	void XSocket::onConnectionAsync(uv_stream_t *pStreamHandle, int iStatus)
	{
		XAsyncResult *pXAsyncResult = (XAsyncResult *)pStreamHandle->data;

		//pXAsyncResult->m_isCompleted = true;//继续监听，不设置完成
		pXAsyncResult->m_iStatus = iStatus == 0 ? X_SUCCESS : X_FAILURE;
		pXAsyncResult->m_iInnerStatus = iStatus;
		if (pXAsyncResult->m_xAsyncCallback == NULL)
		{
			//默认不需要释放，需要释放的应有回调函数存在，在回调中释放
			//delete pXAsyncResult;
			return;
		}

		pXAsyncResult->m_xAsyncCallback(pXAsyncResult);
		return;
	}
}
