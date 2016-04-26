#pragma once
#include "XHttpConnection.h"
#include "XHttpContext.h"
#include <openssl/bio.h>
#include <openssl/ssl.h>  
#include <openssl/err.h>

namespace XNet {

	class XHttpsConnection : public XHttpConnection
	{
	public:
		XHttpsConnection(XProtocolType xProtocolType = XProtocol_IPV4);
		int Initialize(SSL_CTX *pSslCtx);
		~XHttpsConnection();

		int PrepareWriteData(const vector<XBuffer *> &vBuffers);

		SSL *m_pSsl;
		BIO *m_pBioRead;
		BIO *m_pBioWrite;
		bool m_isSslConnected;
	};
}
