#include"header.h"

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