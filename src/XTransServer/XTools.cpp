#include "StdAfx.h"
#include <memory.h>
#include <iconv.h>
#include <openssl/des.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "XTools.h"


static int iOrderXXX = 1;
static ByteOrder byteOrder = *((BYTE*)&iOrderXXX) != 0 ? Reverse : NetWork;

int GetLengthByType(BYTE *pBuffer, int iOffset, int iLength, XConfig::XLengthType xType, uint64_t &ui64Length)
{
	string strLength = "";
	if(xType == XConfig::XLENGTHTYPE_STRING)
	{
		strLength.append((char *)pBuffer, iOffset, iLength);
		//if(!isAllNumberInString(strLength))
		//	return X_FAILURE;
		ui64Length = atoll(strLength.c_str());
	}
	else if(xType == XConfig::XLENGTHTYPE_BCD)
	{
		ByteToString(pBuffer + iOffset, iLength, strLength);
		//if(!isAllNumberInString(strLength))
		//	return X_FAILURE;
		ui64Length = atoll(strLength.c_str());
	}
	else if(xType == XConfig::XLENGTHTYPE_BYTE)
	{
		if(iLength <= 8)
		{
			ui64Length = 0;
			if(byteOrder == Reverse)
			{
				//反转字节序
				for(int i = 0;i < iLength; i ++)
				{
					((BYTE*)&ui64Length)[iLength - 1 - i] = pBuffer[iOffset + i];
				}
			}
			else
			{
				memcpy(&ui64Length + 8 - iLength, pBuffer + iOffset, iLength);
			}
		}
		else
		{
			XLogClass::warn("GetLengthByType byte类型长度 > 8");
			return X_FAILURE;
		}
	}
	else
	{
		XLogClass::warn("GetStringByType 暂时不支持的转换类型：%d", xType);
		return X_FAILURE;
	}

	return X_SUCCESS;
}

int GetStringByType(BYTE *pBuffer, int iOffset, int iLength, XConfig::XLengthType xType, string &strResult)
{
	strResult = "";
	if(xType == XConfig::XLENGTHTYPE_STRING)
	{
		strResult.append((char *)pBuffer, iOffset, iLength);
	}
	else if(xType == XConfig::XLENGTHTYPE_BCD)
	{
		ByteToString(pBuffer + iOffset, iLength, strResult);
	}
	else if(xType == XConfig::XLENGTHTYPE_BYTE)
	{
		if(iLength <= 8)
		{
			uint64_t ui64 = 0;
			if(byteOrder == Reverse)
			{
				//反转字节序
				for(int i = 0;i < iLength; i ++)
				{
					((BYTE*)&ui64)[iLength - 1 - i] = pBuffer[iOffset + i];
				}
			}
			else
			{
				memcpy(&ui64 + 8 - iLength, pBuffer + iOffset, iLength);
			}
			strResult = convert<string>(ui64);
		}
	}
	else
	{
		XLogClass::warn("GetStringByType 暂时不支持的转换类型：%d", xType);
		return X_FAILURE;
	}

	return X_SUCCESS;
}

bool isAllNumberInString(string str)
{
	if(str.length() == 0)
		return false;
	for(int i = 0; i < (int)str.length(); i++)
	{
		if(str[i] < '0' || str[i] > '9')
			return false;
	}
	return true;
}

list<string> split(const string &str, const string &s, bool bRemoveSpace)
{
	list<string> liString;
	string::size_type i = 0;
	string::size_type j = 0;

	while(j < str.size())
	{
		i = j;
		j = str.find(s, j);
		if((int)j < 0)
			j = str.length();
		string strTemp = str.substr(i, j - i);
		if(bRemoveSpace)
			trim(strTemp);
		liString.push_back(strTemp);
		j += s.length();
	}
	return liString;
}

string toLower(string &str)
{
	//transform(str.begin(), str.end(), str.begin(), tolower);
	transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);
	return str;
}

string toUpper(string &str)
{
	//transform(str.begin(), str.end(), str.begin(), toupper);
	transform(str.begin(), str.end(), str.begin(), (int (*)(int))toupper);
	return str;
}

int replace_all(string& str,const string& pattern, const string& newpat)
{
	int count = 0;
	const size_t nsize = newpat.size();
	const size_t psize = pattern.size();

	for(size_t pos = str.find(pattern, 0); pos != string::npos; pos = str.find(pattern,pos + nsize))
	{
		str.replace(pos, psize, newpat);
		count++;
	}
	return count;
}

