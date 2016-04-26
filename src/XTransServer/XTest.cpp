#include "StdAfx.h"
#include "XTest.h"

int TestFunction()
{
	//XDataBase *pXDataBase;
	//GetDataBase("wyzfd", pXDataBase);

	//XDbTransaction xDbTrans;
	//while (pXDataBase->BeginTransaction(xDbTrans) != X_SUCCESS)
	//{
	//	uv_sleep(100);
	//}


	//string strOpenId = "oXkS6sxk9AGC-O2HEXLBxlCPl8m0";
	//string strSql = "select rownum as rid, t2.district_id, t1.* from tbl_house_resource t1, tbl_district t2 "
	//	"where t1.openid = :v_openid<char[65]> and t1.district = t2.district_name order by t2.district_id, t1.resource_id";
	//XDataRow xdrQuery;
	//xdrQuery.AddValue(new XDataValue(strOpenId));

	//XDataTable xdtInfo;
	//int iResult = xDbTrans.Query(strSql, xdrQuery, xdtInfo, 10);
	//if (iResult != X_SUCCESS)
	//{
	//	XLogClass::error("xDbTrans.Query iResult[%08X]", iResult);
	//	return X_FAILURE;
	//}

	//Json::Value jvInfo;
	//jvInfo["TotalCount"] = xdtInfo.RowCount();

	//Json::Value jvDistrict;
	//int64_t i64Temp = -1;
	//for (int i = 0; i < xdtInfo.RowCount(); i++)
	//{
	//	Json::Value jvHouseResource;
	//	int64_t i64DistrictId = xdtInfo[i]["district_id"];
	//	string strDistrictName = (string)xdtInfo[i]["district"];

	//	jvHouseResource["RId"] = (int)xdtInfo[i]["rid"];
	//	jvHouseResource["ResourceId"] = (int)xdtInfo[i]["resource_id"];
	//	jvHouseResource["OpenId"] = (string)xdtInfo[i]["openid"];
	//	jvHouseResource["District"] = (string)xdtInfo[i]["district"];
	//	jvHouseResource["Plate"] = (string)xdtInfo[i]["plate"];
	//	jvHouseResource["Community"] = (string)xdtInfo[i]["community"];
	//	jvHouseResource["Address"] = (string)xdtInfo[i]["address"];

	//	jvHouseResource["Room"] = (int)xdtInfo[i]["room"];
	//	jvHouseResource["Hall"] = (int)xdtInfo[i]["hall"];
	//	jvHouseResource["Toilet"] = (int)xdtInfo[i]["toilet"];
	//	jvHouseResource["Kitchen"] = (int)xdtInfo[i]["kitchen"];
	//	jvHouseResource["Balcony"] = (int)xdtInfo[i]["balcony"];
	//	jvHouseResource["Area"] = (int)xdtInfo[i]["area"];
	//	jvHouseResource["Floor"] = (int)xdtInfo[i]["current_floor"];
	//	jvHouseResource["TotalFloor"] = (int)xdtInfo[i]["total_floor"];
	//	jvHouseResource["Toward"] = (string)xdtInfo[i]["toward"];

	//	jvHouseResource["Decorate"] = (int)xdtInfo[i]["decorate"];
	//	jvHouseResource["Rent"] = (int)xdtInfo[i]["rent_expect"];
	//	jvHouseResource["Status"] = (int)xdtInfo[i]["status_input"];
	//	jvHouseResource["RentDue"] = (string)xdtInfo[i]["due_input"];
	//	jvHouseResource["LandlordName"] = (string)xdtInfo[i]["landlordname"];
	//	jvHouseResource["PhoneNumber"] = (string)xdtInfo[i]["phonenumber"];
	//	jvHouseResource["IsUnique"] = (int)xdtInfo[i]["isunique"];
	//	jvHouseResource["PhotoIds"] = (string)xdtInfo[i]["photoids"];

	//	if (i64Temp != i64DistrictId)
	//	{
	//		i64Temp = i64DistrictId;
	//		if (!jvDistrict.isNull())
	//		{
	//			jvInfo["Districts"].append(jvDistrict);
	//			jvDistrict.clear();
	//		}
	//	}

	//	if (jvDistrict.empty())
	//	{
	//		jvDistrict["DistrictId"] = (int)i64DistrictId;
	//		jvDistrict["DistrictName"] = strDistrictName;
	//	}

	//	jvDistrict["HouseResources"].append(jvHouseResource);
	//}
	//if (!jvDistrict.empty())
	//	jvInfo["Districts"].append(jvDistrict);

	//string strInfo = jvInfo.toStyledString();
	//XLogClass::debug("strInfo[%s]", strInfo.c_str());
	return X_SUCCESS;
}

