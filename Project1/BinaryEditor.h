#pragma once

#define GetPos(handle) LOWORD(SendMessage(handle, EM_GETSEL, NULL, NULL))
#define ReplaceSel(handle, start, end, msg) {SendMessage(handle, EM_SETSEL, (start), (end));SendMessage(handle, EM_REPLACESEL, TRUE, (LPARAM)(msg));}
#define SetSel(handle, index) SendMessage(handle, EM_SETSEL, (index), (index))
#define LineCount(handle) SendMessage(handle, EM_GETLINECOUNT, 0, 0)
#define FirstVisibleLine(handle) SendMessage(handle, EM_GETFIRSTVISIBLELINE, 0, 0)

typedef struct CLIPBOARD_DATA { //클립보드 데이터
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
void autoLineFeed(int opt, HWND hWnd, int pos); //입력하거나 문자를 지울 때 edit의 텍스트를 다시 만들어 자동으로 개행
void Numbering(int increase); //줄 번호 갱신, 스크롤 상태 수정
void RemoveSelections(HWND hWnd); //선택 영역 지움

void KeyDownProcess(int vkey, HWND hWnd, int pos);
int shortCutHandler(int wParam, HWND hWnd); //단축키 처리
void LbuttonDownProcess(HWND hWnd, int pos);
void MouseWheelProcess(int param);
void MouseMoveProcess(HWND hWnd, int lParam, int oldpos);

void ScrollProcess(int lParam, int wParam); //스크롤 바를 통해 스크롤 하는 경우 처리
void ScrollEdits(int nscroll); //세 edit을 같이 스크롤
void SetScroll(int lineCount); //스크롤바 정보 수정

void openBinaryEditorMenu(int x, int y);