string trim(string& s)
{
	const string drop = " ";
	// trim right
	s.erase(s.find_last_not_of(drop)+1);
	// trim left
	return s.erase(0,s.find_first_not_of(drop));
}

string ltrim(string& s)
{
	const string drop = " ";
	// trim left
	return s.erase(0,s.find_first_not_of(drop));
}

string rtrim(string& s)
{
	const string drop = " ";
	// trim right
	return s.erase(s.find_last_not_of(drop)+1);
}

void ByteToString(const BYTE *bBuffer, int32_t i32Length, char *pStr)
{
	for(int32_t i32Count = 0; i32Count < i32Length; )
	{
		if(i32Length - i32Count >= 8)
		{
			sprintf(pStr + 2 * i32Count, "%016llX", htonll(*((uint64_t *)(&bBuffer[i32Count]))));
			i32Count += 8;
		}
		else if(i32Length - i32Count >= 4)
		{
			sprintf(pStr + 2 * i32Count, "%08X", htonl(*((uint32_t *)(&bBuffer[i32Count]))));
			i32Count += 4;
		}
		else if(i32Length - i32Count >= 2)
		{
			sprintf(pStr + 2 * i32Count, "%04X", htons(*((uint16_t *)(&bBuffer[i32Count]))));
			i32Count += 2;
		}
		else
		{
			sprintf(pStr + 2 * i32Count, "%02X", *((uint8_t *)(&bBuffer[i32Count])));
			i32Count += 1;
		}
	}
}

void ByteToString(const BYTE *bBuffer, int32_t i32Length, string &s)
{
	s = "";
	s.resize(i32Length * 2 + 1);
	char *p = (char *)s.data();
	ByteToString(bBuffer, i32Length, p);
	s.resize(i32Length * 2);
}

void StringToByte(const char *pStr, BYTE *bBuffer)
{
	int32_t i32Length = strlen(pStr);
	StringToByte(pStr, i32Length, bBuffer);
}

void StringToByte(const char *pStr, int32_t i32Length, BYTE *bBuffer)
{
	int32_t iTempLen = strlen(pStr);
	int32_t i32XLength = (iTempLen > i32Length ? i32Length : iTempLen) / 2;
	for(int32_t i32Count = 0; i32Count < i32XLength; )
	{
		if(i32XLength - i32Count >= 8)
		{
			sscanf(pStr + 2 * i32Count, "%016llX", (uint64_t *)(&bBuffer[i32Count]));
			*(uint64_t *)(&bBuffer[i32Count]) = htonll(*((uint64_t *)(&bBuffer[i32Count])));
			i32Count += 8;
		}
		else if(i32XLength - i32Count >= 4)
		{
			sscanf(pStr + 2 * i32Count, "%08X", (uint32_t *)(&bBuffer[i32Count]));
			*(uint32_t *)(&bBuffer[i32Count]) = htonl(*((uint32_t *)(&bBuffer[i32Count])));
			i32Count += 4;
		}
		else if(i32XLength - i32Count >= 2)
		{
			uint32_t ui32Temp = 0;
			sscanf(pStr + 2 * i32Count, "%04X", &ui32Temp);
			*(uint16_t *)(&bBuffer[i32Count]) = htons((uint16_t)ui32Temp);
			i32Count += 2;
		}
		else
		{
			uint32_t ui32Temp = 0;
			sscanf(pStr + 2 * i32Count, "%02X", &ui32Temp);
			bBuffer[i32Count] = ui32Temp;
			i32Count += 1;
		}
	}
}

#ifndef _WIN32
#include <unistd.h> /* usleep */
#endif
void uv_sleep(int msec)
{
#ifdef _WIN32
	Sleep(msec);
#else
	usleep(msec * 1000);
#endif
}

void XOR(const BYTE *input1, const BYTE *input2, BYTE *output, int len)
{
	for(int i = 0;i < len;i++)
		output[i]=input1[i]^input2[i];
}

