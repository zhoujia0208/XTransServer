#pragma once
#include "XSocket.h"
#include "XListener.h"
#include "XTcpConnection.h"

namespace XNet {

	class XTcpListener : public XListener
	{
	public:
		XTcpListener();
		int Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle);
		~XTcpListener();

		void deleteConnection(int64_t i64SessionIdInner);
		static int onConnection(XAsyncResult *pXAsyncResult);
		static int onRead(XAsyncResult *pXAsyncResult);
		static int onWrite(XAsyncResult *pXAsyncResult);
		static void onProcess(uv_work_t *pWorkHandle);
		static void afterWork(uv_work_t *pWorkHandle, int iStatus);
	};

	class XTcpThreadData : public XThreadData
	{
	public:
		XTcpThreadData();
		~XTcpThreadData();

		XTcpListener *m_pXListener;
		XBuffer m_xRequestBuffer;
		XBuffer m_xResponseBuffer;
	};
}
