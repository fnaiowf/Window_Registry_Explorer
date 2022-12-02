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
		GetWindowText(hEdit, t_path, sizeof(t_path)); //잘못된 경로를 입력했을 경우 이전에 입력되어 있던 경로로 바꿈
		break;
	case WM_KEYUP:
		if (wParam == VK_RETURN)
		{
			GetWindowText(hEdit, temp, sizeof(temp));
			item = getItemfromPath(temp);
			if (item != 0)
				TreeView_SelectItem(hTV, item);
			else //올바른 경로가 아닌 경우
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
	if (iMessage == WM_KEYDOWN && LOWORD(wParam) == VK_ESCAPE) //multiline edit에서 esc 누르면 WM_CLOSE 메세지 전달되는 것을 막음
		return 0;

	return CallWindowProc(oldDlgEditProc[3], hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK BinaryEditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int pos, rpos, nline, pos2, prevpos[2], ascii, chwidth[2] = { 5, 2 }; //chwidth: 문자 폭, edit 종류마다 다름
	static TCHAR msg[10], msg2[10];
	static int oldpos;
	HWND hWnd2 = hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA) ? GetDlgItem(hDlgModify, IDC_D3_VDATA_ASCII) : GetDlgItem(hDlgModify, IDC_D3_VDATA);

	switch (iMessage)
	{
	case WM_CHAR:
		if (hWnd == GetDlgItem(hDlgModify, IDC_D3_VDATA)) //가운데 edit
		{
			if ((wParam >= '0' && wParam <= '9') || (wParam >= 'a' && wParam <= 'f') || (wParam >= 'A' && wParam <= 'F'))
			{
				if (wParam >= 'a' && wParam <= 'f')
					wParam -= 32;
			}
			else
				return 0;
		}
		else //오른쪽 edit
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
		rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0); //줄의 첫번째를 0으로 한 위치
		nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0); //현재 커서 줄
		pos2 = nline * (chwidth[1] * 8 + 2) + rpos / chwidth[0] * chwidth[1]; //ASCII edit의 커서 위치

		if (chwidth[0] == 2 || inputOnce == 0) //가운데 edit의 경우 바이트 입력 시작(첫번째 문자)
		{
			RemoveSelections(hWnd); //선택 영역이 있다면 지우고 시작

			if (rpos == chwidth[0] * 8) //한 줄의 마지막 위치에서 입력하면 다음 줄로 이동
			{
				SetSel(hWnd, pos + 2);
				SetSel(hWnd2, pos2 + 2);
				pos += 2;
				pos2 += 2;
				rpos = 0;
			}

			if (rpos % (chwidth[0] * 7) == 0 && rpos != 0) //8번째 문자의 경우 개행 문자 추가
			{
				if (chwidth[0] == 5) //가운데 edit
				{
					wsprintf(msg, L" %c0  \r\n", wParam);
					wsprintf(msg2, L"%c \r\n", wParam);
				}
				else //오른쪽 edit
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
				msg2[0] = isprint(ascii) ? ascii : '.'; // 입력 불가능한 문자는 .으로 대체
			}

			prevpos[0] = pos; //기존 위치를 저장해 둔 뒤 개행이 끝나면 기존 위치로 이동
			prevpos[1] = pos2;
			autoLineFeed(2, hWnd, pos);
			SetSel(hWnd, prevpos[0]);
			SetSel(hWnd2, prevpos[1]);

			SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)msg);
			SendMessage(hWnd2, EM_REPLACESEL, TRUE, (LPARAM)msg2);

			if (chwidth[0] == 5)
				SetSel(hWnd, pos + 3); //바이트 첫 문자를 입력하면 두 번째 문자 다음으로 커서를 이동

			//데이터 저장
			int nowindex = nline * 8 + rpos / chwidth[0];
			if (nowindex < nbyte) //중간에 데이터를 추가해야 하는 경우 한 칸씩 밀기
			{
				for (int i = nbyte - 1; i >= nowindex; i--)
					bytes[i + 1] = bytes[i];
			}
			bytes[nowindex] = chwidth[0] == 5 ? ascii : wcstol(msg2, NULL, 16);;

			nbyte++;

			if (nbyte % 8 == 0)
				Numbering(1);

			if (chwidth[0] == 5)
				inputOnce = 1; //다음 입력은 바이트의 두 번째 문자라는 것을 표시
		}
		else //바이트 두 번째 문자
		{
			wsprintf(msg + 2, L"%c", wParam); //msg에는 " XX  "로 저장되어 있어서 msg+2에 입력한 값을 넣으면 바이트 두 번째 문자를 입력한 것이 됨

			ascii = wcstol(msg + 1, NULL, 16);
			wsprintf(msg2, L"%c", isprint(ascii) && ascii != VK_TAB ? ascii : '.');

			ReplaceSel(hWnd, pos - 1, pos, msg + 2); //첫 문자를 입력할 때 0을 임시로 넣었기 때문에 이를 지우고 입력
			SetSel(hWnd, pos + 2);

			ReplaceSel(hWnd2, pos2, pos2 + 1, msg2);

			if ((rpos - 3) % 35 == 0 && rpos != 3) //줄의 마지막 문자인 경우 rpos는 38
				SetSel(hWnd, pos + 4);

			bytes[nline * 8 + rpos / 5] = ascii;
			inputOnce = 0;
		}

		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_TAB) //tab 처리
		{
			CallWindowProc(binaryOldEditProc[chwidth[0] == 5 ? 0 : 1], hWnd, iMessage, wParam, lParam);

			pos = GetPos(hWnd);
			rpos = pos - SendMessage(hWnd, EM_LINEINDEX, -1, 0); //줄의 첫번째를 0으로 한 위치
			nline = SendMessage(hWnd, EM_LINEFROMCHAR, pos, 0); //현재 커서 줄
			pos2 = nline * (chwidth[1] * 8 + 2) + rpos / chwidth[0] * chwidth[1]; //ASCII edit의 커서 위치

			SetSel(hWnd2, pos2);
		}
		else if (!shortCutHandler(wParam, hWnd)) //ctrl + a 같은 단축키를 먼저 처리
		{
			pos = GetPos(hWnd);
			KeyDownProcess(wParam, hWnd, pos);
		}

		return 0;
	case WM_SETFOCUS: //IDC_D3_VNAME에서 Tab으로 이동할 때 edit의 모든 내용이 선택되는데 이를 해결하기 위해 ASCII edit 커서 위치로 커서 위치 설정
		pos = GetPos(hWnd2);
		rpos = pos - SendMessage(hWnd2, EM_LINEINDEX, -1, 0);
		nline = SendMessage(hWnd2, EM_LINEFROMCHAR, pos, 0);
		pos2 = nline * (chwidth[0] * 8 + 2) + rpos / chwidth[1] * chwidth[0];
		SetSel(hWnd, pos2);

		break;
	case WM_CONTEXTMENU: //오른쪽 마우스 눌렀을 때 메뉴
		openBinaryEditorMenu(LOWORD(lParam), HIWORD(lParam));

		return 0;
	case WM_LBUTTONDOWN:
		LbuttonDownProcess(hWnd, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))));
		isDrag = 1;
		oldpos = GetPos(hWnd); //드래그 할 때 시작 위치 저장
		SetCapture(hWnd); //edit 밖으로 마우스 나가도 마우스 정보 갱신됨

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
	case WM_LBUTTONDOWN: //클릭한 줄 위치로 VDATA edit의 커서를 옮김
		index = SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_LINEINDEX, SendMessage(hWnd, EM_LINEFROMCHAR, LOWORD(SendMessage(hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(LOWORD(lParam), HIWORD(lParam)))), 0), 0);
		SendMessage(GetDlgItem(hDlgModify, IDC_D3_VDATA), EM_SETSEL, index, index);
		return 0;
	}

	return CallWindowProc(binaryOldEditProc[2], hWnd, iMessage, wParam, lParam);
}