int MakeDes3S(BYTE* pInData, int iInLength, BYTE *pOutData, int &iOutLength, BYTE *bKey, BYTE bType, BYTE bFill)
{
	if(bType == DES_DECRYPT)//解密一定要8的倍数
	{
		if(iInLength % 8 != 0)
		{
			return X_FAILURE;
		}
	}

	BYTE bData[8];
	for(int i = 0; i < iInLength / 8; i++)
	{
		memcpy(bData, pInData + 8 * i, 8);
		if(bType == DES_ENCRYPT)
			DesEcb3DoubleEncrypt(bData, pOutData + 8 * i, bKey);
		else
			DesEcb3DoubleDecrypt(bData, pOutData + 8 * i, bKey);
	}
	if(iInLength % 8 != 0)
	{
		//补全要加密数据成8的倍数,不足补bFill
		memset(bData, bFill, sizeof(bData));
		memcpy(bData, pInData + (iInLength / 8) * 8, iInLength - (iInLength / 8) * 8);
		if(bType == DES_ENCRYPT)
			DesEcb3DoubleEncrypt(bData, pOutData + (iInLength / 8) * 8, bKey);
		else
			DesEcb3DoubleDecrypt(bData, pOutData + (iInLength / 8) * 8, bKey);
	}

	iOutLength = ((iInLength + 7) / 8) * 8;

	return X_SUCCESS;
}

//1-加密，0-解密
int MakeDes3SCBC(BYTE *pInitVecter, BYTE* pInData, int iInLength, BYTE *pOutData, int &iOutLength, BYTE *bKey, BYTE bType, BYTE bFill)
{
	if(bType == DES_DECRYPT)//解密一定要8的倍数
	{
		if(iInLength % 8 != 0)
		{
			return X_FAILURE;
		}
	}

	BYTE *pXInData = new BYTE[iInLength];
	if(pXInData == NULL)
	{
		return X_FAILURE;
	}
	memcpy(pXInData, pInData, iInLength);

	BYTE sXorData[8];
	memcpy(sXorData, pInitVecter, 8);
	for(int i = 0; i < iInLength / 8; i++)
	{
		if(bType == DES_ENCRYPT)
		{
			XOR(sXorData, pXInData + 8 * i, sXorData, 8);
			DesEcb3DoubleEncrypt(sXorData, sXorData, bKey);
			memcpy(pOutData + 8 * i, sXorData, 8);
		}
		else
		{
			DesEcb3DoubleDecrypt(pXInData + 8 * i, pOutData + 8 * i, bKey);
			XOR(sXorData, pOutData + 8 * i, pOutData + 8 * i, 8);
			memcpy(sXorData, pXInData + 8 * i, 8);
		}
	}
	if(iInLength % 8 != 0)
	{
		//补全要加密数据成8的倍数,不足补bFill
		BYTE sFillData[8];
		memset(sFillData, bFill, sizeof(sFillData));
		memcpy(sFillData, pXInData + (iInLength / 8) * 8, iInLength - (iInLength / 8) * 8);
		if(bType == DES_ENCRYPT)
		{
			XOR(sXorData, sFillData, sXorData, 8);
			DesEcb3DoubleEncrypt(sXorData, pOutData + (iInLength / 8) * 8, bKey);
		}
		else
		{
			DesEcb3DoubleDecrypt(sFillData, pOutData + (iInLength / 8) * 8, bKey);
			XOR(sXorData, pOutData + (iInLength / 8) * 8, pOutData + (iInLength / 8) * 8, 8);
		}
	}
	RELEASE(pXInData);
	iOutLength = ((iInLength + 7) / 8) * 8;

	return X_SUCCESS;
}

