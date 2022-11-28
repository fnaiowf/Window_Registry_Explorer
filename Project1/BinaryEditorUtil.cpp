#include"header.h"
#include"BinaryEditor.h"

WNDPROC binaryOldEditProc[3];
int nbyte, inputOnce, scrollPos, isDrag;;
BYTE bytes[5000];
CLIPBOARD_DATA clipBoardData = { 0 };

//커서 다음 글자부터 개행상태를 수정
//커서 다음 글자부터 edit 텍스트를 다시 만들어서 기존 텍스트를 교체
void autoLineFeed(int opt, HWND hWnd, int pos) 
{
	HWND hWnd2;
	TCHAR* text[2], temp[2][10]; //temp : 바이트 데이터를 문자로 변경할 때 사용
	int chwidth[2] = { 5, 2 }, rpos, nline, pos2, fv, index;

	rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0);
	nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0);

	if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA))
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII);
	else
	{
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA);
		chwidth[0] = 2;
		chwidth[1] = 5;
	}

	fv = FirstVisibleLine(hWnd); //작업 수행하면 스크롤 상태가 변해서 기존 상태를 저장
	pos2 = nline * (chwidth[1] * 8 + 2) + rpos / chwidth[0] * chwidth[1]; //현재 줄 * (자간 * 한 줄에 8 문자 + \r\n 2글자) + 현재 줄에서 커서 위치를 다른 edit 기준으로 바꾼 값
	index = nline * 8 + rpos / chwidth[0]; //현재 커서 위치의 값 인덱스

	text[0] = (TCHAR*)calloc((nbyte - index) * 5 + (nbyte / 8 + 1 - nline) * 2 + 1, sizeof(TCHAR)); //커서 위치 다음 글자부터 nbyte까지 크기
	text[1] = (TCHAR*)calloc((nbyte - index) * 2 + (nbyte / 8 + 1 - nline) * 2 + 1, sizeof(TCHAR));

	for (int i = index; i < nbyte; i++)
	{
		if ((i + opt) % 8 == 0) //opt=0:글자를 지우는 경우 opt=1:여러 값 선택해서 지우거나 붙여넣기 opt=2:글자 입력하는 경우
		{
			wsprintf(temp[0], L" %02X  \r\n", bytes[i]);
			wsprintf(temp[1], L"%c \r\n", isprint(bytes[i]) && bytes[i] != VK_TAB ? bytes[i] : '.');
		}
		else
		{
			wsprintf(temp[0], L" %02X  ", bytes[i]);
			wsprintf(temp[1], L"%c ", isprint(bytes[i]) && bytes[i] != VK_TAB ? bytes[i] : '.');
		}

		wcscat(text[0], temp[0]);
		wcscat(text[1], temp[1]);
	}

	ReplaceSel(hWnd, pos, -1, text[chwidth[0] != 5]); //핸들이 VDATA인 경우 chwidth[0]=5이므로 chwidth[0]!=5는 0 VDATA_ASCII인 경우 chwidth[0]=2이므로 chwidth[0]!=5는 1
	ReplaceSel(hWnd2, pos2, -1, text[chwidth[0] == 5]);

	SendMessage(hWnd, EM_LINESCROLL, 0, fv - FirstVisibleLine(hWnd)); //기존 스크롤 위치로 이동
	SendMessage(hWnd2, EM_LINESCROLL, 0, fv - FirstVisibleLine(hWnd2));

	free(text[0]);
	free(text[1]);
}

void Numbering(int increase) //호출할 때마다 텍스트를 처음부터 구성해서 오래 걸리기 때문에 반드시 줄번호가 변하는 경우에만 호출
{
	int line = LineCount(GetFocus());

	TCHAR* temp = (TCHAR*)calloc((line + increase) * 10 + 1, sizeof(TCHAR));
	for (int i = 0; i < line - 1; i++)
		wsprintf(temp, L"%ws%08x\r\n", temp, i * 8);

	wsprintf(temp, L"%ws%08x", temp, (line - 1) * 8);

	SetWindowText(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), temp);
	free(temp);

	if (increase) //줄이 늘어날 때
	{
		if (line == 12) //스크롤이 가능해지는 줄 번호
		{
			ShowWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SW_SHOW);
			EnableWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), TRUE);
		}
		if (line >= 12) //스크롤 최대값 변경
			SetScroll(line);
	}
	else //줄이 줄어들 때
	{
		if (line <= 11)
			EnableWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), FALSE);
		else
			SetScroll(line);
	}
}

