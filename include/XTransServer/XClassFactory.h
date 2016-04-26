#pragma once
#ifndef __XCLASSFACTORY_H__
#define __XCLASSFACTORY_H__

class XService
{
public:
	XService()
	{
		pXDataBase = NULL;
	}

	virtual ~XService()
	{
	}

	//static void* CreateInstance()
	//{
	//	return new XService();
	//}

	virtual int DoMainEx(void *pVoid, BYTE *pRequestData, size_t szRequestLen, BYTE *&pResponseData, size_t &szResponseLen)
	{
		XLogClass::debug("XService.DoMainEx 未实现");
		return X_SUCCESS;
	};

	virtual int DoLoopEx(void *pVoid, BYTE *pRequestData, size_t szRequestLen, BYTE *&pResponseData, size_t &szResponseLen)
	{
		XLogClass::debug("XService.DoLoopEx 未实现");
		return X_SUCCESS;
	};

	virtual int DoInitializeEx()
	{
		XLogClass::debug("XService.DoInitializeEx 未实现");
		return X_SUCCESS;
	};

	virtual int DoReleaseEx()
	{
		XLogClass::debug("XService.DoReleaseEx 未实现");
		return X_SUCCESS;
	};

	XDataBase *pXDataBase;
};

typedef XService* (*FuncCreateXService)(void);//回调函数指针

class XClassFactory
{
public:
	static int CreateServiceByName(string strServiceName, XService* &pXService);

	void RegistService(string strServiceName, FuncCreateXService pFunc);

	static XClassFactory& sharedServiceFactory();
private:
	map<string, FuncCreateXService> m_classMap;
};

class XRegistyServiceClass
{
public:
	XRegistyServiceClass(string strServiceName, FuncCreateXService pFuncCreate)
	{
		XClassFactory::sharedServiceFactory().RegistService(strServiceName, pFuncCreate);
	}
};

#define IMPL_CLASS_CREATE(class_name) \
public:\
	static XService* CreateInstance(){ \
	return new class_name;\
};\
	static XRegistyServiceClass xRegisty;

#define REG_CLASS_CREATE(class_name) XRegistyServiceClass class_name::xRegisty(#class_name, class_name::CreateInstance)

#endif //__XCLASSFACTORY_H__
