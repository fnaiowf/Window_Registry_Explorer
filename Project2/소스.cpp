#pragma warning(disable:4996)
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include<stdio.h>
#include<windows.h>
#include<conio.h>
#include<locale.h>
#include<richedit.h>
#include<stdlib.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SubProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE g_hInst;
HMODULE hMod;
TCHAR lpszClass[100] = L"Window";
HWND hWndRich, hWndMain;
WNDPROC oldProc;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow)
{
	hMod = LoadLibrary(L"riched20.dll");
	setlocale(LC_ALL, "");

	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 50, 50, 1325, 750, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, NULL, NULL))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message); //메세지를 프로시저로 보냄
	}

	FreeModule(hMod);
	return (int)(Message.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	PARAFORMAT pf;
	CHARFORMAT2 cf;
	HFONT hFont;
	TCHAR msg[100];
	POINT pt;
	HDC hdc;
	RECT rt;
	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		hWndRich = CreateWindowEx(0, RICHEDIT_CLASS, L"", WS_BORDER | WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL, 300, 200, 580, 190, hWnd, 0, g_hInst, NULL);
		oldProc = (WNDPROC)SetWindowLongPtr(hWndRich, GWLP_WNDPROC, (LONG_PTR)SubProc);
		SetFocus(hWndRich);

		memset(&pf, 0, sizeof(PARAFORMAT));
		pf.cbSize = sizeof(PARAFORMAT);
		pf.dwMask = PFM_RIGHTINDENT | PFM_OFFSETINDENT;
		pf.dxRightIndent = 2100;
		pf.dxStartIndent = 1300;
		SendMessage(hWndRich, EM_SETPARAFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&pf);

		memset(&cf, 0, sizeof(CHARFORMAT2));
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_SIZE;
		cf.yHeight = 175;
		SendMessage(hWndRich, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);

		SendMessage(hWndRich, EM_SETEVENTMASK, 0, ENM_CHANGE);

		break;
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case EN_CHANGE:
			//DefWindowProc(hWnd, iMessage, wParam, lParam);
			hdc = GetDC(hWndRich);
			int fvl = SendMessage(hWndRich, EM_GETFIRSTVISIBLELINE, 0, 0);
			int lc = SendMessage(hWndRich, EM_GETLINECOUNT, 0, (LPARAM)&rt);
			printf("%d %d\n", fvl, lc);
			for (int i = fvl; i < lc + fvl; i++)
			{
				int t = SendMessage(hWndRich, EM_LINEINDEX, i, 0);
				int k = SendMessage(hWndRich, EM_POSFROMCHAR, t, 0);

				wsprintf(msg, L"%08x", 8 * i);
				RECT rt = { 0, HIWORD(k), 100, HIWORD(k) + 20 };
				printf("\t%d %d %d\n", i, t, k);
				TextOut(hdc, 0, HIWORD(k), msg, 8);
				//DrawText(hdc, msg, 8, &rt, DT_LEFT)
			}
			ReleaseDC(hWndRich, hdc);
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK SubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	//printf("In Sub : %x\n", iMessage);
	RECT rt;
	HDC hdc;
	PAINTSTRUCT ps;
	POINT pt;
	int t;
	TCHAR msg[100];

	switch (iMessage)
	{
	/*case WM_LBUTTONDOWN:
		CallWindowProc(oldProc, hWnd, iMessage, wParam, lParam);
		t = LOWORD(SendMessage(hWnd, EM_GETSEL, 0, 0));
		printf("%d\n", t);
		
		SendMessage(hWnd, EM_SETSEL, 11*(t/11), 11*(t/11));
		return 0;
	case WM_KEYDOWN:
		CallWindowProc(oldProc, hWnd, iMessage, wParam, lParam);
		switch (wParam)
		{
		case VK_LEFT:
			t = LOWORD(SendMessage(hWnd, EM_GETSEL, 0, 0));
			SendMessage(hWnd, EM_SETSEL, max((t / 11 - 1) * 11, 0), max((t / 11 - 1) * 11, 0));
			break;
		case VK_RIGHT:
			t = LOWORD(SendMessage(hWnd, EM_GETSEL, 0, 0));
			SendMessage(hWnd, EM_SETSEL, min((t / 11 + 1) * 11, 100), min((t / 11 + 1) * 11, 100));
			break;
		}
		return 0;*/
	case WM_PAINT:
	{
		hdc = GetDC(hWndRich);
		/*int fvl = SendMessage(hWndRich, EM_GETFIRSTVISIBLELINE, 0, 0);
		int lc = SendMessage(hWndRich, EM_GETLINECOUNT, 0, (LPARAM)&rt);
		for (int i = fvl; i < lc + fvl; i++)
		{
			int t = SendMessage(hWndRich, EM_LINEINDEX, i, 0);
			int k = SendMessage(hWndRich, EM_POSFROMCHAR, t, 0);

			wsprintf(msg, L"%08x", 8 * i);
			printf("%ws\n", msg);
			RECT rt = { 0, HIWORD(k), 100, HIWORD(k) + 20 };
			DrawText(hdc, msg, 8, &rt, DT_LEFT);
		}*/
		//DrawText(hdc, L"AFAWFF", 8, &rt, DT_LEFT);
		ReleaseDC(hWndRich, hdc);
	}
	break;
	case WM_CHAR:
		break;
	}
	
	return CallWindowProc(oldProc, hWnd, iMessage, wParam, lParam);
}