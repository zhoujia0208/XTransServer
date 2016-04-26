#include "StdAfx.h"
#include "XHttpContext.h"

//char * Gen_Head[] =
//{
//	"Cache-Control",            // Section 14.9
//	"Connection",               // Section 14.10
//	"Date",                     // Section 14.19
//	"Pragma",                   // Section 14.32
//	"Transfer-Encoding",        // Section 14.40
//	"Upgrade",                  // Section 14.41
//	"Via"                       // Section 14.44
//};
//
//char * Req_Method[] =
//{
//	"OPTIONS",                  // Section 9.2
//	"GET",                      // Section 9.3
//	"HEAD",                     // Section 9.4
//	"POST",                     // Section 9.5
//	"PUT",                      // Section 9.6
//	"DELETE",                   // Section 9.7
//	"TRACE"                     // Section 9.8
//};
//
//char * Req_Head[] =
//{
//	"Accept",                   // Section 14.1
//	"Accept-Charset",           // Section 14.2
//	"Accept-Encoding",          // Section 14.3
//	"Accept-Language",          // Section 14.4
//	"Authorization",            // Section 14.8
//	"From",                     // Section 14.22
//	"Host",                     // Section 14.23
//	"If-Modified-Since",        // Section 14.24
//	"If-Match",                 // Section 14.25
//	"If-None-Match",            // Section 14.26
//	"If-Range",                 // Section 14.27
//	"If-Unmodified-Since",      // Section 14.28
//	"Max-Forwards",             // Section 14.31
//	"Proxy-Authorization",      // Section 14.34
//	"Range",                    // Section 14.36
//	"Referer",                  // Section 14.37
//	"User-Agent"                // Section 14.42
//};
//
//char * Res_Head[] =
//{
//	"Age",                      // Section 14.6
//	"Location",                 // Section 14.30
//	"Proxy-Authenticate",       // Section 14.33
//	"Public",                   // Section 14.35
//	"Retry-After",              // Section 14.38
//	"Server",                   // Section 14.39
//	"Vary",                     // Section 14.43
//	"Warning",                  // Section 14.45
//	"WWW-Authenticate"          // Section 14.46
//};
//
//char * Res_Entity[] =
//{
//	"Allow",                   // Section 14.7
//	"Content-Base",            // Section 14.11
//	"Content-Encoding",        // Section 14.12
//	"Content-Language",        // Section 14.13
//	"Content-Length",          // Section 14.14
//	"Content-Location",        // Section 14.15
//	"Content-MD5",             // Section 14.16
//	"Content-Range",           // Section 14.17
//	"Content-Type",            // Section 14.18
//	"ETag",                    // Section 14.20
//	"Expires",                 // Section 14.21
//	"Last-Modified"            // Section 14.29
//};

namespace XNet {

