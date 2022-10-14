#include"header.h"

LPCTSTR lpszClass = TEXT("RegistryChange");
HINSTANCE g_hInst;
HMODULE hMod;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow)
{
	hMod = LoadLibrary(L"riched20.dll");

	setlocale(LC_ALL, "");
	fp = fopen("Log.txt", "w");
	if (fp == NULL)
	{
		printf("File Pointer Error");
		return 0;
	}
	msg = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH_LENGTH);
	if (msg == NULL)
	{
		fwprintf(fp, TEXT("In main : variable \"msg\" memory alloction failed\n"));
		fclose(fp);
		return 0;
	}

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
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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

