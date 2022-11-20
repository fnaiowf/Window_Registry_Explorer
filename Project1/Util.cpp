#include"header.h"

WNDPROC oldEditProc;

SPLIT getSplitter(POINT pt)
{
	RECT crt, vrt, hrt;

	GetClientRect(hWndMain, &crt);
	SetRect(&vrt, treeWidth - GAP, 0, treeWidth, crt.bottom - resultHeight);
	if (PtInRect(&vrt, pt))
		return SP_VERT;
	SetRect(&hrt, 0, crt.bottom - resultHeight, crt.right, crt.bottom - resultHeight + GAP);
	if (PtInRect(&hrt, pt))
	{
		if(GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_CHECKED)
			return SP_HORZ;
	}

	return SP_NONE;
}

const TCHAR* getBasicKey(int idx)
{
	switch (idx)
	{
	case 0:
		return TEXT("HKEY_CLASSES_ROOT");
	case 1:
		return TEXT("HKEY_LOCAL_MACHINE");
	case 2:
		return TEXT("HKEY_CURRENT_USER");
	case 3:
		return TEXT("HKEY_USERS");
	case 4:
		return TEXT("HKEY_CURRENT_CONFIG");
	default:
		return NULL;
	}
}

int getBasicKey(TCHAR* path)
{
	TCHAR temp[MAX_PATH_LENGTH], *pos;

	memcpy(temp, path, sizeof(temp));
	pos = getValidPath(temp);

	if (pos == NULL)
		pos = temp + 4;

	*(pos-1) = NULL;

	if (wcscmp(temp + 4, L"HKEY_CLASSES_ROOT") == 0)
		return 0;
	else if (wcscmp(temp + 4, L"HKEY_LOCAL_MACHINE") == 0)
		return 1;
	else if (wcscmp(temp + 4, L"HKEY_CURRENT_USER") == 0)
		return 2;
	else if (wcscmp(temp + 4, L"HKEY_USERS") == 0)
		return 3;
	else if (wcscmp(temp + 4, L"HKEY_CURRENT_CONFIG") == 0)
		return 4;
	else
		return -1;
}

TCHAR* getValidPath(TCHAR* path)
{
	TCHAR* pos = path + 4;
	
	if (*(pos - 1) == L'\0') //컴퓨터 일 경우
		return NULL;

	while (*pos != L'\\')
	{
		pos++;
		if (*pos == L'\0') //컴퓨터\HKEY_~ 일 경우
			return NULL;
	}

	return pos + 1;
}

int getType(TCHAR* type)
{
	
	if (wcscmp(type, L"REG_DWORD") == 0)
		return 0;
	else if (wcscmp(type, L"REG_QWORD") == 0)
		return 1;
	else if (wcscmp(type, L"REG_SZ") == 0)
		return 2;
	else if (wcscmp(type, L"REG_EXPAND_SZ") == 0)
		return 3;
	else if (wcscmp(type, L"REG_MULTI_SZ") == 0)
		return 4;
	else if (wcscmp(type, L"REG_BINARY") == 0)
		return 5;

	else return -1;
}

