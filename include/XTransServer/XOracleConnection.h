#pragma once
#ifndef __XORACLECONNECTION_H__
#define __XORACLECONNECTION_H__

#define OTL_ORA11G_R2
#define OTL_ORA_UTF8
#define OTL_ORA_TIMESTAMP
#define OTL_STL
#define OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS

#ifdef _WIN32 
#define OTL_BIGINT int64_t
#define OTL_UBIGINT uint64_t
#else
#define OTL_BIGINT long long
#define OTL_UBIGINT unsigned long long
#endif

//#define OTL_STREAM_POOLING_ON
#include "otlv4.h"
using namespace XData;

class XOracleConnection : public XDataBaseConnection
{
public:
	static XDataBaseConnection* CreateInstance()
	{ 
		return new XOracleConnection();
	};
	static XRegistyDataBaseConnectionClass xRegisty;

public:
	XOracleConnection();
	~XOracleConnection();

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

#endif //__XORACLECONNECTION_H__
