#include"header.h"
#include"BinaryEditor.h"

WNDPROC binaryOldEditProc[3];
int nbyte, inputOnce, scrollPos, isDrag;;
BYTE bytes[5000];
CLIPBOARD_DATA clipBoardData = { 0 };

//Ŀ�� ���� ���ں��� ������¸� ����
//Ŀ�� ���� ���ں��� edit �ؽ�Ʈ�� �ٽ� ���� ���� �ؽ�Ʈ�� ��ü
void autoLineFeed(int opt, HWND hWnd, int pos) 
{
	HWND hWnd2;
	TCHAR* text[2], temp[2][10]; //temp : ����Ʈ �����͸� ���ڷ� ������ �� ���
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

	fv = FirstVisibleLine(hWnd); //�۾� �����ϸ� ��ũ�� ���°� ���ؼ� ���� ���¸� ����
	pos2 = nline * (chwidth[1] * 8 + 2) + rpos / chwidth[0] * chwidth[1]; //���� �� * (�ڰ� * �� �ٿ� 8 ���� + \r\n 2����) + ���� �ٿ��� Ŀ�� ��ġ�� �ٸ� edit �������� �ٲ� ��
	index = nline * 8 + rpos / chwidth[0]; //���� Ŀ�� ��ġ�� �� �ε���

	text[0] = (TCHAR*)calloc((nbyte - index) * 5 + (nbyte / 8 + 1 - nline) * 2 + 1, sizeof(TCHAR)); //Ŀ�� ��ġ ���� ���ں��� nbyte���� ũ��
	text[1] = (TCHAR*)calloc((nbyte - index) * 2 + (nbyte / 8 + 1 - nline) * 2 + 1, sizeof(TCHAR));

	for (int i = index; i < nbyte; i++)
	{
		if ((i + opt) % 8 == 0) //opt=0:���ڸ� ����� ��� opt=1:���� �� �����ؼ� ����ų� �ٿ��ֱ� opt=2:���� �Է��ϴ� ���
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

	ReplaceSel(hWnd, pos, -1, text[chwidth[0] != 5]); //�ڵ��� VDATA�� ��� chwidth[0]=5�̹Ƿ� chwidth[0]!=5�� 0 VDATA_ASCII�� ��� chwidth[0]=2�̹Ƿ� chwidth[0]!=5�� 1
	ReplaceSel(hWnd2, pos2, -1, text[chwidth[0] == 5]);

	SendMessage(hWnd, EM_LINESCROLL, 0, fv - FirstVisibleLine(hWnd)); //���� ��ũ�� ��ġ�� �̵�
	SendMessage(hWnd2, EM_LINESCROLL, 0, fv - FirstVisibleLine(hWnd2));

	free(text[0]);
	free(text[1]);
}

void Numbering(int increase) //ȣ���� ������ �ؽ�Ʈ�� ó������ �����ؼ� ���� �ɸ��� ������ �ݵ�� �ٹ�ȣ�� ���ϴ� ��쿡�� ȣ��
{
	int line = LineCount(GetFocus());

	TCHAR* temp = (TCHAR*)calloc((line + increase) * 10 + 1, sizeof(TCHAR));
	for (int i = 0; i < line - 1; i++)
		wsprintf(temp, L"%ws%08x\r\n", temp, i * 8);

	wsprintf(temp, L"%ws%08x", temp, (line - 1) * 8);

	SetWindowText(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), temp);
	free(temp);

	if (increase) //���� �þ ��
	{
		if (line == 12) //��ũ���� ���������� �� ��ȣ
		{
			ShowWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SW_SHOW);
			EnableWindow(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), TRUE);
		}
		if (line >= 12) //��ũ�� �ִ밪 ����
			SetScroll(line);
	}
	else //���� �پ�� ��
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
			if (rpos > 35) //������ ����Ʈ �Է� ��
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
		if (li != 0) //LINEINDEX�� 0�̸� ù��° ���̶�� ��
		{
			SetSel(hWnd, SendMessage(hWnd, EM_LINEINDEX, nline - 1, 0) + (rpos / chwidth[0]) * chwidth[0]);
			inputOnce = 0;
		}
		break;
	case VK_DOWN:
		if (nline < LineCount(hWnd) - 1)
		{
			int dindex = SendMessage(hWnd, EM_LINEINDEX, nline + 1, 0) + (rpos / chwidth[0]) * chwidth[0]; //�ٷ� �Ʒ��� ������ �� ��ġ
			int lindex = SendMessage(hWnd, EM_LINELENGTH, dindex, 0); //�Ʒ� �� ������ ��ġ
			SetSel(hWnd, dindex > lindex ? dindex : lindex);
			inputOnce = 0;
		}
		break;
	case VK_BACK:
		getSel = SendMessage(hWnd, EM_GETSEL, 0, 0);
		if (pos == 0 && HIWORD(getSel) == 0) //�� ó�� ��ġ�̸鼭 ������ ��쵵 �ƴ� ��
			return;

		if (LOWORD(getSel) != HIWORD(getSel)) //���õǾ� �ִ� ���
		{
			RemoveSelections(hWnd);
			return;
		}

		autoLineFeed(0, hWnd, pos + inputOnce * 2); //inputOnce�� 1(����Ʈ �Է� ��)�� ��� pos�� 2�� ���� �ùٸ� ��ġ�� �����ϰ� ȣ��

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

		if (itemIndex < nbyte - 1) //������ �迭 �����
		{
			for (int i = itemIndex; i < nbyte - 1; i++)
				bytes[i] = bytes[i + 1];
		}

		nbyte--;

		if ((nbyte + 1) % 8 == 0) //�ٹ�ȣ�� �ٲ�� ���
			Numbering(0);

		return;
	default:
		return;
	}

	//Ű���� �Է��� ������ ��ũ�� ���¸� ����
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

	//Ű���� �Է����δ� wParam�� ������ ���޵��� �ʱ� ������ ���콺 ������ ��ư���� ���� �޴����� �����ϴ� ��쿡�� wParam�� ���� ������ �ؼ� ���ǹ��� ������Ŵ
	if (wParam == -5 || (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'A')) //��� ����
	{
		SendMessage(hWnd, EM_SETSEL, 0, -1);
		return 1;
	}
	else if ((wParam == -1 || wParam == -2) || (GetKeyState(VK_CONTROL) & 0x8000 && (wParam == 'C' || wParam == 'X'))) //����, �߶󳻱�
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
	else if (wParam == -3 || (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'V')) //�ٿ��ֱ�
	{
		if (clipBoardData.len != 0)
		{
			int len = clipBoardData.len;
			RemoveSelections(hWnd);

			pos = GetPos(hWnd);
			nline = SendMessage(hWnd, EM_LINEFROMCHAR, -1, 0);
			rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0);
			int idx = nline * 8 + rpos / chwidth[0];

			for (int i = nbyte - 1; i >= idx; i--) //������ �б�
				bytes[i + len] = bytes[i];

			memcpy(bytes + idx, clipBoardData.bytes, len);

			nbyte += len;

			pos = rpos == chwidth[0] * 8 ? pos + 2 : pos;
			SetSel(hWnd, pos);

			autoLineFeed(1, hWnd, pos);

			for (int i = idx; i < idx + len; i++) //���� �� ��ġ ���
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

	if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA)) //���� ���콺 ������ �ٸ� edit�� ���� ���� ����
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
	if (LineCount(GetFocus()) < 12) //��ũ�� �� �� ���� ���
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
	if (!isDrag) //���콺 ������ ���� �ʴ� ���
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

	lastLineYpos = HIWORD(SendMessage(hWnd, EM_POSFROMCHAR, SendMessage(hWnd, EM_LINEINDEX, LineCount(hWnd) - 1, 0), 0)); //������ �� y��ǥ
	rMousePos[0] = LOWORD(lParam) > 60000 ? 0 : min(r.right, LOWORD(lParam)); //���콺�� ȭ�� ������ �̵��� �� x�� ����
	rMousePos[1] = HIWORD(lParam) > 60000 ? 0 : min(lastLineYpos, HIWORD(lParam));//y�� ����

	newpos = LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(rMousePos[0], rMousePos[1]))); //���콺 ��ġ -> edit�� Ŀ�� ��ġ(���� ������ ������ ��ġ)
	li[0] = SendMessage(hWnd, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, newpos, 0), 0); //���� ���� ������ �� �ε���
	li[1] = SendMessage(hWnd, EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, oldpos, 0), 0); //���� ���� ù �� �ε���
	rpos[0] = (newpos - li[0]) / chwidth[0] * chwidth[0]; //���� ���� ������ ���� ó�� ���� ��ġ
	rpos[1] = (oldpos - li[1]) / chwidth[0] * chwidth[0];//���� ���� ù ���� ó�� ���� ��ġ

	//�� 5���� ������ �ٸ� edit�� ���� ������ ���� ���� ���� �� ����
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

	//�巡���ϸ鼭 edit �� �Ʒ��� ������ �ڵ� ��ũ��
	if (HIWORD(lParam) > 60000 && FirstVisibleLine(hWnd) > 0) //edit������ ���콺 ������ -1���� �����ؼ� �����÷ο� ������ WORD �ִ밪�� 65535���� ���� ������
	{
		scrollPos--;
		ScrollEdits(-1);
		SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
	}
	else if(HIWORD(lParam) < 5000 && HIWORD(SendMessage(hWnd, EM_POSFROMCHAR, SendMessage(hWnd, EM_LINEINDEX, FirstVisibleLine(hWnd) + 11, 0), 0)) < HIWORD(lParam) && FirstVisibleLine(hWnd) + 11 < LineCount(hWnd)) //edit �Ʒ��� ���콺�� ���� ���� ���콺 �����´�� ���� ������
	{
		scrollPos++;
		ScrollEdits(1);
		SetScrollPos(GetDlgItem(hDlgModify, IDC_D3_SCROLLBAR), SB_CTL, scrollPos, TRUE);
	}
	//HIWORD(lParam) < 5000�� �Ȱɸ� edit ���� ���콺�� ���� �� �����÷ο� �Ǳ� ������ ���ǹ��� ���� ��

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

	//nPage�� 11����, nMax�� 11���� ��ũ�� ������ �þ ������ 1�� ����
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = 11 + lineCount - 12;
	si.nPage = 11;
	si.nPos = FirstVisibleLine(GetFocus());
	scrollPos = si.nPos;
	SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING), EM_LINESCROLL, 0, scrollPos - FirstVisibleLine(GetDlgItem(hDlgModify, IDC_D3_VDATA_NUMBERING))); //������ �� edit�� �ڵ� ��ũ��

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