void initWindow()
{
	RECT crt;
	LVCOLUMN col;
	LVGROUP group;

	GetWindowRect(GetDesktopWindow(), &crt);
	MoveWindow(hWndMain, crt.right / 2 - WINDOW_WIDTH / 2, crt.bottom / 2 - WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT, TRUE);

	treeWidth = 300;
	resultHeight = 0;
	hTV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL, WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_EDITLABELS, 0, 0, 0, 0, hWndMain, (HMENU)ID_TV, g_hInst, NULL);
	hLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS | LVS_EDITLABELS, 0, 0, 0, 0, hWndMain, (HMENU)ID_LV, g_hInst, NULL);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = 150;
	col.pszText = LPWSTR(TEXT("이름"));
	col.iSubItem = 0;
	SendMessage(hLV, LVM_INSERTCOLUMN, 0, (LPARAM)&col);

	col.pszText = LPWSTR(TEXT("종류"));
	col.iSubItem = 1;
	SendMessage(hLV, LVM_INSERTCOLUMN, 1, (LPARAM)&col);

	col.cx = WINDOW_WIDTH - 300;
	col.pszText = LPWSTR(TEXT("데이터"));
	col.iSubItem = 2;
	SendMessage(hLV, LVM_INSERTCOLUMN, 2, (LPARAM)&col);

	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", NULL, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWndMain, (HMENU)ID_EDIT, g_hInst, NULL);
	oldEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)MainEditSubProc);
	
	hStatic = CreateWindowEx(WS_EX_CLIENTEDGE, L"static", NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWndMain, NULL, g_hInst, NULL);
	hProgress = CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWndMain, NULL, g_hInst, NULL);
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 10));

	hresultLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS, 0, 0, 0, 0, hWndMain, (HMENU)ID_resLV, g_hInst, NULL);
	ListView_SetExtendedListViewStyleEx(hresultLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	ListView_EnableGroupView(hresultLV, TRUE);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = 150;
	col.pszText = LPWSTR(TEXT("경로"));
	col.iSubItem = 0;
	SendMessage(hresultLV, LVM_INSERTCOLUMN, 0, (LPARAM)&col);

	col.pszText = LPWSTR(TEXT("이름"));
	col.iSubItem = 1;
	SendMessage(hresultLV, LVM_INSERTCOLUMN, 1, (LPARAM)&col);

	col.pszText = LPWSTR(TEXT("종류"));
	col.iSubItem = 2;
	SendMessage(hresultLV, LVM_INSERTCOLUMN, 2, (LPARAM)&col);

	col.cx = WINDOW_WIDTH - 450;
	col.pszText = LPWSTR(TEXT("데이터"));
	col.iSubItem = 3;
	SendMessage(hresultLV, LVM_INSERTCOLUMN, 3, (LPARAM)&col);

	group.cbSize = sizeof(LVGROUP);
	group.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE;
	group.pszHeader = (LPWSTR)TEXT("제외 항목");
	group.iGroupId = 0;
	group.state = LVGS_COLLAPSIBLE;

	ListView_InsertGroup(hresultLV, -1, &group);

	group.pszHeader = (LPWSTR)TEXT("검색 결과");
	group.iGroupId = 1;

	ListView_InsertGroup(hresultLV, -1, &group);

	group.pszHeader = (LPWSTR)TEXT("변경 기록");
	group.iGroupId = 2;

	ListView_InsertGroup(hresultLV, -1, &group);

	CheckMenuItem(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_UNCHECKED);

	RegisterHotKey(hWndMain, 0, MOD_NOREPEAT, VK_F5);
	RegisterHotKey(hWndMain, 1, MOD_NOREPEAT | MOD_CONTROL, 'F');

	SetFocus(hTV);
}

HTREEITEM addTVitem(const TCHAR* text, HTREEITEM parent, int bkey)
{
	TVINSERTSTRUCT ti;

	ti.hParent = parent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_PARAM;
	ti.item.pszText = (LPWSTR)text;
	ti.item.lParam = (LPARAM)bkey;

	return (HTREEITEM)SendMessage(hTV, TVM_INSERTITEM, 0, (LPARAM)&ti);
}

void addLVitem(HWND hlv, TCHAR* name, TCHAR* type, TCHAR* value, int index, TCHAR* opt, LPARAM lParam)
{
	LVITEM item;
	TCHAR *b = NULL;
	int t = 0, it;

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = index;
	item.iSubItem = t++;
	item.pszText = name;
	item.lParam = lParam;

	if (hlv == hresultLV)
	{
		item.mask = item.mask | LVIF_GROUPID;
		item.iGroupId = 1;
	}
	
	ListView_InsertItem(hlv, &item);

	item.mask = LVIF_TEXT;

	if (opt != NULL)
	{
		item.iSubItem = t++;
		item.pszText = opt;
		ListView_SetItem(hlv, &item);
	}

	item.iSubItem = t++;
	item.pszText = type;
	ListView_SetItem(hlv, &item);

	it = getType(type);
	if (it == 5)
	{
		if (lvData.byteData[lvData.nByte - 1].size != 0)
		{
			b = (TCHAR*)calloc(lvData.byteData[lvData.nByte - 1].size * 3, sizeof(TCHAR));
			byteToString(lvData.byteData[lvData.nByte - 1].bytes, lvData.byteData[lvData.nByte - 1].size, b);

			cutString(b);

			item.pszText = b;
		}
		else
			item.pszText = value;
	}
	else if (it == 4)
	{
		if (hlv == hLV)
		{
			if (lvData.mulstrData[lvData.nMul - 1].size > 2)
			{
				b = (TCHAR*)calloc(lvData.mulstrData[lvData.nMul - 1].size, sizeof(TCHAR));
				wsprintf(b, L"%ws", lvData.mulstrData[lvData.nMul - 1].strings[0]);

				for (int i = 1; i < lvData.mulstrData[lvData.nMul - 1].nString; i++)
					wsprintf(b, L"%ws %ws", b, (lvData.mulstrData[lvData.nMul - 1].strings)[i]);

				cutString(b);

				item.pszText = b;
			}
			else
				item.pszText = value;
		}
		else
		{
			cutString(value);
			item.pszText = value;
		}
	}
	else
		item.pszText = value;

	item.iSubItem = t;
	ListView_SetItem(hlv, &item);

	if (!b) free(b);
}

