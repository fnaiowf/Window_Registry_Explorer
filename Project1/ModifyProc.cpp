#include"header.h"
#include"BinaryEditor.h"

BOOL CALLBACK ModifySzNumDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HKEY hkey;
	static TCHAR text[MAX_VALUE_LENGTH], path[2][MAX_PATH_LENGTH], name[2][MAX_KEY_LENGTH], type[20];
	TCHAR* pos, tempvalue[MAX_VALUE_LENGTH];
	int t = 0, itype;
	static HWND nh = 0;
	static int tindex = -1, isdefault, prevBase;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		hDlgModify = hDlg;
		isdefault = 0;
		if (GetFocus() == hresultLV)
		{
			nh = hresultLV;
			tindex = 1;
		}
		else
		{
			nh = hLV;
			tindex = 0;
		}
		//�˻� ��� ���̸� 1�� �� ���ؼ� �ε��� ����
		ListView_GetItemText(nh, ListView_GetSelectionMark(nh), tindex + 2, text, sizeof(text));
		ListView_GetItemText(nh, ListView_GetSelectionMark(nh), tindex + 1, type, sizeof(type));
		ListView_GetItemText(nh, ListView_GetSelectionMark(nh), tindex + 0, name[0], sizeof(name[0]));
		itype = getType(type);

		if (itype < 2) //����
		{
			EnableWindow(GetDlgItem(hDlg, IDC_D2_DEC), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_D2_HEX), TRUE);
			SendMessage(GetDlgItem(hDlg, IDC_D2_DEC), BM_SETCHECK, BST_CHECKED, 1);
			prevBase = 1;
		}

		if (tindex) //�˻� ��������� 0�� �ε����� ���, ������ ǥ�� ����Ʈ�信���� ��� edit���� ��� ������
			ListView_GetItemText(hresultLV, ListView_GetSelectionMark(hresultLV), 0, path[0], sizeof(path[0]))
		else
			GetWindowText(hEdit, path[0], sizeof(path[0]));

		if (wcscmp(name[0], L"(�⺻��)") == 0) //�⺻���� �ƴϾ �� �̸��� (�⺻��)�� �� ����
		{
			HTREEITEM item = getItemfromPath(path[0]), item2 = TreeView_GetSelection(hTV);
			TreeView_SelectItem(hTV, item);
			if (getListViewItem(hLV, LVIF_PARAM, 0).lParam == DEFAULT_VALUE_PARAM)
				isdefault = 1;
			TreeView_SelectItem(hTV, item2);
		}

		if (itype < 2) //10���� �κи� ������
		{
			long long num;
			swscanf_s(text, L"0x%*I64x (%I64d", &num);
			wsprintf(text, L"%I64d", num);
		}

		if (isdefault && wcscmp(L"(�� ���� �ȵ�)", text) == 0)
			SetDlgItemText(hDlg, IDC_D2_VDATA, L"");
		else
		{
			SetDlgItemText(hDlg, IDC_D2_VDATA, text);
			SendMessage(GetDlgItem(hDlg, IDC_D2_VDATA), EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		}

		SetDlgItemText(hDlg, IDC_D2_VNAME, name[0]);

		SetFocus(GetDlgItem(hDlg, IDC_D2_VDATA));
		oldDlgEditProc[2] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_D2_VDATA), GWLP_WNDPROC, (LONG_PTR)DlgEditSubProc);
		return 0;
	case WM_COMMAND:
		if (wParam == 2) //esc ���� �� ���޵Ǵ� �޼���(������ ��)
		{
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDC_D2_MODIFY_OK:
			ListView_GetItemText(nh, ListView_GetSelectionMark(nh), tindex + 1, type, sizeof(type));
			GetDlgItemText(hDlg, IDC_D2_VDATA, text, sizeof(text));
			GetDlgItemText(hDlg, IDC_D2_VNAME, name[0], sizeof(name[0]));

			if (getType(type) < 2)
			{
				if (!is_number(text, IsDlgButtonChecked(hDlg, IDC_D2_DEC)))
				{
					MessageBox(hWndMain, L"�Է��� ���� ������ �ƴմϴ�.", L"�˸�", MB_OK);
					break;
				}
			}

			if (isdefault)
				wsprintf(name[0], L"");

			if ((hkey = _RegOpenKeyEx(getBasicKey(path[0]), path[0])) != NULL)
			{
				itype = getType(type);
				if (_RegSetValueEx(hkey, name[0], REG_TYPE[itype], (BYTE*)text, -1, IsDlgButtonChecked(hDlg, IDC_D2_DEC), 1))
				{
					if (itype == 0) //DWORD
					{
						int j = wcstol(text, NULL, IsDlgButtonChecked(hDlg, IDC_D2_DEC) ? 10 : 16);
						wsprintf(tempvalue, L"0x%08x (%d)", j, j);
					}
					else if (itype == 1) //QWORD
					{
						long long j = wcstoll(text, NULL, IsDlgButtonChecked(hDlg, IDC_D2_DEC) ? 10 : 16);
						wsprintf(tempvalue, L"0x%08I64x (%I64d)", j, j);
					}
					else
						wsprintf(tempvalue, text);

					ListView_SetItemText(nh, ListView_GetSelectionMark(nh), tindex + 2, tempvalue);

					if (tindex) //�˻� ��� ���� ��� ������ ����Ʈ�信 �����ϴ� ���� �ִٸ� ���� �ٲ���
					{
						GetWindowText(hEdit, path[1], sizeof(path[0]));

						if (wcscmp(path[0], path[1]) == 0)
						{
							HTREEITEM t = TreeView_GetSelection(hTV);
							TreeView_SelectItem(hTV, TreeView_GetRoot(hTV)); //��Ʈ�� �ٲ�ٰ� �ٽ� �ٲ㼭 ���� �ٽ� �ε�
							TreeView_SelectItem(hTV, t);
						}
					}
					else
					{
						t = 0;
						while (t != ListView_GetItemCount(hresultLV)) //�˻� ��� ���� ��� �����ۿ��� �̸��� ��ΰ� ���� ���� �����ϹǷ� �̷� �������� �Ǻ�
						{
							ListView_GetItemText(hresultLV, t, 0, path[1], sizeof(path[0]));
							ListView_GetItemText(hresultLV, t, 1, name[1], sizeof(name[0]));

							if (wcscmp(name[0], name[1]) == 0 && wcscmp(path[0], path[1]) == 0)
							{
								ListView_SetItemText(hresultLV, t, 3, tempvalue);
								break;
							}

							t++;
						}
					}

					RegCloseKey(hkey);
				}

				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return 0;
			}
			break;
		case IDC_D2_DEC:
		case IDC_D2_HEX:
			GetDlgItemText(hDlg, IDC_D2_VDATA, text, sizeof(text));
			if (IsDlgButtonChecked(hDlg, IDC_D2_DEC) && prevBase == 0)
				wsprintf(text, L"%I64d", wcstoll(text, NULL, 16));
			else if (IsDlgButtonChecked(hDlg, IDC_D2_HEX) && prevBase == 1)
				wsprintf(text, L"%I64x", wcstoll(text, NULL, 10));

			SetDlgItemText(hDlg, IDC_D2_VDATA, text);
			prevBase = IsDlgButtonChecked(hDlg, IDC_D2_DEC);
			break;
		case IDC_D2_MODIFY_NO:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
		break;
	case WM_CLOSE:
		hDlgModify = NULL;
		EndDialog(hDlg, 0);
		return 0;
	}

	return 0;
}

