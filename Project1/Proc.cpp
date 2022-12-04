#include"header.h"

HWND hWndMain, hTV, hLV, hEdit, hStatic, hresultLV, hProgress, hDlgFind, hDlgModify;
WNDPROC oldDlgEditProc[3];
TCHAR temp[MAX_PATH_LENGTH];

int treeWidth, resultHeight, nchanged, isDataLoad, funcState; //funcState : enumRegistry함수에서 값이 0이면 함수를 빠져나옴(검색 중지시킬때)
SPLIT nSplit = SP_NONE;

DWORD WINAPI ThreadFunc(LPVOID temp)
{
	funcState = FINDING;

	if (temp != NULL && ((DATA*)temp)->t_type == REFRESH) //F5 눌렀을 때 기존에 추가되어 있던 것들 전부 삭제
	{
		ListView_DeleteAllItems(hLV);
		ListView_DeleteAllItems(hresultLV);
		SetWindowText(hEdit, NULL);
		TreeView_DeleteAllItems(hTV);
	}

	if(temp == NULL || ((DATA*)temp)->t_type == REFRESH)
		if (IsWindow(hDlgFind))
			EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_FIND), FALSE);

	setMarquee(1); //프로그레스바 ON
	if (temp != NULL && ((DATA*)temp)->t_type == CHANGE) //전부 바꾸기 체크한 경우
	{
		for (int i = 0; i < ListView_GetItemCount(hresultLV); i++)
			if (getListViewItem(hresultLV, LVIF_GROUPID, i).iGroupId == 1)
			{
				LVITEM li;
				changeValue(i, (DATA*)temp);
				li.mask = LVIF_GROUPID;
				li.iItem = i;
				li.iGroupId = 2;
				ListView_SetItem(hresultLV, &li);
			}

		MessageBox(hWndMain, L"변경 완료", L"알림", MB_OK);
		EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_FIND), TRUE);
		EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE), FALSE);
	}
	else
	{
		enumRegistry((DATA*)temp);
		
		if(temp == NULL)
			TreeView_SelectItem(hTV, TreeView_GetRoot(hTV));
		else
		{
			HTREEITEM item;

			switch (((DATA*)temp)->t_type)
			{
			case REFRESH:
				item = getItemfromPath(((DATA*)temp)->path); //새로고침 하기 전의 경로로 이동
				TreeView_SelectItem(hTV, item);
				TreeView_EnsureVisible(hTV, item);
				SetWindowText(hEdit, ((DATA*)temp)->path);
				break;
			case FIND:
				SetWindowText(GetDlgItem(hDlgFind, IDC_BUTTON_FIND), L"찾기");
				if (ListView_GetItemCount(hresultLV) != 0 && IsDlgButtonChecked(hDlgFind, IDC_CHECK_CHANGE))
					EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE), TRUE);

				SetFocus(hresultLV);
				ListView_DeSelectAll(hresultLV);
				ListView_SetSelectionMark(hresultLV, 0);
				ListView_SetItemState(hresultLV, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				free((DATA*)temp);
				break;
			}
		}

		if (IsWindow(hDlgFind))
			EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_FIND), TRUE);
	}

	funcState = DEFAULT;
	setMarquee(0); //프로그레스바 OFF

	return 0;
}

int CALLBACK resultLVCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return lParam1 < lParam2 ? -1 : 1; //lParam1, lParam2 : 리스트뷰 인덱스 lParamSort : SortItemsEx 호출할 때 넘겨주는 파라미터
}