void getPathfromItem(HTREEITEM item, TCHAR* retpath)
{
	TVITEM parent;
	HTREEITEM root = TreeView_GetRoot(hTV);
	TCHAR temp[MAX_PATH_LENGTH]={}, temp2[MAX_KEY_LENGTH] = {};
	memset(retpath, 0, sizeof(TCHAR)*MAX_PATH_LENGTH);
	
	parent.mask = TVIF_TEXT;
	parent.hItem = item;
	parent.pszText = temp2;
	parent.cchTextMax = 100;
	TreeView_GetItem(hTV, &parent);

	if (parent.hItem == root)
	{
		wcscpy(retpath, temp2);
		return;
	}
	wsprintf(temp, L"%ws", temp2);

	while (1)
	{
		parent.hItem = TreeView_GetParent(hTV, parent.hItem);
		memset(temp2, 0, sizeof(temp2));
		TreeView_GetItem(hTV, &parent);
		wsprintf(retpath, L"%ws\\%ws", temp2, temp);
		
		if (parent.hItem == root)
			break;
		wsprintf(temp, L"%ws", retpath);
	}
}

HTREEITEM getItemfromPath(const TCHAR* path)
{
	TCHAR temp[MAX_PATH_LENGTH], *ret, *context = NULL, temp2[MAX_PATH_LENGTH]={};
	TVITEM ti;
	HTREEITEM item = TreeView_GetRoot(hTV), titem;
	wcscpy(temp, path);
	ret = wcstok(temp, L"\\", &context);
	ti.mask = TVIF_TEXT;
	ti.hItem = item;
	ti.pszText = temp2;
	ti.cchTextMax = 100;
	
	while(ret != NULL)
	{
		while (1)
		{
			ti.hItem = item;
			TreeView_GetItem(hTV, &ti);
			if (wcscmp(ret, temp2) == 0)
				break;

			titem = TreeView_GetNextSibling(hTV, item);
			if (titem == NULL)
				return 0;
			item = titem;
		}
		ret = wcstok(NULL, L"\\", &context);
		if (ret == NULL)
			return item;

		item = TreeView_GetChild(hTV, item);
	}

	return 0;
}

const TCHAR* getTypeName(int type)
{
	switch (type)
	{
	case REG_SZ:
		return L"REG_SZ";
	case REG_DWORD:
		return L"REG_DWORD";
	case REG_BINARY:
		return L"REG_BINARY";
	case REG_EXPAND_SZ:
		return L"REG_EXPAND_SZ";
	case REG_MULTI_SZ:
		return L"REG_MULTI_SZ";
	case REG_NONE:
		return L"REG_NONE";
	case REG_LINK:
		return L"REG_LINK";
	case REG_QWORD:
		return L"REG_QWORD";
	default:
		wprintf(L"%d", type);
		return L"";
	}
}

void setMarquee(int opt)
{
	LONG style;
	if (opt)
	{
		style = GetWindowLongPtr(hProgress, GWL_STYLE);
		SetWindowLongPtr(hProgress, GWL_STYLE, style | PBS_MARQUEE);
		SendMessage(hProgress, PBM_SETMARQUEE, 1, 20);
	}
	else
	{
		SendMessage(hProgress, PBM_SETMARQUEE, 0, 0);
		style = GetWindowLongPtr(hProgress, GWL_STYLE);
		SetWindowLongPtr(hProgress, GWL_STYLE, style & ~PBS_MARQUEE);
		//프로그레스바는 증가할 때는 목표 값까지 천천히 올라가지만 감소할 때는 한 번에 감소함 이를 이용하여 한 번에 증가시키는 방법
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 11));
		SendMessage(hProgress, PBM_SETPOS, 11, 0);
		SendMessage(hProgress, PBM_SETPOS, 10, 0);
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
	}
}