	const map<int, string>::value_type mapHttpStatusValues[] =
	{
		map<int, string>::value_type(100, "Continue"),                                 //客户端继续发送剩余内容                            
		map<int, string>::value_type(101, "Switching Protocols"),                      //切换协议
		map<int, string>::value_type(102, "Continue Process"),                         //由WebDAV（RFC 2518）扩展的状态码，代表处理将被继续执行

		map<int, string>::value_type(200, "OK"),                                       //成功
		map<int, string>::value_type(201, "Created"),                                  //成功 新的资源建立(POST)
		map<int, string>::value_type(202, "Accepted"),                                 //请求被接受，但处理未完成
		map<int, string>::value_type(203, "Non-Authoritative Information"),
		map<int, string>::value_type(204, "No Content"),                               //成功，但无信息返回
		map<int, string>::value_type(205, "Reset Content"),
		map<int, string>::value_type(206, "Partial Content"),

		map<int, string>::value_type(300, "Multiple Choices"),
		map<int, string>::value_type(301, "Moved Permanently"),                        //重定向，所请求的资源已经被指派为新的固定URL
		map<int, string>::value_type(302, "Moved Temporarily"),                        //重定向，所请求的资源临时位于另外的URL
		map<int, string>::value_type(303, "See Other"),
		map<int, string>::value_type(304, "Not Modified"),                             //文档没有修改 (条件GET) 
		map<int, string>::value_type(305, "Use Proxy"),

		map<int, string>::value_type(400, "Bad Request"),                              //错误的请求，客户端出错
		map<int, string>::value_type(401, "Unauthorized"),                             //未被授权，该请求要求用户认证
		map<int, string>::value_type(402, "Payment Required"),
		map<int, string>::value_type(403, "Forbidden"),                                //不明原因的禁止
		map<int, string>::value_type(404, "Not Found"),                                //没有找到
		map<int, string>::value_type(405, "Method Not Allowed"),
		map<int, string>::value_type(406, "Not Acceptable"),
		map<int, string>::value_type(407, "Proxy Authentication Required"),
		map<int, string>::value_type(408, "Request Time-out"),
		map<int, string>::value_type(409, "Conflict"),
		map<int, string>::value_type(410, "Gone"),
		map<int, string>::value_type(411, "Length Required"),
		map<int, string>::value_type(412, "Precondition Failed"),
		map<int, string>::value_type(413, "Request Entity Too Large"),
		map<int, string>::value_type(414, "Request-URI Too Large"),
		map<int, string>::value_type(415, "Unsupported Media Type"),

		map<int, string>::value_type(500, "Internal Server Error"),                    //内部服务器差错
		map<int, string>::value_type(501, "Not Implemented"),                          //没有实现
		map<int, string>::value_type(502, "Bad Gateway"),                              //错误的网关
		map<int, string>::value_type(503, "Service Unavailable"),                      //服务暂时失效
		map<int, string>::value_type(504, "Gateway Time-out"),
		map<int, string>::value_type(505, "HTTP Version not supported"),
	};
	size_t szHttpStatus = sizeof(mapHttpStatusValues) / sizeof(map<int, string>::value_type);
	const map<int, string> XHttpContext::m_mapHttpStatus(mapHttpStatusValues, mapHttpStatusValues + szHttpStatus);

	const map<string, XHTTP_METHOD>::value_type mapHttpMethodsValues[] =
	{
		map<string, XHTTP_METHOD>::value_type("DELETE", XHTTP_DELETE),
		map<string, XHTTP_METHOD>::value_type("GET", XHTTP_GET),
		map<string, XHTTP_METHOD>::value_type("HEAD", XHTTP_HEAD),
		map<string, XHTTP_METHOD>::value_type("POST", XHTTP_POST),
		map<string, XHTTP_METHOD>::value_type("PUT", XHTTP_PUT),

		map<string, XHTTP_METHOD>::value_type("CONNECT", XHTTP_CONNECT),
		map<string, XHTTP_METHOD>::value_type("OPTIONS", XHTTP_OPTIONS),
		map<string, XHTTP_METHOD>::value_type("TRACE", XHTTP_TRACE),
		map<string, XHTTP_METHOD>::value_type("COPY", XHTTP_COPY),
		map<string, XHTTP_METHOD>::value_type("LOCK", XHTTP_LOCK),
		map<string, XHTTP_METHOD>::value_type("MKCOL", XHTTP_MKCOL),
		map<string, XHTTP_METHOD>::value_type("MOVE", XHTTP_MOVE),
		map<string, XHTTP_METHOD>::value_type("PROPFIND", XHTTP_PROPFIND),
		map<string, XHTTP_METHOD>::value_type("PROPPATCH", XHTTP_PROPPATCH),
		map<string, XHTTP_METHOD>::value_type("UNLOCK", XHTTP_UNLOCK),

		map<string, XHTTP_METHOD>::value_type("REPORT", XHTTP_REPORT),
		map<string, XHTTP_METHOD>::value_type("MKACTIVITY", XHTTP_MKACTIVITY),
		map<string, XHTTP_METHOD>::value_type("CHECKOUT", XHTTP_CHECKOUT),
		map<string, XHTTP_METHOD>::value_type("MERGE", XHTTP_MERGE),

		map<string, XHTTP_METHOD>::value_type("M-SEARCH", XHTTP_MSEARCH),
		map<string, XHTTP_METHOD>::value_type("NOTIFY", XHTTP_NOTIFY),
		map<string, XHTTP_METHOD>::value_type("SUBSCRIBE", XHTTP_SUBSCRIBE),
		map<string, XHTTP_METHOD>::value_type("UNSUBSCRIBE", XHTTP_UNSUBSCRIBE),

		map<string, XHTTP_METHOD>::value_type("PATCH", XHTTP_PATCH),
	};
	size_t szHttpMethods = sizeof(mapHttpMethodsValues) / sizeof(map<string, XHTTP_METHOD>::value_type);
	const map<string, XHTTP_METHOD> XHttpContext::m_mapHttpMethods(mapHttpMethodsValues, mapHttpMethodsValues + szHttpMethods);

