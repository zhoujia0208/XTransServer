#include "StdAfx.h"
#include "XXmlDocument.h"

XXmlNodeList::XXmlNodeList(const xmlXPathObjectPtr pXPathObject)
{
	m_pXPathObject = pXPathObject;
}

XXmlNodeList::~XXmlNodeList()
{
	xmlXPathFreeObject(m_pXPathObject);
}

void XXmlNodeList::Dispose()
{
	xmlXPathFreeObject(m_pXPathObject);
	m_pXPathObject = NULL;
}

size_t XXmlNodeList::size()
{
	if(m_pXPathObject == NULL || m_pXPathObject->nodesetval == NULL)
		return 0;
	return m_pXPathObject->nodesetval->nodeNr;
}

xmlNodePtr XXmlNodeList::operator[](int iIndex)
{
	if(iIndex >= (int)size())
		return NULL;
	return m_pXPathObject->nodesetval->nodeTab[iIndex];
}

XXmlNodeList & XXmlNodeList::operator =(const xmlXPathObjectPtr pXPathObject)
{
	this->m_pXPathObject = pXPathObject;
	return *this;
}

XXmlDocument::XXmlDocument()
{
	m_pDoc = NULL;
	m_pRootNode = NULL;
	m_strFilePath = "";
}

XXmlDocument::~XXmlDocument()
{
	if (m_pDoc)
	{
		xmlFreeDoc(m_pDoc);
		m_pDoc = NULL;
		m_pRootNode = NULL;
		xmlMemoryDump();
	}
}

int XXmlDocument::OpenFile(string strXmlFilePath)
{
	m_strFilePath = strXmlFilePath;

	//xmlKeepBlanksDefault(0);
	m_pDoc = xmlReadFile(m_strFilePath.c_str(), NULL, XML_PARSE_RECOVER);
	//m_pDoc = xmlParseFile(m_strFilePath.c_str());
	m_pRootNode = xmlDocGetRootElement(m_pDoc);

	if (m_pRootNode == NULL)
	{
		return X_FAILURE;
	}

	return X_SUCCESS;
}

//static void XsdValidateError(void * ctx, const char * firstArg, ...)
//{
//	va_list args;
//	va_start(args, firstArg);
//	vprintf(firstArg, args);
//	va_end(args);
//	return;
//}

int XXmlDocument::XsdValidate(string strXsdFilePath)
{
	if (m_pDoc == NULL)
		return X_FAILURE;
	xmlDocPtr pSchemaDoc = xmlParseFile(strXsdFilePath.c_str());
	if (pSchemaDoc == NULL)
	{
		return X_FAILURE;
	}

	xmlSchemaParserCtxtPtr pSchemaParser = xmlSchemaNewDocParserCtxt(pSchemaDoc);
	if (pSchemaParser == NULL)
	{
		xmlFreeDoc(pSchemaDoc);
		return X_FAILURE;
	}

	//xmlSchemaSetParserErrors(pSchemaParser, XsdValidateError, XsdValidateError, NULL);
	xmlSchemaPtr pSchema = xmlSchemaParse(pSchemaParser);
	if (pSchema == NULL)
	{
		xmlSchemaFreeParserCtxt(pSchemaParser);
		xmlFreeDoc(pSchemaDoc);
		return X_FAILURE;
	}

	xmlSchemaValidCtxtPtr pSchemaValid = xmlSchemaNewValidCtxt(pSchema);
	if (pSchemaValid == NULL)
	{
		xmlSchemaFree(pSchema);
		xmlSchemaFreeParserCtxt(pSchemaParser);
		xmlFreeDoc(pSchemaDoc);
		return X_FAILURE;
	}

	//xmlSchemaSetValidErrors(pSchemaValid, XsdValidateError, NULL, NULL);
	int iResult = xmlSchemaValidateDoc(pSchemaValid, m_pDoc);
	if (iResult != 0)
	{
		xmlSchemaFreeValidCtxt(pSchemaValid);
		xmlSchemaFree(pSchema);
		xmlSchemaFreeParserCtxt(pSchemaParser);
		xmlFreeDoc(pSchemaDoc);
		return X_FAILURE;
	}

	xmlSchemaFreeValidCtxt(pSchemaValid);
	xmlSchemaFree(pSchema);
	xmlSchemaFreeParserCtxt(pSchemaParser);
	xmlFreeDoc(pSchemaDoc);

	return X_SUCCESS;
}

xmlXPathObjectPtr XXmlDocument::searchNodesByPath(string strExpress)
{
	return XXmlDocument::searchNodesByPath(this->m_pDoc, strExpress);
}

xmlXPathObjectPtr XXmlDocument::searchNodesByPath(xmlDocPtr pDoc, string strExpress)
{
	xmlXPathContextPtr pXPathContext = xmlXPathNewContext(pDoc);
	xmlXPathObjectPtr pXPathObject = xmlXPathEvalExpression(BAD_CAST(strExpress.c_str()), pXPathContext);
	xmlXPathFreeContext(pXPathContext);

	return pXPathObject;
}