void openPopupMenu(int x, int y)
{
	HMENU menu, hPopup;
	TVHITTESTINFO tvinfo;
	LVHITTESTINFO lvinfo, lvinfo2;
	POINT pt2;
	RECT rt;
	void* item = NULL;
	int id = -1, index = -1;

	GetWindowRect(hLV, &rt);

	pt2 = { x, y };
	ScreenToClient(hTV, &pt2);
	tvinfo.pt = pt2;
	TreeView_HitTest(hTV, &tvinfo);

	pt2 = { x, y };
	ScreenToClient(hLV, &pt2);
	lvinfo.pt = pt2;
	ListView_HitTest(hLV, &lvinfo);

	pt2 = { x, y };
	ScreenToClient(hresultLV, &pt2);
	lvinfo2.pt = pt2;
	ListView_HitTest(hresultLV, &lvinfo2);

	pt2 = { x, y };

	menu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU2));
	hPopup = GetSubMenu(menu, 0);

	if (tvinfo.hItem != NULL)
	{
		TreeView_SelectItem(hTV, tvinfo.hItem);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);

		if (tvinfo.hItem == TreeView_GetRoot(hTV))
		{
			for(int i=0;i<3;i++)
				EnableMenuItem(hPopup, i, MF_BYPOSITION | MF_DISABLED);
		}
		if (TreeView_GetParent(hTV, tvinfo.hItem) == TreeView_GetRoot(hTV))
		{
			EnableMenuItem(hPopup, ID_MENU2_DELETE, MF_BYCOMMAND | MF_DISABLED);
			EnableMenuItem(hPopup, ID_MENU2_RENAME, MF_BYCOMMAND | MF_DISABLED);
		}

		item = (void*)&(tvinfo.hItem);
		index = 0;
		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);
	}
	else if (lvinfo.iItem != -1)
	{
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 1, MF_BYPOSITION);
		DeleteMenu(hPopup, 1, MF_BYPOSITION);
		item = (void*)&(lvinfo.iItem);
		index = 1;
		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);
	}
	else if (PtInRect(&rt, pt2))
	{
		for(int i=0;i<4;i++)
			DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 1, MF_BYPOSITION);
		DeleteMenu(GetSubMenu(hPopup, 0), 0, MF_BYPOSITION);
		DeleteMenu(GetSubMenu(hPopup, 0), 0, MF_BYPOSITION);

		HTREEITEM it = TreeView_GetSelection(hTV);
		item = (void*)&it;
		index = 3;

		if (TreeView_GetSelection(hTV) == TreeView_GetRoot(hTV))
		{
			for(int i=0;i<5;i++)
				EnableMenuItem(GetSubMenu(hPopup, 0), i, MF_BYPOSITION | MF_DISABLED);
		}

		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);
	}
	else if (lvinfo2.iItem != -1)
	{
		if (getListViewItem(hresultLV, LVIF_GROUPID, lvinfo2.iItem).iGroupId == 2)
			return;

		DeleteMenu(hPopup, 3, MF_BYPOSITION);
		DeleteMenu(hPopup, 3, MF_BYPOSITION);
		item = (void*)&(lvinfo2.iItem);
		index = 2;
		
		LVITEM li;
		li.mask = LVIF_GROUPID;
		li.iItem = lvinfo2.iItem;
		ListView_GetItem(hresultLV, &li);

		if (li.iGroupId == 0)
			ModifyMenu(hPopup, 0, MF_BYPOSITION | MF_STRING, ID_MENU2_EXCEPT, L"해제");
		else
			ModifyMenu(hPopup, 0, MF_BYPOSITION | MF_STRING, ID_MENU2_EXCEPT, L"제외");

		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);
	}

	processPopup(id, index, item);

	DestroyMenu(menu);
}

