#include"header.h"
#include"BinaryEditor.h"

WNDPROC binaryOldEditProc[3];
int nbyte, inputOnce, scrollPos;
BYTE bytes[5000];

void autoLineFeed(int opt, HWND hWnd, int pos)
{
	HWND hWnd2;
	int chwidth[2] = { 5, 2 }, rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0), index, index2;

	if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA))
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII);
	else
	{
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA);
		chwidth[0] = 2;
		chwidth[1] = 5;
	}

	if (opt) //문자 입력할 때
	{
		int i = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0) - 1;
		if (i != SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0) && SendMessage(hWnd, EM_LINELENGTH, SendMessage(hWnd, EM_LINEINDEX, i, 0), 0) == chwidth[0] * 7)
		{
			index = SendMessage(hWnd, EM_LINEINDEX, i, 0) + chwidth[0] * 7;
			index2 = SendMessage(hWnd2, EM_LINEINDEX, i, 0) + chwidth[1] * 7;

			ReplaceSel(hWnd, index, index, L"\r\n");
			ReplaceSel(hWnd2, index2, index2, L"\r\n");
		}
		for (i -= 1; i >= SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0); i--) //입력한 줄 아래쪽 자동 개행
		{
			if (SendMessage(hWnd, EM_LINELENGTH, SendMessage(hWnd, EM_LINEINDEX, i, 0), 0) >= chwidth[0] * 8) //개행해야 하는 경우
			{
				index = SendMessage(hWnd, EM_LINEINDEX, i, 0) + chwidth[0] * 8;
				index2 = SendMessage(hWnd2, EM_LINEINDEX, i, 0) + chwidth[1] * 8;

				ReplaceSel(hWnd, index, index + 2, L""); //이전 개행 문자 제거
				ReplaceSel(hWnd2, index2, index2 + 2, L""); //이전 개행 문자 제거

				if (i > SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0)) //입력한 줄 아래 쪽인 경우
				{
					ReplaceSel(hWnd, index - chwidth[0], index - chwidth[0], L"\r\n");
					ReplaceSel(hWnd2, index2 - chwidth[1], index2 - chwidth[1], L"\r\n");
				}
				else if (rpos < chwidth[0] * 7) //입력한 줄인데 8번째 문자가 아닌 경우 7번째 문자에 개행 문자 추가
				{
					ReplaceSel(hWnd, index - chwidth[0], index - chwidth[0], L"\r\n");
					ReplaceSel(hWnd2, index2 - chwidth[1], index2 - chwidth[1], L"\r\n");
				}
			}
		}
	}
	else //문자 지울 때
	{
		for (int i = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0) - 1; i >= SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0) + 1; i--)
		{
			int index = SendMessage(hWnd, EM_LINEINDEX, i, 0);
			int index2 = SendMessage(hWnd2, EM_LINEINDEX, i, 0);

			if (SendMessage(hWnd, EM_LINELENGTH, index, 0) != 0) //줄에 문자가 더 없으면 첫번째 문자에 개행문자 붙이기
			{
				ReplaceSel(hWnd, index + chwidth[0], index + chwidth[0], L"\r\n");
				ReplaceSel(hWnd2, index2 + chwidth[1], index2 + chwidth[1], L"\r\n");
			}
			//원래 개행 문자 지우기
			ReplaceSel(hWnd, index - 2, index, L"");
			ReplaceSel(hWnd2, index2 - 2, index2, L"");
		}
	}
}

