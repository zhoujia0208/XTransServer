#pragma once
#include "XSocket.h"
#include "XHttpListener.h"
#include "XHttpsConnection.h"

namespace XNet {

	class XHttpsListener : public XHttpListener
	{
	public:
		XHttpsListener();
		int Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle);
		~XHttpsListener();

		void deleteConnection(int64_t i64SessionIdInner);
		static int onConnection(XAsyncResult *pXAsyncResult);
		static int onRead(XAsyncResult *pXAsyncResult);
		static int onWrite(XAsyncResult *pXAsyncResult);
		static void onProcess(uv_work_t *pWorkHandle);
		static void afterWork(uv_work_t *pWorkHandle, int iStatus);

	private:
		SSL_CTX *m_pSslCtx;
	};

	int HttpsProcess(XHttpsConnection *pXConnection, XBuffer &xBuffer);

	class XHttpsThreadData : public XThreadData
	{
	public:
		XHttpsThreadData();
		~XHttpsThreadData();

		XHttpsListener *m_pXListener;
		XHttpsConnection *m_pXConnection;
	};
}
