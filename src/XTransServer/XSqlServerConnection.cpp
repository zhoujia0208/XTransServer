#include "StdAfx.h"
#include "XSqlServerConnection.h"

XRegistyDataBaseConnectionClass XSqlServerConnection::xRegisty("sqlserver", XSqlServerConnection::CreateInstance);

static otl_datetime TimeXToOTL(XDataTime & xDataTime)
{
	otl_datetime d;
	d.year = xDataTime.iYear;
	d.month = xDataTime.iMonth;
	d.day = xDataTime.iDay;
	d.hour = xDataTime.iHour;
	d.minute = xDataTime.iMinute;
	d.second = xDataTime.iSecond;
	d.fraction = xDataTime.uiFraction;
	d.frac_precision = xDataTime.iFracPrec;
	//d.tz_hour = xDataTime.iTzHour;
	//d.tz_minute = xDataTime.iTzMinute;
	return d;
}

static XDataTime TimeOLTToX(otl_datetime & d)
{
	XDataTime xDataTime;
	xDataTime.iYear = d.year;
	xDataTime.iMonth = d.month;
	xDataTime.iDay = d.day;
	xDataTime.iHour = d.hour;
	xDataTime.iMinute = d.minute;
	xDataTime.iSecond = d.second;
	xDataTime.uiFraction = d.fraction;
	xDataTime.iFracPrec = d.frac_precision;
	//xDataTime.iTzHour = d.tz_hour;
	//xDataTime.iTzMinute = d.tz_minute;
	return xDataTime;
}

XSqlServerConnection::XSqlServerConnection()
{

}

XSqlServerConnection::~XSqlServerConnection()
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

int XSqlServerConnection::Open(const string &strConnectString)
{
	m_strConnectString = strConnectString;
	try
	{
		if (!db.connected)
		{
			db.auto_commit_off();
			db.set_timeout(5);
			db.rlogon(m_strConnectString.c_str());
			db.set_max_long_size(1024 * 1024);
			//string strSql = "use " + m_strConnectString.substr(m_strConnectString.find('@') + 1);
			//db.direct_exec(strSql.c_str());
			XLogClass::debug("XSqlServerConnection::Open X_SUCCESS");
			return X_SUCCESS;
			//db.set_stream_pool_size(32);
		}
		XLogClass::debug("XSqlServerConnection::Open Already");
		return X_SUCCESS;
	}
	catch (otl_exception &ex)
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
		XLogClass::debug("XSqlServerConnection::Close X_SUCCESS");
		return X_SUCCESS;
	}
	catch (otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Close OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::Query(const string &strSql, XDataRow &xDataRow, const int iArraySize, XDataTable &xDataTable)
{
	try
	{
		otl_stream os(iArraySize, strSql.c_str(), db);
		RowExecuteOtl(os, xDataRow);
		int iResult = DataTableOTLToX(os, xDataTable);
		os.close();
		return iResult;
	}
	catch (otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Query OtlException: Message[%s], Text[%s], Info[%s]",
			ex.msg, ex.stm_text, ex.var_info);
		return X_FAILURE;
	}
}

int XSqlServerConnection::ExecuteSql(const string &strSql, XDataTable &xDataTable, int64_t &i64RecordCount)
{
	try
	{
		int iRowCount = xDataTable.RowCount();
		otl_stream os;
		os.set_commit(0);
		os.open(iRowCount, strSql.c_str(), db);
		for (int i = 0; i < iRowCount; i++)
		{
			RowExecuteOtl(os, xDataTable.GetRow(i));
		}
		i64RecordCount = os.get_rpc();
		os.flush();
		os.close();
		return X_SUCCESS;
	}
	catch (otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::ExecuteSql OtlException: Message[%s], Text[%s], Info[%s]",
			ex.msg, ex.stm_text, ex.var_info);
		return X_FAILURE;
	}
};