int CALLBACK LVCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TCHAR t[2][1000], type[2][20];

	ListView_GetItemText(hLV, lParam1, 0, t[0], sizeof(t[0]));
	ListView_GetItemText(hLV, lParam2, 0, t[1], sizeof(t[0]));

	if (lParam1 == 0) //기본값은 제일 위에 있어야 함
		return -1;
	else if (lParam2 == 0)
		return 1;
	else
		return _wcsicmp(t[0], t[1]); //대소문자 구분 없이 오름차순
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	RECT crt;
	POINT pt;
	DWORD t;
	HDC hdc;
	HMENU hmenu;
	static DATA data;
	static TCHAR oldName[1000];
	static TVITEM tvitem;

	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		
		initWindow();
		lvData.byteData = (BYTE_DATA*)malloc(sizeof(BYTE_DATA));
		lvData.mulstrData = (MULSZ_DATA*)malloc(sizeof(MULSZ_DATA));
		lvData.nByte = 0;
		lvData.nMul = 0;
		isDataLoad = 0;
		
		CreateThread(NULL, 0, ThreadFunc, NULL, NULL, NULL); //맨 처음 레지스트리 키 로드
		return 0;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) //프로그램 크기에 맞춰 자동 조절
		{
			GetClientRect(hWnd, &crt);
			MoveWindow(hEdit, 0, 0, crt.right, 20, TRUE);
			MoveWindow(hTV, 0, 20, treeWidth - GAP, crt.bottom - 40 - resultHeight, TRUE);
			MoveWindow(hLV, treeWidth, 20, crt.right - treeWidth, crt.bottom - resultHeight - 20, TRUE);
			MoveWindow(hStatic, 0, crt.bottom - 20 - resultHeight, 130, 20, TRUE);
			MoveWindow(hProgress, 130, crt.bottom - 20 - resultHeight, 130, 20, TRUE);
			if (resultHeight != 0)
				MoveWindow(hresultLV, 0, crt.bottom - resultHeight + GAP, crt.right, resultHeight - GAP, TRUE);
		}
		return 0;
	case WM_SETCURSOR: //커서 모양 변경
		if (LOWORD(lParam) == HTCLIENT)
		{
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			int t = getSplitter(pt);
			if (t == SP_VERT)
			{
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				return TRUE;
			}
			else if (t == SP_HORZ)
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENS));
				return TRUE;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		nSplit = getSplitter(pt);
		if (nSplit != SP_NONE)
			SetCapture(hWnd);

		return 0;
	case WM_CONTEXTMENU:
		openPopupMenu(LOWORD(lParam), HIWORD(lParam));

		break;
	case WM_MOUSEMOVE:
		GetClientRect(hWnd, &crt);
		if (nSplit == SP_VERT)
		{
			treeWidth = min(max((int)LOWORD(lParam), MIN_TREE_WIDTH), MAX_TREE_WIDTH);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
		}
		else if (nSplit == SP_HORZ)
		{
			resultHeight = min(max(crt.bottom - (int)HIWORD(lParam), MIN_RTREE_HEIGHT), MAX_RTREE_HEIGHT);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
		}
		return 0;
	case WM_LBUTTONUP:
		nSplit = SP_NONE;
		ReleaseCapture();
		return 0;
	case WM_COMMAND:
		if (HIWORD(wParam) == 1)
		{
			AcceleratorProcess(hWnd, LOWORD(wParam));
			break;
		}
		switch (LOWORD(wParam))
		{
		case ID_MENU_FIND:
			if (!IsWindow(hDlgFind))
			{
				hDlgFind = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWndMain, (DLGPROC)FindDlgProc);
				ShowWindow(hDlgFind, SW_SHOW);
			}
			break;
		case ID_MENU_RESULT_TAB:
			hmenu = GetMenu(hWnd);
			if (GetMenuState(hmenu, ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_CHECKED)
			{
				CheckMenuItem(hmenu, ID_MENU_RESULT_TAB, MF_UNCHECKED);
				MoveWindow(hresultLV, 0, 0, 0, 0, TRUE);
				resultHeight = 0;
				SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
			}
			else
			{
				CheckMenuItem(hmenu, ID_MENU_RESULT_TAB, MF_CHECKED);
				resultHeight = 200;
				SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
			}
			break;
		case ID_MENU_EXIT:
			SendMessage(hWndMain, WM_DESTROY, 0, 0);
			break;
		}
	case WM_NOTIFY:
		switch (wParam)
		{
		case ID_TV:
			switch (((LPNMHDR)lParam)->code)
			{
			case TVN_SELCHANGED: //isDataLoad가 1이면 리스트뷰와 경로 edit을 건들지 않음
				if(!isDataLoad) ListView_DeleteAllItems(hLV);
				freeMemory();
				lvData.nByte = 0;
				lvData.nMul = 0;
				
				lvData.byteData = (BYTE_DATA*)malloc(sizeof(BYTE_DATA));
				lvData.mulstrData = (MULSZ_DATA*)malloc(sizeof(MULSZ_DATA));

				tvitem = ((LPNMTREEVIEW)lParam)->itemNew;
				getPathfromItem(tvitem.hItem, temp);
				if (!isDataLoad)
					SetWindowText(hEdit, temp);

				if (tvitem.hItem != TreeView_GetRoot(hTV)) 
					loadValue(getValidPath(temp), BASIC_KEY_HANDLE[tvitem.lParam], isDataLoad);

				if (!isDataLoad)
				{
					ListView_SetColumnWidth(hLV, 2, LVSCW_AUTOSIZE);
					if (ListView_GetColumnWidth(hLV, 2) < 300)
						ListView_SetColumnWidth(hLV, 2, 300);
				}
				return 0;
			case TVN_ENDLABELEDIT:
				{
					HKEY hkey;
					TCHAR pathtemp[MAX_PATH_LENGTH];

					memset(oldName, 0, sizeof(oldName));

					tvitem = ((LPNMTVDISPINFO)lParam)->item;
					tvitem.mask = TVIF_TEXT;
					tvitem.pszText = oldName;
					TreeView_GetItem(hTV, &tvitem);

					GetWindowText(TreeView_GetEditControl(hTV), temp, sizeof(temp));

					getPathfromItem(TreeView_GetParent(hTV, ((LPNMTVDISPINFO)lParam)->item.hItem), path);

					wsprintf(pathtemp, L"%ws\\%ws", path, temp);
					if (wcscmp(oldName, temp) != 0 && _RegOpenKeyEx(tvitem.lParam, pathtemp) != NULL) //이름 중복 검사(기본 할당된 이름 말고 다른 이름을 입력했을 때 레지스트리가 열리면 이미 있는 것)
					{
						MessageBox(hWnd, L"지정한 이름의 키가 이미 있습니다", L"알림", MB_OK);
						wsprintf(temp, oldName);
					}

					if ((hkey = _RegOpenKeyEx(tvitem.lParam, path)) != NULL)
					{
						if (RegRenameKey(hkey, oldName, temp) == ERROR_SUCCESS)
						{
							tvitem.pszText = temp;
							TreeView_SetItem(hTV, &tvitem);
						}
						RegCloseKey(hkey);
					}
					break;
				}
			}
			break;
		case ID_LV:
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_DBLCLK:
				ListView_GetItemText(hLV, ((LPNMITEMACTIVATE)lParam)->iItem, 1, temp, sizeof(temp));
				openModifyDlg(getType(temp));
				break;
			case LVN_BEGINLABELEDIT:
				if (getListViewItem(hLV, LVIF_PARAM, ((LPNMLVDISPINFO)lParam)->item.iItem).lParam != PREV_NEW_VALUE_PARAM) //아이템을 2번 천천히 누르면 labeledit으로 자동으로 들어가는데 이를 막기 위함
					return 1;
				else
					GetWindowText(ListView_GetEditControl(hLV), oldName, sizeof(oldName));
				break;
			case LVN_ENDLABELEDIT:
				{
					HKEY hkey;
					TCHAR type[20], ivalue[15] = L"0x00000000 (0)", data[2];
					int index = ((LPNMLVDISPINFO)lParam)->item.iItem, result, itype;

					GetWindowText(hEdit, path, sizeof(path));
					ListView_GetItemText(hLV, index, 1, type, sizeof(type));
					itype = getType(type);

					if ((hkey = _RegOpenKeyEx(getBasicKey(path), path)) != NULL)
					{
						ListView_DeleteItem(hLV, index);

						GetWindowText(ListView_GetEditControl(hLV), temp, sizeof(temp));

						if (RegQueryValueEx(hkey, temp, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) //이름 중복 검사(값이 가져와지면 이미 있는 것)
						{
							MessageBox(hWnd, L"지정한 이름의 값이 이미 있습니다", L"알림", MB_OK);
							wsprintf(temp, oldName);
						}

						if (itype >= 2)
							*data = 0;
						else
							*data = L'0';

						result = _RegSetValueEx(hkey, temp, REG_TYPE[itype], (BYTE*)data, itype >= 4 ? 0 : -1, 1, 0);

						if (result)
						{
							addLVitem(hLV, temp, type, itype < 2 ? ivalue : NULL, index, NULL, 0);

							ListView_SetItemState(hLV, -1, LVIF_STATE, LVIS_SELECTED); //전부 선택 해제
							ListView_SetItemState(hLV, index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

							if (itype == 4) //데이터 배열 추가 multi_sz
							{
								lvData.mulstrData[lvData.nMul].strings = 0;
								lvData.mulstrData[lvData.nMul].index = index;
								lvData.mulstrData[lvData.nMul].nString = 0;
								wsprintf(lvData.mulstrData[lvData.nMul].name, temp);
								lvData.mulstrData[lvData.nMul].size = 0;
								lvData.mulstrData = (MULSZ_DATA*)realloc(lvData.mulstrData, sizeof(MULSZ_DATA) * (++lvData.nMul + 1));
							}
							else if (itype == 5) //binary
							{
								lvData.byteData[lvData.nByte].bytes = (BYTE*)malloc(1);
								lvData.byteData[lvData.nByte].index = index;
								wsprintf(lvData.byteData[lvData.nByte].name, temp);
								lvData.byteData[lvData.nByte].size = 0;

								lvData.byteData = (BYTE_DATA*)realloc(lvData.byteData, sizeof(BYTE_DATA) * (++lvData.nByte + 1));
							}
						}

						RegCloseKey(hkey);
					}

					break;
				}
			}
			break;
		case ID_resLV:
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_DBLCLK:
				{
					LPNMITEMACTIVATE iteminfo = (LPNMITEMACTIVATE)lParam;
					int selitem = 0;
					TCHAR n[MAX_KEY_LENGTH];
					if (iteminfo->iItem != -1)
					{
						ListView_GetItemText(hresultLV, iteminfo->iItem, 0, temp, sizeof(temp));
						HTREEITEM item = getItemfromPath(temp);
						TreeView_SelectItem(hTV, item);

						ListView_GetItemText(hresultLV, iteminfo->iItem, 1, temp, sizeof(temp));

						if (getListViewItem(hresultLV, LVIF_PARAM, iteminfo->iItem).lParam < 0)
							selitem = 0;

						for (int i = 1; i < ListView_GetItemCount(hLV); i++)
						{
							ListView_GetItemText(hLV, i, 0, n, sizeof(n));
							if (wcscmp(n, temp) == 0)
							{
								selitem = i;
								break;
							}
						}

						ListView_DeSelectAll(hLV);
						ListView_SetItemState(hLV, selitem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						SetFocus(hLV);
					}
				}
				break;
			}
		}

		break;
	case WM_PAINT:
		break;
	case WM_DESTROY:
		freeMemory();
		free(msg);
		fclose(fp);

		PostQuitMessage(0);
		return 0;
	}

	//윈도우 프로시저에서 처리되지 않은 나머지 메시지를 처리해준다
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