void RemoveSelections(HWND hWnd)
{
	int chwidth = hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA) ? 5 : 2;
	int getSel = SendMessage(hWnd, EM_GETSEL, 0, 0);

	if (LOWORD(getSel) != HIWORD(getSel))
	{
		int lfc[2] = { SendMessage(hWnd, EM_LINEFROMCHAR, LOWORD(getSel), 0), SendMessage(hWnd, EM_LINEFROMCHAR, HIWORD(getSel), 0) };

		int idx1 = lfc[0] * 8 + (LOWORD(getSel) - SendMessage(hWnd, EM_LINEINDEX, lfc[0], 0)) / chwidth,
			idx2 = lfc[1] * 8 + (HIWORD(getSel) - SendMessage(hWnd, EM_LINEINDEX, lfc[1], 0)) / chwidth;

		SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)L"");
		for (int i = idx1, j = idx2; j < nbyte; i++, j++)
			bytes[i] = bytes[j];

		nbyte -= (idx2 - idx1);

		autoLineFeed(1, hWnd, LOWORD(getSel));
		SetSel(hWnd, LOWORD(getSel));

		Numbering(0);
	}
}

void KeyDownProcess(int vkey, HWND hWnd, int pos)
{
	if (vkey == VK_RETURN)
	{
		SendMessage(hDlgModify, WM_COMMAND, MAKEWPARAM(IDC_D3_MODIFY_OK, BN_CLICKED), (LPARAM)GetDlgItem(hDlgModify, IDC_D3_MODIFY_OK));
		return;
	}

	HWND hWnd2;
	int chwidth[2] = { 5, 2 }, rpos, nline, pos2, li, getSel, itemIndex;

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
			SetSel(hWnd, pos - chwidth[0] - 2); //-2 : \r\n

		break;
	case VK_RIGHT:
		if (inputOnce == 1)
		{
			if (rpos > 35) //마지막 바이트 입력 중
				SetSel(hWnd, pos + 4);
			else
				SetSel(hWnd, pos + 2);
			inputOnce = 0;
		}
		else
		{
			int itemIndex = nline * 8 + rpos / chwidth[0];
			if (rpos >= chwidth[0] * 7 && itemIndex + 1 <= nbyte)
				SetSel(hWnd, SendMessage(hWnd, EM_LINEINDEX, nline + 1, 0));
			else if (nbyte >= itemIndex + 1)
				SetSel(hWnd, pos + chwidth[0]);
		}

		break;
	case VK_UP:
		li = SendMessage(hWnd, EM_LINEINDEX, -1, 0);
		if (li != 0) //LINEINDEX가 0이면 첫번째 줄이라는 뜻
		{
			SetSel(hWnd, SendMessage(hWnd, EM_LINEINDEX, nline - 1, 0) + (rpos / chwidth[0]) * chwidth[0]);
			inputOnce = 0;
		}
		break;
	case VK_DOWN:
		if (nline < LineCount(hWnd) - 1)
		{
			int dindex = SendMessage(hWnd, EM_LINEINDEX, nline + 1, 0) + (rpos / chwidth[0]) * chwidth[0]; //바로 아래로 내려갈 때 위치
			int lindex = SendMessage(hWnd, EM_LINELENGTH, dindex, 0); //아래 줄 마지막 위치
			SetSel(hWnd, dindex > lindex ? dindex : lindex);
			inputOnce = 0;
		}
		break;
	case VK_BACK:
		getSel = SendMessage(hWnd, EM_GETSEL, 0, 0);
		if (pos == 0 && HIWORD(getSel) == 0) //맨 처음 위치이면서 선택한 경우도 아닐 때
			return;

		if (LOWORD(getSel) != HIWORD(getSel)) //선택되어 있는 경우
		{
			RemoveSelections(hWnd);
			return;
		}

		autoLineFeed(0, hWnd, pos + inputOnce * 2); //inputOnce가 1(바이트 입력 중)인 경우 pos에 2를 더해 올바른 위치로 설정하고 호출

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
				ReplaceSel(hWnd, pos - chwidth[0] - 2, pos, L"");
				ReplaceSel(hWnd2, pos2 - chwidth[1] - 2, pos2, L"");
			}
			else
			{
				ReplaceSel(hWnd, pos - chwidth[0], pos, L"");
				ReplaceSel(hWnd2, pos2 - chwidth[1], pos2, L"");
			}
		}

		itemIndex = nline * 8 + (rpos - 1) / chwidth[0];

		if (itemIndex < nbyte - 1) //데이터 배열 땡기기
		{
			for (int i = itemIndex; i < nbyte - 1; i++)
				bytes[i] = bytes[i + 1];
		}

		nbyte--;

		if ((nbyte + 1) % 8 == 0) //줄번호가 바뀌는 경우
			Numbering(0);

		return;
	default:
		return;
	}

	//키보드 입력의 결과대로 스크롤 상태를 변경
	nline = SendMessage(hWnd, EM_LINEFROMCHAR, GetPos(hWnd), 0);;
	if (nline < FirstVisibleLine(hWnd))
		SendMessage(hDlgModify, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), (LPARAM)GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR));
	else if (nline == FirstVisibleLine(hWnd) + 11)
		SendMessage(hDlgModify, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), (LPARAM)GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR));
}

