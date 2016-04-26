#pragma once
#include "uv.h"

#ifndef _WIN32
typedef int SOCKET;
#endif

namespace XNet {

#define DEFAULT_BUFFER_SIZE 64 * 1024

	//暂时只支持IPV4
	enum XProtocolType
	{
		XProtocol_IPV4 = 1,
		XProtocol_IPV6 = 2,
	};

	class XAsyncResult;
	typedef int(*XAsyncCallback)(XAsyncResult *pXAsyncResult);

	class XAsyncResult
	{
	public:
		XAsyncResult();
		~XAsyncResult();
		
		bool m_isCompleted;//执行是否完成
		int m_iStatus;//执行结果
		int m_iInnerStatus;
		void *m_pAsyncData;//异步标识数据
		int64_t m_i64AsyncId;
		int m_iAsyncFlag;
		void *m_pInnerClass;//内部调用的类指针

		friend class XSocket;
		//friend class XTcpListener;
		//friend class XHttpListener;
		//friend class XHttpsListener;
	private:
		static void onClose(uv_handle_t *pHandle);

		void *m_pRequestHandle;//请求数据
		int m_iTimeOut;
		void *m_pTimerHandle;//超时句柄
		XAsyncCallback m_xAsyncCallback;//异步回调
	};

	class XBuffer
	{
	public:
		XBuffer();
		XBuffer(BYTE *pBuffer, size_t szTotalLen, size_t szUsedLen);
		~XBuffer();
		void Dispose();

		int Malloc(size_t szTotalLen);
		int CopyData(BYTE *pBuffer, size_t szLen);
		int SetData(BYTE *pBuffer, size_t szLen, int iCopyFlag);
		void SetCopyFlag(int iCopyFlag);

		BYTE *m_pBuffer;
		size_t m_szTotalLen;
		size_t m_szUsedLen;
	private:
		void _Dispose();
		int m_iCopyFlag;
	};

	class XAsyncWriteRequest
	{
	public:
		XAsyncWriteRequest();
		~XAsyncWriteRequest();

		uv_write_t uvRequest;
		uv_buf_t *pBuffers;
		uv_buf_t uvBuffers[4];
		int iFreeFlag;
		size_t szBuffer;
	};

	//异步Socket类
	class XSocket
	{
	public:
		XSocket( XProtocolType xProtocolType = XProtocol_IPV4);
		int Initialize(uv_loop_t *pLoopHandle, void *pUserData);
		virtual ~XSocket();
		
		
		int Available();
		bool IsBlocking();
		bool IsConnected();

		//新版libuv不支持取消连接操作，超时没有意义
		int ConnectAsync(string strRemoteIP, int iRemotePort, int iTimeOut, XAsyncResult *pXAsyncResult, XAsyncCallback onConnectCallback);
		void CancelConnectAsync(XAsyncResult *pXAsyncResult);

		//CloseSync();
		int CloseAsync(XAsyncResult *pXAsyncResult, XAsyncCallback onCloseCallBack);
		void CancelCloseAsync(XAsyncResult *pXAsyncResult);

		//ReadSync();
		int ReadAsync(int iTimeOut, XAsyncResult *pXAsyncResult, XAsyncCallback onReadCallBack);
		void CancelReadAsync(XAsyncResult *pXAsyncResult);

		//新版libuv不支持取消写操作，超时暂时没用
		int WriteAsync(vector<XBuffer *> &vBuffers, int iTimeOut, int iFreeFlag, XAsyncResult *pXAsyncResult, XAsyncCallback onWriteCallBack);
		void CancelWriteAsync(XAsyncResult *pXAsyncResult);

		int Accept(XSocket *pXSocket);

		int Bind(int iPort);
		int ListenAsync(int iBackLog, XAsyncResult *pXAsyncResult, XAsyncCallback onConnectionCallBack);

		void *m_pUserData;//调用方可挂载的自定义数据，需要调用方释放
		XBuffer m_xReceiveBuffer;
		XBuffer m_xSendBuffer;
		int64_t m_i64SocketId;

		//friend class XTcpListener;
		//friend class XHttpListener;
		//friend class XHttpsListener;
	private:
		void _Dispose();
		
		XProtocolType m_xProtocolType;
		int m_iReceiveBufferSize;
		int m_iReceiveTimeout;
		int m_iSendBufferSize;
		int m_iSendTimeout;

		uv_loop_t *m_pLoopHandle;
		uv_tcp_t *m_pTcpHandle;

		//用于异步关闭调用外部函数
		//XAsyncResult m_xCloseAsyncResult;
		//XAsyncCallback m_xCloseCallback;

		//uv_timer_t *m_pTimerConnect;
		//int m_iConnectResult;
		//XAsyncCallback m_xAsyncOnClose;
		//XEndPoint LocalEndPoint;
		//XEndPoint RemoteEndPoint;
		//static void onConnectSync(uv_connect_t *pConnHandle, int iStatus);
		//static void onConnectSyncTimeOut(uv_timer_t *pTimerHandle);
		inline SOCKET _GetSocketId();

		static void onConnectAsync(uv_connect_t *pConnHandle, int iStatus);
		static void onConnectAsyncTimeOut(uv_timer_t *pTimerHandle);

		static void onCloseAsync(uv_handle_t *pHandle);

		static void onAlloc(uv_handle_t *pHandle, size_t szSuggested, uv_buf_t *pBuffer);
		static void onReadAsync(uv_stream_t *pStreamHandle, ssize_t szRead, const uv_buf_t *pBuffer);
		static void onReadAsyncTimeOut(uv_timer_t *pTimerHandle);

		static void onWriteAsync(uv_write_t *pWriteHandle, int iStatus);
		static void onWriteAsyncTimeOut(uv_timer_t *pTimerHandle);
		
		static void onConnectionAsync(uv_stream_t *pStreamHandle, int iStatus);
	};
}
