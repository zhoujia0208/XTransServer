#include "StdAfx.h"
#include "XTest.h"
#include <openssl/rsa.h>
#include <openssl/evp.h>  
#include <openssl/x509.h>
#include <openssl/bn.h>
#include <openssl/pem.h>

//typedef struct RSA_KEY
//{
//	uint64_t e;
//	char cModulus[256 + 1];
//	char cPrivateExp[256 + 1];
//} _RSA_KEY;
//
//int MakeRSA(uint64_t e, const char *cModulus, const char *cPrivateExp, RSA *rsa)
//{
//	//构建RSA数据结构
//	BIGNUM *bn_n = BN_new();
//	BIGNUM *bn_e = BN_new();
//	BIGNUM *bn_d = BN_new();
//
//	BN_hex2bn(&bn_n, cModulus);
//	BN_set_word(bn_e, (unsigned long)e);
//	BN_hex2bn(&bn_d, cPrivateExp);
//
//	rsa->n = bn_n; // public modulus
//	rsa->e = bn_e; // public exponent
//	rsa->d = bn_d; // private exponent
//	//RSA_print_fp(stdout, rsa, 5);
//
//	return X_SUCCESS;
//}
//int RSA_EncodeByPublic(RSA *rsa, unsigned char *pInData, int iInLen, unsigned char *pOutData, int &iOutLen)
//{
//	//iInLen = RSA_size(rsa);//strlen((char *)in);
//	int iResult = RSA_public_encrypt(iInLen, pInData, pOutData, rsa, RSA_PKCS1_PADDING);//RSA_NO_PADDING
//	if (iResult < 0)
//	{
//		XLogClass::warn("RSA_public_encrypt failed! iResult=[%d]", iResult);
//		return iResult;
//	}
//	iOutLen = iResult;
//
//	return X_SUCCESS;
//}
//
//int RSA_DecodeByPrivate(RSA *rsa, unsigned char *pInData, int iInLen, unsigned char *pOutData, int &iOutLen)
//{
//	//准备输出的解密数据结构
//	int iResult = RSA_private_decrypt(iInLen, pInData, pOutData, rsa, RSA_PKCS1_PADDING);//RSA_NO_PADDING
//	if (iResult < 0)
//	{
//		XLogClass::warn("RSA_private_decrypt failed! iResult=[%d]", iResult);
//		return iResult;
//	}
//	iOutLen = iResult;
//
//	return X_SUCCESS;
//}
//
//int TestFunction()
//{
//	RSA_KEY rsaKey = { 0x010001,
//		"90CB4CFBEE67B2C523F5C5E014E4AF36D201AB6E0DF951340083C8CD4B3C494B0E0B20A8C8EB3C98A7970B1415DADF6CB4185BA1C4364995971A1531159D5214FD15708231282C1A3FA56BC5B725F1F359037705FF91A1CBED4DB71D31BE5ABD2AEDE061BD261FCB816F92C1D3A0B8235B131AC619839067DD92381B3D7D591F",
//		"830CBE5CB6E67C5319BCF49F5FC9AB1BFEEF2BEDAC101F82E58461560DE63EA003D1E6795D802B942EFAF237F79FE5E0491052BF130BDF85D900EE75428EE51E5E63B25F66CB85F0593AE83580E6230E576DCBB947385EFED1A23FFF7BBEE30851DBAD2DF26838E8F7FEE274938606238F13267998D0A7F1B3E99F4332C84461"
//	};
//	RSA *rsa = RSA_new();
//	MakeRSA(rsaKey.e, rsaKey.cModulus, rsaKey.cPrivateExp, rsa);
//
//	char *cData = "iamchenghao";
//	byte bEncodedData[128];
//	int iEncodedLen = 0;
//	RSA_EncodeByPublic(rsa, (byte *)cData, strlen(cData), bEncodedData, iEncodedLen);
//	string sEncodedData;
//	ByteToString(bEncodedData, iEncodedLen, sEncodedData);
//	
//	byte bDecodedData[128];
//	int iDecodedLen = 0;
//	RSA_DecodeByPrivate(rsa, bEncodedData, iEncodedLen, bDecodedData, iDecodedLen);
//	RSA_free(rsa);
//	return 0;
//}

