#include "StdAfx.h"
#include "XSqlServerConnection.h"

XRegistyDataBaseConnectionClass XSqlServerConnection::xRegisty("sqlserver", XSqlServerConnection::CreateInstance);

XSqlServerConnection::XSqlServerConnection(void)
{
	
}

XSqlServerConnection::~XSqlServerConnection(void)
{
	Close();
}

int XSqlServerConnection::Initialize()
{
	//OTL多线程模式
	otl_connect::otl_initialize(1);

	return X_SUCCESS;
}

int XSqlServerConnection::Terminate()
{
	//otl_connect::otl_terminate();

	return X_SUCCESS;
}

int XSqlServerConnection::Open(string strConnectString)
{
	m_strConnectString = strConnectString;
	try
	{
		if(!db.connected)
		{
			db.set_timeout(5);
			db.rlogon(m_strConnectString.c_str());
			//db.direct_exec("ALTER SESSION SET COMMIT_WRITE = IMMEDIATE, NOWAIT");
			//db.set_stream_pool_size(32);
		}
		return X_SUCCESS;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Open OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::Close()
{
	try
	{
		db.logoff();
		return X_SUCCESS;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Close OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::Query(string strSql, int iArraySize, XDataTable &xDataTable)
{
	try
	{
		otl_stream otl_os(iArraySize, strSql.c_str(), db);
		int iResult = ToXDataTable(otl_os, xDataTable);
		otl_os.close();
		return iResult;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Query OtlException: Message[%s], Text[%s], Info[%s]", 
			ex.msg, ex.stm_text, ex.var_info);
		return X_FAILURE;
	}
}

int XSqlServerConnection::ExecuteSql(string strSql, int64_t &i64RecordCount)
{
	try
	{
		otl_stream otl_os;
		otl_os.open(1, strSql.c_str(), db);
		i64RecordCount = otl_os.get_rpc();
		return X_SUCCESS;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::ExecuteSql OtlException: Message[%s], Text[%s], Info[%s]", 
			ex.msg, ex.stm_text, ex.var_info);
		return X_FAILURE;
	}
}

int XSqlServerConnection::BeginTransaction()
{
	try
	{
		db.auto_commit_off();
		return X_SUCCESS;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::BeginTransaction OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::Commit()
{
	try
	{
		db.commit();
		return X_SUCCESS;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Commit OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::RollBack()
{
	try
	{
		db.rollback();
		return X_SUCCESS;
	}
	catch(otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::RollBack OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::HeartBeat()
{
	XDataTable xDataTable;
	return Query("select 1", 1, xDataTable);
}

int XSqlServerConnection::ToXDataTable(otl_stream &os, XDataTable &xDataTable)
{
	//int desc_len = 0;
	//otl_column_desc *desc = os.describe_select(desc_len);
	//while(!os.eof())
	//{
	//	XDataRow *pRow = new XDataRow();
	//	if(pRow == NULL)
	//	{
	//		XLogClass::error("XDataTable::LoadData XDataRow new Fail");
	//		return X_FAILURE;
	//	}
	//	for(int iCol = 0;iCol < desc_len; iCol ++)
	//	{
	//		XDataValue  *pXDataValue = new XDataValue();
	//		//pXDataValue->m_iType = desc[iCol].otl_var_dbtype;
	//		pXDataValue->m_szSize = desc[iCol].dbsize;
	//		pXDataValue->m_strName = desc[iCol].name;
	//		toUpper(pXDataValue->m_strName);
	//		switch(desc[iCol].otl_var_dbtype)
	//		{
	//			/* 极少数类型 暂不实现
	//			case  otl_var_refcur        :
	//			case  otl_var_long_string   :
	//			case  otl_var_db2time       :
	//			case  otl_var_db2date       :
	//			case  otl_var_tz_timestamp  :
	//			case  otl_var_ltz_timestamp :
	//			*/
	//		case otl_var_none:

	//			break;
	//		case otl_var_char:// null terminated string 
	//		case otl_var_varchar_long:// data type that is mapped into LONG in Oracle 7/8/9/10/11, TEXT in MS SQL Server and Sybase, CLOB in DB2
	//			{
	//				string strTemp;
	//				os>>strTemp;
	//				pXDataValue->m_pValue = (byte *)malloc(strTemp.length() + 1);
	//				strcpy((char *)pXDataValue->m_pValue, strTemp.c_str());
	//				break;
	//			}
	//		case otl_var_raw_long:// data type that is mapped into LONG RAW in Oracle, IMAGE in MS SQL Server and Sybase, BLOB in DB2
	//		case otl_var_clob:// data type that is mapped into CLOB in Oracle 8/9/10/11
	//		case otl_var_blob:// data type that is mapped into BLOB in Oracle 8/9/10/11 
	//			{
	//				pXDataValue->m_pValue = (byte *)malloc(OTL_MAX_CHAR_SIZE+ 1);
	//				os>>(char *)pXDataValue->m_pValue;
	//				((char *)(pXDataValue->m_pValue))[OTL_MAX_CHAR_SIZE] = 0x00;
	//				break;
	//			}
	//		case otl_var_double:// 8-byte floating point number 
	//			{
	//				double tdouble = 0; 
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(double));
	//				os>>tdouble;
	//				memcpy(pXDataValue->m_pValue, &tdouble, sizeof(double));
	//				break;
	//			}
	//		case otl_var_float:// 4-byte floating point number 
	//			{
	//				float tfloat = 0;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(float));
	//				os>>tfloat;
	//				memcpy(pXDataValue->m_pValue, &tfloat, sizeof(float));
	//				break;
	//			}
	//		case otl_var_int:// signed 32-bit  integer
	//			{
	//				int tint = 0;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(int));
	//				os>>tint;
	//				memcpy(pXDataValue->m_pValue, &tint, sizeof(int));
	//				break;
	//			}
	//		case otl_var_unsigned_int:// unsigned 32-bit integer
	//			{
	//				unsigned int tuint = 0;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(unsigned int));
	//				os>>tuint;
	//				memcpy(pXDataValue->m_pValue, &tuint,sizeof(unsigned int));
	//				break;
	//			}
	//		case otl_var_short:// signed 16-bit integer
	//			{
	//				short tshort = 0;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(short));
	//				os>>tshort;
	//				memcpy(pXDataValue->m_pValue, &tshort,sizeof(short));
	//				break;
	//			}
	//		case otl_var_long_int:// signed 32-bit integer (for 32-bit, and LLP64 C++ compilers), signed 64-bit integer (for LP-64 C++ compilers)
	//			{
	//				long tlong = 0;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(long));
	//				os>>tlong;
	//				memcpy(pXDataValue->m_pValue, &tlong, sizeof(long));
	//				break;
	//			}
	//		case otl_var_timestamp:// data type that is mapped into Oracle date/timestamp, DB2 timestamp, MS SQL datetime/datetime2/time/date, Sybase timestamp, etc.
	//			{
	//				otl_datetime tdate;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(otl_datetime));
	//				os>>tdate;
	//				memcpy(pXDataValue->m_pValue, &tdate,sizeof(otl_datetime));
	//				break;
	//			}
	//		case otl_var_bigint:	
	//			{
	//				int64_t tint64;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(tint64));
	//				os >> tint64;
	//				memcpy(pXDataValue->m_pValue, &tint64,sizeof(int64_t));
	//				break;
	//			}
	//		case otl_var_ubigint:
	//			{
	//				uint64_t tuint64;
	//				pXDataValue->m_pValue = (byte *)malloc(sizeof(uint64_t));
	//				os >> tuint64;
	//				memcpy(pXDataValue->m_pValue, &tuint64,sizeof(uint64_t));
	//				break;
	//			}
	//		default:
	//			{
	//				throw runtime_error(pXDataValue->m_strName + "|未知的数据类型!无法进行转换");
	//				break;
	//			}
	//		}
	//		pXDataValue->m_iIsNull = os.is_null();
	//		pRow->AddDataValue(pXDataValue);
	//	}
	//	xDataTable.AddRow(pRow);
	//}

	return X_SUCCESS;
}

//o.flush(); // flush the stream's dirty buffer: 
// execute the INSERT for the rows 
// that are still in the stream buffer

//db.commit_nowait(); // commit with no wait (new feature of Oracle 10.2)
