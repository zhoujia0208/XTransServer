#include "StdAfx.h"
#include "XHttpConnection.h"

namespace XNet {

	XHttpConnection::XHttpConnection(XProtocolType xProtocolType) : XTcpConnection(xProtocolType)
	{
		m_pXHttpContext = NULL;
	}

	XHttpConnection::~XHttpConnection()
	{
		RELEASE(m_pXHttpContext);
	}
}
