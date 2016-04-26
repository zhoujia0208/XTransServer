#include "StdAfx.h"
#include "XDataTable.h"

namespace XData {

	XDataRaw::XDataRaw()
	{
		m_pData = NULL;
		m_szSize = 0;
		m_iCopyFlag = 0;
	}

	XDataRaw::~XDataRaw()
	{
		_Dispose();
	}

	void XDataRaw::_Dispose()
	{
		if (m_iCopyFlag)
			delete m_pData;
	}

	void XDataRaw::Dispose()
	{
		_Dispose();
		m_szSize = 0;
		m_iCopyFlag = 0;
	}

	void XDataRaw::_SetData(const BYTE *pData, size_t szSize, int iCopyFlag)
	{
		m_szSize = szSize;
		m_iCopyFlag = iCopyFlag;
		if (m_iCopyFlag)
		{
			m_pData = new BYTE[m_szSize];
			if (m_pData == NULL)
			{
				throw runtime_error("XDataRaw::_SetRaw(BYTE *pData, size_t szSize, int iCopyFlag) new BYTE FAIL");
			}
			if (pData != NULL)
				memcpy(m_pData, pData, m_szSize);
		}
		else
			m_pData = (BYTE *)pData;
	}

	void XDataRaw::_SetDataByHexString(const string &sHexString)
	{
		m_szSize = sHexString.size() / 2;
		m_iCopyFlag = 1;
		m_pData = new BYTE[m_szSize];
		if (m_pData == NULL)
		{
			throw runtime_error("XDataRaw::_SetRawHex(const string &sHex) new BYTE FAIL");
		}
		StringToByte(sHexString.c_str(), m_pData);
	}

	XDataRaw::XDataRaw(const BYTE *pData, size_t szSize, int iCopyFlag)
	{
		_SetData(pData, szSize, iCopyFlag);
	}

	void XDataRaw::SetData(const BYTE *pData, size_t szSize, int iCopyFlag)
	{
		Dispose();
		_SetData(pData, szSize, iCopyFlag);
	}

	XDataRaw::XDataRaw(const string &sHex)
	{
		_SetDataByHexString(sHex);
	}

	void XDataRaw::SetDataByString(const string &sString)
	{
		Dispose();
		_SetData((BYTE *)sString.c_str(), sString.size(), 1);
	}

	void XDataRaw::SetDataByHexString(const string &sHex)
	{
		Dispose();
		_SetDataByHexString(sHex);
	}

	XDataRaw::XDataRaw(const XDataRaw &x)
	{
		_SetData(x.m_pData, x.m_szSize, x.m_iCopyFlag);
	}

	XDataRaw &XDataRaw::operator =(const XDataRaw &x)
	{
		_SetData(x.m_pData, x.m_szSize, x.m_iCopyFlag);
		return *this;
	}

	BYTE *XDataRaw::GetData()
	{
		return m_pData;
	}

	size_t XDataRaw::GetSize()
	{
		return m_szSize;
	}

	string XDataRaw::GetDataAsString()
	{
		return string((char *)m_pData, m_szSize);
	}

	string XDataRaw::GetDataAsHexString()
	{
		string s;
		ByteToString(m_pData, m_szSize, s);
		return s;
	}

	XDataTime::XDataTime()
	{
		iYear = 0;
		iMonth = 0;
		iDay = 0;
		iHour = 0;
		iMinute = 0;
		iSecond = 0;
		uiFraction = 0;
		iFracPrec = 0;
		iTzHour = 0;
		iTzMinute = 0;
	}

	XDataValue::XDataValue()
	{
		m_szSize = 0;
		m_xDataType = XDATATYPE_UNKNOW;
		m_iIsNull = 1;
		m_pValue = NULL;
	}

	XDataValue::~XDataValue()
	{
		_Dispose();
	}