////数据库 date，timstamp，blob，clob操作测试
//int TestFunction()
//{
//	XDataBase *pDb;
//	GetDataBase("test15", pDb);
//
//	XDataTable xdt;
//	int64_t i64Begin = uv_hrtime();
//	pDb->Query("select * from tbl_log_transaction_info where rownum <= 10000", xdt, 10000);
//	printf("interval1 [%lld]", uv_hrtime() - i64Begin);
//	////tbl_chtest(col1 date, col2 timestamp, col3 blob, col4 clob)
//	XDataTable xdt1;
//	pDb->Query("select * from tbl_chtest", xdt1, 10000);
//
//	XDataRaw d1 = xdt1[0][2];
//	XDataRaw d2 = xdt1[0][3];
//
//	XDbTransaction xDbTrans;
//	pDb->BeginTransaction(xDbTrans);
//
//	XDataTable xdt2;
//	XDataRow *pXdr = new XDataRow();
//	XDataTime dtime1, dtime2;
//
//	time_t t1 = time(NULL);
//	dtime1 = TimeTToX(t1);
//	dtime2 = dtime1;
//
//	pXdr->AddValue(new XDataValue(dtime1));
//	pXdr->AddValue(new XDataValue(dtime2));
//
//	BYTE bData[16] = { 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6 };
//	XDataRaw dRaw1(bData, 16, 0);
//	XDataRaw dRaw2((BYTE *)"12345", 5, 0);
//	
//	pXdr->AddValue(new XDataValue(dRaw1));
//	pXdr->AddValue(new XDataValue(dRaw2));
//
//	xdt2.AddRow(pXdr);
//
//	pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col3,col4) values(:col1<timestamp>, :col2<timestamp>, empty_blob(), empty_clob()) "
//		"returning col3, col4 into :col3<blob>, :col4<clob>", xdt2);
//	//pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col4) values(:col1<timestamp>, :col2<timestamp>, empty_clob()) returning col4 into :col4<clob>", xdt2);
//	//pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col3,col4) values(:v1<timestamp>, :v2<timestamp>, :v3<blob>, :v4<clob>)", xdt2);
//	xDbTrans.Commit();
//
//	return 0;
//}

//数据库连接池分配效率测试
//class XTestClass
//{
//public:
//	XTestClass()
//	{
//		m_isUsed = false;
//		m_hThread = 0;
//		m_ui64LastUseTime = 0;
//	};
//	bool m_isUsed;
//	uv_thread_t m_hThread;
//	uint64_t m_ui64LastUseTime;
//};
//
//vector<XTestClass *> m_vxTest;
//list<XTestClass *> m_lxTestUnUsed;
//list<XTestClass *> m_lxTestUsed;
//
//int GetFree(int iCount)
//{
//	for (int i = 0; i < iCount; i++)
//	{
//		if (!m_vxTest[i]->m_isUsed)
//		{
//			m_vxTest[i]->m_isUsed = true;
//			return 0;
//		}
//	}
//	return -1;
//}
//
//int FreeUsed(int iIndex)
//{
//	m_vxTest[iIndex]->m_isUsed = false;
//	return 0;
//}
//
//int GetFreeX(int iCount)
//{
//	if(m_lxTestUnUsed.size() == 0)
//		return -1;
//	XTestClass *p = *m_lxTestUnUsed.begin();
//	m_lxTestUnUsed.pop_front();
//	m_lxTestUsed.push_back(p);
//	return 0;
//}
//
//int FreeUsedX(int iIndex)
//{
//	m_lxTestUsed.remove(m_vxTest[iIndex]);
//	m_lxTestUnUsed.push_back(m_vxTest[iIndex]);
//	return 0;
//}
//
//int TestFunction()
//{
//	int iTotalCount = 100;
//	for (int i = 0; i < iTotalCount; i++)
//	{
//		XTestClass *p = new XTestClass();
//		m_vxTest.push_back(p);
//	}
//	for (int i = 0; i < iTotalCount; i++)
//	{
//		m_lxTestUnUsed.push_back(m_vxTest[i]);
//	}
//
//	int64_t i64Begin;
//
//	i64Begin = uv_hrtime();
//	for (int n = 0; n < 10000; n++)
//	{
//		for (int i = 0; i < iTotalCount; i++)
//		{
//			GetFreeX(iTotalCount);
//		}
//		for (int i = 0; i < iTotalCount; i++)
//		{
//			FreeUsedX(i);
//		}
//	}
//	printf("NEW %lld\n", uv_hrtime() - i64Begin);
//
//	i64Begin = uv_hrtime();
//	for (int n = 0; n < 10000; n++)
//	{
//		for (int i = 0; i < iTotalCount; i++)
//		{
//			GetFree(iTotalCount);
//		}
//		for (int i = 0; i < iTotalCount; i++)
//		{
//			FreeUsed(i);
//		}
//	}
//
//	printf("OLD %lld\n", uv_hrtime() - i64Begin);
//	return 0;
//}

//windows 下取时间函数测试 SYSTEMTIME
//int TestFunction()
//{
	//SYSTEMTIME  sTime;
	//FILETIME fTime;
	//::GetSystemTimeAsFileTime(&fTime);
	//char buf[64] = {0};
	//DWORD dwLow = fTime.dwLowDateTime;
	//DWORD dwHigh = fTime.dwHighDateTime;
	//uint64_t ui64Time = ((uint64_t)dwHigh << 32) + dwLow;
	//uint64_t ui64Time1 = ui64Time + 0xfe624e212ac18000;
	//
	//dwLow = ui64Time1 & 0x00000000ffffffff;
	//dwHigh = ui64Time1 >> 32;
	//
	//FileTimeToSystemTime(&fTime, &sTime);
	//sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
	//sTime.wYear = 1980;
	//sTime.wMonth = 1;
	//sTime.wDay = 1;
	//sTime.wHour = 0;
	//sTime.wMinute = 0;
	//sTime.wSecond = 0;
	//sTime.wMilliseconds = 0;
	//SystemTimeToFileTime(&sTime, &fTime);
	//FileTimeToSystemTime(&fTime, &sTime);
	//uint32_t x = 0x01ced695^0x4f294920;
	//
	//sprintf(buf, "%4d-%02d-%02d %02d:%02d:%2d", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond); 
	//
	//return X_SUCCESS;
//}
