/*
MULTI_SZ ����
���̳ʸ� ����
*/

#pragma once
#pragma warning(disable : 6387)
#pragma warning(disable : 4267)
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<conio.h>
#include<locale.h>
#include<commctrl.h>
#include<richedit.h>
#include"resource.h"

#ifdef UNICODE
#define LPNMLVDISPINFO          LPNMLVDISPINFOW
#else
#define LPNMLVDISPINFO          LPNMLVDISPINFOA
#endif

//ARRAY SIZE
#define MAX_PATH_LENGTH 3000
#define MAX_KEY_LENGTH 500
#define MAX_VALUE_LENGTH 5000
//WINDOW SIZE
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 800
#define GAP 3
#define MAX_WIDTH 1225
#define MIN_WIDTH 100
#define MAX_HEIGHT 600
#define MIN_HEIGHT 200
//Control ID
#define ID_TV 0
#define ID_LV 1
#define ID_EDIT 2
#define ID_resLV 3

#define CHECKBIT 8388608
#define ListView_DeSelectAll(handle) {LVITEM li; li.mask = LVIF_STATE; li.stateMask = LVIS_SELECTED; SendMessage(handle, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&li); }
enum SPLIT { SP_NONE, SP_VERT, SP_HORZ}; //â ���� ����
enum THREAD_TYPE{REFRESH, FIND, CHANGE, LOAD};

typedef struct DATA { //������ �Ű�����
	THREAD_TYPE t_type;;
	TCHAR path[MAX_PATH_LENGTH];
	TCHAR targetValue[100];
	TCHAR newValue[100];
	BYTE type : 4;
	BYTE base : 1;
};

typedef struct BYTE_DATA {
	BYTE* bytes;
	int size;
	int index;
};

typedef struct MULSZ_DATA {
	TCHAR** strings;
	int size;
	int nString;
	int index;
};

typedef struct LV_DATA_MANAGE {
	BYTE_DATA* byteData;
	MULSZ_DATA* mulstrData;
	int nByte;
	int nMul;
};

extern const HKEY BASIC_KEY_HANDLE[5];
extern const unsigned int REG_TYPE[6];
extern FILE* fp;
extern HWND hWndMain, hTV, hLV, hEdit, hStatic, hresultLV, hProgress, hDlgModify;
extern HINSTANCE g_hInst;
extern WNDPROC oldEditProc;
extern LV_DATA_MANAGE lvData;

extern int treeWidth, resultHeight, nchanged;
extern TCHAR path[MAX_PATH_LENGTH], * msg;
extern SPLIT nSplit;
extern DWORD gCount[2];

//RegistryControl.cpp
HKEY _RegOpenKeyEx(int bKeyIndex, TCHAR* path); //������Ʈ�� ���� �Լ� ����
int enumRegistry(DATA* data); //�⺻Ű enum
void enumKeys(HKEY hkey, HTREEITEM parent, TCHAR* subkeystr, DATA* data, int bkey); //�⺻Ű�� ����Ű enum
void enumValue(HKEY hkey, DATA* data); //�� enum
void changeValue(HKEY hkey, TCHAR* name, TCHAR* value, DATA* data, DWORD pos); //�� ����
void changeValue(int iItem, DATA *data); //�ϳ��� �ٲٱ�
void loadValue(TCHAR* path, HKEY basickey); //listview�� �� �ε�
void deleteAllSubkey(TCHAR* path, HTREEITEM item); //����Ű ��� ����
void deleteAllSubkey(HKEY hkey, HTREEITEM item); //����Ű ��� ����
void createValue(int type, HTREEITEM hitem); //�� �߰�
int _RegSetValueEx(HKEY hkey, TCHAR* name, int type, BYTE* value, int base); //SetVlaue ����

//Proc.cpp
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //���� ������ ���ν���
LRESULT CALLBACK MainEditSubProc(HWND, UINT, WPARAM, LPARAM); //���� ������ EDIT ����Ŭ����
BOOL CALLBACK FindDlgProc(HWND, UINT, WPARAM, LPARAM); //ã��&�ٲٱ� ���̾�α� ���ν���
LRESULT CALLBACK DlgEditSubProc(HWND, UINT, WPARAM, LPARAM); //���̾�α� EDIT ����Ŭ����
BOOL CALLBACK ModifySzNumDlgProc(HWND, UINT, WPARAM, LPARAM); //�� ���� ���̾�α� ���ν���
BOOL CALLBACK ModifyBinaryDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ModifyMultiSzDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK CompareFunc(LPARAM, LPARAM, LPARAM); //�˻� ��� ����Ʈ�� ���� �Լ�
DWORD WINAPI ThreadFunc(LPVOID); // ������ �Լ�

//Util.cpp
const TCHAR* getBasicKey(int idx); //�⺻Ű �̸� ����
int getBasicKey(TCHAR* path); //�⺻Ű ����
TCHAR* getValidPath(TCHAR* path); //��� -> Ű �ּ�
const TCHAR* getTypeName(int type); //������Ʈ�� Ÿ�� ���ڿ� ����
void openPopupMenu(HMENU menu, int x, int y); //������ ���콺 ��ư ���� �� �˾� �޴� ����
void processPopup(int id, int index, void* item); //�˾� �޴� ���ν���
int getType(TCHAR* type); //������Ʈ�� Ÿ�� ���ڿ� -> ���ǵ� ��
void byteToString(BYTE* bytes, int size, TCHAR* dest); //Byte -> String
int is_number(TCHAR* string, int base); //���ڿ��� �������� üũ
int splitMulSz(TCHAR* data, int size, TCHAR*** strings); //MULTI_SZ �� ó��
void concatMulSz(TCHAR* strings, int size, TCHAR* ret); //MULTI_SZ �� NULL���� �������� �ٲ�
void freeMemory(); //��� �Ҵ� �޸� ����

void initWindow(); //��Ʈ�� ����
SPLIT getSplitter(POINT pt); //â ���� ����

HTREEITEM addTVitem(const TCHAR* text, HTREEITEM parent, int basicKey); //Ʈ���� ������ �߰�
void getPathfromItem(HTREEITEM item, TCHAR* retpath); //Ʈ���� ������ -> ���
HTREEITEM getItemfromPath(const TCHAR* path); //��� -> Ʈ���� ������
void addLVitem(HWND hlv, TCHAR* name, TCHAR* type, TCHAR* value, int index, TCHAR* opt, LPARAM lParam); //����Ʈ�� ������ �߰�
LVITEM getListViewItem(HWND handle, UINT mask, UINT index); //����Ʈ�� ������ �Ӽ� �� ����
void setMarquee(int opt); //���α׷����� Marquee ����