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
		GetWindowText(hEdit, t_path, sizeof(t_path));
		break;
	case WM_KEYUP:
		if (wParam == VK_RETURN)
		{
			GetWindowText(hEdit, temp, sizeof(temp));
			item = getItemfromPath(temp);
			if (item != 0)
				TreeView_SelectItem(hTV, item);
			else
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
			{
				if (SendMessage(GetDlgItem(hDlgFind, IDC_CHECK_CHANGE), BM_GETCHECK, 0, 0) == BST_CHECKED)
					SendMessage(hDlgFind, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_CHANGE, BN_CLICKED), (LPARAM)GetDlgItem(hDlgFind, IDC_BUTTON_CHANGE));
				else
					SendMessage(hDlgFind, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_FIND, BN_CLICKED), (LPARAM)GetDlgItem(hDlgFind, IDC_BUTTON_FIND));
			}
			else if (hWnd == GetDlgItem(hDlgModify, IDC_D2_VDATA))
			{
				SendMessage(hDlgModify, WM_COMMAND, MAKEWPARAM(IDC_D2_MODIFY_OK, BN_CLICKED), (LPARAM)GetDlgItem(hDlgModify, IDC_D2_MODIFY_OK));
			}
		}
		break;
	}
	if (hWnd == GetDlgItem(hDlgFind, IDC_EDIT_FIND))
		return CallWindowProc(oldDlgEditProc[0], hWnd, iMessage, wParam, lParam);
	else if (hWnd == GetDlgItem(hDlgModify, IDC_D2_VDATA))
		return CallWindowProc(oldDlgEditProc[1], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK BinaryEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int pos, rpos, li, prevpos[2], nline, pos2, ascii;
	static TCHAR msg[10], msg2[10];
	HWND hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII);

	switch (iMessage)
	{
	case WM_CHAR:
		if ((wParam >= '0' && wParam <= '9') || (wParam >= 'a' && wParam <= 'f') || (wParam >= 'A' && wParam <= 'F'))
		{
			if (wParam >= 'a' && wParam <= 'f')
				wParam -= 32;

			pos = GetPos(hWnd);
			rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0);
			nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0);
			pos2 = nline * 18 + rpos / 5 * 2;

			if (inputOnce == 0) //바이트 입력 시작
			{
				if (rpos == 40) //한 줄의 마지막 위치에서 입력하면 다음 줄로 이동
				{
					SetSel(hWnd, pos + 2);
					SetSel(hWnd2, pos2 + 2);
					pos += 2;
					pos2 += 2;
					rpos = 0;
				}

				if (rpos % 35 == 0 && rpos != 0) //8번째 문자의 경우 개행 문자 추가
				{
					wsprintf(msg, L" %c0  \r\n", wParam);
					wsprintf(msg2, L"%c \r\n", wParam);
				}
				else
				{
					wsprintf(msg, L" %c0  ", wParam);
					wsprintf(msg2, L"%c ", wParam);
				}

				ascii = wcstol(msg + 1, NULL, 16);
				msg2[0] = isprint(ascii) ? ascii : '.';

				prevpos[0] = pos;
				prevpos[1] = pos2;
				autoLineFeed(1, hWnd, pos);
				SetSel(hWnd, prevpos[0]);
				SetSel(hWnd2, prevpos[1]); //ascii 에디트에서는 여기서 문자열을 입력할 위치로 이동

				SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)msg);
				SendMessage(hWnd2, EM_REPLACESEL, TRUE, (LPARAM)msg2);

				SetSel(hWnd, pos + 3);

				//데이터 저장
				int nowindex = nline * 8 + rpos / 5;
				if (nowindex < nbyte) //중간에 데이터를 추가해야 하는 경우 한 칸씩 밀기
				{
					for (int i = nbyte - 1; i >= nowindex; i--)
						bytes[i + 1] = bytes[i];
				}
				bytes[nowindex] = ascii;

				nbyte++;

				Numbering(0);

				inputOnce = 1;
			}
			else
			{
				wsprintf(msg + 2, L"%c", wParam);

				ascii = wcstol(msg + 1, NULL, 16);
				wsprintf(msg2, L"%c", isprint(ascii) ? ascii : '.');

				ReplaceSel(hWnd, pos - 1, pos, msg + 2);
				SetSel(hWnd, pos + 2);

				ReplaceSel(hWnd2, pos2, pos2 + 1, msg2);

				if ((rpos - 3) % 35 == 0 && rpos != 3)
					SetSel(hWnd, pos + 4);

				bytes[nline * 8 + rpos / 5] = ascii;
				inputOnce = 0;
			}

			return 0;
		}
		return 0;
	case WM_KEYDOWN:
		KeyDownProcess(wParam, hWnd, GetPos(hWnd));

		return 0;
	case WM_LBUTTONDOWN:
		LbuttonDownProcess(hWnd, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))));

		return 0;
	case WM_MOUSEWHEEL:
		MouseWheelProcess(wParam);
		return TRUE;
	}

	return CallWindowProc(binaryOldEditProc[0], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK BinaryAsciiEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	TCHAR msg[5], msg2[8];
	int pos, rpos, li, nline, prevpos[2], nowindex, pos2;
	HWND hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA);

	switch (iMessage)
	{
	case WM_CHAR:
		if (isprint(wParam))
		{
			pos = GetPos(hWnd);
			rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0);
			nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0);
			pos2 = nline * 42 + rpos / 2 * 5;

			if (rpos == 16)
			{
				SetSel(hWnd, pos + 2);
				SetSel(hWnd2, pos2 + 2);
				pos += 2;
				pos2 += 2;
				rpos = 0;
			}

			prevpos[0] = pos;
			prevpos[1] = pos2;
			autoLineFeed(1, hWnd, pos);
			SetSel(hWnd, prevpos[0]);
			SetSel(hWnd2, prevpos[1]);

			if (rpos % 14 == 0 && rpos != 0)
			{
				wsprintf(msg, L"%c \r\n", wParam);
				wsprintf(msg2, L" %02X  \r\n", wParam);
			}
			else
			{
				wsprintf(msg, L"%c ", wParam);
				wsprintf(msg2, L" %02X  ", wParam);
			}

			SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)msg);
			SendMessage(hWnd2, EM_REPLACESEL, TRUE, (LPARAM)msg2);

			nowindex = nline * 8 + rpos / 2;
			if (nowindex < nbyte)
			{
				for (int i = nbyte - 1; i >= nowindex; i--)
					bytes[i + 1] = bytes[i];
			}
			bytes[nowindex] = wcstol(msg2, NULL, 16);

			nbyte++;

			Numbering(0);
		}
		return 0;
	case WM_KEYDOWN:
		KeyDownProcess(wParam, hWnd, GetPos(hWnd));

		return 0;
	case WM_LBUTTONDOWN:
		LbuttonDownProcess(hWnd, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))));

		return 0;
	case WM_MOUSEWHEEL:
		MouseWheelProcess(wParam);

		return 0;
	}

	return CallWindowProc(binaryOldEditProc[1], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK BinaryNumberingEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int index;
	switch (iMessage)
	{
	case WM_LBUTTONDOWN:
		index = SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))), 0), 0);
		SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_SETSEL, index, index);
		return 0;
	}

	return CallWindowProc(binaryOldEditProc[2], hWnd, iMessage, wParam, lParam);
}