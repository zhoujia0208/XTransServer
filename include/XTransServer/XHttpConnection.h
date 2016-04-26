#pragma once
#include "XTcpConnection.h"
#include "XHttpContext.h"

namespace XNet {

	class XHttpConnection : public XTcpConnection
	{
	public:
		XHttpConnection(XProtocolType xProtocolType = XProtocol_IPV4);
		~XHttpConnection();

		XHttpContext *m_pXHttpContext;
	};
}
