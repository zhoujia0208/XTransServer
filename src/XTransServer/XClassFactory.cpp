#include "StdAfx.h"
#include "XClassFactory.h"


int XClassFactory::CreateServiceByName(string strClassName, XService* &pXService)
{
	map<string, FuncCreateXService>::const_iterator it;
	it = sharedServiceFactory().m_classMap.find(strClassName);
	if(it == sharedServiceFactory().m_classMap.end())
	{
		return X_NOTEXIST;
	}

	pXService = it->second();
	if(pXService == NULL)
	{
		return X_FAILURE;
	}

	return X_SUCCESS;
};

void XClassFactory::RegistService(string strServiceName, FuncCreateXService pFunc)
{
	m_classMap[strServiceName] = pFunc;
	//m_classMap.insert(std::make_pair(name, pFunc));
}

XClassFactory& XClassFactory::sharedServiceFactory()
{
	static XClassFactory _sharedClassFactory;
	return _sharedClassFactory;
}
