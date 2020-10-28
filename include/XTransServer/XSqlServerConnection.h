#pragma once

#define OTL_ODBC // CompileOTL 4.0/ODBC
#define OTL_ODBC_SELECT_STM_EXECUTE_BEFORE_DESCRIBE
#define OTL_STL
#define OTL_UTF8 // CompileOTL with Unicode
#define OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS

#ifdef _WIN32 
#define OTL_BIGINT int64_t
#define OTL_UBIGINT uint64_t
#else
#define OTL_ODBC_UNIX
#define OTL_BIGINT long long
#define OTL_UBIGINT unsigned long long
#endif

#include "otlv4.h"
using namespace XData;

class XSqlServerConnection : public XDataBaseConnection
{
public:
	static XDataBaseConnection* CreateInstance()
	{
		return new XSqlServerConnection();
	};
	static XRegistyDataBaseConnectionClass xRegisty;

public:
	XSqlServerConnection();
	~XSqlServerConnection();

	int Initialize();
	int Terminate();
	// 连接Oracle数据库
	int Open(const string & strConnectString);

	// 关闭Oracle数据库
	int Close();

	// 查询指定SQL
	int Query(const string &strSql, XDataRow &xDataRow, const int iArraySize, XDataTable &xDataTable);

	// 执行指定SQL
	int ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount);

	//开始事务
	int BeginTransaction();

	// 提交更改
	int Commit();

	// 回滚更改
	int RollBack();

	// 心跳
	int HeartBeat();

private:
	static int DataTableOTLToX(otl_stream &os, XDataTable &xDataTable);
	static int RowExecuteOtl(otl_stream &os, XDataRow &xDataRow);
	otl_connect db;
};
