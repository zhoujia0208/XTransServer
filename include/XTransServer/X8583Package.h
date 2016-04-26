#pragma once

#ifndef __X8583PACKAGE_H__
#define __X8583PACKAGE_H__

#include <string>

using namespace std;
enum BitmapType
{
	Bitmap64 = 64,
	Bitmap128 = 128,
};

class X8583Value
{
public:
	X8583Value(void)
	{
		iMaxLength = 0;

		pData = NULL;
		iDataLength = 0;
	};

	~X8583Value(void)
	{
		Release();
	};
	int SetData(const unsigned char *pInData, int iLength);
	void Release()
	{
		if(pData != NULL)
		{
			free(pData);
			pData = NULL;
		}
	};
	string strFieldName;
	string strType;
	string strFormat;
	string strLengthType;
	int iMaxLength;


private:
	unsigned char *pData;
	int iDataLength;
};

class X8583Package
{

public:
	X8583Package(BitmapType btType = Bitmap64);
	~X8583Package(void);

	int SetConfig();

	int SetData(int iIndex, const unsigned char *pData, int iLength);

	int MakePackage(unsigned char *&pData, int &iLength);
	int ParsePackage(const unsigned char *pData, int iLength);

	int ReleaseData();//清空数据，保留配置信息
private:
	BitmapType bitmapType;//默认为 Bitmap64
	X8583Value *x8583Value;//0域 放报文头Head相关数据

	int iLengthLen;//报文长度所占用的字节数
	string strLengthType;//报文长度的表示类型
};

#endif //__X8583PACKAGE_H__