int shortCutHandler(int wParam, HWND hWnd)
{
	int getSel, chwidth[2] = { 5, 2 }, pos, nline, rpos;

	if (hWnd != GetDlgItem(hDlgModify, IDC_D3_VDATA))
		chwidth[0] = 2, chwidth[1] = 5;

	//키보드 입력으로는 wParam에 음수가 전달되지 않기 때문에 마우스 오른쪽 버튼으로 여는 메뉴에서 선택하는 경우에는 wParam을 음수 값으로 해서 조건문을 만족시킴
	if (wParam == -5 || (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'A')) //모두 선택
	{
		SendMessage(hWnd, EM_SETSEL, 0, -1);
		return 1;
	}
	else if ((wParam == -1 || wParam == -2) || (GetKeyState(VK_CONTROL) & 0x8000 && (wParam == 'C' || wParam == 'X'))) //복사, 잘라내기
	{
		getSel = SendMessage(hWnd, EM_GETSEL, 0, 0);
		if (LOWORD(getSel) == HIWORD(getSel))
			return 1;

		int lfc[2] = { SendMessage(hWnd, EM_LINEFROMCHAR, LOWORD(getSel), 0), SendMessage(hWnd, EM_LINEFROMCHAR, HIWORD(getSel), 0) };

		int idx1 = lfc[0] * 8 + (LOWORD(getSel) - SendMessage(hWnd, EM_LINEINDEX, lfc[0], 0)) / chwidth[0],
			idx2 = lfc[1] * 8 + (HIWORD(getSel) - SendMessage(hWnd, EM_LINEINDEX, lfc[1], 0)) / chwidth[0];

		if (clipBoardData.len != 0)
			clipBoardData.bytes = (BYTE*)realloc(clipBoardData.bytes, idx2 - idx1);
		else
			clipBoardData.bytes = (BYTE*)malloc(idx2 - idx1);

		if (clipBoardData.bytes != NULL)
		{
			clipBoardData.len = idx2 - idx1;
			memcpy(clipBoardData.bytes, bytes + idx1, idx2 - idx1);

			if (wParam == 'X' || wParam == -1)
				RemoveSelections(hWnd);
		}

		return 1;
	}
	else if (wParam == -3 || (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'V')) //붙여넣기
	{
		if (clipBoardData.len != 0)
		{
			int len = clipBoardData.len;
			RemoveSelections(hWnd);

			pos = GetPos(hWnd);
			nline = SendMessage(hWnd, EM_LINEFROMCHAR, -1, 0);
			rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0);
			int idx = nline * 8 + rpos / chwidth[0];

			for (int i = nbyte - 1; i >= idx; i--) //데이터 밀기
				bytes[i + len] = bytes[i];

			memcpy(bytes + idx, clipBoardData.bytes, len);

			nbyte += len;

			pos = rpos == chwidth[0] * 8 ? pos + 2 : pos;
			SetSel(hWnd, pos);

			autoLineFeed(1, hWnd, pos);

			for (int i = idx; i < idx + len; i++) //복사 후 위치 계산
				pos += (i + 1) % 8 == 0 ? chwidth[0] + 2 : chwidth[0];

			SetSel(hWnd, pos);

			Numbering(1);

			return 1;
		}
	}
	else
		return 0;
}

