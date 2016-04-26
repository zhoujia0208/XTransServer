#include "StdAfx.h"
#include "XHttpListener.h"

namespace XNet {

	XHttpListener::XHttpListener()
	{

	}

	int XHttpListener::Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle)
	{
		int iResult = X_SUCCESS;
		m_pXListenerInfo = pXListenerInfo;
		m_pXSocket = new XSocket();
		if (m_pXSocket == NULL)
		{
			XLogClass::error("XHttpListener::Initialize new XSocket Fail");
			return X_FAILURE;
		}

		iResult = m_pXSocket->Initialize(pLoopHandle, this);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpListener::Initialize m_pXSocket->Initialize Fail");
			return X_FAILURE;
		}

		iResult = m_pXSocket->Bind(pXListenerInfo->m_iPort);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpListener::Initialize m_pXSocket->Bind Fail, Port[%d]", pXListenerInfo->m_iPort);
			return X_FAILURE;
		}

		m_xarListen.m_pAsyncData = this;
		iResult = m_pXSocket->ListenAsync(128, &m_xarListen, onConnection);
		if (iResult != X_SUCCESS)
		{
			XLogClass::error("XHttpListener::Initialize m_pXSocket->ListenAsync Fail, Port[%d]", pXListenerInfo->m_iPort);
			return X_FAILURE;
		}

		XLogClass::info("XHttpListener::Initialize Name[%s] Port[%d] Success", pXListenerInfo->m_strName.c_str(), pXListenerInfo->m_iPort);
		return X_SUCCESS;
	}

	XHttpListener::~XHttpListener()
	{
		//原则上需要释放 m_pXSocket
		//但是由于服务端始终运行，除非程序关闭
		//因此这里偷懒不实现释放资源了
	}

	void XHttpListener::deleteConnection(int64_t i64SessionIdInner)
	{
		map<int64_t, XSocket *>::iterator it = m_mapXConnection.find(i64SessionIdInner);
		if (it == m_mapXConnection.end())
			return;
		XHttpConnection *pXConnection = (XHttpConnection *)it->second;
		m_mapXConnection.erase(it);
		mg_iConnectedCount--;
		delete pXConnection;
		return;
	}

	int XHttpListener::onConnection(XAsyncResult *pXAsyncResult)
	{
		int iResult = X_SUCCESS;
		XHttpListener *pXListener = (XHttpListener *)pXAsyncResult->m_pAsyncData;

		if (pXAsyncResult->m_iStatus != X_SUCCESS)
		{
			XLogClass::error("XHttpListener::onConnection pXAsyncResult->m_iStatus != X_SUCCESS");
			return X_FAILURE;
		}

		//超过配置的最大连接数，则不接受这个链接
		if (pXListener->mg_iConnectedCount >= pXListener->m_pXListenerInfo->m_iMaxConnectCount)
		{
			XLogClass::warn("XHttpListener::onConnection ConnectedCount[%d] >= MaxConnectCount[%d], This connection will not be accepted",
				pXListener->mg_iConnectedCount, pXListener->m_pXListenerInfo->m_iMaxConnectCount);
			return X_FAILURE;
		}

		XHttpConnection *pXConnection = new XHttpConnection();
		if (pXConnection == NULL)
		{
			XLogClass::error("XHttpListener::onConnection new XHttpConnection Fail");
			return X_FAILURE;
		}

		iResult = pXListener->m_pXSocket->Accept(pXConnection);
		if (iResult != 0)
		{
			XLogClass::error("XHttpListener::onConnection pXHttpListener->m_pXSocket->Accept Fail");
			delete pXConnection;
			return X_FAILURE;
		}

		iResult = pXConnection->ReadAsync(pXListener->m_pXListenerInfo->m_iTimeOut, &pXConnection->m_xarRead, onRead);
		if (iResult != 0)
		{
			XLogClass::error("XHttpListener::onConnection pXConnection->ReadAsync Fail");
			delete pXConnection;
			return X_FAILURE;
		}

		pXListener->mg_iConnectedCount++;
		pXConnection->m_i64SessionIdInner = ++pXListener->mg_i64SessionIdInner;
		pXListener->m_mapXConnection.insert(map<int64_t, XHttpConnection *>::value_type(pXConnection->m_i64SessionIdInner, pXConnection));

		return X_SUCCESS;
	}

	int XHttpListener::onRead(XAsyncResult *pXAsyncResult)
	{
		int iResult = X_SUCCESS;
		XHttpConnection *pXConnection = (XHttpConnection *)pXAsyncResult->m_pInnerClass;
		XHttpListener *pXListener = (XHttpListener *)pXConnection->m_pUserData;

		if (pXAsyncResult->m_iStatus != X_SUCCESS)
		{
			if (pXAsyncResult->m_iInnerStatus == UV_EOF)
				XLogClass::debug("XHttpListener::onRead Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);
			else
				XLogClass::warn("XHttpListener::onRead Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);

			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			return X_SUCCESS;
		}

		XHttpThreadData *pXThreadData = new XHttpThreadData();
		if (pXThreadData == NULL)
		{
			XLogClass::error("XHttpListener::onRead new XHttpThreadData Fail");
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
			XLogClass::error("XHttpListener::onRead new uv_work_t Fail");
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

	int XHttpListener::onWrite(XAsyncResult *pXAsyncResult)
	{
		XHttpConnection *pXConnection = (XHttpConnection *)pXAsyncResult->m_pInnerClass;
		if (pXAsyncResult->m_iStatus != X_SUCCESS)
			XLogClass::warn("XHttpListener::onWrite Status[%d] InnerStatus[%s] SocketId[%lld]", pXAsyncResult->m_iStatus, uv_strerror(pXAsyncResult->m_iInnerStatus), pXConnection->m_i64SocketId);
		
		//异步标识用于判断是否释放连接
		if (pXAsyncResult->m_iAsyncFlag)
		{
			XHttpListener *pXListener = (XHttpListener *)pXAsyncResult->m_pAsyncData;
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
		}

		delete pXAsyncResult;
		return X_SUCCESS;
	}

	void XHttpListener::onProcess(uv_work_t *pWorkHandle)
	{
		XHttpThreadData *pXThreadData = (XHttpThreadData *)pWorkHandle->data;
		XHttpConnection *pXConnection = pXThreadData->m_pXConnection;

		if (pXConnection->m_pXHttpContext == NULL)
		{
			pXConnection->m_pXHttpContext = new XHttpContext();
			if (pXConnection->m_pXHttpContext == NULL)
			{
				XLogClass::info("XHttpListener::onProcess new XHttpContext Fail");
				pXThreadData->m_iStatus = X_FAILURE;
				return;
			}
			pXConnection->m_pXHttpContext->m_xHttpType = XHTTP_TYPE_XHTTP;
			pXConnection->m_pXHttpContext->m_pXConnection = pXConnection;
		}

		XHttpContext *pXHttpContext = pXConnection->m_pXHttpContext;
		pXConnection->m_xReceiveBuffer.m_pBuffer[pXConnection->m_xReceiveBuffer.m_szUsedLen] = 0x00;
		pXThreadData->m_iStatus = HttpProcess(pXHttpContext, pXConnection->m_xReceiveBuffer);
		pXConnection->m_xReceiveBuffer.m_szUsedLen = 0;//认为数据全部处理完

		return;
	}

	void XHttpListener::afterWork(uv_work_t *pWorkHandle, int iStatus)
	{
		int iResult = X_SUCCESS;
		XHttpThreadData *pXThreadData = (XHttpThreadData *)pWorkHandle->data;
		XHttpListener *pXListener = pXThreadData->m_pXListener;
		XHttpConnection *pXConnection = pXThreadData->m_pXConnection;

		switch (pXThreadData->m_iStatus)
		{
		case X_ASYNC://异步Http模式
			//新增功能，应在异步业务处理中给出应答，这里什么都不做
			////pXConnection->m_pXHttpContext = NULL;
			////pXConnection->ReadAsync(pXListener->m_pXListenerInfo->m_iTimeOut, &pXConnection->m_xarRead, onRead);
			break;
		case X_SUCCESS://处理成功，需要应答
		{
			//线程处理完毕，再次开始ReadAsync
			pXConnection->ReadAsync(pXListener->m_pXListenerInfo->m_iTimeOut, &pXConnection->m_xarRead, onRead);

			if (pXConnection->m_pXHttpContext == NULL)
				break;

			//判断是否有应答数据需要传输
			//没有则可能是报文未收全
			if (pXConnection->m_pXHttpContext->m_xResponseHead.m_pBuffer == NULL)
				break;

			XAsyncResult *pXAsyncResult = new XAsyncResult();
			if (pXAsyncResult == NULL)
			{
				XLogClass::error("XHttpListener::afterWork new XAsyncResult Fail");
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				break;
			}

			//没有后续HttpBody需要发送，而且不用KeepAlive，则设置发送完毕后关闭此连接
			pXAsyncResult->m_pAsyncData = pXListener;
			pXAsyncResult->m_i64AsyncId = pXConnection->m_i64SessionIdInner;
			if (!pXConnection->m_pXHttpContext->m_bKeepAlive)
				pXAsyncResult->m_iAsyncFlag = 1;
			vector<XBuffer *> vBuffers;
			vBuffers.push_back(&pXConnection->m_pXHttpContext->m_xResponseHead);
			if (pXConnection->m_pXHttpContext->m_xResponseBody.m_pBuffer != NULL)
				vBuffers.push_back(&pXConnection->m_pXHttpContext->m_xResponseBody);
			iResult = pXConnection->WriteAsync(vBuffers, 15000, 1, pXAsyncResult, onWrite);
			if (iResult != X_SUCCESS)
			{
				XLogClass::error("XHttpListener::afterWork pXConnection->WriteAsync Fail");
				delete pXAsyncResult;
				pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
				break;
			}
			//复制标志置0，避免m_xResponseBody的析构释放内存
			//这部分内存由WriteAsync的异步自动释放，下同
			pXConnection->m_pXHttpContext->m_xResponseHead.SetCopyFlag(0);
			if (pXConnection->m_pXHttpContext->m_xResponseBody.m_pBuffer != NULL)
				pXConnection->m_pXHttpContext->m_xResponseBody.SetCopyFlag(0);

			//释放，重置pXConnection->m_pXHttpContext为NULL
			RELEASE(pXConnection->m_pXHttpContext);
			break;
		}
		default://发生错误，删除连接
			XLogClass::error("XHttpListener::afterWork pXThreadData->m_iStatus[%d]", pXThreadData->m_iStatus);
			pXListener->deleteConnection(pXConnection->m_i64SessionIdInner);
			break;
		}

		delete pXThreadData;
		delete pWorkHandle;
		return;
	}

	int HttpProcess(XHttpContext *pXHttpContext, XBuffer &xBuffer)
	{
		int iResult = X_SUCCESS;
		XHttpConnection *pXConnection = (XHttpConnection *)pXHttpContext->m_pXConnection;
		XHttpListener *pXListener = (XHttpListener *)pXConnection->m_pUserData;
		XConfig::XHttpListenerInfo *pXListenerInfo = (XConfig::XHttpListenerInfo *)pXListener->m_pXListenerInfo;

		do {
			if (pXHttpContext->m_xHttpParseStep == XHTTP_PARSE_HEAD_FINISH
				&& pXHttpContext->m_szRecvedContentLength + xBuffer.m_szUsedLen > pXHttpContext->m_szContentLength)
			{
				pXHttpContext->SetStatusCode(413);//Http请求过长
				break;
			}

			//判断Http请求是否完整，时间关系，解析函数写的比较粗糙
			//暂时 如果HttpHead没收全，会返回客户端错误
			if (pXHttpContext->ParseRequest(xBuffer) != X_SUCCESS)
			{
				//Http报文解析出错，认为报文有问题，应答错误
				XLogClass::warn("HttpProcess Port[%d] ParseRequest Fail RecvData[ASCII]:[%s]",
					pXListenerInfo->m_iPort, xBuffer.m_pBuffer);
				break;
			}

			//解析Http请求成功了，数据已记录
			if (pXHttpContext->m_xHttpParseStep != XHTTP_PARSE_FINISH)
			{
				//HTTP报文未收完整，等待继续接收
				return X_SUCCESS;
			}

			//打印Head内容
#ifndef XRELEASE
			XLogClass::debug("HttpProcess Port[%d], RecvHead[ASCII]:[%s]",
				pXListenerInfo->m_iPort, pXHttpContext->m_xRequestHead.m_pBuffer);
#endif
			iResult = _HttpProcess(pXHttpContext);

		} while (0);

		if (iResult != X_SUCCESS)
			return iResult;

		if (pXHttpContext->MakeResponseHead() != X_SUCCESS)
		{
			XLogClass::error("HttpProcess pXHttpContext->MakeResponseHead Fail");
			return X_FAILURE;
		}

#ifndef XRELEASE
		XLogClass::debug("HttpProcess Port[%d], SendHttpHead[ASCII]:[%s]",
			pXListenerInfo->m_iPort, pXHttpContext->m_xResponseHead.m_pBuffer);
#endif

		return X_SUCCESS;
	}

	int _HttpProcess(XHttpContext *pXHttpContext)
	{
		XHttpConnection *pXConnection = (XHttpConnection *)pXHttpContext->m_pXConnection;
		XHttpListener *pXListener = (XHttpListener *)pXConnection->m_pUserData;
		XConfig::XHttpListenerInfo *pXListenerInfo = (XConfig::XHttpListenerInfo *)pXListener->m_pXListenerInfo;

		try
		{
			pXHttpContext->m_strLocalPath = pXListenerInfo->m_strLocalPath;
			//如果Uri为'/',则返回301重定向取默认文件
			if (pXHttpContext->m_strHttpUri == "/" && pXListenerInfo->m_strDefaultPage != "")
			{
				pXHttpContext->Redirect("/" + pXListenerInfo->m_strDefaultPage);
				return X_SUCCESS;
			}

			//先判断交易代码map中是否匹配
			int iIndex = 0;
			iIndex = pXHttpContext->m_strHttpUri.find("../");
			if (iIndex >= 0)
			{
				//非法的访问
				pXHttpContext->SetStatusCode(400);
				return X_SUCCESS;
			}

			iIndex = pXHttpContext->m_strHttpUri.find_last_of('/');
			string strTranCode = pXHttpContext->m_strHttpUri.substr(iIndex + 1);

			//先找配置项
			map<string, XConfig::XTransactionInfo>::iterator itTranInfo = pXListenerInfo->m_mapXTranList.find(strTranCode);
			if (itTranInfo != pXListenerInfo->m_mapXTranList.end())
			{
				//有配置该交易代码，则调用对应服务，Http不支持多级服务迭代调用
				if (itTranInfo->second.m_liCallSerivce.size() != 1)
				{
					pXHttpContext->SetStatusCode(500);
					return X_SUCCESS;
				}

				string strServiceName = *itTranInfo->second.m_liCallSerivce.begin();
				//调用服务
				BYTE *pResponseBuffer = NULL;
				size_t szResponseLen = 0;
				int iResult = CallServiceMainByName(strServiceName, pXHttpContext, NULL, 0, pResponseBuffer, szResponseLen);
				RELEASE(pResponseBuffer);
				switch (iResult)
				{
				case X_ASYNC:
					return X_ASYNC;
				case X_NOTEXIST:
					pXHttpContext->SetStatusCode(404);
				case X_SUCCESS:
					if (pXHttpContext->m_iStatusCode == 0)
						pXHttpContext->SetStatusCode(200);//未设置代码，就设置为200成功
					return X_SUCCESS;
				default:
					XLogClass::error("_HttpProcess CallServiceMainByName Fail [%s]", strServiceName.c_str());
					if (pXHttpContext->m_iStatusCode == 0)
						pXHttpContext->SetStatusCode(500);//未设置错误代码，就设置为500服务器内部错误
					return X_SUCCESS;
				}
			}

			//其他类型，找对应的文件
			string strSuffix;
			iIndex = pXHttpContext->m_strHttpUri.find_last_of('.');
			if (iIndex > 0)
				strSuffix = pXHttpContext->m_strHttpUri.substr(iIndex + 1);
			string strFileName = g_xAppConfigInfo.m_strPath + pXHttpContext->m_strLocalPath + "/" + pXHttpContext->m_strHttpUri;
			int iFd = ::open(strFileName.c_str(), O_RDONLY, 00444);
			if (iFd < 0)
			{
				XLogClass::warn("_HttpProcess No Such File [%s]", strFileName.c_str());
				pXHttpContext->SetStatusCode(404);//返回404
				return X_SUCCESS;
			}

			struct stat st;
			int iResult = ::fstat(iFd, &st);
			if (iResult != 0)
			{
				XLogClass::warn("_HttpProcess fstat Fail [%s]", strFileName.c_str());
				pXHttpContext->SetStatusCode(500);
				return X_SUCCESS;
			}
			string strLastModified;
			TimeTToHttpString(st.st_mtime, strLastModified);//获取文件最后修改时间

			//判断静态文件最后修改时间
			string strIfModifiedSince;
			pXHttpContext->GetRequestHeadInfo("If-Modified-Since", strIfModifiedSince);
			if (strIfModifiedSince != "")
			{
				if (strIfModifiedSince == strLastModified)
				{
					pXHttpContext->SetStatusCode(304);//304 Not Modified
					return X_SUCCESS;
				}
			}

			int iFileSize = ::lseek(iFd, 0, SEEK_END);
			if (iFileSize < 0)
			{
				//文件大小 < 0，可能是目录，报404
				::close(iFd);
				pXHttpContext->SetStatusCode(404);
				return X_SUCCESS;
			}

			BYTE *pFileBuffer = new BYTE[iFileSize];
			if (pFileBuffer == NULL)
			{
				::close(iFd);
				pXHttpContext->SetStatusCode(500);
				return X_SUCCESS;
			}

			::lseek(iFd, 0, SEEK_SET);
			int iReadSize = ::read(iFd, pFileBuffer, iFileSize);
			if (iReadSize != iFileSize)
			{
				::close(iFd);
				delete pFileBuffer;
				pXHttpContext->SetStatusCode(500);
				return X_SUCCESS;
			}
			::close(iFd);
			pXHttpContext->SetResponseBody(pFileBuffer, iFileSize, 0);
			//虽然本次不复制内存，但是标志置位，异常的时候可以在析构中回收内存
			pXHttpContext->m_xResponseBody.SetCopyFlag(1);

			pXHttpContext->SetResponseHeadInfo("Last-Modified", strLastModified);
			pXHttpContext->SetResponseHeadInfo("Content-Type", XHttpContext::GetHttpContentType(strSuffix));
			pXHttpContext->SetStatusCode(200);//200 OK

			return X_SUCCESS;
		}
		catch (exception &ex)
		{
			XLogClass::error("_HttpProcess Exception:[%s]", ex.what());
			pXHttpContext->SetStatusCode(500);//返回500
			return X_SUCCESS;
		}
	}

	XHttpThreadData::XHttpThreadData()
	{
		m_pXListener = NULL;
		m_pXConnection = NULL;
	}

	XHttpThreadData::~XHttpThreadData()
	{

	}
}