////互联网闸机项目测试代码
////////////计算MAC////////
//int calcMacDemo() {
//	//消费密钥
//	byte bCosumeKey[16];
//	StringToByte("DEEA73C9A049E91D9ADD1F2AD8C5C7E5", bCosumeKey);
//
//	//需要计算的数据
//	string sMacData = "0001111A0A591AB79201261564591AC5A22C01581B9001";
//	int iMacLen = sMacData.size() / 2;
//	byte bMacData[128];
//	StringToByte(sMacData.c_str(), bMacData);
//
//	byte bVector[8] = { 0,0,0,0,0,0,0,0 };
//	byte bMac[8];
//	//MAC计算，下面得到的bMac的前4个字节就是MAC。
//	MacArithmetic(bCosumeKey, bVector, bMacData, iMacLen, bMac, 0, 1);
//	return 0;
//}
//
//
//////////////计算验证码//////
//int calcValidDemo() {
//	char test[128];
//	StringToByte("687474703A2F2F77656978696E2E71712E636F6D2F722F4A6F526552726A45475F566B7266776C39394530", (byte *)test);
//
//
//	//消费密钥
//	byte bCosumeKey[16];
//	//StringToByte("DEEA73C9A049E91D9ADD1F2AD8C5C7E5", bCosumeKey);
//	StringToByte("DEEA73C9A049E91D9ADD1F2AD8C5C7E5", bCosumeKey);
//
//	//分散因子
//	byte bScatterFator[8];
//	//StringToByte("591AB180000000F0", bScatterFator);
//	StringToByte("59296CC4000000F0", bScatterFator);
//
//	//分散得到过程密钥
//	byte bProcessKey[16];
//	ScatterKey(bCosumeKey, bScatterFator, bProcessKey, 1);
//
//	//计算验证码的数据
//	//string sValidData = "0011223344556606222760021200000016A56396F144332211FFFFD20001111A0A591AB79201261564591AC5A22C01581B00009001EB4C26BF000A000000000000000000591AC698591AB180000000F0";
//	string sValidData = "00E7066E5CA89806222760021200000016A56396F100000000FFFFD2000000000000000000051140095929C323060000000000700700000000000400000000000000000059296D3C59296CC4000000F0";
//	int iValidLen = sValidData.size() / 2;
//	byte bValidData[128];
//	StringToByte(sValidData.c_str(), bValidData);
//	byte bVector[8] = { 0,0,0,0,0,0,0,0 };
//	byte bValid[8];
//	//验证码计算，下面得到的bValid的前4个字节就是验证码。
//	MacArithmetic(bProcessKey, bVector, bValidData, iValidLen, bValid, 0, 1);
//
//	return 0;
//}
//
//
//int TestFunction()
//{
//	BYTE bDataDES[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
//	BYTE bKeyDES[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
//	BYTE bResultDES[8];
//	DesEcbEncrypt(bDataDES, bResultDES, bKeyDES);
//
//	BYTE bData3DES[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
//	BYTE bKey3DES[16] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
//	BYTE bResult3DES[8];
//	DesEcb3DoubleEncrypt(bDataDES, bResult3DES, bKey3DES);
//
//	calcMacDemo();
//	calcValidDemo();
//
//	return 0;
//}