int MacArithmetic(BYTE *sMacKey, BYTE *pInitVector, BYTE *pInData, int iInLen, BYTE *pRetData, BYTE bFill, int iFlag)
{
	//MAC算法:
	//将字符串pInata分为8字节为单位的数据块，不足补x00,分别标号为D1,D2,D3,...,Dn
	//初始向量E0= pInitVector
	//将E0^D1 —---->E1(E0,D1异或的后结果经des加密得到E1)
	//将E1^D2 ----->E2
	//如此类推，知道得出En结束，En即是计算出来的MAC
	BYTE sData[8];
	BYTE sXorData[8];
	int i = 0;

	memset(sData, 0, sizeof(sData));
	if(pInitVector != NULL)
	{
		memcpy(sXorData, pInitVector, sizeof(sXorData));
	}
	else
	{
		memset(sXorData, 0, sizeof(sXorData));
	}

	//异或
	for(i = 0;i < iInLen / 8; i++)
	{
		XOR(sXorData, pInData + i * 8, sXorData,8);
		if(iFlag == 3)
		{
			DesEcb3DoubleEncrypt(sXorData, sXorData, sMacKey);
		}
		else
		{
			DesEcbEncrypt(sXorData, sXorData, sMacKey);
		}
	}

	memset(sData, bFill, sizeof(sData));
	memcpy(sData, pInData + iInLen / 8 * 8, iInLen % 8);
	sData[iInLen % 8] = 0x80;
	memset(sData + 1 + iInLen % 8, 0x00, 8 - 1 - iInLen % 8);

	if(iFlag == 0)//0:PBOC-DES_MAC
	{
		XOR(sXorData, sData, sXorData, 8);
		DesEcbEncrypt(sXorData, sXorData, sMacKey);
	}
	else if(iFlag == 1)//1:PBOC-3DES_MAC
	{
		XOR(sXorData, sData, sXorData, 8);
		DesEcb3DoubleEncrypt(sXorData, sXorData, sMacKey);
	}
	else if(iFlag == 2)//2:ANSI-X9.9-MAC
	{
		if(iInLen % 8 != 0)
		{
			//补全要加密数据成8倍数到sUpData,不足补 bFill
			memset(sData, bFill, sizeof(sData));
			memcpy(sData, pInData + (iInLen / 8) * 8, iInLen % 8);
			XOR(sXorData, sData, sXorData,8);
			DesEcbEncrypt(sXorData, sXorData, sMacKey);
		}
	}
	else if(iFlag == 3)//3:3DES_CBC
	{
		if(iInLen % 8 != 0)
		{
			//补全要加密数据成8倍数到sUpData,不足补 bFill
			memset(sData, bFill, sizeof(sData));
			memcpy(sData, pInData + (iInLen / 8) * 8, iInLen % 8);
			XOR(sXorData, sData, sXorData,8);
			DesEcb3DoubleEncrypt(sXorData, sXorData, sMacKey);
		}
	}
	else if(iFlag == 4)//4:ANSI-X9.19-MAC
	{
		if(iInLen % 8 != 0)
		{
			//补全要加密数据成8倍数到sUpData,不足补 bFill
			memset(sData, bFill, sizeof(sData));
			memcpy(sData, pInData + (iInLen / 8) * 8, iInLen % 8);
			XOR(sXorData, sData, sXorData,8);
			DesEcb3DoubleEncrypt(sXorData, sXorData, sMacKey);
		}
		else
		{
			DesEcbDecrypt(sXorData, sXorData, sMacKey + 8);
			DesEcbEncrypt(sXorData, sXorData, sMacKey);
		}
	}
	else
	{
		return -1;
	}

	memcpy(pRetData, sXorData, 8);
	return 0;
}

//iFlag = 0:PBOC-DES_MAC,1:PBOC-3DES_MAC,2:ANSI-X9.9-MAC,3:3DES_CBC,4:ANSI-X9.19-MAC
int MacArithmetic(BYTE *sMacKey, BYTE *pInData, int iInLen, BYTE *pRetData, BYTE bFill, int iFlag)
{
	return MacArithmetic(sMacKey, NULL, pInData, iInLen, pRetData, bFill, iFlag);
}

int MakeCVN(BYTE *sKey, BYTE *pInData, int iInLen, BYTE *pRetData)
{
	int i = 0,iTemp = 0,n = 0,k = 0;
	BYTE sData[33];
	BYTE sBlockData[16];
	char sTemp[3];

	if(iInLen > 32 || iInLen < 0)
		return -1;
	memset(sTemp,0,sizeof(sTemp));
	memset(sBlockData,0,sizeof(sBlockData));
	memset(sData,'0',sizeof(sData));
	memcpy(sData,pInData,iInLen);
	sData[32] = 0x00;
	for(i = 0; i < 16; i++)
	{
		iTemp = 0;
		memcpy(sTemp,sData + i * 2, 2);
		sscanf(sTemp,"%02X",&iTemp);
		sBlockData[i] = iTemp;
	}

	DesEcbEncrypt(sBlockData, sBlockData, sKey);
	XOR(sBlockData, sBlockData + 8, sBlockData, 8);
	DesEcb3DoubleEncrypt(sBlockData, sBlockData, sKey);

	memset(sData,0,sizeof(sData));
	for(i = 0; i < 8; i++)
	{
		iTemp = sBlockData[i] >> 4;
		if(iTemp <= 9)
			sData[n++] = 0x30 + iTemp;
		else
			sData[16 + k++] = 0x30 + iTemp - 10;
		iTemp = sBlockData[i] & 0x0F;
		if(iTemp <= 9)
			sData[n++] = 0x30 + iTemp;
		else
			sData[16 + k++] = 0x30 + iTemp - 10;
	}
	memcpy(sData + n,sData + 16,k);
	sData[16] = 0x00;
	memcpy(pRetData,sData,3);
	pRetData[3] = 0x00;
	return X_SUCCESS;
}

