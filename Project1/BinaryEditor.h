#pragma once

#define GetPos(handle) LOWORD(SendMessage(handle, EM_GETSEL, NULL, NULL))
#define ReplaceSel(handle, start, end, msg) {SendMessage(handle, EM_SETSEL, (start), (end));SendMessage(handle, EM_REPLACESEL, TRUE, (LPARAM)(msg));}
#define SetSel(handle, index) SendMessage(handle, EM_SETSEL, (index), (index))
#define LineCount(handle) SendMessage(handle, EM_GETLINECOUNT, 0, 0)
#define FirstVisibleLine(handle) SendMessage(handle, EM_GETFIRSTVISIBLELINE, 0, 0)

typedef struct CLIPBOARD_DATA { //Ŭ������ ������
	int len;
	BYTE* bytes;
};

extern WNDPROC binaryOldEditProc[3];
extern int nbyte, inputOnce, isDrag;
extern BYTE bytes[5000];

//SubProc.cpp
LRESULT CALLBACK BinaryEditSubProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BinaryNumberingEditSubProc(HWND, UINT, WPARAM, LPARAM);

//BinaryEditorUtil.cpp
void autoLineFeed(int opt, HWND hWnd, int pos); //�Է��ϰų� ���ڸ� ���� �� edit�� �ؽ�Ʈ�� �ٽ� ����� �ڵ����� ����
void Numbering(int increase); //�� ��ȣ ����, ��ũ�� ���� ����
void RemoveSelections(HWND hWnd); //���� ���� ����

void KeyDownProcess(int vkey, HWND hWnd, int pos);
int shortCutHandler(int wParam, HWND hWnd); //����Ű ó��
void LbuttonDownProcess(HWND hWnd, int pos);
void MouseWheelProcess(int param);
void MouseMoveProcess(HWND hWnd, int lParam, int oldpos);

void ScrollProcess(int lParam, int wParam); //��ũ�� �ٸ� ���� ��ũ�� �ϴ� ��� ó��
void ScrollEdits(int nscroll); //�� edit�� ���� ��ũ��
void SetScroll(int lineCount); //��ũ�ѹ� ���� ����

void openBinaryEditorMenu(int x, int y);