void processPopup(int id, int index, void* item)
{
	HTREEITEM hitem, citem;
	HKEY hkey;
	TCHAR temp[MAX_PATH_LENGTH], tstr[MAX_PATH_LENGTH];
	TVITEM ti;
	LVITEM li;
	int litem, bindex, t, wtoi;

	switch (index)
	{
	case 0: //TreeView
		hitem = *(HTREEITEM*)item;
		switch (id)
		{
		case ID_MENU2_RENAME:
			TreeView_EditLabel(hTV, hitem);
			
			break;
		case ID_MENU2_KEY:
			ti.mask = TVIF_TEXT;
			ti.pszText = temp;
			ti.cchTextMax = sizeof(temp);

			citem = TreeView_GetChild(hTV, hitem);
			t = 0;
			wsprintf(tstr, L"새 키 #");

			while (citem != NULL)
			{
				ti.hItem = citem;
				TreeView_GetItem(hTV, &ti);
				
				if (wcsncmp(temp, tstr, 5) == 0)
				{
					wtoi = _wtoi(temp + 5);
					if (wtoi > t) t = wtoi;
				}

				citem = TreeView_GetNextSibling(hTV, citem);
			}
			wsprintf(temp, L"%ws%d", tstr, t + 1);
			getPathfromItem(hitem, tstr);

			bindex = getBasicKey(tstr);
			if ((hkey = _RegOpenKeyEx(bindex, tstr)) != NULL)
			{
				HKEY createdKey;
				if (RegCreateKey(hkey, temp, &createdKey) == ERROR_SUCCESS)
				{
					RegCloseKey(createdKey);
					citem = addTVitem(temp, hitem, bindex);

					TreeView_EnsureVisible(hTV, citem);

					TreeView_EditLabel(hTV, citem);
				}
				RegCloseKey(hkey);
			}
			break;
		case ID_MENU2_STR:
		case ID_MENU2_DWORD:
		case ID_MENU2_QWORD:
		case ID_MENU_EXSTR:
		case ID_MENU2_MULSTR:
			createValue(id - ID_MENU2_DWORD, hitem);
			break;
		case ID_MENU2_DELETE:
			if (MessageBox(hWndMain, L"정말 삭제하시겠습니까?(하위키까지 전부 삭제됩니다)", L"알림", MB_YESNO) == IDNO)
				break;

			getPathfromItem(hitem, temp);
			deleteAllSubkey(temp, hitem);
			break;
		default:
			break;
		}
		break;
	case 1: //ListView
		litem = *(int*)item;
		switch (id)
		{
		case ID_MENU2_DELETE:
			if (MessageBox(hWndMain, L"정말 삭제하시겠습니까?", L"알림", MB_YESNO) == IDNO)
				break;

			GetWindowText(hEdit, temp, sizeof(temp));

			if ((hkey = _RegOpenKeyEx(getBasicKey(temp), temp)) != NULL)
			{
				if (getListViewItem(hLV, LVIF_PARAM, litem).lParam == -1)
				{
					if (RegDeleteValue(hkey, L"") == ERROR_SUCCESS)
					{
						TCHAR t[11] = L"(값 설정 안 됨)";
						ListView_SetItemText(hLV, 0, 2, t);
					}
				}
				else
				{
					ListView_GetItemText(hLV, litem, 0, temp, sizeof(temp));
					if (RegDeleteValue(hkey, temp) == ERROR_SUCCESS)
						ListView_DeleteItem(hLV, litem);
				}

				RegCloseKey(hkey);
			}
			break;
		case ID_MENU2_MODIFY:
			if (!IsWindow(hDlgModify))
			{
				ListView_GetItemText(hLV, litem, 1, temp, sizeof(temp));
				t = getType(temp);

				if (t == 5) hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWndMain, (DLGPROC)ModifyBinaryDlgProc);
				else if(t == 4) hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWndMain, (DLGPROC)ModifyMultiSzDlgProc);
				else hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWndMain, (DLGPROC)ModifySzNumDlgProc);

				ShowWindow(hDlgModify, SW_SHOW);
			}
			break;
		default:
			break;
		}
		break;
	case 2: //ListView2
		litem = *(int*)item;
		switch (id)
		{
		case ID_MENU2_EXCEPT:
			li.mask = LVIF_GROUPID | LVIF_PARAM;
			li.iItem = litem;
			ListView_GetItem(hresultLV, &li);

			li.iGroupId = li.iGroupId ? 0 : 1;
			ListView_SetItem(hresultLV, &li);

			if (li.iGroupId == 0 && li.lParam & CHECKBIT) //이미 바꾼 항목을 제외로 옮길 때
				nchanged--;

			ListView_SortItemsEx(hresultLV, CompareFunc, 0);
			break;
		case ID_MENU2_MODIFY:
			if (!IsWindow(hDlgModify))
			{
				ListView_GetItemText(hresultLV, litem, 2, temp, sizeof(temp));
				t = getType(temp);
				if (t == 5) hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWndMain, (DLGPROC)ModifyBinaryDlgProc);
				else if (t == 4) hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWndMain, (DLGPROC)ModifyMultiSzDlgProc);
				else hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWndMain, (DLGPROC)ModifySzNumDlgProc);


				ShowWindow(hDlgModify, SW_SHOW);
			}
			break;
		case ID_MENU2_DELETE:
			if (MessageBox(hWndMain, L"정말 삭제하시겠습니까?", L"알림", MB_YESNO) == IDNO)
				break;

			ListView_GetItemText(hresultLV, litem, 0, temp, sizeof(temp));

			if ((hkey = _RegOpenKeyEx(getBasicKey(temp), temp)) != NULL)
			{
				ListView_GetItemText(hresultLV, litem, 1, temp, sizeof(temp));
				RegDeleteValue(hkey, temp);
				ListView_DeleteItem(hresultLV, litem);

				RegCloseKey(hkey);
			}
			break;
		default:
			break;
		}
		break;
	case 3: //리스트뷰에서 항목 아닌 곳
		hitem = *(HTREEITEM*)item;
		switch (id)
		{
		case ID_MENU2_STR:
		case ID_MENU2_DWORD:
		case ID_MENU2_QWORD:
		case ID_MENU_EXSTR:
		case ID_MENU2_MULSTR:
			createValue(id - ID_MENU2_DWORD, hitem);
			break;
		default:
			break;
		}
	}
}