// 32位CRC计算
void CRC32(BYTE *DataBuf, int nLength, BYTE *CRC)
{
	static uint64_t CRC_TAB[256] =
	{
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
		0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
		0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
		0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
		0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
		0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
		0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
		0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
		0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
		0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
		0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
		0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
		0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
		0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
		0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
		0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
		0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
		0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d
	};

	uint64_t nCRC = 0xFFFFFFFF;
	for(int i = 0; i < nLength; i++)
	{
		nCRC = (nCRC >> 8) ^ CRC_TAB[((nCRC & 0xFF)  ^ (DataBuf[i] & 0xFF)) & 0x000000FF];
	}

	nCRC ^= 0xFFFFFFFF;

	CRC[0] = (nCRC >> 24) & 0x000000FF;
	CRC[1] = (nCRC >> 16) & 0x000000FF;
	CRC[2] = (nCRC >> 8) & 0x000000FF;
	CRC[3] = nCRC & 0x000000FF;
}

void ScatterKey(BYTE *bRootKey, BYTE *bInData, BYTE *bOutData, int iCount)
{
	BYTE bRootKeyTemp[16];
	memcpy(bRootKeyTemp, bRootKey, 16);
	for(int i = 0; i < iCount;i++)
	{
		DesEcb3DoubleEncrypt(bInData + i * 8, bOutData, bRootKeyTemp);
		BYTE bReverseData[8];
		for(int j = 0;j < 8;j++)
			bReverseData[j] = ~bInData[i * 8 + j];
		DesEcb3DoubleEncrypt(bReverseData, bOutData + 8, bRootKeyTemp);
		memcpy(bRootKeyTemp, bOutData, 16);
	}
}

void ScatterKey16(BYTE *bRootKey, BYTE *bInData, BYTE *bOutData, int iCount)
{
	BYTE bRootKeyTemp[16];
	memcpy(bRootKeyTemp, bRootKey, 16);
	for(int i = 0; i < iCount;i++)
	{
		DesEcb3DoubleEncrypt(bInData + i * 8, bOutData, bRootKeyTemp);
		DesEcb3DoubleEncrypt(bInData + i * 8 + 8, bOutData + 8, bRootKeyTemp);
		memcpy(bRootKeyTemp, bOutData, 16);
	}
}

int ComputeMD5(BYTE *pInData, int iInLength, char *pOutData)
{
	MD5_CTX md5c;

	MD5_Init(&md5c);
	MD5_Update(&md5c, pInData, iInLength);

	BYTE signature[MD5_DIGEST_LENGTH + 1];
	MD5_Final(signature, &md5c);
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		sprintf(pOutData + 2 * i, "%02X", signature[i]);
	}
	return X_SUCCESS;
}

int ComputeMD5(char *strFileName, char *pOutData)
{
	BYTE buffer[16384], signature[MD5_DIGEST_LENGTH + 1];
	MD5_CTX md5c;

	FILE *fp = fopen(strFileName, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open input file[%s]\n", strFileName);
		return X_FAILURE;
	}
#ifdef _WIN32
	_setmode(_fileno(fp), _O_BINARY);
#endif
	MD5_Init(&md5c);

	while(true)
	{
		size_t szLen = fread(buffer, 1, sizeof(buffer), fp);
		if(szLen <= 0)
			break;
		MD5_Update(&md5c, buffer, szLen);
	}
	fclose(fp);
	MD5_Final(signature, &md5c);
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		sprintf(pOutData + 2 * i, "%02x", signature[i]);
	}

	return X_SUCCESS;
}

#ifndef _WIN32
uint64_t htonll(uint64_t uiHost)
{
	uint64_t uiNet = 0;
	if(byteOrder == Reverse) //需要反转
	{
		int iCount = sizeof(uint64_t);
		for(int i = 0; i < iCount; i++)
		{
			((BYTE*)&uiNet)[i] = ((BYTE*)&uiHost)[iCount - 1 - i];
		}
		return uiNet;
	}

	return uiHost;
}