	void XDataValue::_Dispose()
	{
		if (m_pValue != NULL)
		{
			switch (m_xDataType)
			{
			case XDATATYPE_UNKNOW:
				break;
			case XDATATYPE_FLOAT:
				delete ((float *)m_pValue);
				break;
			case XDATATYPE_DOUBLE:
				delete ((double *)m_pValue);
				break;
			case XDATATYPE_INT32:
				delete ((int32_t *)m_pValue);
				break;
			case XDATATYPE_UINT32:
				delete ((uint32_t *)m_pValue);
				break;
			case XDATATYPE_INT64:
				delete ((int64_t *)m_pValue);
				break;
			case XDATATYPE_UINT64:
				delete ((uint64_t *)m_pValue);
				break;
			case XDATATYPE_STRING:
				delete ((string *)m_pValue);
				break;
			case XDATATYPE_RAW:
				delete ((XDataRaw *)m_pValue);
				break;
			case XDATATYPE_TIME:
				delete ((XDataTime *)m_pValue);
				break;
			default:
				delete (BYTE *)m_pValue;
				break;
			}
			m_pValue = NULL;
		}
	}

	void XDataValue::Dispose()
	{
		_Dispose();
		m_szSize = 0;
		m_xDataType = XDATATYPE_UNKNOW;
		m_iIsNull = 1;
	}

