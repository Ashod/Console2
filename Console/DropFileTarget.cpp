#include "StdAfx.h"
#include "DropFileTarget.h"

CDropFileTarget::CDropFileTarget()
{
	/* This call might fail, in which case OLE sets m_pdth = NULL */
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IDropTargetHelper, (LPVOID*)&m_spdth);
}

STDMETHODIMP CDropFileTarget::DragEnter (IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (m_spdth && GetTargetHwnd()) {              /* Use the helper if we have one */
		POINT pt = { ptl.x, ptl.y };
		m_spdth->DragEnter(GetTargetHwnd(), pDataObj, &pt, *pdwEffect);
		if (MK_CONTROL_ONLY(grfKeyState))
			*pdwEffect = DROPEFFECT_COPY;
	} else
		*pdwEffect = DROPEFFECT_COPY;

	if (! GetData(pDataObj, NULL))
		*pdwEffect = DROPEFFECT_NONE;

	m_dwEffect = *pdwEffect;

	return S_OK;
}

STDMETHODIMP CDropFileTarget::DragOver (DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (m_spdth) {              /* Use the helper if we have one */
		POINT pt = { ptl.x, ptl.y };
		m_spdth->DragOver(&pt, *pdwEffect);
	}

	if (DROPEFFECT_NONE != m_dwEffect) {
		if (MK_CONTROL_ONLY(grfKeyState))
			m_dwEffect = DROPEFFECT_COPY;
		else
			m_dwEffect = *pdwEffect;
	}

	*pdwEffect = m_dwEffect;

	return S_OK;
};

STDMETHODIMP CDropFileTarget::DragLeave() 
{ 
	if (m_spdth) {              /* Use the helper if we have one */
		m_spdth->DragLeave();
	}

	return S_OK;
}

STDMETHODIMP CDropFileTarget::Drop (IDataObject* pDataObj, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	if (m_spdth) {              /* Use the helper if we have one */
		POINT pt = { ptl.x, ptl.y };
		m_spdth->Drop(pDataObj, &pt, *pdwEffect);
	}

	if (DROPEFFECT_NONE != m_dwEffect) {
		if (MK_CONTROL_ONLY(grfKeyState))
			m_dwEffect = DROPEFFECT_COPY;
		else
			m_dwEffect = *pdwEffect;
	}

	*pdwEffect = m_dwEffect;

	return S_OK;
}

bool CDropFileTarget::HDropHandler(IDataObject *pDataObj, CString *pStr)
{
	FORMATETC format = {CF_HDROP, NULL, DVASPECT_CONTENT, -1,
		TYMED_HGLOBAL};
	STGMEDIUM medium;
	HRESULT result = pDataObj->GetData(&format, &medium);
	if (S_OK != result) // can't get the data, fail...
		return false;

	HDROP   hDrop = reinterpret_cast<HDROP>(medium.hGlobal);
	UINT	uFilesCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (0xFFFFFFFF == uFilesCount) // can't count files, fail...
		return false;

	if (NULL == pStr) // the caller is not interested in contents
		return true; // just tell him that we would succeed

	*pStr = _T ("");
	// concatenate all filenames
	for (UINT i = 0; i < uFilesCount; ++i)
	{
		CString strFilename;
		UINT len = ::DragQueryFile(hDrop, i, NULL, 0);
		::DragQueryFile(hDrop, i, strFilename.GetBuffer(len + 1), len + 1);
		strFilename.ReleaseBuffer();

		if (i > 0) *pStr += L" ";
		// append quoted filename
		*pStr += _T ("\"") + strFilename + _T ("\"");

	}

	ReleaseStgMedium(&medium);

	return true;
}

