#pragma once

#define GetPos(handle) LOWORD(SendMessage(handle, EM_GETSEL, NULL, NULL))
#define ReplaceSel(handle, start, end, msg) {SendMessage(handle, EM_SETSEL, (start), (end));SendMessage(handle, EM_REPLACESEL, TRUE, (LPARAM)(msg));}
#define SetSel(handle, index) SendMessage(handle, EM_SETSEL, (index), (index))

extern WNDPROC binaryOldEditProc[3];
extern int nbyte, inputOnce;
extern BYTE bytes[5000];

//SubProc.cpp
LRESULT CALLBACK BinaryEditSubProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BinaryAsciiEditSubProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BinaryNumberingEditSubProc(HWND, UINT, WPARAM, LPARAM);

//BinaryEditorUtil.cpp
void autoLineFeed(int opt, HWND hWnd, int pos);
void Numbering(HWND hWnd);
void KeyDownProcess(int vkey, HWND hWnd, int pos);
void LbuttonDownProcess(HWND hWnd, int pos);
void MouseWheelProcess(int param);
void ScrollEdits(int lParam, int wParam);