	const map<string, string>::value_type mapHttpContentTypeValues[] =
	{
		map<string, string>::value_type("", "*/*"),
		map<string, string>::value_type("htm", "text/html"),
		map<string, string>::value_type("html", "text/html"),
		map<string, string>::value_type("txt", "text/text"),
		map<string, string>::value_type("css", "text/css"),
		map<string, string>::value_type("js", "text/javascript"),
		map<string, string>::value_type("gif", "image/gif"),
		map<string, string>::value_type("jpg", "image/jpg"),
		map<string, string>::value_type("png", "image/png"),
		map<string, string>::value_type("bmp", "image/bmp"),
	};
	size_t szHttpContentType = sizeof(mapHttpContentTypeValues) / sizeof(map<string, string>::value_type);
	const map<string, string> XHttpContext::m_mapHttpContentType(mapHttpContentTypeValues, mapHttpContentTypeValues + szHttpContentType);


	XHttpContext::XHttpContext()
	{
		m_xHttpParseStep = XHTTP_PARSE_BEGIN;
		m_szContentLength = 0;
		m_szRecvedContentLength = 0;
		m_bKeepAlive = false;
		m_iStatusCode = 0;
	}

	XHttpContext::~XHttpContext()
	{
	}

	int XHttpContext::GetHttpMethod(string strHttpMethod, XHTTP_METHOD &xHttpMethod)
	{
		map<string, XHTTP_METHOD>::const_iterator it = m_mapHttpMethods.find(strHttpMethod);
		if (it == m_mapHttpMethods.end())
		{
			return X_FAILURE;
		}

		xHttpMethod = it->second;
		return X_SUCCESS;
	}

	string XHttpContext::GetHttpContentType(string strSuffix)
	{
		map<string, string>::const_iterator it = m_mapHttpContentType.find(strSuffix);
		if (it == m_mapHttpContentType.end())
		{
			return "*/*";
		}

		return it->second;
	}

	int XHttpContext::SetStatusCode(int iStatusCode)
	{
		m_iStatusCode = iStatusCode;
		map<int, string>::const_iterator it = m_mapHttpStatus.find(iStatusCode);
		if (it == m_mapHttpStatus.end())
		{
			return X_FAILURE;
		}

		m_strStatusDesc = it->second;
		return X_SUCCESS;
	}

