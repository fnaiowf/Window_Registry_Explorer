#include"header.h"

HWND hWndMain, hTV, hLV, hEdit, hStatic, hresultLV, hProgress, hDlgFind, hDlgModify;
WNDPROC oldDlgEditProc[2];
TCHAR temp[MAX_PATH_LENGTH];

int treeWidth, resultHeight, nchanged, isDataLoad;
SPLIT nSplit = SP_NONE;

DWORD WINAPI ThreadFunc(LPVOID temp)
{
	if (temp != NULL && ((DATA*)temp)->t_type == REFRESH)
	{
		ListView_DeleteAllItems(hLV);
		ListView_DeleteAllItems(hresultLV);
		SetWindowText(hEdit, NULL);
		TreeView_DeleteAllItems(hTV);
	}

	setMarquee(1);
	if (temp != NULL && ((DATA*)temp)->t_type == CHANGE)
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
				item = getItemfromPath(((DATA*)temp)->path);
				TreeView_SelectItem(hTV, item);
				TreeView_EnsureVisible(hTV, item);
				SetWindowText(hEdit, ((DATA*)temp)->path);
				break;
			case FIND:
				EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_FIND), TRUE);
				if (ListView_GetItemCount(hresultLV) != 0 && IsDlgButtonChecked(hDlgFind, IDC_CHECK_CHANGE))
					EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE), TRUE);

				SetFocus(hresultLV);
				ListView_SetSelectionMark(hresultLV, 0);
				ListView_SetItemState(hresultLV, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				free((DATA*)temp);
				break;
			}
		}
	}

	setMarquee(0);

	return 0;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return lParam1 > lParam2 ? -1 : 1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	RECT crt;
	POINT pt;
	DWORD t;
	HDC hdc;
	HMENU hmenu;
	static DATA data;
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
		
		CreateThread(NULL, 0, ThreadFunc, NULL, NULL, NULL);
		return 0;

	case WM_HOTKEY:
		switch(wParam)
		{
		case 0:
			{
				DATA* d = (DATA*)malloc(sizeof(DATA));
				d->t_type = REFRESH;
				GetWindowText(hEdit, d->path, MAX_PATH_LENGTH);

				CreateThread(NULL, 0, ThreadFunc, d, NULL, NULL);
				break;
			}
		case 1:
			if (!IsWindow(hDlgFind))
			{
				hDlgFind = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWndMain, (DLGPROC)FindDlgProc);
				ShowWindow(hDlgFind, SW_SHOW);
			}
			break;
		}
		break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
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
	case WM_SETCURSOR:
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
			treeWidth = min(max((int)LOWORD(lParam), MIN_WIDTH), MAX_WIDTH);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
		}
		else if (nSplit == SP_HORZ)
		{
			resultHeight = min(max(crt.bottom - (int)HIWORD(lParam), MIN_HEIGHT), MAX_HEIGHT);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
		}
		return 0;
	case WM_LBUTTONUP:
		nSplit = SP_NONE;
		ReleaseCapture();
		return 0;
	case WM_COMMAND:
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
			case TVN_SELCHANGED:
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
			case TVN_BEGINLABELEDIT:
				tvitem = ((LPNMTVDISPINFO)lParam)->item;
				break;
			case TVN_ENDLABELEDIT:
				{
					TCHAR oldName[MAX_KEY_LENGTH];
					HKEY hkey;

					tvitem.mask = TVIF_TEXT;
					tvitem.pszText = oldName;
					TreeView_GetItem(hTV, &tvitem);
					
					GetWindowText(TreeView_GetEditControl(hTV), temp, sizeof(temp));

					getPathfromItem(TreeView_GetParent(hTV, ((LPNMTVDISPINFO)lParam)->item.hItem), path);

					if ((hkey = _RegOpenKeyEx(tvitem.lParam, path)) != NULL)
					{
						if (RegRenameKey(hkey, oldName, temp) == ERROR_SUCCESS)
						{
							tvitem.hItem = ((LPNMTVDISPINFO)lParam)->item.hItem;
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
				if (!IsWindow(hDlgModify))
				{
					ListView_GetItemText(hLV, ((LPNMITEMACTIVATE)lParam)->iItem, 1, temp, sizeof(temp));
					t = getType(temp);

					if (t == 5) hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWndMain, (DLGPROC)ModifyBinaryDlgProc);
					else if (t == 4) hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWndMain, (DLGPROC)ModifyMultiSzDlgProc);
					else hDlgModify = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWndMain, (DLGPROC)ModifySzNumDlgProc);

					ShowWindow(hDlgModify, SW_SHOW);
				}
				break;
			case LVN_BEGINLABELEDIT:
				if (getListViewItem(hLV, LVIF_PARAM, ((LPNMLVDISPINFO)lParam)->item.iItem).lParam != PREV_NEW_VALUE_PARAM)
					return 1;

				ListView_GetItemText(hLV, ((LPNMLVDISPINFO)lParam)->item.iItem, 0, temp, sizeof(temp));
				break;
			case LVN_ENDLABELEDIT:
				{
					HKEY hkey;
					TCHAR type[20], ivalue[15] = L"0x00000000 (0)", *data;
					int index = ((LPNMLVDISPINFO)lParam)->item.iItem, result;

					GetWindowText(hEdit, path, sizeof(path));
					ListView_GetItemText(hLV, index, 1, type, sizeof(type));
					type[19] = getType(type);

					if ((hkey = _RegOpenKeyEx(getBasicKey(path), path)) != NULL)
					{
						ListView_DeleteItem(hLV, index);

						GetWindowText(ListView_GetEditControl(hLV), temp, sizeof(temp));

						if (type[19] >= 2)
							data = NULL;
						else
						{
							data = (TCHAR*)malloc(sizeof(TCHAR));
							*data = L'0';
						}
						result = _RegSetValueEx(hkey, temp, REG_TYPE[type[19]], (BYTE*)data, -1, 1);

						if (data != NULL) free(data);

						if (result)
						{
							addLVitem(hLV, temp, type, type[19] < 2 ? ivalue : NULL, index, NULL, 0);

							ListView_SetItemState(hLV, -1, LVIF_STATE, LVIS_SELECTED);
							ListView_SetItemState(hLV, index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
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
					LVFINDINFO lvi;
					if (iteminfo->iItem != -1)
					{
						ListView_GetItemText(hresultLV, iteminfo->iItem, 0, temp, sizeof(temp));
						HTREEITEM item = getItemfromPath(temp);
						TreeView_SelectItem(hTV, item);

						ListView_GetItemText(hresultLV, iteminfo->iItem, 1, temp, sizeof(temp));
						lvi.flags = LVFI_STRING;
						lvi.psz = temp;

						ListView_DeSelectAll(hLV);
						ListView_SetItemState(hLV, ListView_FindItem(hLV, 0, &lvi), LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
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
	static DATA changeData;
	LVITEM li;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		oldDlgEditProc[0] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_EDIT_FIND), GWLP_WNDPROC, (LONG_PTR)DlgEditSubProc);
		GetClientRect(hWndMain, &rt);
		GetClientRect(hDlg, &rt2);
		MoveWindow(hDlg, (rt.left + rt.right) / 2, (rt.top + rt.bottom) / 2, rt2.right - rt2.left + 10, rt2.bottom - rt2.top + 30, TRUE);
		SendMessage(GetDlgItem(hDlg, IDC_D1_STR), BM_SETCHECK, BST_CHECKED, 1);
		SendMessage(GetDlgItem(hDlg, IDC_D1_DEC), BM_SETCHECK, BST_CHECKED, 1);

		startChange = 0;
		index = -1;
		return 1;
	case WM_COMMAND:
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
			if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_UNCHECKED)
				SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(ID_MENU_RESULT_TAB, 0), 0);

			data = (DATA*)malloc(sizeof(DATA));
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FIND), data->targetValue, sizeof(data->targetValue));
			if (wcscmp(data->targetValue, L"") != 0)
			{
				wsprintf(changeData.targetValue, data->targetValue);
				
				wsprintf(data->newValue, L"");
				data->type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
				data->t_type = FIND;

				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
					data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				changeData.type = data->type;
				changeData.base = data->base;

				index = -1;

				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR) && is_number(data->targetValue, data->base) == 0)
					MessageBox(hWndMain, L"찾는 값이 정수가 아닙니다.", L"알림", MB_OK);
				else
				{
					ListView_DeleteAllItems(hresultLV);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FIND), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHANGE), FALSE);

					CreateThread(NULL, 0, ThreadFunc, data, NULL, NULL);
				}
			}
			break;
		case IDC_BUTTON_CHANGE:
			if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_UNCHECKED)
				SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(ID_MENU_RESULT_TAB, 0), 0);

			if (startChange == 0)
			{
				data = (DATA*)malloc(sizeof(DATA));
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FIND), data->targetValue, sizeof(data->targetValue));
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_CHANGE), data->newValue, sizeof(data->newValue));
				data->type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
				
				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
					data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				if (wcscmp(data->targetValue, L"") != 0 && wcscmp(data->newValue, L"") != 0)
				{
					if (wcscmp(data->targetValue, changeData.targetValue) == 0 && data->type == changeData.type)
					{
						if (SendMessage(GetDlgItem(hDlg, IDC_CHECK_ALL), BM_GETCHECK, 0, 0) == BST_CHECKED)
						{
							data->t_type = CHANGE;

							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR) && is_number(data->newValue, data->base) == 0)
								MessageBox(hWndMain, L"바꾸는 값이 정수가 아닙니다.", L"알림", MB_OK);
							else
							{
								EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FIND), FALSE);
								EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHANGE), FALSE);

								CreateThread(NULL, 0, ThreadFunc, data, NULL, NULL);
							}
						}
						else
						{
							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR) && is_number(data->newValue, data->base) == 0)
								MessageBox(hWndMain, L"바꾸는 값이 정수가 아닙니다.", L"알림", MB_OK);
							else
							{
								wsprintf(changeData.targetValue, data->targetValue);
								wsprintf(changeData.newValue, data->newValue);
								changeData.type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
								if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
									data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

								startChange = 1;
								index = 0;
								nchanged = 0;
							}
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
				if ( !(getListViewItem(hresultLV, LVIF_PARAM, index).lParam & CHECKBIT) && getListViewItem(hresultLV, LVIF_GROUPID, index).iGroupId == 1)
				{
					changeValue(index, &changeData);
					li.mask = LVIF_PARAM | LVIF_GROUPID;
					li.iItem = index;
					li.iGroupId = 2;
					li.lParam = getListViewItem(hresultLV, LVIF_PARAM, index).lParam | CHECKBIT; //8388608 : 1000 0000 0000 0000 0000 0000
					ListView_SetItem(hresultLV, &li);

					nchanged++;

					ListView_DeSelectAll(hresultLV);
				}

				if (nchanged != (lg.cItems + lg2.cItems))
				{
					while (1)
					{
						if (++index == ListView_GetItemCount(hresultLV)) index = 0;
						if (getListViewItem(hresultLV, LVIF_PARAM, index).lParam & CHECKBIT || getListViewItem(hresultLV, LVIF_GROUPID, index).iGroupId != 1)
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
					ListView_DeleteAllItems(hresultLV);
					EnableWindow(GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE), FALSE);
					MessageBox(hDlgFind, L"더이상 바꿀 항목이 없습니다.", L"알림", MB_OK);
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