bool CDropFileTarget::ShellIDListHandler(IDataObject *pDataObj, CString *pStr)
{
	FORMATETC format = {CF_SHELLIDLIST, NULL, DVASPECT_CONTENT, -1,
		TYMED_HGLOBAL};
	STGMEDIUM medium;
	HRESULT result = pDataObj->GetData(&format, &medium);
	if (S_OK != result)
		return false;

	CIDA *pCIDA = (CIDA *)GlobalLock(medium.hGlobal);

	CString strFilenames = _T("");

	for (UINT i = 0; i < pCIDA->cidl; ++i) {
		CString strFilename;
		BOOL gotPath = ::SHGetPathFromIDList(GetPIDLItem(pCIDA, i), strFilename.GetBuffer(MAX_PATH));
		strFilename.ReleaseBuffer();
		if (! gotPath)
			continue;

		// don't insert empty strings
		if (strFilename.IsEmpty())
			continue;

		// don't rely on the value of index, because we could have skipped an empty one
		if (! strFilenames.IsEmpty())
			strFilenames += _T(" ");

		strFilenames += _T ("\"") + strFilename + _T ("\"");
	}

	ReleaseStgMedium(&medium);

	// if we did not find any files, fail without touching result string
	if (strFilenames.IsEmpty())
		return false;

	if (NULL != pStr)
		*pStr = strFilenames;

	return true;
}

bool CDropFileTarget::TextHandler(IDataObject *pDataObj, CString *pStr)
{
	CString str (_T (""));
	FORMATETC format = {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1,
		TYMED_HGLOBAL};
	STGMEDIUM medium;
	HRESULT result = pDataObj->GetData(&format, &medium);
	if (S_OK == result) {
		str = CString ((LPCWSTR) ::GlobalLock(medium.hGlobal));
	} else {
		FORMATETC format = {CF_TEXT, NULL, DVASPECT_CONTENT, -1,
			TYMED_HGLOBAL};
		result = pDataObj->GetData(&format, &medium);
		if (S_OK == result) {
			str = CString ((LPCSTR) ::GlobalLock(medium.hGlobal));
		} else
			return false; // don't try to release non-existing medium
	}

	ReleaseStgMedium (&medium);

	if (pStr)
		*pStr = str;
	return true;
}

bool CDropFileTarget::GetData(IDataObject *pDataObj, CString *pStr)
{
	// the order is important: try CF_HDROP first, then ShellIDList,
	// because CF_HDROP works better for Drives in My Computer and
	// for files on network drives
	if (HDropHandler(pDataObj, pStr))
		return true;

	if (ShellIDListHandler(pDataObj, pStr))
		return true;

	if (TextHandler(pDataObj, pStr))
		return true;

	return false;
}

// Copy from Shell Links on MSDN
HRESULT ResolveShortcut(HWND hwnd, LPCWSTR lpszLinkFile, CString &path) 
{ 
	HRESULT hres; 
	IShellLink* psl; 

	path = _T (""); // assume failure 

	// Get a pointer to the IShellLink interface. 
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
		IID_IShellLink, (LPVOID*)&psl); 
	if (SUCCEEDED(hres)) 
	{ 
		IPersistFile* ppf; 

		// Get a pointer to the IPersistFile interface. 
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf); 

		if (SUCCEEDED(hres)) 
		{ 

			// Load the shortcut. 
			hres = ppf->Load(lpszLinkFile, STGM_READ); 

			if (SUCCEEDED(hres)) 
			{ 
				// Resolve the link. 
				hres = psl->Resolve(hwnd, 0); 

				if (SUCCEEDED(hres)) 
				{ 
					// Get the path to the link target. 
					hres = psl->GetPath(path.GetBuffer(MAX_PATH), 
						MAX_PATH, 
						NULL, 
						SLGP_SHORTPATH);
					path.ReleaseBuffer();
					path = _T ("\"") + path + _T ("\"");
				} 
			} 

			// Release the pointer to the IPersistFile interface. 
			ppf->Release(); 
		} 

		// Release the pointer to the IShellLink interface. 
		psl->Release(); 
	} 
	return hres; 
}
