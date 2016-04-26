#pragma once
#ifndef __XSQLSERVERCONNECTION_H__
#define __XSQLSERVERCONNECTION_H__

#define OTL_ODBC_MSSQL_2008
#define OTL_STL
//#define OTL_ANSI_CPP
#define OTL_MAX_CHAR_SIZE 1024

#ifdef _WIN32 
#define OTL_BIGINT int64_t
#define OTL_UBIGINT uint64_t
#endif

//#define OTL_STREAM_POOLING_ON
#include "otlv4.h"

class XSqlServerConnection : public XDataBaseConnection
{
public:
	static XDataBaseConnection* CreateInstance()
	{ 
		return new XSqlServerConnection();
	};
	static XRegistyDataBaseConnectionClass xRegisty;

public:
	XSqlServerConnection(void);
	~XSqlServerConnection(void);

	int Initialize();
	int Terminate();
	// 连接SqlServer数据库
	int Open(string strConnectString);

	// 关闭SqlServer数据库
	int Close();

	// 查询指定SQL
	int Query(string strSql, int iArraySize, XDataTable &xDataTable);

	// 执行指定SQL
	int ExecuteSql(string strSql, int64_t &i64RecordCount);

	//开始事务
	int BeginTransaction();

	// 提交更改
	int Commit();

	// 回滚更改
	int RollBack();

	// 心跳
	int HeartBeat();

private:
	static int ToXDataTable(otl_stream &os, XDataTable &xDataTable);
	otl_connect db;
};

//#undef OTL_ODBC_MSSQL_2008
//#undef OTL_STL
//#undef OTL_ANSI_CPP
////#undef OTL_MAX_CHAR_SIZE

#endif //__XORACLECONNECTION_H__
