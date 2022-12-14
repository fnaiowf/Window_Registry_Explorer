#include"header.h"

WNDPROC oldEditProc;
SPLIT nSplit = SP_NONE;

int CALLBACK resultLVCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return lParam1 < lParam2 ? -1 : 1; //lParam1, lParam2 : 리스트뷰 인덱스 lParamSort : SortItemsEx 호출할 때 넘겨주는 파라미터
}

int CALLBACK LVCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TCHAR t[2][1000];

	ListView_GetItemText(hLV, lParam1, 0, t[0], sizeof(t[0]));
	ListView_GetItemText(hLV, lParam2, 0, t[1], sizeof(t[0]));

	if (lParam1 == 0) //기본값은 제일 위에 있어야 함
		return -1;
	else if (lParam2 == 0)
		return 1;
	else
		return _wcsicmp(t[0], t[1]); //대소문자 구분 없이 오름차순
}

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
		if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_CHECKED)
			return SP_HORZ;
	}

	return SP_NONE;
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

void addLVitem(HWND hlv, TCHAR* name, TCHAR* type, TCHAR* value, int index, TCHAR* path, LPARAM lParam)
{
	LVITEM item;
	TCHAR* b = NULL;
	int t = 0, it; //데이터 출력하는 리스트뷰에서는 name, type, value의 subitem 인덱스가 0 1 2인데 검색 결과 탭은 앞에 path가 더 있기 때문에 t변수를 더하는 식으로 해서 코드를 줄임

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = index;

	item.iSubItem = t++;
	item.pszText = hlv == hresultLV ? path : name;
	item.lParam = lParam;

	if (hlv == hresultLV)
	{
		item.mask = item.mask | LVIF_GROUPID;
		item.iGroupId = 1;
	}

	ListView_InsertItem(hlv, &item);

	item.mask = LVIF_TEXT;

	if (hlv == hresultLV)
	{
		item.iSubItem = t++;
		item.pszText = name;
		ListView_SetItem(hlv, &item);
	}

	item.iSubItem = t++;
	item.pszText = type;
	ListView_SetItem(hlv, &item);

	item.pszText = value;

	item.iSubItem = t;
	ListView_SetItem(hlv, &item);

	if (!b) free(b);
}

LVITEM getListViewItem(HWND handle, UINT mask, UINT index)
{
	static LVITEM li;
	li.mask = mask;
	li.iItem = index;
	ListView_GetItem(handle, &li);

	return li;
}