//struct PartStruct {
//	int iPartId;
//	int iDeviceId;
//	int iStatus;//0-ok;1-error
//};
//struct DeviceStruct {
//	int iDeviceId;
//	int iStationId;
//	int iStatus;
//	list<PartStruct *> lParts;
//};
//struct StationStruct {
//	int iStationId;
//	int iLineId;
//	int iStatus;
//	list<DeviceStruct *> lDevices;
//};
//struct LineStruct {
//	int iLineId;
//	int iStatus;
//	list<StationStruct *> lStations;
//};
//
//map<int, PartStruct *> mPart;
//map<int, DeviceStruct *> mDevice;
//map<int, StationStruct *> mStation;
//map<int, LineStruct *> mLine;
//
//int iInitStatus = 0;
//void InitStruct() {
//	for (int i = 0; i < 5; i++) {
//		LineStruct *ls = new LineStruct();
//		ls->iLineId = i;
//		ls->iStatus = iInitStatus;
//		mLine[ls->iLineId] = ls;
//		for (int j = 0; j < 20; j++) {
//			StationStruct *ss = new StationStruct();
//			ls->lStations.push_back(ss);
//			ss->iStationId = (ls->iLineId << 8) + j;
//			ss->iLineId = ls->iLineId;
//			ss->iStatus = iInitStatus;
//			mStation[ss->iStationId] = ss;
//			for (int k = 0; k < 50; k++) {
//				DeviceStruct *ds = new DeviceStruct();
//				ss->lDevices.push_back(ds);
//				ds->iDeviceId = (ss->iStationId << 8) + k;
//				ds->iStationId = ss->iStationId;
//				ds->iStatus = iInitStatus;
//				mDevice[ds->iDeviceId] = ds;
//				for (int n = 0; n < 20; n++) {
//					PartStruct *ps = new PartStruct();
//					ds->lParts.push_back(ps);
//					ps->iPartId = (ds->iDeviceId << 8) + n;
//					ps->iDeviceId = ds->iDeviceId;
//					ps->iStatus = iInitStatus;
//					mPart[ps->iPartId] = ps;
//				}
//			}
//		}
//	}
//}
//void JudgeUpdate(int iPartId, int iStatus) {
//	
//	map<int, PartStruct *>::iterator itmPS = mPart.find(iPartId);
//	if (itmPS == mPart.end())
//		return;
//	if (itmPS->second->iStatus == iStatus)
//		return;
//	itmPS->second->iStatus = iStatus;
//
//	//判断设备状态
//	if (iStatus != 0) {
//		DeviceStruct *ds = mDevice[itmPS->second->iDeviceId];
//		ds->iStatus = 1;
//		StationStruct *ss = mStation[ds->iStationId];
//		ss->iStatus = 1;
//		LineStruct *ls = mLine[ss->iLineId];
//		ls->iStatus = 1;
//		return;
//	}
//
//	int iXStatus = 0;
//	DeviceStruct *ds = mDevice[itmPS->second->iDeviceId];
//	list<PartStruct *> &lParts = ds->lParts;
//	for (list<PartStruct *>::iterator itlPS = lParts.begin(); itlPS != lParts.end(); itlPS++) {
//		if ((*itlPS)->iStatus == 0)
//			continue;
//		iXStatus = 1;
//		break;
//	}
//	if (iXStatus != 0) {
//		ds->iStatus = 1;
//		StationStruct *ss = mStation[ds->iStationId];
//		ss->iStatus = 1;
//		LineStruct *ls = mLine[ss->iLineId];
//		ls->iStatus = 1;
//		return;
//	}
//
//	//判断站状态
//	iXStatus = 0;
//	StationStruct *ss = mStation[ds->iStationId];
//	list<DeviceStruct *> &lDevices = ss->lDevices;
//	for (list<DeviceStruct *>::iterator itlDS = lDevices.begin(); itlDS != lDevices.end(); itlDS++) {
//		if ((*itlDS)->iStatus == 0)
//			continue;
//		iXStatus = 1;
//		break;
//	}
//	if (iXStatus != 0) {
//		ss->iStatus = 1;
//		LineStruct *ls = mLine[ss->iLineId];
//		ls->iStatus = 1;
//		return;
//	}
//
//	//判断线路状态
//	iXStatus = 0;
//	LineStruct *ls = mLine[ss->iLineId];
//	list<StationStruct *> &lStations = ls->lStations;
//	for (list<StationStruct *>::iterator itlSS = lStations.begin(); itlSS != lStations.end(); itlSS++) {
//		if ((*itlSS)->iStatus == 0)
//			continue;
//		iXStatus = 1;
//		break;
//	}
//	if (iXStatus != 0) {
//		ls->iStatus = 1;
//		return;
//	}
//
//}
//int TestFunction()
//{
//	//init
//	int64_t i64Begin = uv_hrtime();
//	InitStruct();
//	printf("InitStruct Time [%lld]\n", uv_hrtime() - i64Begin);
//
//	int64_t i64TotalTime = 0;
//	srand(time(NULL));
//	for (int x = 0; x < 1000; x++) {
//		int i = rand() % 5;
//		int j = rand() % 20;
//		int k = rand() % 50;
//		int n = rand() % 20;
//		int iStationId = (i << 8) + j;
//		int iDeviceId = (iStationId << 8) + k;
//		int iPartId = (iDeviceId << 8) + n;
//		int iStatus = rand() % 2;
//		i64Begin = uv_hrtime();
//		JudgeUpdate(iPartId, iStatus);
//		i64TotalTime += uv_hrtime() - i64Begin;
//	}
//
//	printf("1000s judgeUpdate Time [%lld]\n", i64TotalTime);
//	return 0;
//}