LVITEM getListViewItem(HWND handle, UINT mask, UINT index)
{
	static LVITEM li;
	li.mask = mask;
	li.iItem = index;
	ListView_GetItem(handle, &li);

	return li;
}

void byteToString(BYTE* bytes, int size, TCHAR* dest)
{
	wsprintf(dest, L"%02X", bytes[0]);
	for (int i = 1; i < size; ++i)
	{
		TCHAR hex[4];
		wsprintf(hex, L" %02X", bytes[i]);
		wcscat(dest, hex);
	}
}

int is_number(TCHAR* string, int base)
{
	TCHAR* pos = string;
	while (*pos != NULL)
	{
		if (base)
		{
			if (!(*pos <= '9' && *pos >= '0'))
				return 0;
		}
		else
		{
			if (!(*pos <= '9' && *pos >= '0') && !(*pos <= 'F' && *pos >= 'A') && !(*pos <= 'f' && *pos >= 'a'))
				return 0;
		}
		
		pos++;
	}

	return 1;
}

int splitMulSz(TCHAR* data, int size, TCHAR*** strings, int alloc)
{
	int t = 0, count = 0, len;
	TCHAR* adr;
	adr = data;

	while (1)
	{
		len = wcslen(adr);

		if(alloc)
			(*strings)[count] = (TCHAR*)malloc(sizeof(TCHAR) * (len + 1));

		wsprintf((*strings)[count], adr);

		t += len + 1;
		adr += len + 1;
		count++;

		if (t >= size / sizeof(TCHAR) - 1)
			break;

		if(alloc)
			*strings = (TCHAR**)realloc(*strings, sizeof(TCHAR*) * (count + 1));
	}

	return count;
}

void concatMulSz(TCHAR* strings, int len, TCHAR* ret) //for문으로 바꾸기
{
	memset(ret, 0, (len + 1) * 2);
	for (int i = 0; i < len; i++)
	{
		if (strings[i] == 0) ret[i] = ' ';
		else ret[i] = strings[i];
	}
}

void freeMemory()
{
	if (lvData.byteData != NULL)
	{
		for (int i = 0; i < lvData.nByte; i++)
			free(lvData.byteData[i].bytes);
		free(lvData.byteData);
	}

	if (lvData.mulstrData != NULL)
	{
		for (int i = 0; i < lvData.nMul; i++)
		{
			for (int j = 0; j < lvData.mulstrData[j].nString; j++)
				free((lvData.mulstrData[i]).strings[j]);

			free((lvData.mulstrData[i]).strings);
		}

		free(lvData.mulstrData);
	}
	
}

void cutString(TCHAR* string)
{
	if (wcslen(string) >= 200)
	{
		string[191] = 0; //레지스트리 탐색기에서 보여주는 최대 길이
		wsprintf(string, L"%ws...", string);
	}
}