int XSqlServerConnection::BeginTransaction()
{
	try
	{
		db.auto_commit_off();
		return X_SUCCESS;
	}
	catch (otl_exception &ex)
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
	catch (otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::Commit OtlzException: Message[%s]", ex.msg);
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
	catch (otl_exception &ex)
	{
		XLogClass::error("XSqlServerConnection::RollBack OtlException: Message[%s]", ex.msg);
		return X_FAILURE;
	}
}

int XSqlServerConnection::HeartBeat()
{
	XDataRow xDataRow;
	XDataTable xDataTable;
	//SELECT orderid:#orderid<bigint>, sysdate FROM DUAL 
	//insert :f1<int>
	return Query("SELECT 1:#1<int>, sysdate() ", xDataRow, 1, xDataTable);
}

int XSqlServerConnection::DataTableOTLToX(otl_stream &os, XDataTable &xDataTable)
{
	int iColumnCount = 0;
	otl_column_desc *pColumnDesc = os.describe_select(iColumnCount);

	for (int i = 0; i < iColumnCount; i++)
	{
		XDataColumn *pColumn = new XDataColumn();
		pColumn->m_strName = pColumnDesc[i].name;
		toUpper(pColumn->m_strName);
		pColumn->m_szSize = pColumnDesc[i].dbsize;
		pColumn->m_iIsNullable = pColumnDesc[i].nullok;
		int iType = pColumnDesc[i].otl_var_dbtype;
		//判断long类型如果为4字节，则为32位系统，otl_var_long_int 则等同于 otl_var_int
		if (iType == otl_var_long_int && sizeof(long int) == 4)
		{
			iType = otl_var_int;
		}
		switch (iType)
		{
			/* 极少数类型 暂不实现
			case otl_var_none:
			case  otl_var_refcur        :
			case  otl_var_long_string   :
			case  otl_var_db2time       :
			case  otl_var_db2date       :
			case  otl_var_tz_timestamp  :
			case  otl_var_ltz_timestamp :
			*/
		case otl_var_char:// null terminated string
		case otl_var_varchar_long:// data type that is mapped into LONG in Oracle 7/8/9/10/11, TEXT in MS SQL Server and Sybase, CLOB in DB2
			pColumn->m_xDataType = XDATATYPE_STRING;
			break;
		case otl_var_double:// 8-byte floating point number 
			pColumn->m_xDataType = XDATATYPE_DOUBLE;
			break;
		case otl_var_float:// 4-byte floating point number 
			pColumn->m_xDataType = XDATATYPE_FLOAT;
			break;
		case otl_var_int:// signed 32-bit  integer
		case otl_var_short:// signed 16-bit integer
			pColumn->m_xDataType = XDATATYPE_INT32;
			break;
		case otl_var_unsigned_int:// unsigned 32-bit integer
			pColumn->m_xDataType = XDATATYPE_UINT32;
			break;
		case otl_var_long_int:// signed 32-bit integer (for 32-bit, and LLP64 C++ compilers), signed 64-bit integer (for LP-64 C++ compilers)
			pColumn->m_xDataType = XDATATYPE_INT64;
			break;
		case otl_var_clob:// data type that is mapped into CLOB in Oracle 8/9/10/11
		case otl_var_blob:// data type that is mapped into BLOB in Oracle 8/9/10/11 
		case otl_var_raw_long:// data type that is mapped into LONG RAW in Oracle, IMAGE in MS SQL Server and Sybase, BLOB in DB2
			pColumn->m_xDataType = XDATATYPE_RAW;
			break;
		case otl_var_timestamp:// data type that is mapped into Oracle date/timestamp, DB2 timestamp, MS SQL datetime/datetime2/time/date, Sybase timestamp, etc.
			pColumn->m_xDataType = XDATATYPE_TIME;
			break;
		case otl_var_bigint:
			pColumn->m_xDataType = XDATATYPE_INT64;
			break;
		case otl_var_ubigint:
			pColumn->m_xDataType = XDATATYPE_UINT64;
			break;
		default:
			throw runtime_error(pColumn->m_strName + "|不支持的数据类型!无法进行转换");
		}

		xDataTable.AddColumn(pColumn);
	}

	while (!os.eof())
	{
		XDataRow & xDataRow = xDataTable.NewRow();
		for (int i = 0; i < iColumnCount; i++)
		{
			XDataValue *pValue = NULL;
			switch (xDataTable.GetColumn(i).m_xDataType)
			{
			case XDATATYPE_FLOAT:
				pValue = new XDataValue(XDATATYPE_FLOAT);
				os >> *((float *)pValue->GetDataPtr());
				break;
			case XDATATYPE_DOUBLE:
				pValue = new XDataValue(XDATATYPE_DOUBLE);
				os >> *((double *)pValue->GetDataPtr());
				break;
			case XDATATYPE_INT32:
				pValue = new XDataValue(XDATATYPE_INT32);
				os >> *((int32_t *)pValue->GetDataPtr());
				break;
			case XDATATYPE_UINT32:
				pValue = new XDataValue(XDATATYPE_UINT32);
				os >> *((uint32_t *)pValue->GetDataPtr());
				break;
			case XDATATYPE_INT64:
				pValue = new XDataValue(XDATATYPE_INT64);
				os >> *((int64_t *)pValue->GetDataPtr());
				break;
			case XDATATYPE_UINT64:
				pValue = new XDataValue(XDATATYPE_UINT64);
				os >> *((uint64_t *)pValue->GetDataPtr());
				break;
			case XDATATYPE_STRING:
				pValue = new XDataValue(XDATATYPE_STRING);
				os >> *((string *)pValue->GetDataPtr());
				break;
			case XDATATYPE_RAW:
			{
				pValue = new XDataValue(XDATATYPE_RAW);
				otl_long_string ls(os.get_adb_max_long_size());
				os >> ls;
				((XDataRaw *)pValue->GetDataPtr())->SetData(ls.v, ls.len(), 1);
				break;
			}
			case XDATATYPE_TIME:
			{
				pValue = new XDataValue(XDATATYPE_TIME);
				otl_datetime d;
				os >> d;
				*(XDataTime *)pValue->GetDataPtr() = TimeOLTToX(d);
				break;
			}
			default:
				throw runtime_error(xDataTable.GetColumn(i).m_strName + "|未知的数据类型!无法进行转换");
			}

			if (!os.is_null())
				pValue->SetNull(0);

			xDataRow.AddValue(pValue);
		}
	}

	return X_SUCCESS;
}

int XSqlServerConnection::RowExecuteOtl(otl_stream &os, XDataRow &xDataRow)
{
	int iItemCount = xDataRow.ItemCount();
	for (int j = 0; j < iItemCount; j++)
	{
		if (xDataRow[j].IsNull())
		{
			os << otl_null();
			continue;
		}

		XDataType xDataType = xDataRow[j].GetDataType();
		switch (xDataType)
		{
		case XDATATYPE_FLOAT:
			os << *(float *)(xDataRow[j].GetDataPtr());
			break;
		case XDATATYPE_DOUBLE:
			os << *(double *)(xDataRow[j].GetDataPtr());
			break;
		case XDATATYPE_INT32:
			os << *(int32_t *)(xDataRow[j].GetDataPtr());
			break;
		case XDATATYPE_UINT32:
			os << *(uint32_t *)(xDataRow[j].GetDataPtr());
			break;
		case XDATATYPE_INT64:
#ifdef _WIN32 
			os << *(int64_t *)(xDataRow[j].GetDataPtr());
#else
			os << *(long long *)(xDataRow[j].GetDataPtr());
#endif
			break;
		case XDATATYPE_UINT64:
#ifdef _WIN32 
			os << *(uint64_t *)(xDataRow[j].GetDataPtr());
#else
			os << *(unsigned long long *)(xDataRow[j].GetDataPtr());
#endif
			break;
		case XDATATYPE_STRING:
			os << *(string *)(xDataRow[j].GetDataPtr());
			break;
		case XDATATYPE_RAW:
		{
			BYTE *pV = ((XDataRaw *)xDataRow[j].GetDataPtr())->GetData();
			size_t szLen = ((XDataRaw *)xDataRow[j].GetDataPtr())->GetSize();
			otl_long_string ls(pV, szLen, szLen);
			os << ls;
			break;
		}
		case XDATATYPE_TIME:
			os << TimeXToOTL(*(XDataTime *)xDataRow[j].GetDataPtr());
			break;
		default:
			throw runtime_error("xDataTable[i][j].m_xType Not Defined");
		}
	}

	return X_SUCCESS;
}