uint64_t ntohll(uint64_t uiNet)
{
	return htonll(uiNet);
}
#endif

void strUpr(char *str)
{
	size_t szLen = strlen(str);
	for(int i = 0; i < (int)szLen; i ++)
	{
		int j = str[i] - 'a';
		if(j >=0 && j < 26)
			str[i] = 'A' + j;
	}
	return;
}

int CodeConvert(const string &strFromCharset, const string &strToCharset, const string &strIn, string &strOut)
{
	iconv_t cd;
	if(strIn.length() <= 0)//输入字符为空，不转换，直接返回成功
		return X_SUCCESS;
	strOut.resize(strIn.length() * 2 -1);
	char *inp = (char *)strIn.c_str();
	char** pin = &inp;
	char *outp = (char *)strOut.data();
	char** pout = &outp;

	cd = iconv_open(strToCharset.c_str(), strFromCharset.c_str());
	if(cd == (iconv_t)-1)
		return X_FAILURE;

	size_t szInLen = strIn.length();
	size_t szOutLen = strOut.length();
#ifdef _WIN32
	size_t szResult = iconv(cd, (const char**)pin, &szInLen, pout, &szOutLen);
#else
	size_t szResult = iconv(cd, pin, &szInLen, pout, &szOutLen);
#endif
	if((int)szResult == -1)
	{
		iconv_close(cd);
		return X_FAILURE;
	}

	iconv_close(cd);
	strOut.resize(strOut.size() - szOutLen);
	return X_SUCCESS;
}

string UTF8ToGBK(const string &strUTF8)
{
	string strGBK = "";
	int iResult = CodeConvert("UTF-8", "GBK", strUTF8, strGBK);
	if(iResult != X_SUCCESS)
	{
		XLogClass::error("UTF8ToGBK Fail: strUTF8[%s]", strUTF8.c_str());
		strGBK = strUTF8;
	}
	return strGBK;
}

string GBKToUTF8(const string &strGBK)
{
	string strUTF8 = "";
	int iResult = CodeConvert("GBK", "UTF-8", strGBK, strUTF8);
	if(iResult != X_SUCCESS)
	{
		XLogClass::error("GBKToUTF8 Fail: strGBK[%s]", strGBK.c_str());
		strUTF8 = strGBK;
	}
	return strUTF8;
}

void DesEcbEncrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey)
{
	DES_cblock desKey;
	memcpy(desKey, pKey, sizeof(DES_cblock));
	DES_key_schedule ks;
	DES_set_key_unchecked(&desKey, &ks);
	DES_ecb_encrypt((const_DES_cblock *)pInData, (DES_cblock *)pOutData, &ks, DES_ENCRYPT);
}

void DesEcbDecrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey)
{
	DES_cblock desKey;
	memcpy(desKey, pKey, sizeof(DES_cblock));
	DES_key_schedule ks;
	DES_set_key_unchecked(&desKey, &ks);
	DES_ecb_encrypt((const_DES_cblock *)pInData, (DES_cblock *)pOutData, &ks, DES_DECRYPT);
}

void DesEcb3DoubleEncrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey)
{
	DES_cblock desKey1,desKey2;
	memcpy(desKey1, pKey, sizeof(DES_cblock));
	memcpy(desKey2, pKey + 8, sizeof(DES_cblock));

	DES_key_schedule ks1,ks2,ks3;
	DES_set_key_unchecked(&desKey1, &ks1);
	DES_set_key_unchecked(&desKey2, &ks2);
	DES_set_key_unchecked(&desKey1, &ks3);

	DES_ecb3_encrypt((const_DES_cblock *)pInData, (DES_cblock *)pOutData, &ks1, &ks2, &ks3, DES_ENCRYPT);
}

void DesEcb3DoubleDecrypt(const BYTE *pInData, BYTE *pOutData, const BYTE *pKey)
{
	DES_cblock desKey1,desKey2;
	memcpy(desKey1, pKey, sizeof(DES_cblock));
	memcpy(desKey2, pKey + 8, sizeof(DES_cblock));

	DES_key_schedule ks1,ks2,ks3;
	DES_set_key_unchecked(&desKey1, &ks1);
	DES_set_key_unchecked(&desKey2, &ks2);
	DES_set_key_unchecked(&desKey1, &ks3);

	DES_ecb3_encrypt((const_DES_cblock *)pInData, (DES_cblock *)pOutData, &ks1, &ks2, &ks3, DES_DECRYPT);
}