void LbuttonDownProcess(HWND hWnd, int pos)
{
	int li, rpos, chwidth = (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA) ? 5 : 2);

	if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA)) //왼쪽 마우스 누르면 다른 edit의 선택 상태 지움
		SetSel(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), 0);
	else
		SetSel(GetDlgItem(hDlgModify, IDC_D3_VDATA), 0);

	SetFocus(hWnd);

	li = SendMessage(hWnd, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0), 0);
	rpos = pos - li;

	SetSel(hWnd, li + (rpos / chwidth) * chwidth);
	inputOnce = 0;

	if (SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0) == FirstVisibleLine(hWnd) + 11)
	{
		scrollPos++;
		ScrollEdits(1);
		SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
	}
}

void MouseWheelProcess(int param)
{
	if (LineCount(GetFocus()) < 12) //스크롤 할 수 없는 경우
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
		if (scrollPos + 1 <= LineCount(GetDlgItem(hDlgModify, IDC_D3_VDATA)) - 11)
		{
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), WM_VSCROLL, SB_LINEDOWN, NULL);
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), WM_VSCROLL, SB_LINEDOWN, NULL);
			SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), WM_VSCROLL, SB_LINEDOWN, NULL);

			scrollPos++;
			SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
		}
	}
}

void MouseMoveProcess(HWND hWnd, int lParam, int oldpos)
{
	if (!isDrag) //마우스 누르고 있지 않는 경우
		return;

	HWND hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII);
	int rpos[2], newpos, chwidth[2] = {5, 2}, li[2], li2, oldpos2, newpos2, rpos2, lastLineYpos, rMousePos[2];
	RECT r;
	SendMessage(hWnd, EM_GETRECT, 0, (LPARAM)&r);

	if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII))
	{
		hWnd2 = GetDlgItem(hDlgModify, IDC_D3_VDATA);
		chwidth[0] = 2;
		chwidth[1] = 5;
	}

	lastLineYpos = HIWORD(SendMessage(hWnd, EM_POSFROMCHAR, SendMessage(hWnd, EM_LINEINDEX, LineCount(hWnd) - 1, 0), 0)); //마지막 줄 y좌표
	rMousePos[0] = LOWORD(lParam) > 60000 ? 0 : min(r.right, LOWORD(lParam)); //마우스가 화면 밖으로 이동할 때 x값 고정
	rMousePos[1] = HIWORD(lParam) > 60000 ? 0 : min(lastLineYpos, HIWORD(lParam));//y값 고정

	newpos = LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(rMousePos[0], rMousePos[1]))); //마우스 위치 -> edit의 커서 위치(선택 영역의 마지막 위치)
	li[0] = SendMessage(hWnd, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, newpos, 0), 0); //선택 영역 마지막 줄 인덱스
	li[1] = SendMessage(hWnd, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, oldpos, 0), 0); //선택 영역 첫 줄 인덱스
	rpos[0] = (newpos - li[0]) / chwidth[0] * chwidth[0]; //선택 영역 마지막 줄의 처음 기준 위치
	rpos[1] = (oldpos - li[1]) / chwidth[0] * chwidth[0];//선택 영역 첫 줄의 처음 기준 위치

	//위 5가지 정보로 다른 edit의 선택 영역에 관한 값를 구할 수 있음
	newpos2 = SendMessage(hWnd, EM_LINEFROMCHAR, newpos, 0) * (8 * chwidth[1] + 2) + rpos[0] / chwidth[0] * chwidth[1];
	oldpos2 = SendMessage(hWnd, EM_LINEFROMCHAR, oldpos, 0) * (8 * chwidth[1] + 2) + rpos[1] / chwidth[0] * chwidth[1];
	li2 = SendMessage(hWnd2, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, newpos, 0), 0);
	rpos2 = (newpos2 - li2) / chwidth[1] * chwidth[1];

	if (oldpos <= newpos)
	{
		SendMessage(hWnd, EM_SETSEL, oldpos, li[0] + rpos[0]);
		SendMessage(hWnd2, EM_SETSEL, oldpos2, li2 + rpos2);
	}
	else
	{
		SendMessage(hWnd, EM_SETSEL, li[0] + rpos[0], oldpos);
		SendMessage(hWnd2, EM_SETSEL, li2 + rpos2, oldpos2);
	}

	//드래그하면서 edit 위 아래로 나가면 자동 스크롤
	if (HIWORD(lParam) > 60000 && FirstVisibleLine(hWnd) > 0) //edit밖으로 마우스 나가면 -1부터 시작해서 오버플로우 때문에 WORD 최대값인 65535부터 값이 감소함
	{
		scrollPos--;
		ScrollEdits(-1);
		SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
	}
	else if(HIWORD(lParam) < 5000 && HIWORD(SendMessage(hWnd, EM_POSFROMCHAR, SendMessage(hWnd, EM_LINEINDEX, FirstVisibleLine(hWnd) + 11, 0), 0)) < HIWORD(lParam) && FirstVisibleLine(hWnd) + 11 < LineCount(hWnd)) //edit 아래로 마우스가 나갈 때는 마우스 내리는대로 값이 내려감
	{
		scrollPos++;
		ScrollEdits(1);
		SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
	}
	//HIWORD(lParam) < 5000을 안걸면 edit 위로 마우스가 나갈 때 오버플로우 되기 때문에 조건문이 참이 됨

	return;
}