BOOL CALLBACK ModifyMultiSzDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HWND nh;
	static int tindex, tarindex;
	static HTREEITEM oldItem;
	static TCHAR name[2][MAX_KEY_LENGTH], path[2][MAX_PATH_LENGTH];

	HKEY hkey;
	TCHAR text[MAX_VALUE_LENGTH], * pos;
	int i, len, idx, sublen, count;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		hDlgModify = hDlg;

		if (GetFocus() == hresultLV)
		{
			nh = hresultLV;
			tindex = 1;
		}
		else
		{
			nh = hLV;
			tindex = 0;
		}

		ListView_GetItemText(nh, ListView_GetSelectionMark(nh), tindex + 0, name[0], sizeof(name[0]));

		oldItem = TreeView_GetSelection(hTV); //MULTI_SZ�� �����͸� ���� �ҷ����µ� �̸� ���� �˻� ��� �ǿ����� ���� ������ ������ �� �����͸� �ҷ����� ���� �� Ű�� �����ؼ� ó���ϰ� ������ �������� ���ƿ�

		if (tindex)
		{
			ListView_GetItemText(hresultLV, ListView_GetSelectionMark(hresultLV), 0, path[0], sizeof(path[0]));

			isDataLoad = 1; //TVN_SELCHANGED���� ����Ʈ�� �ʱ�ȭ ���� �ʰ� �����͸� �ҷ���

			NMTREEVIEW t;
			TVITEM b;
			b.mask = TVIF_PARAM;
			b.hItem = getItemfromPath(path[0]);
			TreeView_GetItem(hTV, &b);

			t.itemNew = b;
			t.hdr.code = TVN_SELCHANGED;
			SendMessage(hWndMain, WM_NOTIFY, ID_TV, (LPARAM)&t);
		}
		else
			GetWindowText(hEdit, path[0], sizeof(path[0]));

		SetWindowText(GetDlgItem(hDlg, IDC_D4_VNAME), name[0]);

		if (tindex)
		{
			for (int i = 0; i < lvData.nMul; i++) //������ �׸� �ش��ϴ� ������ �迭 �ε��� ã��
			{
				if (wcscmp(lvData.mulstrData[i].name, name[0]) == 0)
				{
					tarindex = i;
					break;
				}
			}

			memset(text, 0, sizeof(text));
			for (int j = 0; j < lvData.mulstrData[tarindex].nString; j++)
			{
				if (j == 0)
					wsprintf(text, L"%ws\r\n", lvData.mulstrData[tarindex].strings[j]);
				else
					wsprintf(text, L"%ws%ws\r\n", text, lvData.mulstrData[tarindex].strings[j]);
			}
			SetWindowText(GetDlgItem(hDlg, IDC_D4_VDATA), text);
			SendMessage(GetDlgItem(hDlg, IDC_D4_VDATA), EM_SETSEL, 0, -1);
		}
		else
		{
			for (int i = 0; i < lvData.nMul; i++)
			{
				if (lvData.mulstrData[i].index == ListView_GetSelectionMark(nh)) //hLV������ �ε����� ���ϴ� ���� ������ �� ����
				{
					tarindex = i;
					memset(text, 0, sizeof(text));
					for (int j = 0; j < lvData.mulstrData[i].nString; j++)
					{
						if (j == 0)
							wsprintf(text, L"%ws\r\n", lvData.mulstrData[i].strings[j]);
						else
							wsprintf(text, L"%ws%ws\r\n", text, lvData.mulstrData[i].strings[j]);
					}

					SetWindowText(GetDlgItem(hDlg, IDC_D4_VDATA), text);
					SendMessage(GetDlgItem(hDlg, IDC_D4_VDATA), EM_SETSEL, 0, -1);
					break;
				}
			}
		}

		SetFocus(GetDlgItem(hDlg, IDC_D4_VDATA));
		return 0;
	case WM_COMMAND:
		if (wParam == 2) //esc ���� �� ���޵Ǵ� �޼���(������ ��)
		{
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDC_D4_MODIFY_OK:
			GetWindowText(GetDlgItem(hDlg, IDC_D4_VDATA), text, MAX_VALUE_LENGTH);
			pos = text;
			len = wcslen(text);
			idx = 0;
			count = 0;
			memset(temp, 0, sizeof(temp));

			for (i = 0; i < len; i++)
			{
				if (text[i] == '\r')
				{
					if (i == 0 || text[i - 1] == 10) //�� ���� \r\n�� �����ų� \r\n\r\n�� ��� \r\n�� �ǳ� ��
					{
						pos += 2;
						i++;
						if (i + 1 == len) //\r\n���� ���� ��� ����
							break;

						continue;
					}

					count++;
					text[i] = 0; //���ڿ� �ڸ��� ���� NULL�� �ٲ�

					sublen = wcslen(pos);

					if (lvData.mulstrData[tarindex].nString < count) //���� ���ڿ� �������� �������� �޸� �߰� �Ҵ�
					{
						if (count == 1)
							lvData.mulstrData[tarindex].strings = (TCHAR**)malloc(sizeof(TCHAR*)); //count�� 1�� ��쿡�� strings�� �Ҵ��� �ȵǾ�����
						else
							lvData.mulstrData[tarindex].strings = (TCHAR**)realloc(lvData.mulstrData[tarindex].strings, sizeof(TCHAR*) * count); //���ڿ��� ��� ���� ũ�� ���Ҵ�

						lvData.mulstrData[tarindex].strings[count - 1] = (TCHAR*)malloc(sizeof(TCHAR) * (sublen + 1)); //���ڿ� ���̸�ŭ �Ҵ�
					}
					else
						lvData.mulstrData[tarindex].strings[count - 1] = (TCHAR*)realloc(lvData.mulstrData[tarindex].strings[count - 1], sizeof(TCHAR) * (sublen + 1));

					wsprintf(lvData.mulstrData[tarindex].strings[count - 1], pos); //������ ������ �Է�

					wcscpy(temp + idx, pos);
					idx += sublen + 1;
					i++; //\n �ǳʶ�
					pos = text + i + 1;

					if (i + 1 == len)
						break;
				}
			}

			if (i == len && i != 0) //���ڿ��� ���� \r\n�� �ƴ� ��쿡�� i+1==len�� �������� �ʾ� ������ ���ڿ��� �� ��, ���� ó�� / idx++ ���ִ� ������ ���ڿ� ���� NULL �߰�
			{
				sublen = wcslen(pos);
				wcscpy(temp + idx++, pos);
				count++;

				if (lvData.mulstrData[tarindex].nString < count)
				{
					if (count == 1)
						lvData.mulstrData[tarindex].strings = (TCHAR**)malloc(sizeof(TCHAR*));
					else
						lvData.mulstrData[tarindex].strings = (TCHAR**)realloc(lvData.mulstrData[tarindex].strings, sizeof(TCHAR*) * count);

					lvData.mulstrData[tarindex].strings[count - 1] = (TCHAR*)malloc(sizeof(TCHAR) * (sublen + 1));
				}
				else
					lvData.mulstrData[tarindex].strings[count - 1] = (TCHAR*)realloc(lvData.mulstrData[tarindex].strings[count - 1], sizeof(TCHAR) * (sublen + 1));

				wsprintf(lvData.mulstrData[tarindex].strings[count - 1], pos);
			}

			idx += wcslen(pos) + 1; //+1 ���ִ� ������ MULTI_SZ�� ���� �� ���� �ϳ� �� �߰��Ǿ� �ֱ� ����

			if (lvData.mulstrData[tarindex].nString > count) //���� ���ڿ� �������� ���� ��� �׸�ŭ free
			{
				for (int k = count; k < lvData.mulstrData[tarindex].nString; k++)
					free(lvData.mulstrData[tarindex].strings[k]);

				lvData.mulstrData[tarindex].strings = (TCHAR**)realloc(lvData.mulstrData[tarindex].strings, sizeof(TCHAR*) * count);
			}

			lvData.mulstrData[tarindex].nString = count;
			lvData.mulstrData[tarindex].size = idx * sizeof(TCHAR);

			if ((hkey = _RegOpenKeyEx(getBasicKey(path[0]), path[0])) != NULL)
			{
				if (_RegSetValueEx(hkey, name[0], REG_MULTI_SZ, (BYTE*)temp, idx * sizeof(TCHAR), -1, 1))
				{
					concatMulSz(temp, idx - 2, text); //�� �ڿ� �� 2�� �־ idx-2

					ListView_SetItemText(nh, ListView_GetSelectionMark(nh), tindex + 2, text);

					if (tindex)
					{
						GetWindowText(hEdit, path[1], sizeof(path[0]));

						if (wcscmp(path[0], path[1]) == 0)
							ListView_SetItemText(hLV, lvData.mulstrData[tarindex].index, 2, text);
					}
					else
					{
						int t = 0;
						while (t != ListView_GetItemCount(hresultLV))
						{
							ListView_GetItemText(hresultLV, t, 0, path[1], sizeof(path[0]));
							ListView_GetItemText(hresultLV, t, 1, name[1], sizeof(name[0]));

							if (wcscmp(name[0], name[1]) == 0 && wcscmp(path[0], path[1]) == 0)
							{
								ListView_SetItemText(hresultLV, t, 3, text);
								break;
							}

							t++;
						}
					}
				}

				RegCloseKey(hkey);
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			else
				MessageBox(hWndMain, L"Error", L"Error", MB_OK);

			return 0;
		case IDC_D4_MODIFY_NO:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
		break;
	case WM_CLOSE:
		TreeView_SelectItem(hTV, oldItem);
		isDataLoad = 0;

		hDlgModify = NULL;
		EndDialog(hDlg, 0);
		return 0;
	}

	return 0;
}