void Numbering(HWND hWnd)
{
	int line;
	SCROLLINFO si = {};

	if (hWnd == 0)
	{
		if (nbyte % 8 == 0) //줄이 늘어날 때
		{
			TCHAR* temp = (TCHAR*)malloc(sizeof(TCHAR) * (nbyte / 8 + 2) * 10 + 1);
			GetWindowText(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), temp, 1000);

			wsprintf(temp, L"%ws\r\n%08x", temp, (nbyte / 8 + 1) * 8);
			SetWindowText(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), temp);

			free(temp);

			line = SendMessage(GetFocus(), EM_GETLINECOUNT, 0, 0);
			if (line == 12)
			{
				SetWindowPos(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), NULL, 0, 0, 138, 195, SWP_NOMOVE);
				ShowWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SW_SHOW);
				EnableWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), TRUE);

				si.fMask = SIF_PAGE;
				si.nPage = 11;
			}
			if (line >= 12)
			{
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = si.fMask | SIF_RANGE | SIF_POS;
				si.nMin = 0;
				si.nMax = 11 + line - 12;
				si.nPos = SendMessage(GetFocus(), EM_GETFIRSTVISIBLELINE, 0, 0);
				scrollPos = si.nPos;
				SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), EM_LINESCROLL, 0, scrollPos - SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), EM_GETFIRSTVISIBLELINE, 0, 0));

				SetScrollInfo(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, &si, TRUE);
			}
		}
	}
	else //줄이 줄어들 때
	{
		if ((nbyte + 1) % 8 == 0)
		{
			TCHAR* temp = (TCHAR*)malloc(sizeof(TCHAR) * (nbyte / 8 + 2) * 10 + 1);
			GetWindowText(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), temp, 1000);

			memset(temp + (SendMessage(hWnd, EM_GETLINECOUNT, 0, 0) * 10 - 2), 0, 20);
			SetWindowText(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), temp);

			free(temp);
		}

		line = SendMessage(GetFocus(), EM_GETLINECOUNT, 0, 0);
		if (line == 11)
		{
			EnableWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), FALSE);
		}
	}
}

void KeyDownProcess(int vkey, HWND hWnd, int pos)
{
	HWND hWnd2;
	int chwidth[2] = { 5, 2 }, rpos, nline, pos2, li;

	if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA))
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII);
	else
	{
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA);
		chwidth[0] = 2;
		chwidth[1] = 5;
	}

	rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0);
	nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0);
	pos2 = nline * (8 * chwidth[1] + 2) + rpos / chwidth[0] * chwidth[1];

	switch (vkey)
	{
	case VK_LEFT:
		if (rpos > 0)
		{
			if (inputOnce == 1)
			{
				SetSel(hWnd, pos - 3);
				inputOnce = 0;
			}
			else
				SetSel(hWnd, pos - chwidth[0]);
		}
		else if (pos != 0)
			SetSel(hWnd, pos - chwidth[0] - 2);

		break;
	case VK_RIGHT:
		if (inputOnce == 1)
		{
			if (rpos > 35)
				SetSel(hWnd, pos + 4);
			else
				SetSel(hWnd, pos + 2);
			inputOnce = 0;
		}
		else
		{
			int itemToPos = nline * 8 + rpos / chwidth[0];
			if (rpos >= chwidth[0] * 7 && itemToPos + 1 <= nbyte)
				SetSel(hWnd, SendMessage(hWnd, EM_LINEINDEX, nline + 1, 0));
			else if (nbyte >= itemToPos + 1)
				SetSel(hWnd, pos + chwidth[0]);
		}

		break;
	case VK_UP:
		li = SendMessage(hWnd, EM_LINEINDEX, -1, 0);
		if (li != 0)
		{
			SetSel(hWnd, SendMessage(hWnd, EM_LINEINDEX, nline - 1, 0) + (rpos / chwidth[0]) * chwidth[0]);
			inputOnce = 0;
		}
		break;
		break;
	case VK_DOWN:
		if (nline < SendMessage(hWnd, EM_GETLINECOUNT, 0, 0) - 1)
		{
			int dindex = SendMessage(hWnd, EM_LINEINDEX, nline + 1, 0) + (rpos / chwidth[0]) * chwidth[0];
			int lindex = SendMessage(hWnd, EM_LINELENGTH, dindex, 0);
			SetSel(hWnd, dindex > lindex ? dindex : lindex);
			inputOnce = 0;
		}
		break;
	case VK_BACK:
		if (pos != 0)
		{
			autoLineFeed(0, hWnd, pos);

			if (inputOnce == 1 && hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA))
			{
				ReplaceSel(hWnd, pos - 3, pos + 2, L"");
				ReplaceSel(hWnd2, pos2, pos2 + 2, L"");
				inputOnce = 0;
			}
			else
			{
				if (rpos == 0)
				{
					if (SendMessage(hWnd, EM_LINELENGTH, pos, 0) != 0)
					{
						ReplaceSel(hWnd, pos + chwidth[0], pos + chwidth[0], L"\r\n");
						ReplaceSel(hWnd2, pos2 + chwidth[1], pos2 + chwidth[1], L"\r\n");
					}

					ReplaceSel(hWnd, pos - chwidth[0] - 2, pos, L"");
					ReplaceSel(hWnd2, pos2 - chwidth[1] - 2, pos2, L"");
				}
				else
				{
					ReplaceSel(hWnd, pos - chwidth[0], pos, L"");
					ReplaceSel(hWnd2, pos2 - chwidth[1], pos2, L"");
				}
			}

			int nowindex = nline * 8 + (rpos - 1) / 5;

			if (nowindex < nbyte - 1)
			{
				for (int i = nowindex; i < nbyte - 1; i++)
					bytes[i] = bytes[i + 1];
			}

			nbyte--;
			Numbering(hWnd);
		}
		break;
	}
}