void ScrollProcess(int lParam, int wParam)
{
	int line = LineCount(GetDlgItem(hDlgModify, IDC_D3_VDATA)), nscroll = 0;

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
		scrollPos = max(0, scrollPos - 12);
		nscroll = -12;
		break;
	case SB_PAGEDOWN:
		scrollPos = min(line - 11, scrollPos + 12);
		nscroll = 12;
		break;
	case SB_THUMBTRACK:
		nscroll = HIWORD(wParam) - scrollPos;
		scrollPos = HIWORD(wParam);
		break;
	}
	SetScrollPos((HWND)lParam, SB_CTL, scrollPos, TRUE);

	ScrollEdits(nscroll);
}

void ScrollEdits(int nscroll)
{
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_LINESCROLL, 0, nscroll);
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII), EM_LINESCROLL, 0, nscroll);
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), EM_LINESCROLL, 0, nscroll);
}

void SetScroll(int lineCount)
{
	SCROLLINFO si = {};

	//nPage는 11고정, nMax는 11부터 스크롤 범위가 늘어날 때마다 1씩 더함
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = 11 + lineCount - 12;
	si.nPage = 11;
	si.nPos = FirstVisibleLine(GetFocus());
	scrollPos = si.nPos;
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), EM_LINESCROLL, 0, scrollPos - FirstVisibleLine(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING))); //나머지 두 edit은 자동 스크롤

	SetScrollInfo(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, &si, TRUE);
}

void openBinaryEditorMenu(int x, int y)
{
	HMENU menu, hPopup;
	int id, getSel;

	menu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU3));
	hPopup = GetSubMenu(menu, 0);

	if (nbyte == 0)
		EnableMenuItem(hPopup, 5, MF_BYPOSITION | MF_DISABLED);

	getSel = SendMessage(GetFocus(), EM_GETSEL, 0, 0);
	if (LOWORD(getSel) == HIWORD(getSel))
	{
		EnableMenuItem(hPopup, 0, MF_BYPOSITION | MF_DISABLED);
		EnableMenuItem(hPopup, 1, MF_BYPOSITION | MF_DISABLED);
		EnableMenuItem(hPopup, 3, MF_BYPOSITION | MF_DISABLED);
	}

	if(clipBoardData.len == 0)
		EnableMenuItem(hPopup, 2, MF_BYPOSITION | MF_DISABLED);

	id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWndMain, NULL);

	if (id == ID_MENU3_DELETE)
		RemoveSelections(GetFocus());
	else if(id != 0)
		shortCutHandler(ID_MENU3_CUT - id - 1, GetFocus());

	DestroyMenu(menu);
}