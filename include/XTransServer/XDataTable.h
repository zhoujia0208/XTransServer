#pragma once
#ifndef __XDATATABLE_H__
#define __XDATATABLE_H__

namespace XData {

	enum XDataType
	{
		XDATATYPE_UNKNOW = 0,
		XDATATYPE_FLOAT,
		XDATATYPE_DOUBLE,
		XDATATYPE_INT32,
		XDATATYPE_UINT32,
		XDATATYPE_INT64,
		XDATATYPE_UINT64,
		XDATATYPE_STRING,
		XDATATYPE_RAW,
		XDATATYPE_TIME,
	};

	class XDataRaw
	{
	public:
		XDataRaw();
		~XDataRaw();
		void Dispose();

		XDataRaw(const BYTE *pData, size_t szSize, int iCopyFlag = 1);
		void SetData(const BYTE *pData, size_t szSize, int iCopyFlag = 1);
		void SetDataByString(const string &sString);

		XDataRaw(const string &sHex);
		void SetDataByHexString(const string &sHex);


		

		XDataRaw(const XDataRaw &x);
		XDataRaw &operator =(const XDataRaw &x);

		BYTE *GetData();
		size_t GetSize();
		string GetDataAsString();
		string GetDataAsHexString();
		

		friend class XDataValue;
	private:
		void _Dispose();
		void _SetData(const BYTE *pData, size_t szSize, int iCopyFlag);
		void _SetDataByHexString(const string &sHex);

		BYTE *m_pData;
		size_t m_szSize;
		int m_iCopyFlag;
	};

	class XDataTime
	{
	public:
		XDataTime();
		int iYear;
		int iMonth;
		int iDay;
		int iHour;
		int iMinute;
		int iSecond;
		uint64_t uiFraction;
		int iFracPrec;
		int iTzHour;
		int iTzMinute;
	};

	class XDataValue
	{
	public:
		XDataValue();
		~XDataValue();
		void Dispose();

		XDataValue(const XDataValue &x);
		XDataValue &operator =(const XDataValue &x);

		XDataValue(XDataType xDataType);
		XDataValue(const int32_t &i32Value);
		XDataValue &operator =(const int32_t &i32Value);
		XDataValue(const uint32_t &ui32Value);
		XDataValue &operator =(const uint32_t &ui32Value);
		XDataValue(const int64_t &i64Value);
		XDataValue &operator =(const int64_t &i64Value);
		XDataValue(const uint64_t &ui64Value);
		XDataValue &operator =(const uint64_t &ui64Value);
		XDataValue(const float &fValue);
		XDataValue &operator =(const float &fValue);
		XDataValue(const double &dValue);
		XDataValue &operator =(const double &dValue);
		XDataValue(const string &strValue);
		XDataValue &operator =(const string &strValue);
		XDataValue(const char *pValue);
		XDataValue &operator =(const char *pValue);

		void SetRaw(const BYTE *pValue, size_t szLen, int iCopyFlag = 1);
		XDataValue(const XDataRaw &xRawValue);
		XDataValue &operator =(const XDataRaw &xRawValue);

		void SetTime(const XDataTime &xDataTime);
		XDataValue(const XDataTime &xDataTime);
		XDataValue &operator =(const XDataTime &xDataTime);

		void *GetDataPtr();
		int IsNull();
		void SetNull(int iIsNull);
		XDataType GetDataType();

		operator int32_t();
		operator uint32_t();
		operator int64_t();
		operator uint64_t();
		operator float();
		operator double();
		operator string();
		operator XDataRaw();
		operator XDataTime();

		//为了效率，这里只能public，让DataBaseConnection的继承类可以直接访问
		//但是业务代码中注意，尽量不要直接访问以下变量
		friend class XDataRow;
	private:
		void _Dispose();
		void _Copy(const XDataValue &x);
		void _SetInt32(const int32_t &i32Value);
		void _SetUInt32(const uint32_t &ui32Value);
		void _SetInt64(const int64_t &i64Value);
		void _SetUInt64(const uint64_t &ui64Value);
		void _SetFloat(const float &fValue);
		void _SetDouble(const double &dValue);
		void _SetString(const char *pValue);
		void _SetRaw(const BYTE *pValue, size_t szLen, int iCopyFlag = 1);
		void _SetTime(const XDataTime &xDataTime);

		size_t m_szSize;
		XDataType m_xDataType;
		//string m_strName;
		int m_iIsNull;
		void *m_pValue;
	};

	class XDataColumn
	{
	public:
		XDataColumn();
		string m_strName;
		XDataType m_xDataType;
		size_t m_szSize;
		int m_iIsNullable;
	};

	class XDataTable;
	class XDataRow
	{
	public:
		XDataRow();
		~XDataRow();

		XDataValue & operator[](string strName);
		XDataValue & operator[](int iIndex);
		XDataValue & GetItem(int iIndex);
		void AddValue(XDataValue *pValue);
		size_t ItemCount();

		friend class XDataTable;
	private:
		vector<XDataValue *> m_vItems;
		XDataTable *m_pTable;
	};

	class XDataTable
	{
	public:
		XDataTable();
		~XDataTable();
		void Dispose();
		XDataColumn & GetColumn(int iColIndex);

		XDataRow & GetRow(int iRowIndex);
		XDataRow & operator[](int iRowIndex);
		
		void AddColumn(XDataColumn *pColumn);
		void AddRow(XDataRow *pRow);
		XDataRow & NewRow();
		size_t RowCount();

		friend class XDataRow;
	private:
		//重名的字段要注意，会保存为最后一个同名字段列的信息
		map<string, int> m_mapColumnIndex;
		vector<XDataColumn *> m_vColumns;
		vector<XDataRow *> m_vRows;
	};

	tm TimeXToTM(const XDataTime &xDataTime);
	time_t TimeXToT(const XDataTime &xDataTime);
	XDataTime TimeTMToX(const tm &sTM);
	XDataTime TimeTToX(const time_t tTime);
}
#endif //__XDATATABLE_H__
