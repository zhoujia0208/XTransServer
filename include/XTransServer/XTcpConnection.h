#pragma once
#include "uv.h"
#include "XSocket.h"

namespace XNet {

	class XTcpConnection : public XSocket
	{
	public:
		XTcpConnection(XProtocolType xProtocolType = XProtocol_IPV4);
		~XTcpConnection();

		XAsyncResult m_xarRead;
		int64_t m_i64SessionIdInner;
	};
}
