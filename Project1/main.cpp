#include"header.h"

LPCTSTR lpszClass = TEXT("RegistryChange");
HINSTANCE g_hInst;
TCHAR* msg, temp[MAX_PATH_LENGTH];

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow)
{
	setlocale(LC_ALL, "");
	msg = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH_LENGTH);

	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	HACCEL hAccel;

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

	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	while (GetMessage(&Message, NULL, NULL, NULL))
	{
		if (!IsDialogMessage(hDlgFind, &Message))
		{
			if (!TranslateAccelerator(hWnd, hAccel, &Message))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message); //메세지를 프로시저로 보냄
			}
		}
	}

	return (int)(Message.wParam);
}

