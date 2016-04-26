//XVERSIONNO "3.2.2.0"

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#pragma warning(disable:4996)
#pragma execution_character_set("utf-8") //默认使用UTF8
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <limits.h>

#ifdef _WIN32
#include <crtdbg.h>
#include <io.h>
#else
#include <unistd.h>
#include <errno.h>
#endif

#include <iostream>
#include <memory>
#include <typeinfo>

using namespace std;

#define		X_SUCCESS       0
#define		X_FAILURE       -1
#define		X_NOTEXIST      -2
#define		X_STOP          -3
#define		X_TIMEOUT       -4
#define		X_CANCEL        -5
#define     X_ASYNC         -6
#define		RELEASE(x)    if((x) != NULL){delete (x); (x) = NULL;} 
#define		RELEASEA(x)   if((x) != NULL){delete [](x); (x) = NULL;} 


//_WIN64 平台编译暂时没有做
//#ifdef _WIN64
//#endif

#ifdef _WIN32 
#define I64D "I64d"
#define I64U "I64u"
#else
#define INFINITE 0xFFFFFFFF
#define I64D "lld"
#define I64U "llu"
#endif

typedef unsigned char BYTE;
typedef BYTE byte;

#include "XConfig.h"
#include "XLogClass.h"
#include "XListener.h"
#include "XDataBase.h"
#include "XTools.h"
#include "XPublic.h"