	void XDataValue::_Copy(const XDataValue &x)
	{
		m_szSize = x.m_szSize;
		m_xDataType = x.m_xDataType;
		m_iIsNull = x.m_iIsNull;
		switch (m_xDataType)
		{
		case XDATATYPE_UNKNOW:
			m_pValue = NULL;
			return;
		case XDATATYPE_FLOAT:
			m_pValue = new float(*(float *)x.m_pValue);
			break;
		case XDATATYPE_DOUBLE:
			m_pValue = new double(*(double *)x.m_pValue);
			break;
		case XDATATYPE_INT32:
			m_pValue = new int32_t(*(int32_t *)x.m_pValue);
			break;
		case XDATATYPE_UINT32:
			m_pValue = new uint32_t(*(uint32_t *)x.m_pValue);
			break;
		case XDATATYPE_INT64:
			m_pValue = new int64_t(*(int64_t *)x.m_pValue);
			break;
		case XDATATYPE_UINT64:
			m_pValue = new uint64_t(*(uint64_t *)x.m_pValue);
			break;
		case XDATATYPE_STRING:
			m_pValue = new string(*(string *)x.m_pValue);
			break;
		case XDATATYPE_RAW:
			m_pValue = new XDataRaw(*(XDataRaw *)x.m_pValue);
			break;
		case XDATATYPE_TIME:
			m_pValue = new XDataTime(*(XDataTime *)x.m_pValue);
			break;
		default:
			m_pValue = NULL;
			break;
		}

		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_Copy(const XDataValue &x) new FAIL");
		}
	}

	XDataValue::XDataValue(const XDataValue &x)
	{
		_Copy(x);
	}

	XDataValue &XDataValue::operator =(const XDataValue &x)
	{
		if (this == &x)
			return *this;

		_Dispose();
		_Copy(x);
		return *this;
	}

	XDataValue::XDataValue(XDataType xDataType)
	{
		switch (xDataType)
		{
		case XDATATYPE_UNKNOW:
			m_xDataType = XDATATYPE_UNKNOW;
			m_szSize = 0;
			m_iIsNull = 1;
			m_pValue = NULL;
			return;
		case XDATATYPE_FLOAT:
			m_xDataType = XDATATYPE_FLOAT;
			m_szSize = sizeof(float);
			m_pValue = new float();
			break;
		case XDATATYPE_DOUBLE:
			m_xDataType = XDATATYPE_DOUBLE;
			m_szSize = sizeof(double);
			m_pValue = new double();
			break;
		case XDATATYPE_INT32:
			m_xDataType = XDATATYPE_INT32;
			m_szSize = sizeof(int32_t);
			m_pValue = new int32_t();
			break;
		case XDATATYPE_UINT32:
			m_xDataType = XDATATYPE_UINT32;
			m_szSize = sizeof(uint32_t);
			m_pValue = new uint32_t();
			break;
		case XDATATYPE_INT64:
			m_xDataType = XDATATYPE_INT64;
			m_szSize = sizeof(int64_t);
			m_pValue = new int64_t();
			break;
		case XDATATYPE_UINT64:
			m_xDataType = XDATATYPE_UINT64;
			m_szSize = sizeof(uint64_t);
			m_pValue = new uint64_t();
			break;
		case XDATATYPE_STRING:
			m_xDataType = XDATATYPE_STRING;
			m_szSize = sizeof(string);
			m_pValue = new string();
			break;
		case XDATATYPE_RAW:
			m_xDataType = XDATATYPE_RAW;
			m_szSize = sizeof(XDataRaw);
			m_pValue = new XDataRaw();
			break;
		case XDATATYPE_TIME:
			m_xDataType = XDATATYPE_TIME;
			m_szSize = sizeof(XDataTime);
			m_pValue = new XDataTime();
			break;
		default:
			m_pValue = NULL;
			break;
		}
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::XDataValue(XDataType xDataType) new FAIL");
		}

		m_iIsNull = 1;//此种方式构造的XDataValue默认值为空，数据类型不是空
	}

	void XDataValue::_SetInt32(const int32_t &i32Value)
	{
		m_xDataType = XDATATYPE_INT32;
		m_szSize = sizeof(int32_t);
		m_iIsNull = 0;
		m_pValue = new int32_t(i32Value);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetInt32(const int32_t &i32Value) new BYTE FAIL");
		}
	}

	XDataValue::XDataValue(const int32_t &i32Value)
	{
		_SetInt32(i32Value);
	}

	XDataValue &XDataValue::operator =(const int32_t &i32Value)
	{
		_Dispose();
		_SetInt32(i32Value);
		return *this;
	}

	void XDataValue::_SetUInt32(const uint32_t &ui32Value)
	{
		m_xDataType = XDATATYPE_UINT32;
		m_szSize = sizeof(uint32_t);
		m_iIsNull = 0;
		m_pValue = new uint32_t(ui32Value);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetUInt32(const uint32_t &ui32Value) new FAIL");
		}
	}

	XDataValue::XDataValue(const uint32_t &ui32Value)
	{
		_SetUInt32(ui32Value);
	}

	XDataValue &XDataValue::operator =(const uint32_t &ui32Value)
	{
		_Dispose();
		_SetUInt32(ui32Value);
		return *this;
	}

	void XDataValue::_SetInt64(const int64_t &i64Value)
	{
		m_xDataType = XDATATYPE_INT64;
		m_szSize = sizeof(int64_t);
		m_iIsNull = 0;
		m_pValue = new int64_t(i64Value);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetInt64(const int64_t &i64Value) new FAIL");
		}
	}

	XDataValue::XDataValue(const int64_t &i64Value)
	{
		_SetInt64(i64Value);
	}

	XDataValue &XDataValue::operator =(const int64_t &i64Value)
	{
		_Dispose();
		_SetInt64(i64Value);
		return *this;
	}

	void XDataValue::_SetUInt64(const uint64_t &ui64Value)
	{
		m_xDataType = XDATATYPE_UINT64;
		m_szSize = sizeof(uint64_t);
		m_iIsNull = 0;
		m_pValue = new uint64_t(ui64Value);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetUInt64(const uint64_t &ui64Value) new FAIL");
		}
	}

	XDataValue::XDataValue(const uint64_t &ui64Value)
	{
		_SetUInt64(ui64Value);
	}

	XDataValue &XDataValue::operator =(const uint64_t &ui64Value)
	{
		_Dispose();
		_SetUInt64(ui64Value);
		return *this;
	}

	void XDataValue::_SetFloat(const float &fValue)
	{
		m_xDataType = XDATATYPE_FLOAT;
		m_szSize = sizeof(float);
		m_iIsNull = 0;
		m_pValue = new float(fValue);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetFloat(const float &fValue) new FAIL");
		}
	}

	XDataValue::XDataValue(const float &fValue)
	{
		_SetFloat(fValue);
	}

	XDataValue &XDataValue::operator =(const float &fValue)
	{
		_Dispose();
		_SetFloat(fValue);
		return *this;
	}

	void XDataValue::_SetDouble(const double &dValue)
	{
		m_xDataType = XDATATYPE_DOUBLE;
		m_szSize = sizeof(double);
		m_iIsNull = 0;
		m_pValue = new double(dValue);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetDouble(const double &dValue) new FAIL");
		}
	}

	XDataValue::XDataValue(const double &dValue)
	{
		_SetDouble(dValue);
	}

	XDataValue &XDataValue::operator =(const double &dValue)
	{
		_Dispose();
		_SetDouble(dValue);
		return *this;
	}

	void XDataValue::_SetString(const char *pValue)
	{
		m_xDataType = XDATATYPE_STRING;
		m_szSize = sizeof(string);
		m_iIsNull = 0;
		m_pValue = new string(pValue);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetString(const char *pValue) new FAIL");
		}
	}

	XDataValue::XDataValue(const string &strValue)
	{
		_SetString(strValue.c_str());
	}

	XDataValue &XDataValue::operator =(const string &strValue)
	{
		_Dispose();
		_SetString(strValue.c_str());
		return *this;
	}

	XDataValue::XDataValue(const char *pValue)
	{
		_SetString(pValue);
	}

	XDataValue &XDataValue::operator =(const char *pValue)
	{
		_Dispose();
		_SetString(pValue);
		return *this;
	}

	void XDataValue::_SetRaw(const BYTE *pValue, size_t szLen, int iCopyFlag)
	{
		m_xDataType = XDATATYPE_RAW;
		m_szSize = sizeof(XDataRaw);
		m_iIsNull = 0;
		m_pValue = new XDataRaw(pValue, szLen, iCopyFlag);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetRaw(const BYTE *pValue, size_t szLen, int iCopy) new FAIL");
		}
	}

	void XDataValue::SetRaw(const BYTE *pValue, size_t szLen, int iCopyFlag)
	{
		_Dispose();
		_SetRaw(pValue, szLen, iCopyFlag);
	}

	XDataValue::XDataValue(const XDataRaw &xRawValue)
	{
		_SetRaw(xRawValue.m_pData, xRawValue.m_szSize, xRawValue.m_iCopyFlag);
	}

	XDataValue &XDataValue::operator =(const XDataRaw &xRawValue)
	{
		_Dispose();
		_SetRaw(xRawValue.m_pData, xRawValue.m_szSize, xRawValue.m_iCopyFlag);
		return *this;
	}

	void XDataValue::_SetTime(const XDataTime &xDataTime)
	{
		m_xDataType = XDATATYPE_TIME;
		m_szSize = sizeof(XDataTime);
		m_iIsNull = 0;
		m_pValue = new XDataTime(xDataTime);
		if (m_pValue == NULL)
		{
			throw runtime_error("XDataValue::_SetTime(const XDataTime &xDataTime) new FAIL");
		}
	}

	void XDataValue::SetTime(const XDataTime &xDataTime)
	{
		_Dispose();
		_SetTime(xDataTime);
	}

	XDataValue::XDataValue(const XDataTime &xDataTime)
	{
		_SetTime(xDataTime);
	}

	XDataValue &XDataValue::operator =(const XDataTime &xDataTime)
	{
		_Dispose();
		_SetTime(xDataTime);
		return *this;
	}

	void *XDataValue::GetDataPtr()
	{
		return m_pValue;
	}

	int XDataValue::IsNull()
	{
		return m_iIsNull;
	}

	void XDataValue::SetNull(int iIsNull)
	{
		m_iIsNull = iIsNull;
	}

	XDataType XDataValue::GetDataType()
	{
		return m_xDataType;
	}

	XDataValue::operator int32_t()
	{
		int32_t i32Value = 0;
		switch (m_xDataType)
		{
		case XDATATYPE_FLOAT:
		{
			float f = 0;
			memcpy(&f, m_pValue, sizeof(float));
			if (f > 0)
				i32Value = (int32_t)(f + 0.5);//float 强制转换，忽略错误
			else
				i32Value = (int32_t)(f - 0.5);
			break;
		}
		case XDATATYPE_DOUBLE:
		{
			double d = 0;
			memcpy(&d, m_pValue, sizeof(double));
			if (d > 0)
				i32Value = (int32_t)(d + 0.5);//double 强制转换，忽略错误
			else
				i32Value = (int32_t)(d - 0.5);
			break;
		}
		case XDATATYPE_INT32:
		{
			memcpy(&i32Value, m_pValue, sizeof(int32_t));
			break;
		}
		case XDATATYPE_UINT32:
		{
			uint32_t ui32 = 0;
			memcpy(&ui32, m_pValue, sizeof(uint32_t));
			if (ui32 > INT_MAX)
			{
				throw runtime_error("> INT_MAX|无法进行类型转换[XDATATYPE_UINT32->int32_t]:值超出范围");
			}
			i32Value = ui32;
			break;
		}
		case XDATATYPE_INT64:
		{
			int64_t i64 = 0;
			memcpy(&i64, m_pValue, sizeof(int64_t));
			if (i64 > INT_MAX || i64 < INT_MIN)
			{
				throw runtime_error("> INT_MAX||< INT_MIN|无法进行类型转换[XDATATYPE_INT64->int32_t]:值超出范围");
			}
			i32Value = (int32_t)i64;
			break;
		}
		case XDATATYPE_UINT64:
		{
			uint64_t ui64 = 0;
			memcpy(&ui64, m_pValue, sizeof(uint64_t));
			if (ui64 > INT_MAX)
			{
				throw runtime_error("> INT_MAX|无法进行类型转换[XDATATYPE_UINT64->int32_t]:值超出范围");
			}
			i32Value = (int32_t)ui64;
			break;
		}
		case XDATATYPE_STRING:
		{
			i32Value = convert<int32_t>(*(string *)m_pValue);
			break;
		}
		default:
			throw runtime_error("|无法进行类型转换[->int32_t]");
			break;
		}

		return i32Value;
	}

	XDataValue::operator uint32_t()
	{
		uint32_t ui32Value = 0;
		switch (m_xDataType)
		{
		case XDATATYPE_FLOAT:
		{
			float f = 0;
			memcpy(&f, m_pValue, sizeof(float));
			if (f < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_FLOAT->uint32_t]:值超出范围");
			}
			ui32Value = (uint32_t)(f + 0.5);//float 强制转换，忽略错误
			break;
		}
		case XDATATYPE_DOUBLE:
		{
			double d = 0;
			memcpy(&d, m_pValue, sizeof(double));
			if (d < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_DOUBLE->uint32_t]:值超出范围");
			}
			ui32Value = uint32_t(d + 0.5);
			break;
		}
		case XDATATYPE_INT32:
		{
			int32_t i32 = 0;
			memcpy(&i32, m_pValue, sizeof(int));
			if (i32 < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_INT32->uint32_t]:值超出范围");
			}
			ui32Value = i32;
			break;
		}
		case XDATATYPE_UINT32:
		{
			memcpy(&ui32Value, m_pValue, sizeof(uint32_t));
			break;
		}
		case XDATATYPE_INT64:
		{
			int64_t i64 = 0;
			memcpy(&i64, m_pValue, sizeof(int64_t));
			if (i64 > UINT_MAX || i64 < 0)
			{
				throw runtime_error("> INT_MAX||< 0|无法进行类型转换[XDATATYPE_INT64->uint32_t]:值超出范围");
			}
			ui32Value = (uint32_t)i64;
			break;
		}
		case XDATATYPE_UINT64:
		{
			uint64_t ui64 = 0;
			memcpy(&ui64, m_pValue, sizeof(uint64_t));
			if (ui64 > UINT_MAX)
			{
				throw runtime_error("> INT_MAX|无法进行类型转换[XDATATYPE_UINT64->uint32_t]:值超出范围");
			}
			ui32Value = (uint32_t)ui64;
			break;
		}
		case XDATATYPE_STRING:
		{
			ui32Value = convert<uint32_t>(*(string *)m_pValue);
			break;
		}
		default:
			throw runtime_error("|无法进行类型转换[->uint32_t]");
			break;
		}

		return ui32Value;
	}

	XDataValue::operator int64_t()
	{
		int64_t i64Value = 0;
		switch (m_xDataType)
		{
		case XDATATYPE_FLOAT:
		{
			float f = 0;
			memcpy(&f, m_pValue, sizeof(float));
			if (f > 0)
				i64Value = (int64_t)(f + 0.5);//float 强制转换，忽略错误
			else
				i64Value = (int64_t)(f - 0.5);
			break;
		}
		case XDATATYPE_DOUBLE:
		{
			double d = 0;
			memcpy(&d, m_pValue, sizeof(double));
			if (d > 0)
				i64Value = (int64_t)(d + 0.5);//double 强制转换，忽略错误
			else
				i64Value = (int64_t)(d - 0.5);
			break;
		}
		case XDATATYPE_INT32:
		{
			int32_t i32 = 0;
			memcpy(&i32, m_pValue, sizeof(int32_t));
			i64Value = i32;
			break;
		}
		case XDATATYPE_UINT32:
		{
			uint32_t ui32 = 0;
			memcpy(&ui32, m_pValue, sizeof(uint32_t));
			i64Value = ui32;
			break;
		}
		case XDATATYPE_INT64:
		{
			memcpy(&i64Value, m_pValue, sizeof(int64_t));
			break;
		}
		case XDATATYPE_UINT64:
		{
			uint64_t ui64 = 0;
			memcpy(&ui64, m_pValue, sizeof(uint64_t));
			if (ui64 & 0x8000000000000000)
			{
				throw runtime_error(" > 0x8000000000000000|无法进行类型转换[XDATATYPE_UINT64->int64_t]:值超出范围");
			}
			i64Value = ui64;
			break;
		}
		case XDATATYPE_STRING:
		{
			i64Value = convert<int64_t>(*(string *)m_pValue);
			break;
		}
		default:
		{
			throw runtime_error("|无法进行类型转换[->int64_t]");
			break;
		}
		}

		return i64Value;
	}

	XDataValue::operator uint64_t()
	{
		uint64_t ui64Value = 0;
		switch (m_xDataType)
		{
		case XDATATYPE_FLOAT:
		{
			float f = 0;
			memcpy(&f, m_pValue, sizeof(float));
			if (f < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_FLOAT->uint32_t]:值超出范围");
			}
			ui64Value = uint64_t(f + 0.5);
			break;
		}
		case XDATATYPE_DOUBLE:
		{
			double d = 0;
			memcpy(&d, m_pValue, sizeof(double));
			if (d < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_DOUBLE->uint32_t]:值超出范围");
			}
			ui64Value = uint64_t(d + 0.5);
			break;
		}
		case XDATATYPE_INT32:
		{
			int32_t i32 = 0;
			memcpy(&i32, m_pValue, sizeof(int32_t));
			if (i32 < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_INT32->uint64_t]:值超出范围");
			}
			ui64Value = i32;
			break;
		}
		case XDATATYPE_UINT32:
		{
			uint32_t ui32 = 0;
			memcpy(&ui32, m_pValue, sizeof(uint32_t));
			ui64Value = ui32;
			break;
		}
		case XDATATYPE_INT64:
		{
			int64_t i64 = 0;
			memcpy(&i64, m_pValue, sizeof(int64_t));
			if (i64 < 0)
			{
				throw runtime_error("< 0|无法进行类型转换[XDATATYPE_INT64->uint64_t]:值超出范围");
			}
			ui64Value = i64;
			break;
		}
		case XDATATYPE_UINT64:
		{
			memcpy(&ui64Value, m_pValue, sizeof(uint64_t));
			break;
		}
		case XDATATYPE_STRING:
		{
			ui64Value = convert<uint64_t>(*(string *)m_pValue);
			break;
		}
		default:
		{
			throw runtime_error("|无法进行类型转换[->uint64_t]");
			break;
		}
		}

		return ui64Value;
	}

	XDataValue:: operator float()
	{
		float fValue = 0;
		switch (m_xDataType)
		{
		case XDATATYPE_FLOAT:
			memcpy(&fValue, m_pValue, sizeof(float));
			break;
		default:
			throw runtime_error("|无法进行类型转换[->float]");
			break;
		}

		return fValue;
	}

	XDataValue::operator double()
	{
		double dValue = 0;
		switch (m_xDataType)
		{
		case XDATATYPE_FLOAT:
		{
			float f = 0;
			memcpy(&f, m_pValue, sizeof(float));
			dValue = f;
			break;
		}
		case XDATATYPE_DOUBLE:
		{
			memcpy(&dValue, m_pValue, sizeof(double));
			break;
		}
		case XDATATYPE_INT32:
		{
			int32_t i32 = 0;
			memcpy(&i32, m_pValue, sizeof(int32_t));
			dValue = i32;
			break;
		}
		case XDATATYPE_UINT32:
		{
			uint32_t ui32 = 0;
			memcpy(&ui32, m_pValue, sizeof(uint32_t));
			dValue = ui32;
			break;
		}
		case XDATATYPE_INT64:
		{
			int64_t i64 = 0;
			memcpy(&i64, m_pValue, sizeof(int64_t));
			dValue = (double)i64;
			break;
		}
		case XDATATYPE_UINT64:
		{
			uint64_t ui64 = 0;
			memcpy(&ui64, m_pValue, sizeof(uint64_t));
			dValue = (double)ui64;
			break;
		}
		default:
			throw runtime_error("|无法进行类型转换[->double]");

		}

		return dValue;
	}

	XDataValue::operator string()
	{
		switch (m_xDataType)
		{
		case XDATATYPE_STRING:
			return *(string *)m_pValue;
		default:
			throw runtime_error("|无法进行类型转换[->string]");
		}
	}

	XDataValue::operator XDataRaw()
	{
		switch (m_xDataType)
		{
		case XDATATYPE_RAW:
			return XDataRaw(((XDataRaw *)m_pValue)->m_pData, ((XDataRaw *)m_pValue)->m_szSize, 0);//提高效率，这里不复制整个Raw的数据部分
		default:
			throw runtime_error("|无法进行类型转换[->string]");
		}
	}

	XDataValue::operator XDataTime()
	{
		switch (m_xDataType)
		{
		case XDATATYPE_TIME:
			return *((XDataTime *)m_pValue);
		default:
			throw runtime_error("|未知的数据类型!无法进行日期转换");
		}
	}

	XDataColumn::XDataColumn()
	{
		m_xDataType = XDATATYPE_UNKNOW;
		m_szSize = 0;
		m_iIsNullable = 0;
	}

	XDataRow::XDataRow()
	{
		m_pTable = NULL;
	}

	XDataRow::~XDataRow()
	{
		for (int i = 0; i < (int)m_vItems.size(); i++)
		{
			RELEASE(m_vItems[i]);
		}
		m_vItems.clear();
	}

	XDataValue & XDataRow::operator[](string strName)
	{
		if(m_pTable == NULL)
			throw runtime_error("XDataRow未绑定XDataTable，不能用列名索引");
		toUpper(strName);
		map<string, int>::iterator it = m_pTable->m_mapColumnIndex.find(strName);
		if (it == m_pTable->m_mapColumnIndex.end())
		{
			string strErrMsg = "无法在该行中找到数据字段:" + strName;
			throw runtime_error(strErrMsg.c_str());
		}

		return *(m_vItems[it->second]);
	}

	XDataValue & XDataRow::operator[](int iIndex)
	{
		if ((int)m_vItems.size() <= iIndex)
		{
			char cMsg[64];
			sprintf(cMsg, "行中Index过大: size[%d], index[%d]", (int)m_vItems.size(), iIndex);
			XLogClass::warn(cMsg);
			throw runtime_error(cMsg);
		}
		return *(m_vItems[iIndex]);
	}

	XDataValue & XDataRow::GetItem(int iIndex)
	{
		return this->operator[](iIndex);
	}

	void XDataRow::AddValue(XDataValue *pValue)
	{
		m_vItems.push_back(pValue);
	}

	size_t XDataRow::ItemCount()
	{
		return m_vItems.size();
	}


	XDataTable::XDataTable()
	{
	}

	XDataTable::~XDataTable(void)
	{
		Dispose();
	}

	void XDataTable::Dispose()
	{
		for (int i = 0; i < (int)m_vColumns.size(); i++)
		{
			RELEASE(m_vColumns[i])
		}
		m_vColumns.clear();

		for (int i = 0; i < (int)m_vRows.size(); i++)
		{
			RELEASE(m_vRows[i])
		}
		m_vRows.clear();
	}

	XDataColumn &XDataTable::GetColumn(int iColIndex)
	{
		if (iColIndex >= (int)m_vColumns.size())
		{
			stringstream ss;
			ss << "数据表中没有该列:" << iColIndex;
			throw runtime_error(ss.str().c_str());
		}
		return *(m_vColumns[iColIndex]);
	}

	XDataRow &XDataTable::GetRow(int iRowIndex)
	{
		if (iRowIndex >= (int)m_vRows.size())
		{
			stringstream ss;
			ss << "数据表中没有该行:" << iRowIndex;
			throw runtime_error(ss.str().c_str());
		}
		return *(m_vRows[iRowIndex]);
	}

	XDataRow &XDataTable::operator[](int iRowIndex)
	{
		return GetRow(iRowIndex);
	}

	void XDataTable::AddColumn(XDataColumn *pColumn)
	{
		m_vColumns.push_back(pColumn);
		m_mapColumnIndex[pColumn->m_strName] = m_vColumns.size() - 1;
	}

	void XDataTable::AddRow(XDataRow *pRow)
	{
		pRow->m_pTable = this;
		m_vRows.push_back(pRow);
	}

	XDataRow & XDataTable::NewRow()
	{
		XDataRow *pRow = new XDataRow();
		pRow->m_pTable = this;
		m_vRows.push_back(pRow);
		return *pRow;
	}

	size_t XDataTable::RowCount()
	{
		return m_vRows.size();
	}

	tm TimeXToTM(const XDataTime &xDataTime)
	{
		tm sTM;
		sTM.tm_year = xDataTime.iYear - 1900;
		sTM.tm_mon = xDataTime.iMonth - 1;
		sTM.tm_mday = xDataTime.iDay;
		sTM.tm_hour = xDataTime.iHour;
		sTM.tm_min = xDataTime.iMinute;
		sTM.tm_sec = xDataTime.iSecond;
		return sTM;
	}

	time_t TimeXToT(const XDataTime &xDataTime)
	{
		tm sTM = TimeXToTM(xDataTime);
		return mktime(&sTM);
	}

	XDataTime TimeTMToX(const tm &sTM)
	{
		XDataTime xDataTime;
		xDataTime.iYear  = sTM.tm_year + 1900;
		xDataTime.iMonth = sTM.tm_mon + 1;
		xDataTime.iDay = sTM.tm_mday;
		xDataTime.iHour = sTM.tm_hour;
		xDataTime.iMinute = sTM.tm_min;
		xDataTime.iSecond = sTM.tm_sec;

		return xDataTime;
	}

	XDataTime TimeTToX(const time_t tTime)
	{
		tm sTM;
		TimeTToTM(tTime, sTM);
		return TimeTMToX(sTM);
	}
}
