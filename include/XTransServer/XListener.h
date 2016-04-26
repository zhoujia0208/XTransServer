#pragma once
#include "XConfig.h"
#include "XSocket.h"

namespace XNet {

	class XListener
	{
	public:
		XListener();
		virtual int Initialize(XConfig::XListenerInfo *pXListenerInfo, uv_loop_t *pLoopHandle) = 0;
		~XListener();
		
		virtual void deleteConnection(int64_t i64SessionIdInner) = 0;
	public:
		map<int64_t, XSocket *> m_mapXConnection;
		XConfig::XListenerInfo *m_pXListenerInfo;
		XSocket *m_pXSocket;
		XAsyncResult m_xarListen;

		int64_t mg_i64SessionIdInner;
		int mg_iConnectedCount;
	};

	class XThreadData
	{
	public:
		XThreadData();
		virtual ~XThreadData();

		int m_iStatus;
		int64_t m_i64SessionIdInner;
		int64_t m_i64SocketId;//Socket Id
		uv_thread_t m_hThread;//线程句柄
	};
}