void LbuttonDownProcess(HWND hWnd, int pos)
{
	int li, rpos, chwidth = (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA) ? 5 : 2);

	SetFocus(hWnd);

	li = SendMessage(hWnd, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0), 0);
	rpos = pos - li;

	SetSel(hWnd, li + (rpos / chwidth) * chwidth);
	inputOnce = 0;
}

void MouseWheelProcess(int param)
{
	if (SendMessage(GetFocus(), EM_GETLINECOUNT, 0, 0) < 12)
		return;

	if (GET_WHEEL_DELTA_WPARAM(param) > 0)
	{
		if (scrollPos - 1 >= 0)
		{
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), WM_VSCROLL, SB_LINEUP, NULL);
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), WM_VSCROLL, SB_LINEUP, NULL);
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), WM_VSCROLL, SB_LINEUP, NULL);

			scrollPos--;
			SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
		}
	}
	else if (GET_WHEEL_DELTA_WPARAM(param) < 0)
	{
		if (scrollPos + 1 <= SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_GETLINECOUNT, 0, 0) - 11)
		{
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), WM_VSCROLL, SB_LINEDOWN, NULL);
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), WM_VSCROLL, SB_LINEDOWN, NULL);
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), WM_VSCROLL, SB_LINEDOWN, NULL);

			scrollPos++;
			SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
		}
	}
}

void ScrollEdits(int lParam, int wParam)
{
	int line = SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_GETLINECOUNT, 0, 0), nscroll = 0;

	switch (LOWORD(wParam))
	{
	case SB_LINEUP:
		scrollPos = max(0, scrollPos - 1);
		nscroll = -1;
		break;
	case SB_LINEDOWN:
		scrollPos = min(line - 11, scrollPos + 1);
		nscroll = 1;
		break;
	case SB_PAGEUP:
		scrollPos = max(0, scrollPos - 5);
		nscroll = -5;
		break;
	case SB_PAGEDOWN:
		scrollPos = min(line - 11, scrollPos + 5);
		nscroll = 5;
		break;
	case SB_THUMBTRACK:
		nscroll = HIWORD(wParam) - scrollPos;
		scrollPos = HIWORD(wParam);
		break;
	}
	SetScrollPos((HWND)lParam, SB_CTL, scrollPos, TRUE);

	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_LINESCROLL, 0, nscroll);
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), EM_LINESCROLL, 0, nscroll);
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), EM_LINESCROLL, 0, nscroll);
}