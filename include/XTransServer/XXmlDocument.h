#pragma once
#ifndef __CONFIGXML_H__
#define __CONFIGXML_H__

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpointer.h>
#include <libxml/xmlschemas.h>

class XXmlNodeList
{
public:
	XXmlNodeList(const xmlXPathObjectPtr pXPathObject = NULL);
	~XXmlNodeList();
	void Dispose();

	size_t size();
	xmlNodePtr operator[](int iIndex);
	XXmlNodeList &operator =(const xmlXPathObjectPtr pXPathObject);
	xmlXPathObjectPtr m_pXPathObject;
};

class XXmlDocument
{
public:
	XXmlDocument();
	~XXmlDocument();

	void Dispose();
	int OpenFile(string strXmlFilePath);
	int ReadString(string &strXmlString);
	int XsdValidate(string strXsdFilePath);
	xmlXPathObjectPtr searchNodesByPath(string strExpress);

	static xmlXPathObjectPtr searchNodesByPath(xmlDocPtr pDoc, string strExpress);

public:
	string m_strFilePath;

	xmlDocPtr m_pDoc;
	xmlNodePtr m_pRootNode;
};

#endif //__CONFIGXML_H__
