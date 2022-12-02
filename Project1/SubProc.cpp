#include"header.h"
#include"BinaryEditor.h"

LRESULT CALLBACK MainEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HTREEITEM item;
	static TCHAR t_path[MAX_PATH_LENGTH];
	HDC hdc;

	switch (iMessage)
	{
	case WM_CHAR: //remove beeping
		if (wParam == VK_RETURN)
			return 0;
		break;
	case WM_SETFOCUS:
		GetWindowText(hEdit, t_path, sizeof(t_path)); //�߸��� ��θ� �Է����� ��� ������ �ԷµǾ� �ִ� ��η� �ٲ�
		break;
	case WM_KEYUP:
		if (wParam == VK_RETURN)
		{
			GetWindowText(hEdit, temp, sizeof(temp));
			item = getItemfromPath(temp);
			if (item != 0)
				TreeView_SelectItem(hTV, item);
			else //�ùٸ� ��ΰ� �ƴ� ���
			{
				SetWindowText(hEdit, t_path);
				hdc = GetDC(hWndMain);
				MessageBeep(0xFFFFFFFF);
				ReleaseDC(hWnd, hdc);
			}
			SetFocus(hTV);

			break;
		}
		break;
	}

	return CallWindowProc(oldEditProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK DlgEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CHAR: //remove beeping
		if (wParam == VK_RETURN)
			return 0;
		break;
	case WM_KEYUP:
		if (wParam == VK_RETURN)
		{
			if (hWnd == GetDlgItem(hDlgFind, IDC_EDIT_FIND))
				SendMessage(hDlgFind, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_FIND, BN_CLICKED), (LPARAM)GetDlgItem(hDlgFind, IDC_BUTTON_FIND));
			else if (hWnd == GetDlgItem(hDlgFind, IDC_EDIT_CHANGE)) 
				SendMessage(hDlgFind, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_CHANGE, BN_CLICKED), (LPARAM)GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE));
			else if (hWnd == GetDlgItem(hDlgModify, IDC_D2_VDATA))
				SendMessage(hDlgModify, WM_COMMAND, MAKEWPARAM(IDC_D2_MODIFY_OK, BN_CLICKED), (LPARAM)GetDlgItem(hDlgModify, IDC_D2_MODIFY_OK));
		}
		break;
	}
	if (hWnd == GetDlgItem(hDlgFind, IDC_EDIT_FIND))
		return CallWindowProc(oldDlgEditProc[0], hWnd, iMessage, wParam, lParam);
	else if(hWnd == GetDlgItem(hDlgFind, IDC_EDIT_CHANGE))
		return CallWindowProc(oldDlgEditProc[1], hWnd, iMessage, wParam, lParam);
	else if (hWnd == GetDlgItem(hDlgModify, IDC_D2_VDATA))
		return CallWindowProc(oldDlgEditProc[2], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK MultiSzEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_KEYDOWN && LOWORD(wParam) == VK_ESCAPE) //multiline edit���� esc ������ WM_CLOSE �޼��� ���޵Ǵ� ���� ����
		return 0;

	return CallWindowProc(oldDlgEditProc[3], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK BinaryEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int pos, rpos, nline, pos2, prevpos[2], ascii, chwidth[2] = { 5, 2 }; //chwidth: ���� ��, edit �������� �ٸ�
	static TCHAR msg[10], msg2[10];
	static int oldpos;
	HWND hWnd2 = hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA) ? GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII) : GetDlgItem(hDlgModify, IDC_D3_VDATA);

	switch (iMessage)
	{
	case WM_CHAR:
		if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA)) //��� edit
		{
			if ((wParam >= '0' && wParam <= '9') || (wParam >= 'a' && wParam <= 'f') || (wParam >= 'A' && wParam <= 'F'))
			{
				if (wParam >= 'a' && wParam <= 'f')
					wParam -= 32;
			}
			else
				return 0;
		}
		else //������ edit
		{
			if (isprint(wParam) && wParam != VK_TAB)
			{
				chwidth[0] = 2;
				chwidth[1] = 5;
			}
			else
				return 0;
		}

		pos = GetPos(hWnd);
		rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0); //���� ù��°�� 0���� �� ��ġ
		nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0); //���� Ŀ�� ��
		pos2 = nline * (chwidth[1] * 8 + 2) + rpos / chwidth[0] * chwidth[1]; //ASCII edit�� Ŀ�� ��ġ

		if (chwidth[0] == 2 || inputOnce == 0) //��� edit�� ��� ����Ʈ �Է� ����(ù��° ����)
		{
			RemoveSelections(hWnd); //���� ������ �ִٸ� ����� ����

			if (rpos == chwidth[0] * 8) //�� ���� ������ ��ġ���� �Է��ϸ� ���� �ٷ� �̵�
			{
				SetSel(hWnd, pos + 2);
				SetSel(hWnd2, pos2 + 2);
				pos += 2;
				pos2 += 2;
				rpos = 0;
			}

			if (rpos % (chwidth[0] * 7) == 0 && rpos != 0) //8��° ������ ��� ���� ���� �߰�
			{
				if (chwidth[0] == 5) //��� edit
				{
					wsprintf(msg, L" %c0  \r\n", wParam);
					wsprintf(msg2, L"%c \r\n", wParam);
				}
				else //������ edit
				{
					wsprintf(msg, L"%c \r\n", wParam);
					wsprintf(msg2, L" %02X  \r\n", wParam);
				}
			}
			else
			{
				if(chwidth[0] == 5)
				{
					wsprintf(msg, L" %c0  ", wParam);
					wsprintf(msg2, L"%c ", wParam);
				}
				else
				{
					wsprintf(msg, L"%c ", wParam);
					wsprintf(msg2, L" %02X  ", wParam);
				}
			}

			if (chwidth[0] == 5)
			{
				ascii = wcstol(msg + 1, NULL, 16);
				msg2[0] = isprint(ascii) ? ascii : '.'; // �Է� �Ұ����� ���ڴ� .���� ��ü
			}

			prevpos[0] = pos; //���� ��ġ�� ������ �� �� ������ ������ ���� ��ġ�� �̵�
			prevpos[1] = pos2;
			autoLineFeed(2, hWnd, pos);
			SetSel(hWnd, prevpos[0]);
			SetSel(hWnd2, prevpos[1]);

			SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)msg);
			SendMessage(hWnd2, EM_REPLACESEL, TRUE, (LPARAM)msg2);

			if (chwidth[0] == 5)
				SetSel(hWnd, pos + 3); //����Ʈ ù ���ڸ� �Է��ϸ� �� ��° ���� �������� Ŀ���� �̵�

			//������ ����
			int nowindex = nline * 8 + rpos / chwidth[0];
			if (nowindex < nbyte) //�߰��� �����͸� �߰��ؾ� �ϴ� ��� �� ĭ�� �б�
			{
				for (int i = nbyte - 1; i >= nowindex; i--)
					bytes[i + 1] = bytes[i];
			}
			bytes[nowindex] = chwidth[0] == 5 ? ascii : wcstol(msg2, NULL, 16);;

			nbyte++;

			if (nbyte % 8 == 0)
				Numbering(1);

			if (chwidth[0] == 5)
				inputOnce = 1; //���� �Է��� ����Ʈ�� �� ��° ���ڶ�� ���� ǥ��
		}
		else //����Ʈ �� ��° ����
		{
			wsprintf(msg + 2, L"%c", wParam); //msg���� " XX  "�� ����Ǿ� �־ msg+2�� �Է��� ���� ������ ����Ʈ �� ��° ���ڸ� �Է��� ���� ��

			ascii = wcstol(msg + 1, NULL, 16);
			wsprintf(msg2, L"%c", isprint(ascii) && ascii != VK_TAB ? ascii : '.');

			ReplaceSel(hWnd, pos - 1, pos, msg + 2); //ù ���ڸ� �Է��� �� 0�� �ӽ÷� �־��� ������ �̸� ����� �Է�
			SetSel(hWnd, pos + 2);

			ReplaceSel(hWnd2, pos2, pos2 + 1, msg2);

			if ((rpos - 3) % 35 == 0 && rpos != 3) //���� ������ ������ ��� rpos�� 38
				SetSel(hWnd, pos + 4);

			bytes[nline * 8 + rpos / 5] = ascii;
			inputOnce = 0;
		}

		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_TAB) //tab ó��
		{
			CallWindowProc(binaryOldEditProc[chwidth[0] == 5 ? 0 : 1], hWnd, iMessage, wParam, lParam);

			pos = GetPos(hWnd);
			rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0); //���� ù��°�� 0���� �� ��ġ
			nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0); //���� Ŀ�� ��
			pos2 = nline * (chwidth[1] * 8 + 2) + rpos / chwidth[0] * chwidth[1]; //ASCII edit�� Ŀ�� ��ġ

			SetSel(hWnd2, pos2);
		}
		else if (!shortCutHandler(wParam, hWnd)) //ctrl + a ���� ����Ű�� ���� ó��
		{
			pos = GetPos(hWnd);
			KeyDownProcess(wParam, hWnd, pos);
		}

		return 0;
	case WM_SETFOCUS: //IDC_D3_VNAME���� Tab���� �̵��� �� edit�� ��� ������ ���õǴµ� �̸� �ذ��ϱ� ���� ASCII edit Ŀ�� ��ġ�� Ŀ�� ��ġ ����
		pos = GetPos(hWnd2);
		rpos = pos - SendMessage(hWnd2, EM_LINEINDEX, -1, 0);
		nline = SendMessage(hWnd2, EM_LINEFROMCHAR, pos, 0);
		pos2 = nline * (chwidth[0] * 8 + 2) + rpos / chwidth[1] * chwidth[0];
		SetSel(hWnd, pos2);

		break;
	case WM_CONTEXTMENU: //������ ���콺 ������ �� �޴�
		openBinaryEditorMenu(LOWORD(lParam), HIWORD(lParam));

		return 0;
	case WM_LBUTTONDOWN:
		LbuttonDownProcess(hWnd, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))));
		isDrag = 1;
		oldpos = GetPos(hWnd); //�巡�� �� �� ���� ��ġ ����
		SetCapture(hWnd); //edit ������ ���콺 ������ ���콺 ���� ���ŵ�

		return 0;
	case WM_LBUTTONUP:
		isDrag = 0;
		ReleaseCapture();

		break;
	case WM_MOUSEMOVE:
		MouseMoveProcess(hWnd, lParam, oldpos);
		return 0;
	case WM_MOUSEWHEEL:
		MouseWheelProcess(wParam);
		return TRUE;
	}

	return CallWindowProc(binaryOldEditProc[chwidth[0] == 5 ? 0 : 1], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK BinaryNumberingEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int index;
	switch (iMessage)
	{
	case WM_LBUTTONDOWN: //Ŭ���� �� ��ġ�� VDATA edit�� Ŀ���� �ű�
		index = SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))), 0), 0);
		SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_SETSEL, index, index);
		return 0;
	}

	return CallWindowProc(binaryOldEditProc[2], hWnd, iMessage, wParam, lParam);
}