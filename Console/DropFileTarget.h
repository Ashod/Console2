#pragma once

static const CLIPFORMAT CF_SHELLIDLIST = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_SHELLIDLIST);

#define GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])
#define GetPIDLFolder(pida) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])

#define MK_CONTROL_ONLY(x) (((x) & MK_CONTROL) && ! ((x) & (MK_SHIFT | MK_ALT)))
HRESULT ResolveShortcut(HWND hwnd, LPCWSTR lpszLinkFile, CString &path);

class CDropFileTarget :
	public CComObjectRoot,
	public IDropTarget
{
private:
	CComPtr<IDropTargetHelper> m_spdth;

	DWORD m_dwEffect;

protected:
	CDropFileTarget();
	virtual bool HDropHandler(IDataObject *pDataObj, CString *pStr);
	virtual bool ShellIDListHandler(IDataObject *pDataObj, CString *pStr);
	virtual bool TextHandler(IDataObject *pDataObj, CString *pStr);

public:
	BEGIN_COM_MAP(CDropFileTarget)
		COM_INTERFACE_ENTRY(IDropTarget)
	END_COM_MAP()

	STDMETHOD(DragEnter) (IDataObject* , DWORD , POINTL , DWORD* );
	STDMETHOD(DragOver) (DWORD , POINTL , DWORD* );
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop) (IDataObject* , DWORD , POINTL , DWORD* );

	virtual HWND GetTargetHwnd() { return 0; };

	virtual bool GetData(IDataObject *, CString *);
};