string GetLocalDate()
{
	time_t t;
	struct tm sTM;
	time(&t);//获取系统时间
#ifdef _WIN32
	localtime_s(&sTM, &t);//转换本地时间
#else
	localtime_r(&t, &sTM);
#endif
	char cTime[14 + 1];
	sprintf(cTime, "%04d%02d%02d", sTM.tm_year + 1900,
		sTM.tm_mon + 1, sTM.tm_mday);

	return cTime;
}

string GetLocalTime()
{
	time_t t;
	struct tm sTM;

	time(&t);//获取系统时间
#ifdef _WIN32
	localtime_s(&sTM, &t);//转换本地时间
#else
	localtime_r(&t, &sTM);
#endif

	string s;
	TimeTToString(t, s);

	return s;
}

int TimeTToTM(const time_t tTime, struct tm &sTM)
{
#ifdef _WIN32
	localtime_s(&sTM, &tTime);//转换本地时间
#else
	localtime_r(&tTime, &sTM);
#endif

	return 0;
}

int TimeTToString(const time_t tTime, string &strTime)
{
	struct tm sTM;
	TimeTToTM(tTime, sTM);

	strTime.resize(14);
	sprintf((char *)strTime.c_str(), "%04d%02d%02d%02d%02d%02d", sTM.tm_year + 1900,
		sTM.tm_mon + 1, sTM.tm_mday, sTM.tm_hour, sTM.tm_min, sTM.tm_sec);

	return 0;
}

