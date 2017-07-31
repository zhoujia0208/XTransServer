#include "StdAfx.h"
#include "XDataBase.h"

namespace XData {

	XDbTransaction::XDbTransaction()
	{
		_Dispose();
	}

	XDbTransaction::~XDbTransaction()
	{
		if (m_pXDb != NULL)
			m_pXDb->RollBack(*this);
	}

	void XDbTransaction::_Dispose()
	{
		m_pXDb = NULL;
		m_pXDbConn = NULL;
	}

	int XDbTransaction::Query(const string &strSql, XDataTable &xDataTable, const int iArraySize)
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->Query(*this, strSql, xDataTable, iArraySize);
	}

	int XDbTransaction::Query(const string &strSql, XDataRow &xDr, XDataTable &xDataTable, const int iArraySize)
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->Query(*this, strSql, xDr, xDataTable, iArraySize);
	}

	int XDbTransaction::ExecuteSql(const string &strSql)
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->ExecuteSql(*this, strSql);
	}

	int XDbTransaction::ExecuteSql(const string &strSql, int64_t &i64RecordCount)
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->ExecuteSql(*this, strSql, i64RecordCount);
	}

	int XDbTransaction::ExecuteSql(const string &strSql, XDataTable &xDataTable)
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->ExecuteSql(*this, strSql, xDataTable);
	}

	int XDbTransaction::ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount)
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->ExecuteSql(*this, strSql, xDataTable, i64RecordCount);
	}

	int XDbTransaction::Commit()
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->Commit(*this);
	}

	int XDbTransaction::RollBack()
	{
		return m_pXDb == NULL ? X_FAILURE : m_pXDb->RollBack(*this);
	}

	XDataBase::XDataBase()
	{
		m_iConnectCount = 0;
		m_iHeartbeatInterval = 0;
		hHeartbeatHandle = 0;
		isHeartbeat = false;
		uv_mutex_init(&mutex);
	};

	XDataBase::~XDataBase()
	{
		if (isHeartbeat)
		{
			isHeartbeat = false;
			uv_thread_join(&hHeartbeatHandle);
		}
		Close();
		uv_mutex_destroy(&mutex);
	}

	int XDataBase::Initialize(string strDbType, string strDbName, string strConnectString, int iConnectCount, int iHeartbeatInterval)
	{
		m_strDbType = strDbType;
		m_strDbName = strDbName;
		m_iConnectCount = iConnectCount;
		m_iHeartbeatInterval = iHeartbeatInterval;
		int iResult = Open(strConnectString);
		if (iResult != X_SUCCESS)
			return iResult;

		if (m_iHeartbeatInterval > 0)
		{
			isHeartbeat = true;
			iResult = uv_thread_create(&hHeartbeatHandle, StartHeartBeat, this);
			if (iResult != 0)
			{
				isHeartbeat = false;
				XLogClass::error("XDataBase::Initialize uv_thread_create StartHeartbeat 失败");
				return X_FAILURE;
			}
		}

		return X_SUCCESS;
	}

	int XDataBase::Open(string strConnectString)
	{
		m_strConnectString = strConnectString;

		for (int i = 0; i < m_iConnectCount; i++)
		{
			XDataBaseConnection *pXDataBaseConnection = NULL;
			int iResult = XDataBaseConnectionFactory::CreateDataBaseConnectionByName(m_strDbType, pXDataBaseConnection);
			if (iResult != X_SUCCESS)
			{
				XLogClass::info("数据库链接创建失败:[%s][%d]", m_strDbType.c_str(), iResult);
				return X_FAILURE;
			}
			m_vxDbConns.push_back(pXDataBaseConnection);
		}

		if (m_iConnectCount > 0)
		{
			m_vxDbConns[0]->Initialize();
		}

		for (int i = 0; i < m_iConnectCount; i++)
		{
			if (m_vxDbConns[i]->Open(m_strConnectString) != X_SUCCESS)
				return X_FAILURE;
		}
		return X_SUCCESS;
	}

	int XDataBase::Close()
	{
		for (int i = 0; i < (int)m_vxDbConns.size(); i++)
		{
			//在XDataBaseConnection的实现中，析构一定要关闭连接
			//这里就不用容易操作关闭了
			//m_vxDbConns[i]->Close();
			RELEASE(m_vxDbConns[i]);
		}
		m_vxDbConns.clear();
		return X_SUCCESS;
	}

	int XDataBase::Query(const string &strSql, XDataTable &xDataTable, const int iArraySize)
	{
		XDataRow xDataRow;
		return Query(strSql, xDataRow, xDataTable, iArraySize);
	}

	int XDataBase::Query(const string &strSql, XDataRow &xDataRow, XDataTable &xDataTable, const int iArraySize)
	{
		XDbTransaction xDbTrans;
		int iResult = _GetFreeConnect(xDbTrans);
		if (iResult != X_SUCCESS)
		{
			return X_FAILURE;
		}

		iResult = xDbTrans.m_pXDbConn->Query(strSql, xDataRow, iArraySize, xDataTable);
		_ReleaseConnect(xDbTrans.m_pXDbConn);
		xDbTrans._Dispose();
		return iResult;
	}

	int XDataBase::Query(const XDbTransaction &xDbTrans, const string &strSql, XDataTable &xDataTable, const int iArraySize)
	{
		XDataRow xDataRow;
		return Query(xDbTrans, strSql, xDataRow, xDataTable, iArraySize);
	}

	int XDataBase::Query(const XDbTransaction &xDbTrans, const string &strSql, XDataRow &xDataRow, XDataTable &xDataTable, const int iArraySize)
	{
		return xDbTrans.m_pXDbConn->Query(strSql, xDataRow, iArraySize, xDataTable);
	}

	int XDataBase::ExecuteSql(const string &strSql)
	{
		int64_t i64RecordCount = 0;
		return ExecuteSql(strSql, i64RecordCount);
	}

	int XDataBase::ExecuteSql(const string &strSql, int64_t &i64RecordCount)
	{
		XDataTable xDataTable;
		xDataTable.NewRow();
		return ExecuteSql(strSql, xDataTable, i64RecordCount);
	}

	int XDataBase::ExecuteSql(const string &strSql, XDataTable &xDataTable)
	{
		int64_t i64RecordCount = 0;
		return ExecuteSql(strSql, xDataTable, i64RecordCount);
	}

	int XDataBase::ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount)
	{
		XDbTransaction xDbTrans;
		int iResult = _GetFreeConnect(xDbTrans);
		if (iResult != X_SUCCESS)
			return X_FAILURE;
		iResult = ExecuteSql(xDbTrans, strSql, xDataTable, i64RecordCount);
		if (iResult != X_SUCCESS)
		{
			RollBack(xDbTrans);
			return X_FAILURE;
		}

		return Commit(xDbTrans);
	}

	int XDataBase::ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql)
	{
		int64_t i64RecordCount = 0;
		return ExecuteSql(xDbTrans, strSql, i64RecordCount);
	}

	int XDataBase::ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql, int64_t &i64RecordCount)
	{
		XDataTable xDataTable;
		xDataTable.NewRow();
		return xDbTrans.m_pXDbConn->ExecuteSql(strSql, xDataTable, i64RecordCount);
	}

	int XDataBase::ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql, XDataTable &xDataTable)
	{
		int64_t i64RecordCount = 0;
		return ExecuteSql(xDbTrans, strSql, xDataTable, i64RecordCount);
	}

	int XDataBase::ExecuteSql(const XDbTransaction &xDbTrans, const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount)
	{
		return xDbTrans.m_pXDbConn->ExecuteSql(strSql, xDataTable, i64RecordCount);
	}

	int XDataBase::BeginTransaction(XDbTransaction &xDbTrans)
	{
		if (xDbTrans.m_pXDb != NULL || xDbTrans.m_pXDbConn != NULL)
		{
			XLogClass::error("XDataBase::BeginTransaction xDbTrans.m_pXDb != NULL || xDbTrans.m_pXDbConn != NULL");
			return X_FAILURE;
		}

		return _GetFreeConnect(xDbTrans);
	}

	int XDataBase::Commit(XDbTransaction &xDbTrans)
	{
		int iResult = _Commit(xDbTrans.m_pXDbConn);
		xDbTrans._Dispose();

		return iResult;
	}

	int XDataBase::RollBack(XDbTransaction &xDbTrans)
	{
		int iResult = _RollBack(xDbTrans.m_pXDbConn);
		xDbTrans._Dispose();

		return iResult;
	}

	int XDataBase::_GetFreeConnect(XDbTransaction &xDbTrans)
	{
		uv_mutex_lock(&mutex);
		//if(uv_mutex_trylock(&mutex) == X_SUCCESS)
		{
			for (int i = 0; i < m_iConnectCount; i++)
			{
				if (!m_vxDbConns[i]->m_isUsed)
				{
					xDbTrans.m_pXDb = this;
					xDbTrans.m_pXDbConn = m_vxDbConns[i];

					xDbTrans.m_pXDbConn->m_isUsed = true;
					//xDbTrans.m_hThread = (uv_thread_t)uv_thread_self();
					//xDbTrans.m_pXDbConn->m_hThread = xDbTrans.m_hThread;
					//xDbTrans.m_pXDbConn->lLastUseTime = uv_hrtime();//申请的时候不设置最后使用时间
					uv_mutex_unlock(&mutex);
					return X_SUCCESS;
				}
			}
		}
		uv_mutex_unlock(&mutex);

		XLogClass::warn("数据库连接使用已满");
		return X_FAILURE;
	}

	int XDataBase::_ReleaseConnect(XDataBaseConnection *pXDbConn)
	{
		uv_mutex_lock(&mutex);
		pXDbConn->m_isUsed = false;
		pXDbConn->m_hThread = 0;
		pXDbConn->m_ui64LastUseTime = uv_hrtime();
		uv_mutex_unlock(&mutex);

		return X_SUCCESS;
	}

	int XDataBase::_Commit(XDataBaseConnection *pXDbConn)
	{
		int iResult = pXDbConn->Commit();
		_ReleaseConnect(pXDbConn);

		return iResult;
	}

	int XDataBase::_RollBack(XDataBaseConnection *pXDbConn)
	{
		int iResult = pXDbConn->RollBack();
		_ReleaseConnect(pXDbConn);
		return iResult;
	}

	int XDataBase::_HeartBeat()
	{
		//FOR TEST
		//return X_SUCCESS;
		XLogClass::debug("XDataBase[" + m_strDbName + "] HeartBeat ......");
		uv_mutex_lock(&mutex);
		for (int i = 0; i < m_iConnectCount; i++)
		{
			if (!m_vxDbConns[i]->m_isUsed)
			{
				if (uv_hrtime() - m_vxDbConns[i]->m_ui64LastUseTime >(uint64_t)m_iHeartbeatInterval * 1000000)
				{
					m_vxDbConns[i]->m_isUsed = true;
					m_vxDbConns[i]->m_hThread = (uv_thread_t)uv_thread_self();
					uv_mutex_unlock(&mutex);
					int iResult = m_vxDbConns[i]->HeartBeat();
					if (iResult != X_SUCCESS)
					{
						//执行简单sql失败就重连
						m_vxDbConns[i]->Close();
						m_vxDbConns[i]->Open(m_strConnectString);
					}
					_ReleaseConnect(m_vxDbConns[i]);
					uv_mutex_lock(&mutex);
				}
			}
		}
		uv_mutex_unlock(&mutex);

		return X_FAILURE;
	}

	void XDataBase::StartHeartBeat(void *pVoid)
	{
		XDataBase *pXDB = (XDataBase *)pVoid;
		while (pXDB->isHeartbeat)
		{
			try
			{
				pXDB->_HeartBeat();
			}
			catch (exception &ex)
			{
				XLogClass::error("Exception: Name[%s] Message[%s]", typeid(ex).name(), ex.what());
			}
			//if(iResult != X_SUCCESS)
			//	break;
			uv_sleep(pXDB->m_iHeartbeatInterval);
		}
	}
}