BOOL CALLBACK FindDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt2;
	TCHAR temp[100];
	DATA* data;
	static int startChange, index;
	static DATA changeData; //하나씩 바꾸는 경우에는 입력했던 데이터를 저장해 놔야 됨
	LVITEM li;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		oldDlgEditProc[0] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_EDIT_FIND), GWLP_WNDPROC, (LONG_PTR)DlgEditSubProc);
		oldDlgEditProc[1] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_EDIT_CHANGE), GWLP_WNDPROC, (LONG_PTR)DlgEditSubProc);
		GetClientRect(hWndMain, &rt);
		GetClientRect(hDlg, &rt2);
		MoveWindow(hDlg, (rt.left + rt.right) / 2, (rt.top + rt.bottom) / 2, rt2.right - rt2.left + 10, rt2.bottom - rt2.top + 30, TRUE);
		SendMessage(GetDlgItem(hDlg, IDC_D1_STR), BM_SETCHECK, BST_CHECKED, 1);
		SendMessage(GetDlgItem(hDlg, IDC_D1_DEC), BM_SETCHECK, BST_CHECKED, 1);

		startChange = 0;
		funcState = DEFAULT;
		index = -1; //초기값 음수로 설정
		return 0;
	case WM_COMMAND:
		if (wParam == 2) //esc 누를 때 전달되는 메세지(왜인지 모름)
		{
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDC_CHECK_CHANGE:
			if (SendMessage(GetDlgItem(hDlg, IDC_CHECK_CHANGE), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				if (ListView_GetItemCount(hresultLV) != 0)
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHANGE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CHANGE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_ALL), TRUE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_CHANGE), 0);
				SendMessage(GetDlgItem(hDlg, IDC_CHECK_ALL), BM_SETCHECK, BST_UNCHECKED, 0);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHANGE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CHANGE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_ALL), FALSE);
			}
			break;
		case IDC_BUTTON_FIND:
			if (funcState == FINDING) { //찾는 중에 중지 버튼
				funcState = SUSPEND;
				break;
			}

			if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_UNCHECKED)
				SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(ID_MENU_RESULT_TAB, 0), 0);

			data = (DATA*)malloc(sizeof(DATA));
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FIND), data->targetValue, sizeof(data->targetValue));
			if (wcscmp(data->targetValue, L"") != 0)
			{
				wsprintf(changeData.targetValue, data->targetValue);
				
				data->type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
				data->t_type = FIND;

				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
					data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				changeData.type = data->type;
				changeData.base = data->base;

				index = -1;

				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
				{
					if(is_number(data->targetValue, data->base) == 0)
					{
						MessageBox(hWndMain, L"찾는 값이 정수가 아닙니다.", L"알림", MB_OK);
						break;
					}
					else if (checkStringOverflow(data->targetValue, data->base, IsDlgButtonChecked(hDlg, IDC_D1_QWORD)))
					{
						MessageBox(hWndMain, L"찾는 값이 범위를 벗어납니다.", L"알림", MB_OK);
						break;
					}
				}

				ListView_DeleteAllItems(hresultLV);
				SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_FIND), L"중지");
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHANGE), FALSE);

				CreateThread(NULL, 0, ThreadFunc, data, NULL, NULL);
			}
			break;
		case IDC_BUTTON_CHANGE:
			if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_UNCHECKED)
				SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(ID_MENU_RESULT_TAB, 0), 0);

			if (startChange == 0) //아직 바꾸기 한 번도 누르지 않은 경우
			{
				data = (DATA*)malloc(sizeof(DATA));
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FIND), data->targetValue, sizeof(data->targetValue));
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_CHANGE), data->newValue, sizeof(data->newValue));
				data->type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
				
				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
					data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				if (wcscmp(data->targetValue, L"") != 0)
				{
					if (wcscmp(data->targetValue, changeData.targetValue) == 0 && data->type == changeData.type)
					{
						if (SendMessage(GetDlgItem(hDlg, IDC_CHECK_ALL), BM_GETCHECK, 0, 0) == BST_CHECKED)
						{
							data->t_type = CHANGE;

							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
							{
								if (is_number(data->newValue, data->base) == 0)
								{
									MessageBox(hWndMain, L"바꾸는 값이 정수가 아닙니다.", L"알림", MB_OK);
									break;
								}
								else if (checkStringOverflow(data->newValue, data->base, IsDlgButtonChecked(hDlg, IDC_D1_QWORD))) //범위 검사
								{
									MessageBox(hWndMain, L"바꾸는 값이 범위를 벗어납니다.", L"알림", MB_OK);
									break;
								}
							}

							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FIND), FALSE);
							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHANGE), FALSE);

							CreateThread(NULL, 0, ThreadFunc, data, NULL, NULL);
						}
						else
						{
							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
							{
								if (is_number(data->newValue, data->base) == 0)
								{
									MessageBox(hWndMain, L"바꾸는 값이 정수가 아닙니다.", L"알림", MB_OK);
									break;
								}
								else if (checkStringOverflow(data->newValue, data->base, IsDlgButtonChecked(hDlg, IDC_D1_QWORD)))
								{
									MessageBox(hWndMain, L"바꾸는 값이 범위를 벗어납니다.", L"알림", MB_OK);
									break;
								}
							}

							wsprintf(changeData.targetValue, data->targetValue);
							wsprintf(changeData.newValue, data->newValue);
							changeData.type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
								data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

							startChange = 1; 
							index = 0; //전부 바꾸는 경우는 index가 초기값인 음수여서 아래 바꾸기 과정을 수행하기 위한 index >=0 을 만족하지 않음
							nchanged = 0;
						}
					}
					else
					{
						if (data->type != changeData.type)
							wsprintf(temp, L"찾은 유형과 현재 선택된 유형이 다릅니다.");
						else
						{
							wsprintf(temp, L"%ws에 대한 검색 결과가 없습니다.", data->targetValue);
							EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE), FALSE);
						}

						MessageBox(hWndMain, temp, L"알림", MB_OK);
					}
				}
			}
			
			if(index>=0)
			{
				changeData.base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				//다 바꿨는지 검사하기 위한 정보
				LVGROUP lg = {}, lg2 = {}; 
				lg.cbSize = sizeof(LVGROUP);
				lg.mask = LVGF_ITEMS;
				lg.iGroupId = 1;
				ListView_GetGroupInfo(hresultLV, 1, &lg);

				lg2.cbSize = sizeof(LVGROUP);
				lg2.mask = LVGF_ITEMS;
				lg2.iGroupId = 2;
				ListView_GetGroupInfo(hresultLV, 2, &lg2);

				index = ListView_GetSelectionMark(hresultLV);
				if (getListViewItem(hresultLV, LVIF_GROUPID, index).iGroupId == 1)
				{
					if(changeValue(index, &changeData))
					{
						li.mask = LVIF_GROUPID;
						li.iItem = index;
						li.iGroupId = 2;
						ListView_SetItem(hresultLV, &li);

						nchanged++;

						ListView_DeSelectAll(hresultLV);
					}
				}

				if (nchanged != (lg.cItems + lg2.cItems))
				{
					while (1)
					{
						if (++index == ListView_GetItemCount(hresultLV)) index = 0; //끝까지 갔으면 처음으로
						if (getListViewItem(hresultLV, LVIF_GROUPID, index).iGroupId != 1) //바꿀 수 있는 다음 index 선택
							continue;
						else
							break;
					}

					ListView_DeSelectAll(hresultLV);
					ListView_SetSelectionMark(hresultLV, index);
					ListView_SetItemState(hresultLV, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
				else
				{
					startChange = 0;
					index = -1;
					EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE), FALSE);
					MessageBox(hWndMain, L"더이상 바꿀 항목이 없습니다.", L"알림", MB_OK);
				}
			}
			break;
			case IDC_D1_STR:
				if (IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_DEC)))
				{
					EnableWindow(GetDlgItem(hDlg, IDC_D1_DEC), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_D1_HEX), FALSE);
				}
				break;
			case IDC_D1_DWORD:
			case IDC_D1_QWORD:
				if (!IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_DEC)))
				{
					EnableWindow(GetDlgItem(hDlg, IDC_D1_DEC), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_D1_HEX), TRUE);
				}
				break;
		}
		break;
	case WM_CLOSE:
		hDlgFind = NULL;
		startChange = 0;
		EndDialog(hDlg, 0);
		return 1;
	}

	return 0;
}