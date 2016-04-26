#pragma once
#ifndef __XDATABASECONNECTION_H__
#define __XDATABASECONNECTION_H__

#include "XDataTable.h"
namespace XData {

	class XDataBaseConnection
	{
	public:
		virtual int Initialize()
		{
			return X_SUCCESS;
		};

		virtual int Terminate()
		{
			return X_SUCCESS;
		};

	public:
		XDataBaseConnection();
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
		// vc++ 14 or higher
		virtual ~XDataBaseConnection() noexcept(false)
#else
		virtual ~XDataBaseConnection()
#endif
		{
		};

		// 连接数据库
		virtual int Open(const string &strConnectString)
		{
			XLogClass::error("XDataBaseConnection::Open 未实现");
			return X_FAILURE;
		};

		// 关闭连接
		virtual int Close()
		{
			XLogClass::error("XDataBaseConnection::Close 未实现");
			return X_FAILURE;
		};

		//// 查询指定SQL
		//virtual int Query(const string &strSql, const int iArraySize, XDataTable &xDataTable)
		//{
		//	XLogClass::error("XDataBaseConnection::Query 未实现");
		//	return X_FAILURE;
		//};

		virtual int Query(const string &strSql, XDataRow &xDataRow, const int iArraySize, XDataTable &xDataTable)
		{
			XLogClass::error("XDataBaseConnection::Query 未实现");
			return X_FAILURE;
		};

		//// 执行指定SQL
		//virtual int ExecuteSql(const string &strSql, int64_t &i64RecordCount)
		//{
		//	XLogClass::error("XDataBaseConnection::ExecuteSql 未实现");
		//	return X_FAILURE;
		//};

		virtual int ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount)
		{
			XLogClass::error("XDataBaseConnection::ExecuteSql 未实现");
			return X_FAILURE;
		};

		//开始事务
		virtual int BeginTransaction()
		{
			XLogClass::error("XDataBaseConnection::BeginTransaction 未实现");
			return X_FAILURE;
		};

		// 提交更改
		virtual int Commit()
		{
			XLogClass::error("XDataBaseConnection::Commit 未实现");
			return X_FAILURE;
		};

		// 回滚更改
		virtual int RollBack()
		{
			XLogClass::error("XDataBaseConnection::RollBack 未实现");
			return X_FAILURE;
		};

		// 心跳测试
		virtual int HeartBeat()
		{
			XLogClass::error("XDataBaseConnection::HeartBeat 未实现");
			return X_FAILURE;
		};

		friend class XDataBase;

	private:
		bool m_isUsed;
		uv_thread_t m_hThread;
		uint64_t m_ui64LastUseTime;
	protected:
		string m_strConnectString;
	};

	typedef XDataBaseConnection* (*FuncCreateDataBaseConnection)(void);//回调函数指针

	class XDataBaseConnectionFactory
	{
	public:
		static int CreateDataBaseConnectionByName(string strDataBaseConnectionName, XDataBaseConnection* &pXDataBaseConnection);

		void RegistDataBaseConnection(string strDataBaseConnectionName, FuncCreateDataBaseConnection pFunc);

		static XDataBaseConnectionFactory& sharedDataBaseConnectionFactory();
	private:
		map<string, FuncCreateDataBaseConnection> m_classMap;
	};

	class XRegistyDataBaseConnectionClass
	{
	public:
		XRegistyDataBaseConnectionClass(string strDataBaseConnectionName, FuncCreateDataBaseConnection pFuncCreate)
		{
			XDataBaseConnectionFactory::sharedDataBaseConnectionFactory().RegistDataBaseConnection(strDataBaseConnectionName, pFuncCreate);
		}
	};
}
#endif //__XDATABASECONNECTION_H__
