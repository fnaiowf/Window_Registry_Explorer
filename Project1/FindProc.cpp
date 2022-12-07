#include "header.h"

FIND_TYPE nowFindType = NONE;
int nchanged;

BOOL CALLBACK FindDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt2;
	TCHAR temp[100];
	THREAD_DATA* data;
	static int startChange, index;
	static THREAD_DATA changeData; //�ϳ��� �ٲٴ� ��쿡�� �Է��ߴ� �����͸� ������ ���� ��
	LVITEM li;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		oldDlgEditProc[0] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_D1_EDIT_FIND), GWLP_WNDPROC, (LONG_PTR)DlgEditSubProc);
		oldDlgEditProc[1] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_D1_EDIT_CHANGE), GWLP_WNDPROC, (LONG_PTR)DlgEditSubProc);
		GetClientRect(hWndMain, &rt);
		GetClientRect(hDlg, &rt2);
		MoveWindow(hDlg, (rt.left + rt.right) / 2, (rt.top + rt.bottom) / 2, rt2.right - rt2.left + 10, rt2.bottom - rt2.top + 30, TRUE);
		SendMessage(GetDlgItem(hDlg, IDC_D1_STR), BM_SETCHECK, BST_CHECKED, 1);
		SendMessage(GetDlgItem(hDlg, IDC_D1_DEC), BM_SETCHECK, BST_CHECKED, 1);
		SendMessage(GetDlgItem(hDlg, IDC_D1_DATA), BM_SETCHECK, BST_CHECKED, 1);

		startChange = 0;
		funcState = DEFAULT;
		index = -1; //�ʱⰪ ������ ����
		return 0;
	case WM_COMMAND:
		if (wParam == 2) //esc ���� �� ���޵Ǵ� �޼���(������ ��)
		{
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDC_D1_CHECK_CHANGE:
			if (SendMessage(GetDlgItem(hDlg, IDC_D1_CHECK_CHANGE), BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				if (ListView_GetItemCount(hresultLV) != 0)
					EnableWindow(GetDlgItem(hDlg, IDC_D1_CHANGE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_D1_EDIT_CHANGE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_CHECK_ALL), TRUE);
				SetWindowText(GetDlgItem(hDlg, IDC_D1_EDIT_CHANGE), 0);
				SendMessage(GetDlgItem(hDlg, IDC_D1_CHECK_ALL), BM_SETCHECK, BST_UNCHECKED, 0);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_D1_CHANGE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_EDIT_CHANGE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_CHECK_ALL), FALSE);
			}
			break;
		case IDC_D1_FIND:
			if (funcState == FINDING) { //ã�� �߿� ���� ��ư
				funcState = SUSPEND;
				break;
			}

			if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_UNCHECKED)
				SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(ID_MENU_RESULT_TAB, 0), 0);

			data = (THREAD_DATA*)malloc(sizeof(THREAD_DATA));
			GetWindowText(GetDlgItem(hDlg, IDC_D1_EDIT_FIND), data->targetValue, sizeof(data->targetValue));
			if (wcscmp(data->targetValue, L"") != 0)
			{
				wsprintf(changeData.targetValue, data->targetValue);
				data->threadType = FIND;
				data->findType = IsDlgButtonChecked(hDlg, IDC_D1_KEY) ? KEY : (IsDlgButtonChecked(hDlg, IDC_D1_VALUE) ? VALUE : DATA);

				if (data->findType == DATA)
				{
					data->type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);

					if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
						data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

					changeData.type = data->type;
					changeData.base = data->base;

					index = -1;

					if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
					{
						if (is_number(data->targetValue, data->base) == 0)
						{
							MessageBox(hWndMain, L"ã�� ���� ������ �ƴմϴ�.", L"�˸�", MB_OK);
							break;
						}
						else if (checkStringOverflow(data->targetValue, data->base, IsDlgButtonChecked(hDlg, IDC_D1_QWORD)))
						{
							MessageBox(hWndMain, L"ã�� ���� ������ ����ϴ�.", L"�˸�", MB_OK);
							break;
						}
					}
				}

				ListView_DeleteAllItems(hresultLV);
				SetWindowText(GetDlgItem(hDlg, IDC_D1_FIND), L"����");
				EnableWindow(GetDlgItem(hDlg, IDC_D1_CHANGE), FALSE);

				CreateThread(NULL, 0, ThreadFunc, data, NULL, NULL);
				nowFindType = data->findType;
			}
			break;
		case IDC_D1_CHANGE:
			if (GetMenuState(GetMenu(hWndMain), ID_MENU_RESULT_TAB, MF_BYCOMMAND) == MF_UNCHECKED)
				SendMessage(hWndMain, WM_COMMAND, MAKEWPARAM(ID_MENU_RESULT_TAB, 0), 0);

			if (startChange == 0) //���� �ٲٱ� �� ���� ������ ���� ���
			{
				data = (THREAD_DATA*)malloc(sizeof(THREAD_DATA));
				GetWindowText(GetDlgItem(hDlg, IDC_D1_EDIT_FIND), data->targetValue, sizeof(data->targetValue));
				GetWindowText(GetDlgItem(hDlg, IDC_D1_EDIT_CHANGE), data->newValue, sizeof(data->newValue));
				data->type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);

				if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
					data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				if (wcscmp(data->targetValue, L"") != 0)
				{
					if (wcscmp(data->targetValue, changeData.targetValue) == 0 && data->type == changeData.type)
					{
						if (SendMessage(GetDlgItem(hDlg, IDC_D1_CHECK_ALL), BM_GETCHECK, 0, 0) == BST_CHECKED)
						{
							data->threadType = CHANGE;

							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
							{
								if (is_number(data->newValue, data->base) == 0)
								{
									MessageBox(hWndMain, L"�ٲٴ� ���� ������ �ƴմϴ�.", L"�˸�", MB_OK);
									break;
								}
								else if (checkStringOverflow(data->newValue, data->base, IsDlgButtonChecked(hDlg, IDC_D1_QWORD))) //���� �˻�
								{
									MessageBox(hWndMain, L"�ٲٴ� ���� ������ ����ϴ�.", L"�˸�", MB_OK);
									break;
								}
							}

							EnableWindow(GetDlgItem(hDlg, IDC_D1_FIND), FALSE);
							EnableWindow(GetDlgItem(hDlg, IDC_D1_CHANGE), FALSE);

							CreateThread(NULL, 0, ThreadFunc, data, NULL, NULL);
						}
						else
						{
							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
							{
								if (is_number(data->newValue, data->base) == 0)
								{
									MessageBox(hWndMain, L"�ٲٴ� ���� ������ �ƴմϴ�.", L"�˸�", MB_OK);
									break;
								}
								else if (checkStringOverflow(data->newValue, data->base, IsDlgButtonChecked(hDlg, IDC_D1_QWORD)))
								{
									MessageBox(hWndMain, L"�ٲٴ� ���� ������ ����ϴ�.", L"�˸�", MB_OK);
									break;
								}
							}

							wsprintf(changeData.targetValue, data->targetValue);
							wsprintf(changeData.newValue, data->newValue);
							changeData.type = IsDlgButtonChecked(hDlg, IDC_D1_STR) ? REG_SZ : (IsDlgButtonChecked(hDlg, IDC_D1_DWORD) ? REG_DWORD : REG_QWORD);
							if (!IsDlgButtonChecked(hDlg, IDC_D1_STR))
								data->base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

							startChange = 1;
							index = 0; //���� �ٲٴ� ���� index�� �ʱⰪ�� �������� �Ʒ� �ٲٱ� ������ �����ϱ� ���� index >=0 �� �������� ����
							nchanged = 0;
						}
					}
					else
					{
						if (data->type != changeData.type)
							wsprintf(temp, L"ã�� ������ ���� ���õ� ������ �ٸ��ϴ�.");
						else
						{
							wsprintf(temp, L"%ws�� ���� �˻� ����� �����ϴ�.", data->targetValue);
							EnableWindow(GetDlgItem(hDlgFind, IDC_D1_CHANGE), FALSE);
						}

						MessageBox(hWndMain, temp, L"�˸�", MB_OK);
					}
				}
			}

			if (index >= 0)
			{
				changeData.base = IsDlgButtonChecked(hDlg, IDC_D1_DEC);

				//�� �ٲ���� �˻��ϱ� ���� ����
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
					if (changeValue(index, &changeData))
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
						if (++index == ListView_GetItemCount(hresultLV)) index = 0; //������ ������ ó������
						if (getListViewItem(hresultLV, LVIF_GROUPID, index).iGroupId != 1) //�ٲ� �� �ִ� ���� index ����
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
					EnableWindow(GetDlgItem(hDlgFind, IDC_D1_CHANGE), FALSE);
					MessageBox(hWndMain, L"���̻� �ٲ� �׸��� �����ϴ�.", L"�˸�", MB_OK);
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
		case IDC_D1_KEY:
			if (IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_STR)))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_D1_STR), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_DWORD), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_QWORD), FALSE);
			}
			if (IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_DEC)))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_D1_DEC), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_HEX), FALSE);
			}
			break;
		case IDC_D1_VALUE:
			if (IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_STR)))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_D1_STR), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_DWORD), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_QWORD), FALSE);
			}
			if (IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_DEC)))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_D1_DEC), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_HEX), FALSE);
			}
			break;
		case IDC_D1_DATA:
			if (!IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_STR)))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_D1_STR), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_DWORD), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_D1_QWORD), TRUE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_D1_STR), BM_GETCHECK, 0, 0) == BST_UNCHECKED && !IsWindowEnabled(GetDlgItem(hDlg, IDC_D1_DEC)))
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