//int TestFunction()
//{
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
//	return X_SUCCESS;
//}

//数据库 date，timstamp，blob，clob操作测试
int TestFunction()
{
	XDataBase *pDb;
	GetDataBase("test15", pDb);

	//XDataTable xdt;
	//int64_t i64Begin = uv_hrtime();
	//pDb->Query("select * from tbl_log_transaction_info where rownum <= 10000", xdt, 10000);
	//printf("interval1 [%lld]", uv_hrtime() - i64Begin);
	//////tbl_chtest(col1 date, col2 timestamp, col3 blob, col4 clob)
	//XDataTable xdt1;
	//pDb->Query("select * from tbl_chtest", xdt1, 10000);

	//XDataRaw d1 = xdt1[0][2];
	//XDataRaw d2 = xdt1[0][3];

	XDbTransaction xDbTrans;
	pDb->BeginTransaction(xDbTrans);

	XDataTable xdt2;
	XDataRow *pXdr = new XDataRow();
	XDataTime dtime1, dtime2;

	time_t t1 = time(NULL);
	dtime1 = TimeTToX(t1);
	dtime2 = dtime1;

	pXdr->AddValue(new XDataValue(dtime1));
	pXdr->AddValue(new XDataValue(dtime2));

	BYTE bData[16] = { 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6 };
	XDataRaw dRaw1(bData, 70000, 0);
	XDataRaw dRaw2((BYTE *)"12345", 5, 0);
	
	pXdr->AddValue(new XDataValue(dRaw1));
	pXdr->AddValue(new XDataValue(dRaw2));

	xdt2.AddRow(pXdr);

	//pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col3) values(:col1<timestamp>, :col2<timestamp>,:v3<raw_long>)", xdt2);
	//pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col3,col4) values(:col1<timestamp>, :col2<timestamp>, empty_blob(), empty_clob()) "
	//	"returning col3, col4 into :col3<blob>, :col4<clob>", xdt2);
	//pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col4) values(:col1<timestamp>, :col2<timestamp>, empty_clob()) returning col4 into :col4<clob>", xdt2);
	pDb->ExecuteSql(xDbTrans, "insert into tbl_chtest(col1,col2,col3,col4) values(:v1<timestamp>, :v2<timestamp>, :v3<raw_long>, :v4<raw_long>)", xdt2);
	xDbTrans.Commit();

	return 0;
}

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
