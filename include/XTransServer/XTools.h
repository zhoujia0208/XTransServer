#pragma once
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <sstream>
#include <string>
#include <list>
#include <iostream>
#include <cctype>
#include <algorithm>

using namespace std;


enum ByteOrder
{
	NetWork,
	Reverse,
};

//该函数在指针为空的时候会有异常，慎用
template<class T_out,class T_in>
T_out convert(const T_in & t)
{
	stringstream ss;

	ss << t;
	T_out result;
	ss >> result;

	return result;
}

int GetLengthByType(BYTE *pBuffer, int iOffset, int iLength, XConfig::XLengthType xType, uint64_t &ui64Length);

int GetStringByType(BYTE *pBuffer, int iOffset, int iLength, XConfig::XLengthType xType, string &strResult);

bool isAllNumberInString(string str);
string UrlEncode(const string &strSource);
string UrlDecode(const string &strSource);
int HttpParseQueryString(const string &strQueryString, map<string, string> &mapQueryString);

int GetRandomNumber(BYTE *bRandom, int iLength);
int CreateDir(string strDir);

//字符串操作
list<string> split(const string &str, const string &s, bool bRemoveSpace);
string toLower(string &str);
string toUpper(string &str);
int replace_all(string& str,const string& pattern, const string& newpat);
string trim(string& s);
string ltrim(string& s);
string rtrim(string& s);

void ByteToString(const BYTE *bBuffer, int32_t i32Length, char *pStr);
void ByteToString(const BYTE *bBuffer, int32_t i32Length, string &s);
void StringToByte(const char *pStr, int32_t i32Length, BYTE *bBuffer);
void StringToByte(const char *pStr, BYTE *bBuffer);
void uv_sleep(int msec);//自己写uv_sleep

//算法函数
void DesEcbEncrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey);
void DesEcbDecrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey);
void DesEcb3DoubleEncrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey);
void DesEcb3DoubleDecrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey);

void XOR(const BYTE *input1, const BYTE *input2, BYTE *output, int len);
int MacArithmetic(BYTE *sMacKey, BYTE *pInData, int iInLen, BYTE *pRetData, BYTE bFill, int iFlag);
int MacArithmetic(BYTE *sMacKey, BYTE *pInitVector, BYTE *pInData, int iInLen, BYTE *pRetData, BYTE bFill, int iFlag);
int MakeCVN(BYTE *sKey, BYTE *pInData, int iInLen, BYTE *pRetData);
int MakeDes3S(BYTE *pInData, int iInLength, BYTE *pOutData, int &iOutLength, BYTE *bKey, BYTE bType, BYTE bFill = 0x00);
int MakeDes3SCBC(BYTE *pInitVecter, BYTE *pInData, int iInLength, BYTE *pOutData, int &iOutLength, BYTE *bKey, BYTE bType, BYTE bFill = 0x00);
void CRC32(BYTE *DataBuf, int nLength, BYTE *CRC);
void ScatterKey(BYTE *bRootKey, BYTE *bInData, BYTE *bOutData, int iCount);
void ScatterKey16(BYTE *bRootKey, BYTE *bInData, BYTE *bOutData, int iCount);
int ComputeMD5(BYTE *pInData, int iInLength, char *pOutData);
int ComputeMD5(char *cFileName, char *pOutData);

#ifndef _WIN32
uint64_t htonll(uint64_t uiHost);
uint64_t ntohll(uint64_t uiNet);
#endif

//字符编码转换
int CodeConvert(const string &strFromCharset, const string &strToCharset, const string &strIn, string &strOut);
string UTF8ToGBK(const string &strUTF8);
string GBKToUTF8(const string &strGBK);

//获取时间
string GetLocalDate();
string GetLocalTime();
int TimeTToString(const time_t tTime, string &strTime);
int TimeTToTM(const time_t tTime, struct tm &sTM);
int TimeTToHttpString(const time_t tTime, string &strLongTime);

int Base64Encode(BYTE *pInBuffer, int iInLength, string &strResult);
int Base64Decode(string &strInData, BYTE *&pOutData, int &iOutLength);

int Hash_sha1(BYTE *pInData, int iInLen, BYTE *pOutData, int &iOutLen);
#endif //__TOOLS_H__
