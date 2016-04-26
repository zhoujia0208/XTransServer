#include "StdAfx.h"
#include "XHttpsListener.h"

namespace XNet {

	XHttpsListener::XHttpsListener()
	{

	}

	int XHttpsListener::Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle)
	{
		int iResult = X_SUCCESS;
		m_pXListenerInfo = pXListenerInfo;
		XConfig::XHttpsListenerInfo *pXHttpsListenerInfo = (XConfig::XHttpsListenerInfo *)pXListenerInfo;

		SSL_load_error_strings();
		iResult = SSL_library_init();
		if (iResult == 0)
		{
			XLogClass::error("XHttpsListener::Initialize SSL_library_init Fail");
			return X_FAILURE;
		}

		m_pSslCtx = SSL_CTX_new(SSLv23_method());
		if (m_pSslCtx == NULL)
		{
			XLogClass::error("XHttpsListener::Initialize SSL_CTX_new Fail");
			return X_FAILURE;
		}
		//m_pSslCtx->options |= SSL_OP_ALL;
		SSL_CTX_set_options(m_pSslCtx, SSL_OP_ALL);

		string strCertFile = g_xAppConfigInfo.m_strPath + pXHttpsListenerInfo->m_strServerCert;
		iResult = SSL_CTX_use_certificate_file(m_pSslCtx, strCertFile.c_str(), SSL_FILETYPE_PEM);
		if (iResult <= 0)
		{
			XLogClass::error("XHttpsListener::Initialize SSL_CTX_use_certificate_file Fail:[%d]", iResult);
			return X_FAILURE;
		}

		string strPrivateKeyFile = g_xAppConfigInfo.m_strPath + pXHttpsListenerInfo->m_strServerPrivateKey;
		iResult = SSL_CTX_use_PrivateKey_file(m_pSslCtx, strPrivateKeyFile.c_str(), SSL_FILETYPE_PEM);
		if (iResult <= 0)
		{
			XLogClass::error("XHttpsListener::Initialize SSL_CTX_use_PrivateKey_file Fail:[%d]", iResult);
			return X_FAILURE;
		}

		iResult = SSL_CTX_check_private_key(m_pSslCtx);
		if (iResult == 0)
		{
			XLogClass::error("XHttpsListener::Initialize SSL_CTX_check_private_key Fail");
			return X_FAILURE;
		}

		//是否需要强制验证客户端证书
		if (pXHttpsListenerInfo->m_iVerifyPeer)
		{
			SSL_CTX_set_verify(m_pSslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
			SSL_CTX_set_verify_depth(m_pSslCtx, 9);//设置验证深度
			string strClientCAFile = g_xAppConfigInfo.m_strPath + pXHttpsListenerInfo->m_strTrustCert;
			iResult = SSL_CTX_load_verify_locations(m_pSslCtx, strClientCAFile.c_str(), NULL);//设置CA证书
			if (iResult != 1)
			{
				XLogClass::error("XHttpsListener::Initialize SSL_CTX_load_verify_locations Fail");
				return X_FAILURE;
			}
			//设置客户端列表
			//string strClientCAFile = g_xAppConfigInfo.m_strPath + "./MyCert/NFCOS.crt";
			//SSL_CTX_set_client_CA_list(m_pSslCtx, SSL_load_client_CA_file(strClientCAFile.c_str()));

			//设置密码
			//SSL_CTX_set_default_passwd_cb_userdata(m_pSslCtx, (void*)"fmshzsg");
			//SSL_CTX_set_default_passwd_cb(soap->ctx, ssl_password);
		}
		else
		{
			SSL_CTX_set_verify(m_pSslCtx, SSL_VERIFY_NONE, NULL);
		}

		m_pXSocket = new XSocket();
		if (m_pXSocket == NULL)
		{
			XLogClass::error("XHttpsListener::Initialize new XSocket Fail");
			return X_FAILURE;
		}

		iResult = m_pXSocket->Initialize(pLoopHandle, this);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpsListener::Initialize m_pXSocket->Initialize Fail");
			return X_FAILURE;
		}

		iResult = m_pXSocket->Bind(pXHttpsListenerInfo->m_iPort);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpsListener::Initialize m_pXSocket->Bind Fail, Port[%d]", pXHttpsListenerInfo->m_iPort);
			return X_FAILURE;
		}

		m_xarListen.m_pAsyncData = this;
		iResult = m_pXSocket->ListenAsync(128, &m_xarListen, onConnection);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpsListener::Initialize m_pXSocket->ListenAsync Fail, Port[%d]", pXHttpsListenerInfo->m_iPort);
			return X_FAILURE;
		}

		XLogClass::info("XHttpsListener::Initialize Name[%s] Port[%d] Success", pXHttpsListenerInfo->m_strName.c_str(), pXHttpsListenerInfo->m_iPort);
		return X_SUCCESS;
	}

	XHttpsListener::~XHttpsListener()
	{
		//原则上需要释放 m_pXSocket
		//但是由于服务端始终运行，除非程序关闭
		//因此这里偷懒不实现释放资源了
		if (m_pSslCtx != NULL)
		{
			SSL_CTX_free(m_pSslCtx);
			m_pSslCtx = NULL;
		}
	}

	void XHttpsListener::deleteConnection(int64_t i64SessionIdInner)
	{
		map<int64_t, XSocket *>::iterator it = m_mapXConnection.find(i64SessionIdInner);
		if (it == m_mapXConnection.end())
			return;
		XHttpsConnection *pXConnection = (XHttpsConnection *)it->second;
		m_mapXConnection.erase(it);
		mg_iConnectedCount--;
		delete pXConnection;
		return;
	}

	int XHttpsListener::onConnection(XAsyncResult *pXAsyncResult)
	{
		int iResult = X_SUCCESS;
		XHttpsListener *pXHttpsListener = (XHttpsListener *)pXAsyncResult->m_pAsyncData;

		if (pXAsyncResult->m_iStatus != X_SUCCESS)
		{
			XLogClass::error("XHttpsListener::onConnection pXAsyncResult->m_iStatus != X_SUCCESS");
			return X_FAILURE;
		}

		//超过配置的最大连接数，则不接受这个链接
		if (pXHttpsListener->mg_iConnectedCount >= pXHttpsListener->m_pXListenerInfo->m_iMaxConnectCount)
		{
			XLogClass::warn("XHttpsListener::onConnection ConnectedCount[%d] >= MaxConnectCount[%d], This connection will not be accepted",
				pXHttpsListener->mg_iConnectedCount, pXHttpsListener->m_pXListenerInfo->m_iMaxConnectCount);
			return X_FAILURE;
		}

		XHttpsConnection *pXConnection = new XHttpsConnection();
		if (pXConnection == NULL)
		{
			XLogClass::error("XHttpsListener::onConnection new XHttpsConnection Fail");
			return X_FAILURE;
		}

		iResult = pXHttpsListener->m_pXSocket->Accept(pXConnection);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpsListener::onConnection pXHttpsListener->m_pXSocket->Accept Fail");
			delete pXConnection;
			return X_FAILURE;
		}

		iResult = pXConnection->Initialize(pXHttpsListener->m_pSslCtx);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpsListener::onConnection SSL_new Fail");
			delete pXConnection;
			return X_FAILURE;
		}

		iResult = pXConnection->ReadAsync(pXHttpsListener->m_pXListenerInfo->m_iTimeOut, &pXConnection->m_xarRead, onRead);
		if (iResult != 0)
		{
			XLogClass::error("XHttpsListener::onConnection pXConnection->ReadAsync Fail");
			delete pXConnection;
			return X_FAILURE;
		}

		pXHttpsListener->mg_iConnectedCount++;
		pXConnection->m_i64SessionIdInner = ++pXHttpsListener->mg_i64SessionIdInner;
		pXHttpsListener->m_mapXConnection.insert(map<int64_t, XHttpsConnection *>::value_type(pXConnection->m_i64SessionIdInner, pXConnection));

		return X_SUCCESS;
	}

	int XHttpsListener::onRead(XAsyncResult *pXAsyncResult)
	{
		int iResult = X_SUCCESS;
		XHttpsConnection *pXConnection = (XHttpsConnection *)pXAsyncResult->m_pInnerClass;
		XHttpsListener *pXListener = (XHttpsListener *)pXConnection->m_pUserData;

		if (pXAsyncResult->m_iStatus != X_SUCCESS)
		{
			if (pXAsyncResult->m_iInnerStatus == UV_EOF)
				XLogClass::debug("XHttpsListener::onRead Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);
			else
				XLogClass::warn("XHttpsListener::onRead Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);

			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			return X_SUCCESS;
		}

		XHttpsThreadData *pXThreadData = new XHttpsThreadData();
		if (pXThreadData == NULL)
		{
			XLogClass::error("XHttpsListener::onRead new XHttpsThreadData Fail");
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			return X_FAILURE;
		}

		//开线程处理
		pXThreadData->m_i64SessionIdInner = pXConnection->m_i64SessionIdInner;
		pXThreadData->m_i64SocketId = pXConnection->m_i64SocketId;
		pXThreadData->m_pXListener = pXListener;
		pXThreadData->m_pXConnection = pXConnection;

		uv_work_t *pWorkHandle = new uv_work_t();
		if (pWorkHandle == NULL)
		{
			XLogClass::error("XHttpsListener::onRead new uv_work_t Fail");
			delete pXThreadData;
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			return X_FAILURE;
		}
		pWorkHandle->data = pXThreadData;

		//先暂停ReadAsync，防止多次触发onRead，导致多线程冲突
		pXConnection->CancelReadAsync(&pXConnection->m_xarRead);
		iResult = uv_queue_work(uv_default_loop(), pWorkHandle, onProcess, afterWork);
		if (iResult != 0)
		{
			XLogClass::error("XHttpListener::onRead uv_queue_work 调用失败");
			delete pXThreadData;
			delete pWorkHandle;
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			return X_FAILURE;
		}

		return X_SUCCESS;
	}

	int XHttpsListener::onWrite(XAsyncResult *pXAsyncResult)
	{
		XHttpsConnection *pXConnection = (XHttpsConnection *)pXAsyncResult->m_pInnerClass;
		if (pXAsyncResult->m_iStatus != X_SUCCESS)
			XLogClass::warn("XTcpListener::onWrite Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);

		//异步标识用于判断是否释放连接
		if (pXAsyncResult->m_iAsyncFlag)
		{
			XHttpsListener *pXListener = (XHttpsListener *)pXAsyncResult->m_pAsyncData;
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
		}

		delete pXAsyncResult;
		return X_SUCCESS;
	}

	void XHttpsListener::onProcess(uv_work_t *pWorkHandle)
	{
		XHttpsThreadData *pXThreadData = (XHttpsThreadData *)pWorkHandle->data;
		XHttpsConnection *pXConnection = pXThreadData->m_pXConnection;
		vector<XBuffer *> vBuffers;

		BIO_write(pXConnection->m_pBioRead, pXConnection->m_xReceiveBuffer.m_pBuffer, pXConnection->m_xReceiveBuffer.m_szUsedLen);
		pXConnection->m_xReceiveBuffer.m_szUsedLen = 0;//认为数据全部处理完

		if (!pXConnection->m_isSslConnected)
		{
			//没有完成ssl的握手，继续进行握手。
			int iResult = SSL_do_handshake(pXConnection->m_pSsl);//SSL_connect(ssl);
			if (iResult != 1)
			{
				//握手还未成功，判断原因
				int iError = SSL_get_error(pXConnection->m_pSsl, iResult);
				if (iError != SSL_ERROR_WANT_READ && iError != SSL_ERROR_WANT_WRITE)
				{
					XLogClass::error("HttpsProcess SSL_do_handshake Fail:[%d] Error[%d]", iResult, iError);
					pXThreadData->m_iStatus = X_FAILURE;
					return;
				}

				XLogClass::debug("HttpsProcess handshake iFlag[%d]", iError);
				pXThreadData->m_iStatus = pXConnection->PrepareWriteData(vBuffers);
				return;
			}

			//握手正常完成,再判断后面有没有收到数据
			pXConnection->m_isSslConnected = true;
			XLogClass::debug("HttpsProcess SSL_do_handshake OK SocketId[%lld]", pXConnection->m_i64SocketId);
		}

		if (pXConnection->m_pXHttpContext == NULL)
		{
			pXConnection->m_pXHttpContext = new XHttpContext();
			if (pXConnection->m_pXHttpContext == NULL)
			{
				XLogClass::info("HttpsProcess new XHttpContext Fail");
				pXThreadData->m_iStatus = X_FAILURE;
				return;
			}
			pXConnection->m_pXHttpContext->m_xHttpType = XHTTP_TYPE_XHTTPS;
			pXConnection->m_pXHttpContext->m_pXConnection = pXConnection;
		}
		XHttpContext *pXHttpContext = pXConnection->m_pXHttpContext;
		//XHttpsListener *pXListener = (XHttpsListener *)pXConnection->m_pUserData;
		//XConfig::XHttpsListenerInfo *pXListenerInfo = (XConfig::XHttpsListenerInfo *)pXListener->m_pXListenerInfo;
		//Https默认保持连接
		pXHttpContext->m_bKeepAlive = true;

		BYTE bReadBuffer[MAX_HTTP_HEAD_LENGTH + 1];
		size_t szMaxLength = MAX_HTTP_HEAD_LENGTH;
		size_t iReadLength = 0;
		do
		{
			do {
				int iResult = SSL_read(pXConnection->m_pSsl, bReadBuffer + iReadLength, szMaxLength - iReadLength);
				if (iResult == 0)
					break;
				if (iResult < 0)
				{
					int iError = SSL_get_error(pXConnection->m_pSsl, iResult);
					if (iError != SSL_ERROR_WANT_READ && iError != SSL_ERROR_WANT_WRITE)
					{
						XLogClass::error("XHttpsListener::onRead SSL_read Fail:[%d] Error[%d]", iResult, iError);
						pXThreadData->m_iStatus = X_FAILURE;
						return;
					}

					break;
					//XLogClass::debug("XHttpsListener::onRead SSL_read iFlag[%d]", iError);
					//pXThreadData->m_iStatus = pXConnection->PrepareWriteData(vBuffers);
					//return;
				}

				iReadLength += iResult;
				if (iReadLength == szMaxLength)
					break;//最大长度收满了

			} while (true);

			if (iReadLength == 0)
				break;//没有新数据

			XBuffer xBuffer(bReadBuffer, iReadLength + 1, iReadLength);
			xBuffer.m_pBuffer[xBuffer.m_szUsedLen] = 0x00;
			pXThreadData->m_iStatus = HttpProcess(pXHttpContext, xBuffer);
			if (pXThreadData->m_iStatus != X_SUCCESS)
				return;
			if (pXHttpContext->m_xHttpParseStep != XHTTP_PARSE_FINISH
				&& pXHttpContext->m_iStatusCode == 0)
			{
				//未收满报文，未解析完整，分2种情况
				if (iReadLength == szMaxLength)
				{
					//1-之前缓冲区满，未收全本次内容的，继续SSL_read
					iReadLength = 0;
					continue;
				}
				//2-缓冲区足够，已收全本次所有内容，本次处理结束，等待再次触发
				break;
			}

			//正常处理完成
			vBuffers.push_back(&pXHttpContext->m_xResponseHead);
			if (pXHttpContext->m_xResponseBody.m_pBuffer != NULL)
				vBuffers.push_back(&pXHttpContext->m_xResponseBody);
			string strKeepAlive;
			pXHttpContext->GetRequestHeadInfo("Connection", strKeepAlive);
			toLower(strKeepAlive);
			if (strKeepAlive != "keep-alive")
				pXHttpContext->m_bKeepAlive = false;
		} while (0);

		pXThreadData->m_iStatus = pXConnection->PrepareWriteData(vBuffers);
		return;
	}

	void XHttpsListener::afterWork(uv_work_t *pWorkHandle, int iStatus)
	{
		int iResult = X_SUCCESS;
		XHttpsThreadData *pXThreadData = (XHttpsThreadData *)pWorkHandle->data;
		XHttpsListener *pXListener = pXThreadData->m_pXListener;
		XHttpsConnection *pXConnection = pXThreadData->m_pXConnection;

		switch (pXThreadData->m_iStatus)
		{
		case X_ASYNC://异步Http模式
					 //新增功能，应在异步业务处理中给出应答，这里什么都不做
					 ////pXConnection->m_pXHttpContext = NULL;
					 ////pXConnection->ReadAsync(pXListener->m_pXListenerInfo->m_iTimeOut, &pXConnection->m_xarRead, onRead);
			break;
		case X_SUCCESS://处理成功，需要应答
		{
			//if (pXConnection->m_pXHttpContext == NULL)
			//	break;

			//线程处理完毕，再次开始ReadAsync
			pXConnection->ReadAsync(pXListener->m_pXListenerInfo->m_iTimeOut, &pXConnection->m_xarRead, onRead);

			//判断是否有应答数据需要传输
			//没有则可能是报文未收全
			if (pXConnection->m_xSendBuffer.m_pBuffer == NULL)
				break;

			XAsyncResult *pXAsyncResult = new XAsyncResult();
			if (pXAsyncResult == NULL)
			{
				XLogClass::error("XHttpsListener::afterWork new XAsyncResult Fail");
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				break;
			}

			//没有后续HttpBody需要发送，而且不用KeepAlive，则设置发送完毕后关闭此连接
			pXAsyncResult->m_pAsyncData = pXListener;
			pXAsyncResult->m_i64AsyncId = pXConnection->m_i64SessionIdInner;
			if (pXConnection->m_pXHttpContext && !pXConnection->m_pXHttpContext->m_bKeepAlive)
				pXAsyncResult->m_iAsyncFlag = 1;
			vector<XBuffer *> vBuffers;
			vBuffers.push_back(&pXConnection->m_xSendBuffer);
			iResult = pXConnection->WriteAsync(vBuffers, 15000, 1, pXAsyncResult, onWrite);
			if (iResult != X_SUCCESS)
			{
				XLogClass::error("XHttpListener::afterWork pXConnection->WriteAsync Fail");
				delete pXAsyncResult;
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				break;
			}
			//复制标志置0，避免析构释放内存
			pXConnection->m_xSendBuffer.SetCopyFlag(0);
			pXConnection->m_xSendBuffer.Dispose();

			//释放，重置pXConnection->m_pXHttpContext为NULL
			if (pXConnection->m_pXHttpContext && pXConnection->m_pXHttpContext->m_iStatusCode != 0)
			{
				//有做过处理应答，本次XHttpContext已完成，释放
				RELEASE(pXConnection->m_pXHttpContext);
			}
			break;
		}
		default://发生错误，删除连接
			XLogClass::error("XHttpsListener::afterWork pXThreadData->m_iStatus[%d]", pXThreadData->m_iStatus);
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			break;
		}

		delete pXThreadData;
		delete pWorkHandle;
		return;
	}

	XHttpsThreadData::XHttpsThreadData()
	{
		m_pXListener = NULL;
		m_pXConnection = NULL;
	}

	XHttpsThreadData::~XHttpsThreadData()
	{

	}
}