	int XHttpContext::ParseRequest(const XBuffer &xBuffer)
	{
		const string &strHttpRequest = string((char *)xBuffer.m_pBuffer, xBuffer.m_szUsedLen);
		int iParsedLen = 0;
		int iIndex = 0;
		if (m_xHttpParseStep == XHTTP_PARSE_BEGIN)
		{
			iIndex = strHttpRequest.find("\r\n", iParsedLen);
			if (iIndex < 0)
			{
				//没有找到一行
				SetStatusCode(400);
				return X_FAILURE;
			}

			iParsedLen = iIndex + 2;
			//解析第一行 RequestLine 开始
			string strRequestLine = strHttpRequest.substr(0, iIndex);
			list<string> liRequestStr = split(strRequestLine, " ", true);
			if (liRequestStr.size() != 3)
			{
				//HttpRequest 的请求行。应该有3段空格分隔的内容。
				SetStatusCode(400);
				return X_FAILURE;
			}

			//判断HttpMethod
			list<string>::iterator it = liRequestStr.begin();
			string &strHttpMethod = *it;
			int iResult = GetHttpMethod(strHttpMethod, m_xHttpMethod);
			if (iResult != X_SUCCESS)
			{
				SetStatusCode(400);//未知的HttpMethod
				return X_FAILURE;
			}

			//暂时只支持POST和GET
			if (m_xHttpMethod != XHTTP_GET && m_xHttpMethod != XHTTP_POST)
			{
				SetStatusCode(501);
				return X_FAILURE;
			}

			//解析HttpUri和QueryString
			m_strWholeHttpUri = *(++it);
			iIndex = m_strWholeHttpUri.find('?', 0);
			if (iIndex < 0)
			{
				m_strHttpUri = m_strWholeHttpUri;
			}
			else
			{
				m_strHttpUri = m_strWholeHttpUri.substr(0, iIndex);
				//解析QueryString
				string s = m_strWholeHttpUri.substr(iIndex + 1);
				iResult = HttpParseQueryString(s, m_mapQueryString);
				if (iResult != X_SUCCESS)
				{
					SetStatusCode(400);
					return X_FAILURE;
				}
			}

			//解析Http版本
			it++;
			string &strHttpVersion = *it;
			iIndex = strHttpVersion.find("HTTP/", 0);
			if (iIndex < 0)//不是 HTTP/ 标识
			{
				SetStatusCode(400);
				return X_FAILURE;
			}

			iIndex = strHttpVersion.find(".", 0);
			if (iIndex < 0)
			{
				SetStatusCode(400);
				return X_FAILURE;
			}

			string strHttpVersionMajor = strHttpVersion.substr(5, iIndex - 5);//"HTTP/"长度为5
			string strHttpVersionMinor = strHttpVersion.substr(iIndex + 1);
			if (!isAllNumberInString(strHttpVersionMajor) || !isAllNumberInString(strHttpVersionMinor))
			{
				SetStatusCode(400);
				return X_FAILURE;
			}

			m_xHttpVersion.ui16Major = convert<uint16_t>(strHttpVersionMajor);
			m_xHttpVersion.ui16Minor = convert<uint16_t>(strHttpVersionMinor);
			//解析第一行 RequestLine 结束

			//解析接下来的 HttpHead 信息
			while (true)
			{
				iIndex = strHttpRequest.find("\r\n", iParsedLen);
				if (iIndex < 0)//没有找到下一行
				{
					SetStatusCode(400);
					return X_FAILURE;
				}

				if (iIndex == iParsedLen)//连续2次的"\r\n",表示 HttpHead 结束
				{
					iParsedLen += 2;
					break;
				}

				string strHeadLine = strHttpRequest.substr(iParsedLen, iIndex - iParsedLen);
				int iIndexTemp = strHeadLine.find(':', 0);
				if (iIndexTemp < 0)//HttpHead 应该是 Key:Value 形式的
				{
					SetStatusCode(400);
					return X_FAILURE;
				}
				string strKey = strHeadLine.substr(0, iIndexTemp);
				string strValue = strHeadLine.substr(iIndexTemp + 1);
				trim(strValue);
				m_mapRequestHead[toLower(strKey)] = strValue;

				iParsedLen = iIndex + 2;
			}

			if (m_xRequestHead.Malloc(iParsedLen + 1) != X_SUCCESS)
			{
				SetStatusCode(500);
				return X_FAILURE;
			}

			m_xRequestHead.CopyData((BYTE *)strHttpRequest.c_str(), iParsedLen);
			m_xRequestHead.m_pBuffer[iParsedLen] = 0x00;

			string strContentLength = m_mapRequestHead["content-length"];
			if (strContentLength != "")
			{
				trim(strContentLength);
				if (!isAllNumberInString(strContentLength))
				{
					SetStatusCode(400);
					return X_FAILURE;
				}
				m_szContentLength = convert<size_t>(strContentLength);
				if (m_szContentLength > MAX_HTTP_CONTENT_LENGTH)
				{
					SetStatusCode(413);
					return X_FAILURE;
				}

				iResult = m_xRequestBody.Malloc(m_szContentLength);//预分配内存
				if (iResult != X_SUCCESS)
				{
					SetStatusCode(500);
					return X_FAILURE;
				}
			}
			m_xHttpParseStep = XHTTP_PARSE_HEAD_FINISH;
		}

		if (m_xHttpParseStep == XHTTP_PARSE_HEAD_FINISH)
		{
			size_t szLeftLength = m_szContentLength - m_szRecvedContentLength;
			size_t szRecvedContentLength = strHttpRequest.size() - iParsedLen;
			//如果接收到的长度超过了标识的内容长度，则忽略多余的内容
			size_t szTempLength = szLeftLength < szRecvedContentLength ? szLeftLength : szRecvedContentLength;
			int iResult = m_xRequestBody.CopyData((BYTE *)strHttpRequest.c_str() + iParsedLen, szTempLength);
			if (iResult != X_SUCCESS)
			{
				SetStatusCode(500);
				return X_FAILURE;
			}

			m_szRecvedContentLength += szTempLength;
			if (m_szContentLength > m_szRecvedContentLength)
			{
				//等待继续接收
				return X_SUCCESS;
			}

			m_xHttpParseStep = XHTTP_PARSE_FINISH;
		}

		return X_SUCCESS;
	}

