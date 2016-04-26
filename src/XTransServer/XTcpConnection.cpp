#include "StdAfx.h"
#include "XTcpConnection.h"

namespace XNet {

	XTcpConnection::XTcpConnection(XProtocolType xProtocolType) : XSocket(xProtocolType)
	{
		m_i64SessionIdInner = 0;
	}

	XTcpConnection::~XTcpConnection()
	{
		CancelReadAsync(&m_xarRead);
	}
}
