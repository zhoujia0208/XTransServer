#pragma once
#include "XSocket.h"
#include "XTcpListener.h"
#include "XHttpConnection.h"

namespace XNet {

	class XHttpListener : public XTcpListener
	{
	public:
		XHttpListener();
		int Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle);
		~XHttpListener();

		void deleteConnection(int64_t i64SessionIdInner);
		static int onConnection(XAsyncResult *pXAsyncResult);
		static int onRead(XAsyncResult *pXAsyncResult);
		static int onWrite(XAsyncResult *pXAsyncResult);
		static void onProcess(uv_work_t *pWorkHandle);
		static void afterWork(uv_work_t *pWorkHandle, int iStatus);
	};

	int HttpProcess(XHttpContext *pXHttpContext, XBuffer &xBuffer);
	int _HttpProcess(XHttpContext *pXHttpContext);

	class XHttpThreadData : public XThreadData
	{
	public:
		XHttpThreadData();
		~XHttpThreadData();

		XHttpListener *m_pXListener;
		XHttpConnection *m_pXConnection;
	};
}