	int XHttpContext::GetRequestHeadInfo(string strKey, string &strValue)
	{
		toLower(strKey);
		strValue = m_mapRequestHead[strKey];
		
		return X_SUCCESS;
	}

	string XHttpContext::GetRequestBodyAsString()
	{
		return string((char *)m_xRequestBody.m_pBuffer, m_xRequestBody.m_szUsedLen);
	}

	int XHttpContext::SetResponseHeadInfo(string strKey, string strValue)
	{
		m_mapResponseHead[strKey] = strValue;

		return X_SUCCESS;
	}

	int XHttpContext::SetResponseBody(BYTE *pBodyData, size_t szBodyLen, int iCopyFlag)
	{
		return m_xResponseBody.SetData(pBodyData, szBodyLen, iCopyFlag);
	}

	int XHttpContext::MakeResponseHead()
	{
		string strResponseHead;
		//SetResponseHeadInfo("Connection", "Keep-Alive");
		string strHttpTime;
		TimeTToHttpString(time(NULL), strHttpTime);
		SetResponseHeadInfo("Date", strHttpTime);
		SetResponseHeadInfo("Server", "XServer");
		SetResponseHeadInfo("Content-Length", convert<string>(m_xResponseBody.m_szUsedLen));

		strResponseHead = "HTTP/1.1 ";// +convert<string>(m_xHttpVersion.ui16Major) + "." + convert<string>(m_xHttpVersion.ui16Minor) + " ";
		strResponseHead += convert<string>(m_iStatusCode) + " ";
		strResponseHead += m_strStatusDesc + "\r\n";
		for (map<string, string>::const_iterator it = m_mapResponseHead.begin(); it != m_mapResponseHead.end(); it++)
		{
			strResponseHead += it->first + ": " + it->second + "\r\n";
		}
		strResponseHead += "\r\n";

		if (m_xResponseHead.Malloc(strResponseHead.size() + 1) != X_SUCCESS)
			return X_FAILURE;

		if (m_xResponseHead.CopyData((BYTE *)strResponseHead.c_str(), strResponseHead.size()) != X_SUCCESS)
			return X_FAILURE;

		m_xResponseHead.m_pBuffer[strResponseHead.size()] = 0x00;
		return X_SUCCESS;
	}

	int XHttpContext::Redirect(string strNewUri)
	{
		SetStatusCode(302);
		SetResponseHeadInfo("Location", strNewUri);
		return X_SUCCESS;
	}
}