BOOL CALLBACK ModifyBinaryDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HFONT hf;
	static TCHAR name[MAX_KEY_LENGTH], path[MAX_PATH_LENGTH];
	static int i;

	TCHAR byte1[10], byte2[10];
	HKEY hkey;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		hDlgModify = hDlg; //CreateDialog�� WM_INITDIALOG�� ó���ǰ� ������ ������
		inputOnce = 0;
		nbyte = 0;
		binaryOldEditProc[0] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_D3_VDATA), GWLP_WNDPROC, (LONG_PTR)BinaryEditSubProc);
		binaryOldEditProc[1] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_D3_VDATA_ASCII), GWLP_WNDPROC, (LONG_PTR)BinaryEditSubProc);
		binaryOldEditProc[2] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_D3_VDATA_NUMBERING), GWLP_WNDPROC, (LONG_PTR)BinaryNumberingEditSubProc);
		
 		hf = CreateFont(17, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, L"Raize"); //monospaced ��Ʈ(���� ũ�� ����)
	 	SendMessage(GetDlgItem(hDlg, IDC_D3_VDATA), WM_SETFONT, (WPARAM)hf, TRUE);
		SendMessage(GetDlgItem(hDlg, IDC_D3_VDATA_NUMBERING), WM_SETFONT, (WPARAM)hf, TRUE);
	 	SendMessage(GetDlgItem(hDlg, IDC_D3_VDATA_ASCII), WM_SETFONT, (WPARAM)hf, TRUE);
		 
		ListView_GetItemText(hLV, ListView_GetSelectionMark(hLV), 0, name, sizeof(name));
		SetWindowText(GetDlgItem(hDlg, IDC_D3_VNAME), name);

		GetWindowText(hEdit, path, sizeof(path));

		SetWindowText(GetDlgItem(hDlg, IDC_D3_VDATA_NUMBERING), L"00000000");
		
		for (i = 0; i < lvData.nByte; i++)
			if (lvData.byteData[i].index == ListView_GetSelectionMark(hLV))
				break;

		SetFocus(GetDlgItem(hDlg, IDC_D3_VDATA));

		for (int j = 0; j < lvData.byteData[i].size; j++)
		{
			if ((j + 1) % 8 == 0)
			{
				wsprintf(byte1, L" %02X  \r\n", lvData.byteData[i].bytes[j]);
				wsprintf(byte2, L"%c \r\n", isprint(lvData.byteData[i].bytes[j]) ? lvData.byteData[i].bytes[j] : '.');
			}
			else
			{
				wsprintf(byte1, L" %02X  ", lvData.byteData[i].bytes[j]);
				wsprintf(byte2, L"%c ", isprint(lvData.byteData[i].bytes[j]) ? lvData.byteData[i].bytes[j] : '.');
			}
			bytes[j] = lvData.byteData[i].bytes[j];

			SendMessage(GetDlgItem(hDlg, IDC_D3_VDATA), EM_REPLACESEL, TRUE, (LPARAM)byte1);
			SendMessage(GetDlgItem(hDlg, IDC_D3_VDATA_ASCII), EM_REPLACESEL, TRUE, (LPARAM)byte2);
			nbyte++;

			if ((j + 1) % 8 == 0)
				Numbering(1);
		}

		SetSel(GetDlgItem(hDlg, IDC_D3_VDATA), 0);
		SetSel(GetDlgItem(hDlg, IDC_D3_VDATA_ASCII), 0);
		return 0;
	case WM_CTLCOLORSTATIC: //numbering edit background color
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_D3_VDATA_NUMBERING))
			return (INT_PTR)((HBRUSH)GetStockObject(WHITE_BRUSH));
		else
			return DefWindowProc(hDlg, iMessage, wParam, lParam);
		break;
	case WM_COMMAND :
		if (wParam == 2) //esc ���� �� ���޵Ǵ� �޼���(������ ��)
		{
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDC_D3_MODIFY_OK:
			if ((hkey = _RegOpenKeyEx(getBasicKey(path), path)) != NULL)
			{
				if (_RegSetValueEx(hkey, name, REG_BINARY, bytes, nbyte, -1, 1))
				{
					if (nbyte == 0)
					{
						wsprintf(temp, L"(���̰� 0�� ������)");
						ListView_SetItemText(hLV, ListView_GetSelectionMark(hLV), 2, temp);

						memset(lvData.byteData[i].bytes, 0, sizeof(BYTE) * lvData.byteData[i].size);
						lvData.byteData[i].bytes = (BYTE*)realloc(lvData.byteData[i].bytes, sizeof(BYTE));
					}
					else
					{
						TCHAR* b = (TCHAR*)calloc(nbyte * 3, sizeof(TCHAR));
						byteToString(bytes, nbyte, b);

						ListView_SetItemText(hLV, ListView_GetSelectionMark(hLV), 2, b);

						memset(lvData.byteData[i].bytes, 0, sizeof(BYTE) * lvData.byteData[i].size);
						lvData.byteData[i].bytes = (BYTE*)realloc(lvData.byteData[i].bytes, sizeof(BYTE) * nbyte);
						memcpy(lvData.byteData[i].bytes, bytes, sizeof(BYTE) * nbyte);
					}

					lvData.byteData[i].size = nbyte;
				}

				RegCloseKey(hkey);
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			else
				MessageBox(hWndMain, L"Error", L"Error", MB_OK);

			return 1;
		case IDC_D3_MODIFY_NO:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
	break;
	case WM_VSCROLL:
		if (lParam == NULL) //ǥ�� ��ũ�ѹ��� ���� lParam�� NULL
			break;
		
		ScrollProcess(lParam, wParam);
		return 1;
	case WM_CLOSE:
		DeleteObject(hf);

		hDlgModify = NULL;
		EndDialog(hDlg, 0);
		return 1;
	}

	return 0;
}