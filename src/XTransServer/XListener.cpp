#include "StdAfx.h"
#include "XListener.h"

namespace XNet {

	XListener::XListener()
	{
		m_pXListenerInfo = NULL;
		m_pXSocket = NULL;
		mg_i64SessionIdInner = 0;
		mg_iConnectedCount = 0;
	}

	XListener::~XListener()
	{
		RELEASE(m_pXSocket);
	}

	XThreadData::XThreadData()
	{
		m_iStatus = X_SUCCESS;
		m_i64SessionIdInner = 0;
		m_i64SocketId = -1;
		m_hThread = (uv_thread_t)-1;
	}

	XThreadData::~XThreadData()
	{

	}
}
