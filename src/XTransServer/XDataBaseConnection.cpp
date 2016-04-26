#include "StdAfx.h"
#include "XDataBaseConnection.h"

namespace XData {

	XDataBaseConnection::XDataBaseConnection()
	{
		m_isUsed = false;
		m_hThread = 0;
		m_ui64LastUseTime = 0;
	}

	int XDataBaseConnectionFactory::CreateDataBaseConnectionByName(string strDataBaseConnectionName, XDataBaseConnection* &pXDataBaseConnection)
	{
		map<string, FuncCreateDataBaseConnection>::const_iterator it;
		it = sharedDataBaseConnectionFactory().m_classMap.find(strDataBaseConnectionName);
		if (it == sharedDataBaseConnectionFactory().m_classMap.end())
		{
			return X_NOTEXIST;
		}

		pXDataBaseConnection = it->second();
		if (pXDataBaseConnection == NULL)
		{
			return X_FAILURE;
		}

		return X_SUCCESS;
	};

	void XDataBaseConnectionFactory::RegistDataBaseConnection(string strDataBaseConnectionName, FuncCreateDataBaseConnection pFunc)
	{
		m_classMap[strDataBaseConnectionName] = pFunc;
	}

	XDataBaseConnectionFactory& XDataBaseConnectionFactory::sharedDataBaseConnectionFactory()
	{
		static XDataBaseConnectionFactory _sharedDataBaseConnectionFactory;
		return _sharedDataBaseConnectionFactory;
	}

}
