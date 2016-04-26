#include "StdAfx.h"
#include "XTcpListener.h"

namespace XNet {

	XTcpListener::XTcpListener()
	{
	}

	int XTcpListener::Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle)
	{
		int iResult = X_SUCCESS;
		m_pXListenerInfo = (XConfig::XTcpListenerInfo *)pXListenerInfo;
		m_pXSocket = new XSocket();
		if (m_pXSocket == NULL)
		{
			XLogClass::error("XTcpListener::Initialize new XSocket Fail");
			return X_FAILURE;
		}

		iResult = m_pXSocket->Initialize(pLoopHandle, this);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XTcpListener::Initialize m_pXSocket->Initialize Fail");
			return X_FAILURE;
		}

		iResult = m_pXSocket->Bind(m_pXListenerInfo->m_iPort);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XTcpListener::Initialize m_pXSocket->Bind Fail, Port[%d]", m_pXListenerInfo->m_iPort);
			return X_FAILURE;
		}

		m_xarListen.m_pAsyncData = this;
		iResult = m_pXSocket->ListenAsync(128, &m_xarListen, onConnection);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XTcpListener::Initialize m_pXSocket->ListenAsync Fail, Port[%d]", m_pXListenerInfo->m_iPort);
			return X_FAILURE;
		}

		XLogClass::info("XTcpListener::Initialize Name[%s] Port[%d] Success", m_pXListenerInfo->m_strName.c_str(), m_pXListenerInfo->m_iPort);
		return X_SUCCESS;
	}

	XTcpListener::~XTcpListener()
	{
		//原则上需要释放 m_pXSocket
		//但是由于服务端始终运行，除非程序关闭
		//因此这里偷懒不实现释放资源了
	}

	void XTcpListener::deleteConnection(int64_t i64SessionIdInner)
	{
		map<int64_t, XSocket *>::iterator it = m_mapXConnection.find(i64SessionIdInner);
		if (it == m_mapXConnection.end())
			return;
		XTcpConnection *pXConnection = (XTcpConnection *)it->second;
		m_mapXConnection.erase(it);
		mg_iConnectedCount--;
		delete pXConnection;
		return;
	}

	int XTcpListener::onConnection(XAsyncResult *pXAsyncResult)
	{
		int iResult = X_SUCCESS;
		XTcpListener *pXTcpListener = (XTcpListener *)pXAsyncResult->m_pAsyncData;

		if (pXAsyncResult->m_iStatus != X_SUCCESS)
		{
			XLogClass::error("XTcpListener::onConnection pXAsyncResult->m_iStatus != X_SUCCESS");
			return X_FAILURE;
		}

		//超过配置的最大连接数，则不接受这个链接
		if (pXTcpListener->mg_iConnectedCount >= pXTcpListener->m_pXListenerInfo->m_iMaxConnectCount)
		{
			XLogClass::warn("XTcpListener::onConnection ConnectedCount[%d] >= MaxConnectCount[%d], This connection will not be accepted",
				pXTcpListener->mg_iConnectedCount, pXTcpListener->m_pXListenerInfo->m_iMaxConnectCount);
			return X_FAILURE;
		}

		XTcpConnection *pXTcpConn = new XTcpConnection();
		if (pXTcpConn == NULL)
		{
			XLogClass::error("XTcpListener::onConnection new XTcpConnection Fail");
			return X_FAILURE;
		}

		iResult = pXTcpListener->m_pXSocket->Accept(pXTcpConn);
		if (iResult != 0)
		{
			XLogClass::error("XTcpListener::onConnection pXTcpListener->m_pXSocket->Accept Fail");
			delete pXTcpConn;
			return X_FAILURE;
		}

		iResult = pXTcpConn->ReadAsync(pXTcpListener->m_pXListenerInfo->m_iTimeOut, &pXTcpConn->m_xarRead, onRead);
		if (iResult != 0)
		{
			XLogClass::error("XTcpListener::onConnection pXTcpConn->ReadAsync Fail");
			delete pXTcpConn;
			return X_FAILURE;
		}

		pXTcpListener->mg_iConnectedCount++;
		pXTcpConn->m_i64SessionIdInner = ++pXTcpListener->mg_i64SessionIdInner;
		pXTcpListener->m_mapXConnection.insert(map<int64_t, XTcpConnection *>::value_type(pXTcpConn->m_i64SessionIdInner, pXTcpConn));

		return X_SUCCESS;
	}

	int XTcpListener::onRead(XAsyncResult *pXAsyncResult)
	{
		int iResult = X_SUCCESS;
		XTcpConnection *pXConnection = (XTcpConnection *)pXAsyncResult->m_pInnerClass;
		XTcpListener *pXListener = (XTcpListener *)pXConnection->m_pUserData;
		XConfig::XTcpListenerInfo *pXListenerInfo = (XConfig::XTcpListenerInfo *)pXListener->m_pXListenerInfo;

		if (pXAsyncResult->m_iStatus != X_SUCCESS)
		{
			if (pXAsyncResult->m_iInnerStatus == UV_EOF)
				XLogClass::debug("XTcpListener::onRead Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);
			else
				XLogClass::warn("XTcpListener::onRead Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);

			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			return X_SUCCESS;
		}

		//循环判断合法报文，将其分组，传入后续工作线程
		XConfig::XLengthInfo xLengthInfo = pXListenerInfo->m_xLengthInfo;
		size_t szDealedLength = 0;
		BYTE *pReadBuffer = pXConnection->m_xReceiveBuffer.m_pBuffer;
		size_t szReadLength = pXConnection->m_xReceiveBuffer.m_szUsedLen;
		do
		{
			//没有数据要处理，直接返回
			if (szReadLength == 0)
			{
				pXConnection->m_xReceiveBuffer.m_szUsedLen = 0;
				return X_SUCCESS;
			}

			//长度部分没有收满,继续等待接收
			if (szReadLength < (size_t)xLengthInfo.m_iOffset + xLengthInfo.m_iLength)
			{
				if (pReadBuffer != pXConnection->m_xReceiveBuffer.m_pBuffer)
				{
					pXConnection->m_xReceiveBuffer.m_szUsedLen = szReadLength;
					memcpy(pXConnection->m_xReceiveBuffer.m_pBuffer, pReadBuffer, szReadLength);
				}

				return X_SUCCESS;
			}

			//获取长度
			uint64_t ui64Length = 0;
			if (xLengthInfo.m_xType == XConfig::XLENGTHTYPE_NONE)
			{
				//没有长度类型，则本次收到的所有报文都直接认为合法
				ui64Length = szReadLength;
			}
			else
			{
				//其他长度类型，需要判断
				int iResult = GetLengthByType(pReadBuffer, xLengthInfo.m_iOffset, xLengthInfo.m_iLength, xLengthInfo.m_xType, ui64Length);
				if (iResult != X_SUCCESS)
				{
					string strTempLength;
					ByteToString(pReadBuffer + xLengthInfo.m_iOffset, xLengthInfo.m_iLength, strTempLength);
					XLogClass::warn("XTcpListener::onRead Length Parse Fail:Type=[%d],Content[HEX]=[%s]", xLengthInfo.m_xType, strTempLength.c_str());
					pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
					return X_FAILURE;
				}

				//整包还没有收满，继续收
				if (szReadLength < (size_t)xLengthInfo.m_iLengthOffset + ui64Length)
				{
					//XLogClass::debug("szReadLength[%u] < (size_t)pListenerInfo->lengthInfo.iLengthOffset[%d] + ui64Length[%llu]",
					//	szReadLength, pListenerInfo->lengthInfo.iLengthOffset, ui64Length);
					if (pReadBuffer != pXConnection->m_xReceiveBuffer.m_pBuffer)
					{
						pXConnection->m_xReceiveBuffer.m_szUsedLen = szReadLength;
						memcpy(pXConnection->m_xReceiveBuffer.m_pBuffer, pReadBuffer, szReadLength);
					}
					return X_SUCCESS;
				}
			}

			//收全报文了，存下报文，开线程处理
			size_t szNeedProcessLength = (size_t)(xLengthInfo.m_iLengthOffset + ui64Length);
			//先减处理长度，如之后的操作出错，连接将被关闭
			szReadLength -= szNeedProcessLength;
			szDealedLength += szNeedProcessLength;
			pReadBuffer += szNeedProcessLength;

			XTcpThreadData *pXThreadData = new XTcpThreadData();
			if (pXThreadData == NULL)
			{
				XLogClass::error("XTcpListener::onRead new XTcpThreadData Fail");
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				return X_FAILURE;
			}

			iResult = pXThreadData->m_xRequestBuffer.SetData(pReadBuffer - szNeedProcessLength, szNeedProcessLength, 1);
			if (iResult != X_SUCCESS)
			{
				XLogClass::error("XTcpListener::onRead pXThreadData->m_xRequestBuffer.SetData Fail");
				delete pXThreadData;
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				return X_FAILURE;
			}

			pXThreadData->m_i64SessionIdInner = pXConnection->m_i64SessionIdInner;
			pXThreadData->m_i64SocketId = pXConnection->m_i64SocketId;
			pXThreadData->m_pXListener = pXListener;

			uv_work_t *pWorkHandle = new uv_work_t();
			if (pWorkHandle == NULL)
			{
				XLogClass::error("XTcpListener::onRead pWorkHandle new Fail");
				delete pXThreadData;
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				return X_FAILURE;
			}

			pWorkHandle->data = pXThreadData;
			iResult = uv_queue_work(uv_default_loop(), pWorkHandle, onProcess, afterWork);
			if (iResult != 0)
			{
				XLogClass::error("XTcpListener::onRead uv_queue_work Fail");
				delete pXThreadData;
				delete pWorkHandle;
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				return X_FAILURE;
			}

		} while (true);
	}

	int XTcpListener::onWrite(XAsyncResult *pXAsyncResult)
	{
		XTcpConnection *pXTcpConn = (XTcpConnection *)pXAsyncResult->m_pInnerClass;
		if(pXAsyncResult->m_iStatus != X_SUCCESS)
			XLogClass::warn("XTcpListener::onWrite Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXTcpConn->m_i64SocketId);
		delete pXAsyncResult;
		return X_SUCCESS;
	}

	void XTcpListener::onProcess(uv_work_t *pWorkHandle)
	{
		XTcpThreadData *pXThreadData = (XTcpThreadData *)pWorkHandle->data;

		BYTE *pRequestBuffer = pXThreadData->m_xRequestBuffer.m_pBuffer;
		size_t szRequestLen = pXThreadData->m_xRequestBuffer.m_szUsedLen;
		BYTE *pRequestBufferClone = pRequestBuffer;
		//size_t szRequestLenClone = szRequestLen;
		try
		{
			//显示收到的内容
#ifndef XRELEASE
			string strRequest = "";
			ByteToString(pRequestBuffer, szRequestLen, strRequest);
			XLogClass::debug("XTcpListener::onProcess TcpPort[%d], RecvData[HEX]:[%s]", pXThreadData->m_pXListener->m_pXListenerInfo->m_iPort, strRequest.c_str());
#endif

			//获取交易代码
			string strTranCode;
			int iResult = GetStringByType(pRequestBuffer, pXThreadData->m_pXListener->m_pXListenerInfo->m_xTranCodeInfo.m_iOffset, pXThreadData->m_pXListener->m_pXListenerInfo->m_xTranCodeInfo.m_iLength, (XConfig::XLengthType)(pXThreadData->m_pXListener->m_pXListenerInfo->m_xTranCodeInfo.m_xType), strTranCode);
			if (iResult != X_SUCCESS)
			{
				string strTempTranCode;
				ByteToString(pRequestBuffer + pXThreadData->m_pXListener->m_pXListenerInfo->m_xTranCodeInfo.m_iOffset, pXThreadData->m_pXListener->m_pXListenerInfo->m_xTranCodeInfo.m_iLength, strTempTranCode);
				XLogClass::warn("XTcpListener::onProcess TranCode Parse Fail: Type=[%d],Content[HEX]=[%s]",
					pXThreadData->m_pXListener->m_pXListenerInfo->m_xTranCodeInfo.m_xType, strTempTranCode.c_str());
				return;
			}

			map<string, XConfig::XTransactionInfo>::iterator itTranInfo = pXThreadData->m_pXListener->m_pXListenerInfo->m_mapXTranList.find(strTranCode);
			//没有配置该交易代码
			if (itTranInfo == pXThreadData->m_pXListener->m_pXListenerInfo->m_mapXTranList.end())
			{
				XLogClass::warn("XTcpListener::onProcess TranCode not in Config: [%s]", strTranCode.c_str());
				return;
			}

			for (list<string>::iterator itServiceName = itTranInfo->second.m_liCallSerivce.begin(); itServiceName != itTranInfo->second.m_liCallSerivce.end(); itServiceName++)
			{
				unsigned char *pResponseBuffer = NULL;
				size_t szResponseLen = 0;
				//调用服务
				iResult = CallServiceMainByName(*itServiceName, pXThreadData, pRequestBuffer, szRequestLen, pResponseBuffer, szResponseLen);
				if (iResult != X_SUCCESS)
				{
					XLogClass::error("XTcpListener::onProcess CallServiceMain Fail [%s]", (*itServiceName).c_str());
					RELEASE(pResponseBuffer);
					return;
				}

				//取返回数据，作为下一个服务的请求数据，多次服务调用的中间数据释放
				if (pRequestBuffer != pRequestBufferClone)
				{
					RELEASE(pRequestBuffer);
				}

				pRequestBuffer = pResponseBuffer;
				szRequestLen = szResponseLen;
			}

			//无需应答的业务直接结束
			if (!itTranInfo->second.m_iNeedResponse)
			{
				if (pRequestBuffer != pRequestBufferClone)
				{
					RELEASE(pRequestBuffer);
				}
				return;
			}

			//至少进行过一次服务处理，有处理结果的，才应答，否则不应答数据
			if (pRequestBuffer != pRequestBufferClone && pRequestBuffer != NULL && szRequestLen != 0)
			{
				//先不复制内存，再设置CopyFlag为1，以在后续异常时，m_xResponseBuffer析构能释放内存
				pXThreadData->m_xResponseBuffer.SetData(pRequestBuffer, szRequestLen, 0);
				pXThreadData->m_xResponseBuffer.SetCopyFlag(1);
#ifndef XRELEASE
				//显示发送的内容
				string strResponse;
				ByteToString(pRequestBuffer, szRequestLen, strResponse);
				XLogClass::debug("XTcpListener::onProcess Tcp Port[%d], SendData[HEX]:[%s]", pXThreadData->m_pXListener->m_pXListenerInfo->m_iPort, strResponse.c_str());
#endif
			}
		}
		catch (exception &ex)
		{
			XLogClass::error("XTcpListener::onProcess Exception:[%s]", ex.what());
		}
	}

	void XTcpListener::afterWork(uv_work_t *pWorkHandle, int iStatus)
	{
		int iResult = X_SUCCESS;
		XTcpThreadData *pXThreadData = (XTcpThreadData *)pWorkHandle->data;
		XTcpConnection *pXConnection = NULL;

		map<int64_t, XSocket *>::iterator it = pXThreadData->m_pXListener->m_mapXConnection.find(pXThreadData->m_i64SessionIdInner);
		if (it == pXThreadData->m_pXListener->m_mapXConnection.end())
			goto AFTERWORK_END;

		pXConnection = (XTcpConnection *)it->second;
		if (pXConnection->m_i64SocketId != pXThreadData->m_i64SocketId)
			goto AFTERWORK_END;

		if (pXThreadData->m_xResponseBuffer.m_pBuffer != NULL
			&& pXThreadData->m_xResponseBuffer.m_szUsedLen > 0)
		{
			XAsyncResult *pXAsyncResult = new XAsyncResult();
			if (pXAsyncResult == NULL)
			{
				XLogClass::error("XTcpListener::afterWork new XAsyncResult Fail");
				goto AFTERWORK_END;
			}

			vector<XBuffer *> vBuffers;
			vBuffers.push_back(&pXThreadData->m_xResponseBuffer);
			iResult = pXConnection->WriteAsync(vBuffers, 15000, 1, pXAsyncResult, onWrite);
			if (iResult != X_SUCCESS)
			{
				XLogClass::error("XTcpListener::afterWork pXTcpConn->WriteAsync Fail");
				delete pXAsyncResult;
				goto AFTERWORK_END;
			}
			//复制标志置0，避免m_xResponseBuffer的析构释放内存
			//这部分内存由WriteAsync的异步自动释放
			pXThreadData->m_xResponseBuffer.SetCopyFlag(0);
		}
	AFTERWORK_END:
		RELEASE(pXThreadData);
		RELEASE(pWorkHandle);
		return;
	}

	XTcpThreadData::XTcpThreadData()
	{
		m_pXListener = NULL;
	}

	XTcpThreadData::~XTcpThreadData()
	{

	}
}
