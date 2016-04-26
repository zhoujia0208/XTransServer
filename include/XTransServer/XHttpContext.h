#pragma once
#include "XSocket.h"

namespace XNet {

#define MAX_HTTP_CONTENT_LENGTH  1024 * 64 //防止恶意攻击，大于64K的内容目前先拒收
#define MAX_HTTP_HEAD_LENGTH     1024 * 32 //防止恶意攻击，大于32K的Http头拒收
	enum XHTTP_TYPE
	{
		XHTTP_TYPE_XHTTP = 0,
		XHTTP_TYPE_XHTTPS,
	};

	enum XHTTP_PARSE_STEP
	{
		XHTTP_PARSE_BEGIN = 0,
		XHTTP_PARSE_HEAD_FINISH,
		XHTTP_PARSE_FINISH,
	};

	enum XHTTP_METHOD
	{
		XHTTP_DELETE = 0,
		XHTTP_GET,
		XHTTP_HEAD,
		XHTTP_POST,
		XHTTP_PUT,
		/* pathological */
		XHTTP_CONNECT,
		XHTTP_OPTIONS,
		XHTTP_TRACE,
		/* webdav */
		XHTTP_COPY,
		XHTTP_LOCK,
		XHTTP_MKCOL,
		XHTTP_MOVE,
		XHTTP_PROPFIND,
		XHTTP_PROPPATCH,
		XHTTP_UNLOCK,
		/* subversion */
		XHTTP_REPORT,
		XHTTP_MKACTIVITY,
		XHTTP_CHECKOUT,
		XHTTP_MERGE,
		/* upnp */
		XHTTP_MSEARCH,
		XHTTP_NOTIFY,
		XHTTP_SUBSCRIBE,
		XHTTP_UNSUBSCRIBE,
		/* RFC-5789 */
		XHTTP_PATCH,
	};



	class XHttpVersion
	{
	public:
		XHttpVersion()
		{
			ui16Major = 1;
			ui16Minor = 0;
		}
		uint16_t ui16Major;
		uint16_t ui16Minor;
	};

	class XHttpContext
	{
	public:
		XHttpContext();
		~XHttpContext();

		static string GetHttpContentType(string strSuffix);
		//解析HttpRequest
		int ParseRequest(const XBuffer &xBuffer);
		
		int GetRequestHeadInfo(string strKey, string &strValue);
		string GetRequestBodyAsString();

		int SetResponseHeadInfo(string strKey, string strValue);
		int SetResponseBody(BYTE *pBodyData, size_t szBodyLen, int iCopyFlag = 1);
		int SetStatusCode(int iStatusCode);
		int Redirect(string strNewUri);
		int MakeResponseHead();

		void *m_pXConnection;
		XHTTP_TYPE m_xHttpType;
		XHTTP_METHOD m_xHttpMethod;
		string m_strLocalPath;
		string m_strWholeHttpUri;
		string m_strHttpUri;
		XHttpVersion m_xHttpVersion;
		map<string, string> m_mapQueryString;

		XBuffer m_xRequestHead;
		XBuffer m_xRequestBody;

		XBuffer m_xResponseHead;
		XBuffer m_xResponseBody;

		XHTTP_PARSE_STEP m_xHttpParseStep;
		map<string, string> m_mapRequestHead;
		map<string, string> m_mapResponseHead;

		size_t m_szContentLength;
		size_t m_szRecvedContentLength;
		bool m_bKeepAlive;
		int m_iStatusCode;
		string m_strStatusDesc;

	protected:
		const static map<int, string> m_mapHttpStatus;
		const static map<string, XHTTP_METHOD> m_mapHttpMethods;
		const static map<string, string> m_mapHttpContentType;

		static int GetHttpMethod(string strHttpMethod, XHTTP_METHOD &xHttpMethod);
	};
}
