#include "stdafx.h"
#include "XmlHelper.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HRESULT XmlHelper::OpenXmlDocument(const wstring& strFilename, CComPtr<IXMLDOMDocument>& pXmlDocument, CComPtr<IXMLDOMElement>& pRootElement)
{
	HRESULT hr					= S_OK;
	VARIANT_BOOL bLoadSuccess	= 0; // FALSE

	pXmlDocument.Release();
	pRootElement.Release();

	hr = pXmlDocument.CoCreateInstance(__uuidof(DOMDocument));
	if (FAILED(hr) || (pXmlDocument.p == NULL)) return E_FAIL;

	hr = pXmlDocument->load(CComVariant(strFilename.c_str()), &bLoadSuccess);
	if (FAILED(hr) || (!bLoadSuccess)) return E_FAIL;

/*
	if (FAILED(hr) || (!bLoadSuccess))
	{
		if (strDefaultFilename.length() == 0) return wstring(L"");

		strXmlFilename = Helpers::GetModulePath(NULL) + strDefaultFilename;

/ *
		wchar_t szModuleFileName[MAX_PATH + 1];
		::ZeroMemory(szModuleFileName, (MAX_PATH+1)*sizeof(wchar_t));
		::GetModuleFileName(NULL, szModuleFileName, MAX_PATH);

		wstring strModuleFileName(szModuleFileName);
		wstring strDefaultOptionsFileName(strModuleFileName.substr(0, strModuleFileName.rfind(L'\\')+1));

		strDefaultOptionsFileName += strDefaultFilename;
* /

		hr = pXmlDocument->load(CComVariant(strXmlFilename.c_str()), &bLoadSuccess);
		if (FAILED(hr) || (!bLoadSuccess)) return wstring(L"");
	}
*/

	hr = pXmlDocument->get_documentElement(&pRootElement);
	if (FAILED(hr)) return E_FAIL;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

HRESULT XmlHelper::LoadXmlDocument(const wstring& strXml, CComPtr<IXMLDOMDocument>& pXmlDocument, CComPtr<IXMLDOMElement>& pRootElement)
{
	pXmlDocument.Release();
	pRootElement.Release();

    VARIANT_BOOL bLoadSuccess = 0;
    HRESULT hr = pXmlDocument.CoCreateInstance(__uuidof(DOMDocument));
    if (SUCCEEDED(hr) && (pXmlDocument.p != NULL))
    {
        hr = pXmlDocument->loadXML(CComBSTR(strXml.c_str()), &bLoadSuccess);
    }

    if (SUCCEEDED(hr) && bLoadSuccess)
    {
        hr = pXmlDocument->get_documentElement(&pRootElement);
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////

HRESULT XmlHelper::GetDomElement(const CComPtr<IXMLDOMElement>& pRootElement, const CComBSTR& bstrPath, CComPtr<IXMLDOMElement>& pElement)
{
	HRESULT					hr = S_OK;
	CComPtr<IXMLDOMNode>	pNode;

	if (pRootElement.p == NULL) return E_FAIL;

	hr = pRootElement->selectSingleNode(bstrPath, &pNode);
    return (SUCCEEDED(hr) ? pNode.QueryInterface(&pElement) : hr);
}

/*
 * Same as GetDomElement, but creates missing parts of the path
 */
HRESULT XmlHelper::CreateDomElement(const CComPtr<IXMLDOMElement>& pRootElement, CString path, CComPtr<IXMLDOMElement>& pElement)
{
	if (NULL == pRootElement)
		return E_FAIL;

	CComPtr<IXMLDOMDocument> pDocument;
	if (FAILED(pRootElement->get_ownerDocument(&pDocument)))
		return E_FAIL;

	CComPtr<IXMLDOMElement> pParent = pRootElement;

	int pos = 0;
	CString token = path.Tokenize(L"/", pos);
	while (!token.IsEmpty()) {
		CComPtr<IXMLDOMNode> pNode;
		if (S_OK != pParent->selectSingleNode(CComBSTR(token), &pNode)) {
			CComPtr<IXMLDOMElement> pChild;
			int tagLength = token.Find(L'[');
			if (-1 == tagLength)
				tagLength = token.GetLength();

			if (FAILED(pDocument->createElement(CComBSTR(token.Left(tagLength)), &pChild)))
				return E_FAIL;
			if (FAILED(pParent->appendChild(pChild, &pNode)))
				return E_FAIL;
			// TODO: add some text nodes for a better formatting
		}

		pParent = pNode;

		token = path.Tokenize(L"/", pos);
	}

	return pParent.QueryInterface(&pElement);
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue)
{
	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK)
	{
		dwValue = _wtol(varValue.bstrVal);
	}
	else
	{
		dwValue = dwDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, int& nValue, int nDefaultValue)
{
	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK)
	{
		nValue = _wtol(varValue.bstrVal);
	}
	else
	{
		nValue = nDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE& byValue, BYTE byDefaultValue)
{
	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK)
	{
		byValue = static_cast<BYTE>(_wtoi(varValue.bstrVal));
	}
	else
	{
		byValue = byDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue)
{
	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK)
	{
		bValue = (_wtol(varValue.bstrVal) > 0);
	}
	else
	{
		bValue = bDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue)
{
	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK)
	{
		strValue = varValue.bstrVal;
	}
	else
	{
		strValue = strDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue, COLORREF crDefaultValue)
{
	DWORD r;
	DWORD g;
	DWORD b;

	GetAttribute(pElement, CComBSTR(L"r"), r, GetRValue(crDefaultValue));
	GetAttribute(pElement, CComBSTR(L"g"), g, GetGValue(crDefaultValue));
	GetAttribute(pElement, CComBSTR(L"b"), b, GetBValue(crDefaultValue));

	crValue = RGB(r, g, b);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD dwValue)
{
	CComVariant	varValue(str(wformat(L"%1%") % dwValue).c_str());

	pElement->setAttribute(bstrName, varValue);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, int nValue)
{
	CComVariant	varValue(str(wformat(L"%1%") % nValue).c_str());

	pElement->setAttribute(bstrName, varValue);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE byValue)
{
	CComVariant	varValue(str(wformat(L"%1%") % byValue).c_str());

	pElement->setAttribute(bstrName, varValue);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool bValue)
{
	CComVariant	varValue(bValue ? L"1" : L"0");

	pElement->setAttribute(bstrName, varValue);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, const wstring& strValue)
{
	CComVariant	varValue(strValue.c_str());

	pElement->setAttribute(bstrName, varValue);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::SetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, const COLORREF& crValue)
{
	SetAttribute(pElement, CComBSTR(L"r"), GetRValue(crValue));
	SetAttribute(pElement, CComBSTR(L"g"), GetGValue(crValue));
	SetAttribute(pElement, CComBSTR(L"b"), GetBValue(crValue));
}

//////////////////////////////////////////////////////////////////////////////

void XmlHelper::LoadColors(const CComPtr<IXMLDOMElement>& pElement, COLORREF colors[16])
{
	for (DWORD i = 0; i < 16; ++i)
	{
		CComPtr<IXMLDOMElement>	pFontColorElement;

		if (FAILED(GetDomElement(pElement, CComBSTR(str(wformat(L"colors/color[%1%]") % i).c_str()), pFontColorElement))) continue;

		DWORD id;

		GetAttribute(pFontColorElement, CComBSTR(L"id"), id, i);
		GetRGBAttribute(pFontColorElement, colors[id], colors[i]);
	}
}

void XmlHelper::SaveColors(CComPtr<IXMLDOMElement>& pElement, const COLORREF colors[16])
{
	for (DWORD i = 0; i < 16; ++i)
	{
		CComPtr<IXMLDOMElement>	pFontColorElement;

		if (FAILED(CreateDomElement(pElement, CString(str(wformat(L"colors/color[%1%]") % i).c_str()), pFontColorElement))) continue;

		SetAttribute(pFontColorElement, CComBSTR(L"id"), i);
		SetRGBAttribute(pFontColorElement, colors[i]);
	}
}