int TimeTToHttpString(const time_t tTime, string &strLongTime)
{
	static const char *wDay[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
	static const char *wMonth[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

	struct tm sTM;
#ifdef _WIN32
	gmtime_s(&sTM, &tTime);
#else
	gmtime_r(&tTime, &sTM);
#endif
	char cTime[32];
	//Sat 11 Oct 2008 13:45:00 GMT
	sprintf(cTime, "%s, %d %s %d %02d:%02d:%02d GMT",
		wDay[sTM.tm_wday], sTM.tm_mday, wMonth[sTM.tm_mon], 1900 + sTM.tm_year, sTM.tm_hour, sTM.tm_min, sTM.tm_sec);

	strLongTime = cTime;

	return 0;
}

string UrlEncode(const string &strSource)
{
	static char hex[] = "0123456789ABCDEF";
	string strDest;
	for (size_t i = 0; i < strSource.size(); i++)
	{
		BYTE ch = strSource[i];

		if (isalnum(ch))
		{
			strDest += ch;
		}
		else
		{
			if (ch == ' ')
			{
				strDest += '+';
			}
			else
			{
				strDest += '%';
				strDest += hex[ch / 16];
				strDest += hex[ch % 16];
			}
		}
	}
	return strDest;
}

string UrlDecode(const string &strSource)
{
	string strDest, strDestUrl;
	size_t szSrcLen = strSource.size();

	for (size_t i = 0; i < szSrcLen; i++)
	{
		if (strSource[i] == '%')
		{
			if(isxdigit(strSource[i + 1]) && isxdigit(strSource[i + 2]))
			{
				char c1 = strSource[++i];
				char c2 = strSource[++i];

				c1 = c1 - '0' - ((c1 >= 'A') ? 7 : 0) - ((c1 >= 'a') ? 32 : 0);
				c2 = c2 - '0' - ((c2 >= 'A') ? 7 : 0) - ((c2 >= 'a') ? 32 : 0);

				strDest += (BYTE)(c1 * 16 + c2);
			}
		}
		else
		{
			if (strSource[i] == '+')
			{
				strDest += ' ';
			}
			else
			{
				strDest += strSource[i];
			}
		}
	}

	return strDest;
}

int HttpParseQueryString(const string &strQueryString, map<string, string> &mapQueryString)
{
	list<string> liTemp = split(strQueryString, "&", true);
	for(list<string>::const_iterator itTemp = liTemp.begin(); itTemp != liTemp.end(); itTemp++)
	{
		if(*itTemp == "")
			continue;
		int iIndex = (*itTemp).find('=', 0);
		if(iIndex < 0)
		{
			XLogClass::error("HttpParseQueryString find '=' Error");
			return X_FAILURE;
		}
		mapQueryString[UrlDecode((*itTemp).substr(0, iIndex))] = UrlDecode((*itTemp).substr(iIndex + 1));
	}
	return X_SUCCESS;
}

//4的倍数取随机数
int GetRandomNumber(BYTE *bRandom, int iLength)
{
	uint64_t i64Time = uv_hrtime();
	uint32_t i32Seed = (i64Time & 0x00000000FFFFFFFF);
	srand(i32Seed);
	for(int i = 0; i < iLength; i += 4)
	{
		uint32_t ui32Random = rand() % 0x10000;
		ui32Random = (rand() % 0x10000 << 16)| ui32Random;
		memcpy(bRandom + i, &ui32Random, 4);
	}
	return X_SUCCESS;
}


#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <stdarg.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#define ACCESS ::access
#define MKDIR(a) ::mkdir((a))
#else
#define ACCESS ::access
#define MKDIR(a) ::mkdir((a), 0777)
#endif

int CreateDir(string strDir)
{
	int32_t iRet;
	int32_t iLen = strDir.length();

	//创建目录
	string strMidDir = "";
	for(int i = 1; i < iLen; i ++)
	{
		if(strDir[i] == '\\' || strDir[i] == '/' || i == iLen - 1)
		{
			if(strDir[i] == '\\' || strDir[i] == '/')
				strMidDir = strDir.substr(0, i);
			else
				strMidDir = strDir.substr(0, i + 1);
			//如果不存在,创建
			iRet = ACCESS(strMidDir.c_str(),0);
			if (iRet != 0)
			{
				iRet = MKDIR(strMidDir.c_str());
				if (iRet != 0)
				{
					cout << "CreatDir FAIL: " << strMidDir << endl;
					return X_FAILURE;
				}
			}
			//支持linux,将所有'\'换成'/'
			strDir[i] = '/';
		}
	}

	return X_SUCCESS;
}

int Base64Encode(BYTE *pInBuffer, int iInLength, string &strResult)
{
	int iResult = 0;
	int iEncodeLen = (((iInLength + 2) / 3) * 4) + 1; // Base64 text length
	int iPemLen = iEncodeLen + iEncodeLen / 64; // PEM adds a newline every 64 bytes
	strResult.resize(iPemLen);
	iResult = EVP_EncodeBlock((BYTE *)strResult.data(), pInBuffer, iInLength);
	if(iResult < 0)
		return X_FAILURE;
	//EVP_ENCODE_CTX	ctx;
	//EVP_EncodeInit(&ctx);
	//EVP_EncodeUpdate(&ctx, (BYTE *)strResult.data(), &iResult, pInBuffer, iInLength);
	//EVP_EncodeFinal(&ctx, (BYTE *)strResult.data() + iResult, &iResult);

	strResult.resize(iResult);
	return X_SUCCESS;
}

int Base64Decode(string &strInData, BYTE *&pOutData, int &iOutLength)
{
	int iResult = 0;
	int iLen = strInData.length();
	int iDecodeLen = (((iLen + 2) / 4) * 3) + 1;
	pOutData = new BYTE[iDecodeLen];

	iResult = EVP_DecodeBlock(pOutData, (BYTE *)strInData.data(), iLen);
	if(iResult < 0)
	{
		RELEASE(pOutData);
		return X_FAILURE;
	}
	while(strInData[--iLen] == '=')
	{
		iResult--;
	}

	//EVP_ENCODE_CTX	ctx;
	//int iTmpLen = 0;
	//EVP_DecodeInit(&ctx);
	//EVP_DecodeUpdate(&ctx, pOutData, &iResult, (BYTE *)strInData.data(), iLen);
	//EVP_DecodeFinal(&ctx, (BYTE *)&pOutData[iResult], &iTmpLen);
	//iResult += iTmpLen;

	iOutLength = iResult;
	return X_SUCCESS;
}

int Hash_sha1(BYTE *pInData, int iInLen, BYTE *pOutData, int &iOutLen)
{
	EVP_MD_CTX ctx;
	const EVP_MD *pMd = EVP_sha1();

	EVP_DigestInit(&ctx, pMd);
	EVP_DigestUpdate(&ctx, pInData, iInLen);

	unsigned int uiLen;
	EVP_DigestFinal(&ctx, pOutData, &uiLen);
	iOutLen = uiLen;

	return X_SUCCESS;
}
