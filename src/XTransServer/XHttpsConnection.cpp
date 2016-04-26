#include "StdAfx.h"
#include "XHttpsConnection.h"

namespace XNet {

	XHttpsConnection::XHttpsConnection(XProtocolType xProtocolType) : XHttpConnection(xProtocolType)
	{
		m_i64SessionIdInner = 0;
		m_pXHttpContext = NULL;
		m_isSslConnected = false;
		m_pSsl = NULL;
		m_pBioRead = NULL;
		m_pBioWrite = NULL;
	}

	int XHttpsConnection::Initialize(SSL_CTX *pSslCtx)
	{
		m_pSsl = SSL_new(pSslCtx);
		if (m_pSsl == NULL)
		{
			XLogClass::error("XHttpsConnection::Initialize SSL_new Fail");
			return X_FAILURE;
		}

		m_pBioRead = BIO_new(BIO_s_mem());
		if (m_pBioRead == NULL)
		{
			XLogClass::error("XHttpsConnection::Initialize BIO_new m_pBioRead Fail");
			return X_FAILURE;
		}
		m_pBioWrite = BIO_new(BIO_s_mem());
		if (m_pBioWrite == NULL)
		{
			XLogClass::error("XHttpsConnection::Initialize BIO_new m_pBioWrite Fail");
			return X_FAILURE;
		}

		SSL_set_bio(m_pSsl, m_pBioRead, m_pBioWrite);
		SSL_set_accept_state(m_pSsl);

		return X_SUCCESS;
	}

	XHttpsConnection::~XHttpsConnection()
	{
		//if (m_pBioRead)
		//{
		//	BIO_free(m_pBioRead);
		//	m_pBioRead = NULL;
		//}
		//if (m_pBioWrite)
		//{
		//	BIO_free(m_pBioWrite);
		//	m_pBioWrite = NULL;
		//}
		if (m_pSsl)
		{
			SSL_shutdown(m_pSsl);
			SSL_free(m_pSsl);
			m_pSsl = NULL;
		}
	}

	int XHttpsConnection::PrepareWriteData(const vector<XBuffer *> &vBuffers)
	{
		if (m_pBioWrite == NULL)
			return X_FAILURE;
		
		for (size_t i = 0; i < vBuffers.size(); i++)
		{
			int iResult = SSL_write(m_pSsl, vBuffers[i]->m_pBuffer, vBuffers[i]->m_szUsedLen);
			if (iResult <= 0)
			{
				int iError = SSL_get_error(m_pSsl, iResult);
				XLogClass::error("XHttpsConnection::PrepareWriteData SSL_write Fail:[%d] Error[%d]", iResult, iError);
				return X_FAILURE;
			}
		}

		int iWriteLen = BIO_ctrl_pending(m_pBioWrite);
		if (iWriteLen <= 0)
		{
			return X_SUCCESS;//没有数据需要发送
		}

		if(m_xSendBuffer.Malloc(iWriteLen) != X_SUCCESS)
			return X_FAILURE;

		int iRead = BIO_read(m_pBioWrite, m_xSendBuffer.m_pBuffer, iWriteLen);
		if (iRead != iWriteLen)
		{
			XLogClass::error("XHttpsConnection::WriteAsyncSsl BIO_read iRead != iWriteLen");
			m_xSendBuffer.Dispose();
			return X_FAILURE;
		}
		m_xSendBuffer.m_szUsedLen = iWriteLen;

		return X_SUCCESS;
	}
}
