#pragma once
#ifndef __XDATABASE_H__
#define __XDATABASE_H__

#include "XDataBaseConnection.h"
#include "XDataTable.h"

namespace XData {

	class XDataBase;

	class XDbTransaction
	{
	public:

		XDbTransaction();
		~XDbTransaction();
		
		int Query(const string &strSql, XDataTable &xDataTable, const int iArraySize = 1);
		int Query(const string &strSql, XDataRow &xDr, XDataTable &xDataTable, const int iArraySize = 1);
		int ExecuteSql(const string &strSql);
		int ExecuteSql(const string &strSql, int64_t &i64RecordCount);
		int ExecuteSql(const string &strSql, XDataTable &xDataTable);
		int ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount);

		int Commit();
		int RollBack();

		friend class XDataBase;
	private:
		void _Dispose();
		XDataBase *m_pXDb;
		XDataBaseConnection *m_pXDbConn;
	};

	class XDataBase
	{
	public:
		XDataBase();
		~XDataBase();

		int Initialize(string strDbType, string strDbName, string strConnectString, int iConnectCount, int iHeartbeatInterval);

		int Open(string strConnectString);
		int Close();

		int Query(const string &strSql, XDataTable &xDataTable, const int iArraySize = 1);
		int Query(const string &strSql, XDataRow &xDr, XDataTable &xDataTable, const int iArraySize = 1);
		int Query(const XDbTransaction &xDbTrans, const string &strSql, XDataTable &xDataTable, const int iArraySize = 1);
		int Query(const XDbTransaction &xDbTrans, const string &strSql, XDataRow &xDr, XDataTable &xDataTable, const int iArraySize = 1);

		int ExecuteSql(const string &strSql);
		int ExecuteSql(const string &strSql, int64_t &i64RecordCount);
		int ExecuteSql(const string &strSql, XDataTable &xDataTable);
		int ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount);

		int ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql);
		int ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql, int64_t &i64RecordCount);
		int ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql, XDataTable &xDataTable);
		int ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount);

		int BeginTransaction(XDbTransaction &xDbTrans);
		int Commit(XDbTransaction &xDbTrans);
		int RollBack(XDbTransaction &xDbTrans);

	private:
		string m_strDbType;
		string m_strDbName;
		string m_strConnectString;

		int m_iConnectCount;
		int m_iHeartbeatInterval;
		uv_thread_t hHeartbeatHandle;
		bool isHeartbeat;
		uv_mutex_t mutex;

		static void StartHeartBeat(void *pVoid);

		vector<XDataBaseConnection *> m_vxDbConns;

		int _GetFreeConnect(XDbTransaction &xDbTrans);
		int _ReleaseConnect(XDataBaseConnection *pXDbConn);
		int _Commit(XDataBaseConnection *pXDbConn);
		int _RollBack(XDataBaseConnection *pXDbConn);
		int _HeartBeat();
	};
}
#endif //__XDATABASE_H__