void getPathfromItem(HTREEITEM item, TCHAR* retpath)
{
	TVITEM parent;
	HTREEITEM root = TreeView_GetRoot(hTV);
	TCHAR temp[MAX_PATH_LENGTH] = {}, temp2[MAX_KEY_LENGTH] = {};
	memset(retpath, 0, sizeof(TCHAR) * MAX_PATH_LENGTH);

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

	while (1) //루트가 될 때까지 상위 노드로 올라가면서 경로 추가
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
	TCHAR temp[MAX_PATH_LENGTH], * ret, * context = NULL, temp2[MAX_PATH_LENGTH] = {};
	TVITEM ti;
	HTREEITEM item = TreeView_GetRoot(hTV), titem;
	wcscpy(temp, path);
	ret = wcstok(temp, L"\\", &context);
	ti.mask = TVIF_TEXT;
	ti.pszText = temp2;
	ti.cchTextMax = 100;

	while (1) //현재 노드에서 아래로 내려가면서 같은 위치에 있는 노드들을 검사해 경로에 해당하는 노드를 골라 아래로 계속 내려감, 더이상 wcstok로 경로를 자를 수 없으면 리턴
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

void setMarquee(int opt)
{
	LONG style;
	if (opt) //on
	{
		style = GetWindowLongPtr(hProgress, GWL_STYLE);
		SetWindowLongPtr(hProgress, GWL_STYLE, style | PBS_MARQUEE);
		SendMessage(hProgress, PBM_SETMARQUEE, 1, 20);
	}
	else //off
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
	POINT pt;
	RECT rt;
	void* item = NULL; //트리뷰의 item인지 리스트뷰의 item인지 모름
	int id = -1, index = -1;

	GetWindowRect(hLV, &rt);

	pt = { x, y };
	ScreenToClient(hTV, &pt);
	tvinfo.pt = pt;
	TreeView_HitTest(hTV, &tvinfo);

	pt = { x, y };
	ScreenToClient(hLV, &pt);
	lvinfo.pt = pt;
	ListView_HitTest(hLV, &lvinfo);

	pt = { x, y };
	ScreenToClient(hresultLV, &pt);
	lvinfo2.pt = pt;
	ListView_HitTest(hresultLV, &lvinfo2);

	pt = { x, y };

	menu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU2));
	hPopup = GetSubMenu(menu, 0);

	if (tvinfo.hItem != NULL) //TreeView
	{
		TreeView_SelectItem(hTV, tvinfo.hItem);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);

		if (tvinfo.hItem == TreeView_GetRoot(hTV))
		{
			for (int i = 0; i < 3; i++)
				EnableMenuItem(hPopup, i, MF_BYPOSITION | MF_DISABLED);
		}
		if (TreeView_GetParent(hTV, tvinfo.hItem) == TreeView_GetRoot(hTV))
		{
			EnableMenuItem(hPopup, ID_MENU2_DELETE, MF_BYCOMMAND | MF_DISABLED);
			EnableMenuItem(hPopup, ID_MENU2_RENAME, MF_BYCOMMAND | MF_DISABLED);
		}

		item = (void*)&(tvinfo.hItem);
		index = 0;
		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL); //TPM_RETURNCMD : 선택하면 값을 바로 리턴, 원래는 WM_COMMAND로 결과를 전달함
	}
	else if (lvinfo.iItem != -1) //ListView1
	{
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 1, MF_BYPOSITION);
		DeleteMenu(hPopup, 1, MF_BYPOSITION);
		item = (void*)&(lvinfo.iItem);
		index = 1;
		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);
	}
	else if (PtInRect(&rt, pt)) //데이터 표시하는 리스트뷰의 빈공간
	{
		for (int i = 0; i < 4; i++)
			DeleteMenu(hPopup, 0, MF_BYPOSITION);
		DeleteMenu(hPopup, 1, MF_BYPOSITION);
		DeleteMenu(GetSubMenu(hPopup, 0), 0, MF_BYPOSITION);
		DeleteMenu(GetSubMenu(hPopup, 0), 0, MF_BYPOSITION);

		HTREEITEM it = TreeView_GetSelection(hTV);
		item = (void*)&it;
		index = 3;

		if (TreeView_GetSelection(hTV) == TreeView_GetRoot(hTV))
		{
			for (int i = 0; i < 5; i++)
				EnableMenuItem(GetSubMenu(hPopup, 0), i, MF_BYPOSITION | MF_DISABLED);
		}

		id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);
	}
	else if (lvinfo2.iItem != -1) //ListView2
	{
		if (getListViewItem(hresultLV, LVIF_GROUPID, lvinfo2.iItem).iGroupId == 2)
			return;

		item = (void*)&(lvinfo2.iItem);
		index = 2;

		if (nowFindType == KEY)
			return;
		else if (nowFindType == VALUE)
		{
			DeleteMenu(hPopup, 3, MF_BYPOSITION);
			DeleteMenu(hPopup, 3, MF_BYPOSITION);
			DeleteMenu(hPopup, 0, MF_BYPOSITION);
			DeleteMenu(hPopup, 0, MF_BYPOSITION);
		}
		else
		{
			DeleteMenu(hPopup, 3, MF_BYPOSITION);
			DeleteMenu(hPopup, 3, MF_BYPOSITION);

			LVITEM li;
			li.mask = LVIF_GROUPID;
			li.iItem = lvinfo2.iItem;
			ListView_GetItem(hresultLV, &li);

			if (li.iGroupId == 0)
				ModifyMenu(hPopup, 0, MF_BYPOSITION | MF_STRING, ID_MENU2_EXCEPT, L"해제");
			else
				ModifyMenu(hPopup, 0, MF_BYPOSITION | MF_STRING, ID_MENU2_EXCEPT, L"제외");
		}

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

			while (citem != NULL) //새 키 #? 가 중복되게 하지 않기 위해 정확한 값을 구함
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

					TreeView_SelectItem(hTV, citem);
					TreeView_EditLabel(hTV, citem);
				}
				RegCloseKey(hkey);
			}
			break;
		case ID_MENU2_STR:
		case ID_MENU2_BINARY:
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
				if (litem == 0) //기본값인 경우
				{
					if (RegDeleteValue(hkey, L"") == ERROR_SUCCESS)
					{
						if (ListView_GetItemCount(hresultLV) != 0) //resultLV에서도 삭제
						{
							for (int i = 0; i < ListView_GetItemCount(hresultLV); i++)
							{
								if (getListViewItem(hresultLV, LVIF_PARAM, i).lParam < 0) //기본값인 경우
								{
									TCHAR path[MAX_PATH_LENGTH];
									ListView_GetItemText(hresultLV, i, 0, path, sizeof(path));
									if (wcscmp(temp, path) == 0)
									{
										ListView_DeleteItem(hresultLV, i);
										break;
									}
								}
							}
						}

						TCHAR t[11] = L"(값 설정 안 됨)";
						ListView_SetItemText(hLV, 0, 2, t);
					}
				}
				else
				{
					ListView_GetItemText(hLV, litem, 0, tstr, sizeof(tstr));
					if (RegDeleteValue(hkey, tstr) == ERROR_SUCCESS)
					{
						if (ListView_GetItemCount(hresultLV) != 0) //resultLV에서도 삭제
						{
							TCHAR tp[3000];
							for (int i = 0; i < ListView_GetItemCount(hresultLV); i++)
							{
								ListView_GetItemText(hresultLV, i, 1, tp, sizeof(tp)); //name
								if (wcscmp(tstr, tp) == 0)
								{
									ListView_GetItemText(hresultLV, i, 0, tp, sizeof(tp)); //path
									if (wcscmp(temp, tp) == 0)
									{
										ListView_DeleteItem(hresultLV, i);
										break;
									}
								}
							}
						}
						ListView_GetItemText(hLV, litem, 2, tstr, sizeof(tstr)); //type
						if (getType(tstr) >= 4) //데이터만 다시 로드
						{
							isDataLoad = 1;
							HTREEITEM it = TreeView_GetSelection(hTV);
							TreeView_SelectItem(hTV, TreeView_GetRoot(hTV));
							TreeView_SelectItem(hTV, it);
							isDataLoad = 0;
						}

						ListView_DeleteItem(hLV, litem);
					}
				}
				
				RegCloseKey(hkey);
			}
			break;
		case ID_MENU2_MODIFY:
			ListView_GetItemText(hLV, litem, 1, temp, sizeof(temp));
			openModifyDlg(getType(temp));
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

			ListView_SortItemsEx(hresultLV, resultLVCompareFunc, 0); //아이템 그룹을 바꾸면 바꾼 아이템은 무조건 제일 아래로 가기 때문에 원래 순서대로 정렬
			break;
		case ID_MENU2_MODIFY:
			ListView_GetItemText(hresultLV, litem, 2, temp, sizeof(temp));
			openModifyDlg(getType(temp));

			break;
		case ID_MENU2_DELETE:
			if (MessageBox(hWndMain, L"정말 삭제하시겠습니까?", L"알림", MB_YESNO) == IDNO)
				break;

			ListView_GetItemText(hresultLV, litem, 0, temp, sizeof(temp));

			if ((hkey = _RegOpenKeyEx(getBasicKey(temp), temp)) != NULL)
			{
				if (getListViewItem(hresultLV, LVIF_PARAM, litem).lParam < 0) //기본값 체크
					*temp = 0;
				else
					ListView_GetItemText(hresultLV, litem, 1, temp, sizeof(temp));

				if (RegDeleteValue(hkey, temp) == ERROR_SUCCESS)
				{
					TCHAR tp[2][MAX_PATH_LENGTH];
					GetWindowText(hEdit, tp[0], 3000);
					ListView_GetItemText(hresultLV, litem, 0, tp[1], MAX_PATH_LENGTH);
					if (wcscmp(tp[0], tp[1]) == 0) //현재 선택되어 있는 path와 검색된 경로 비교해서 동일하면 hLV에서도 항목 삭제
					{
						if (*temp == 0) //기본값 체크
						{
							TCHAR t[11] = L"(값 설정 안 됨)";
							ListView_SetItemText(hLV, 0, 2, t);
						}
						else
						{
							for (int i = 1; i < ListView_GetItemCount(hLV); i++) //검색된 아이템 이름과 동일한 아이템 찾기
							{
								ListView_GetItemText(hLV, i, 0, tp[0], sizeof(tp[0]));
								if (wcscmp(tp[0], temp) == 0)
								{
									ListView_DeleteItem(hLV, i);
									break;
								}
							}

							ListView_GetItemText(hresultLV, litem, 2, tstr, sizeof(tstr)); //type
							if (getType(tstr) >= 4) //데이터만 다시 로드
							{
								isDataLoad = 1;
								HTREEITEM it = TreeView_GetSelection(hTV);
								TreeView_SelectItem(hTV, TreeView_GetRoot(hTV));
								TreeView_SelectItem(hTV, it);
								isDataLoad = 0;
							}
						}
					}

					ListView_DeleteItem(hresultLV, litem);
				}

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
		case ID_MENU2_BINARY:
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

void openModifyDlg(int type)
{
	if (!IsWindow(hDlgModify))
	{
		if (type >= 5) DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWndMain, (DLGPROC)ModifyBinaryDlgProc);
		else if (type == 4) DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWndMain, (DLGPROC)ModifyMultiSzDlgProc);
		else DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWndMain, (DLGPROC)ModifySzNumDlgProc);

		ShowWindow(hDlgModify, TRUE);
	}
}

void AcceleratorProcess(HWND hWnd, int id)
{
	static char tabOrder[8] = {};

	switch (id)
	{
	case ID_ACCELERATOR_F5: //F5
	{
		if (funcState == ING) //검색 중이면 X
			break;

		THREAD_DATA* d = (THREAD_DATA*)malloc(sizeof(THREAD_DATA));
		d->threadType = REFRESH;
		GetWindowText(hEdit, d->path, MAX_PATH_LENGTH);

		CreateThread(NULL, 0, ThreadFunc, d, NULL, NULL);
		break;
	}
	case ID_ACCELERATOR_CTRL_F: //Ctrl + F
		if (!IsWindow(hDlgFind))
		{
			hDlgFind = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWndMain, (DLGPROC)FindDlgProc);
			ShowWindow(hDlgFind, SW_SHOW);
		}
		break;
	}
}