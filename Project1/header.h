#pragma once
#pragma warning(disable : 4996)
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<conio.h>
#include<locale.h>
#include<commctrl.h>
#include<time.h>
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
#define MAX_TREE_WIDTH 1225
#define MIN_TREE_WIDTH 100
#define MAX_RTREE_HEIGHT 600 //��� �� Ʈ�� �ִ�, �ּ� ����
#define MIN_RTREE_HEIGHT 200
//Control ID
#define ID_TV 0
#define ID_LV 1
#define ID_EDIT 2
#define ID_resLV 3

#define DEFAULT_VALUE_PARAM -1 //Ű�� �⺻���� ��� �Ķ���� ��
#define PREV_NEW_VALUE_PARAM -2 //�� �߰� �� �� ������ ������Ʈ�� �� �����ϱ� �� �ӽ÷� ����Ʈ�� �Ķ���Ϳ� �ִ� ��(�� �� �߰� �� ���� BEGINLABELEDIT�� �����ϰ� �ϱ� ����)

#define ListView_DeSelectAll(handle) {LVITEM li; li.mask = LVIF_STATE; li.stateMask = LVIS_SELECTED; SendMessage(handle, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&li); }
enum SPLIT { SP_NONE, SP_VERT, SP_HORZ}; //â ���� ����
enum THREAD_TYPE{REFRESH, FIND, CHANGE, LOAD, DATA_LOAD}; //DATA_LOAD : ���� ����Ʈ�� �߰� �Ǿ� �ִ� �͵� �ǵ帮�� �ʰ� �����͸� ������ ��
enum FUNCSTATE{DEFAULT, ING, SUSPEND};
enum FIND_TYPE {NONE, KEY, VALUE, DATA};

const unsigned int REG_TYPE[7] = { REG_DWORD, REG_QWORD, REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ, REG_BINARY, REG_NONE};
const HKEY BASIC_KEY_HANDLE[5] = { HKEY_CLASSES_ROOT, HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG };

typedef struct THREAD_DATA { //������ �Ű�����
	TCHAR path[MAX_PATH_LENGTH];
	TCHAR targetValue[100];
	TCHAR newValue[100];
	THREAD_TYPE threadType; //��Ʈ �ʵ�
	FIND_TYPE findType;
	BYTE type : 4; 
	BYTE base : 1;
};

typedef struct BYTE_DATA { //Ű ������ �� REG_BINARYŸ���� ������ ���� ����
	int index;
	BYTE* bytes;
	TCHAR name[MAX_KEY_LENGTH];
	int size;
};

typedef struct MULSZ_DATA {//Ű ������ �� REG_MULTI_SZ Ÿ���� ������ ���� ����
	int index;
	TCHAR** strings;
	TCHAR name[MAX_KEY_LENGTH];
	int size;
	int nString;
};

typedef struct LV_DATA_MANAGE {
	BYTE_DATA* byteData;
	MULSZ_DATA* mulstrData;
	int nByte;
	int nMul;
};

extern HWND hWndMain, hTV, hLV, hEdit, hStatic, hresultLV, hProgress, hDlgFind, hDlgModify;
extern HINSTANCE g_hInst;
extern WNDPROC oldEditProc, oldDlgEditProc[3];
extern LV_DATA_MANAGE lvData;

extern int treeWidth, resultHeight, isDataLoad, funcState;
extern TCHAR path[MAX_PATH_LENGTH], * msg, temp[MAX_PATH_LENGTH];
extern SPLIT nSplit;
extern FIND_TYPE nowFindType;

//RegistryControl.cpp
HKEY _RegOpenKeyEx(int bKeyIndex, TCHAR* path); //������Ʈ�� ���� �Լ� ����
int _RegSetValueEx(HKEY hkey, TCHAR* name, int type, BYTE* value, int size, int base, int ismodify); //SetValue ����
void enumRegistry(THREAD_DATA* data); //�⺻Ű enum
void enumKeys(HKEY hkey, HTREEITEM parent, TCHAR* subkeystr, THREAD_DATA* data, int bkey); //�⺻Ű�� ����Ű enum
void enumValue(HKEY hkey, THREAD_DATA* data); //�� enum
int changeValue(HKEY hkey, TCHAR* name, TCHAR* value, THREAD_DATA data, DWORD pos); //�� ����
int changeValue(int iItem, THREAD_DATA *data); //�ϳ��� �ٲٱ�
void loadValue(TCHAR* path, HKEY basickey, int isDataLoad); //listview�� �� �ε�
void deleteAllSubkey(TCHAR* path, HTREEITEM item); //����Ű ��� ����
void deleteAllSubkey(HKEY hkey, HTREEITEM item); //����Ű ��� ����
void createValue(int type, HTREEITEM hitem); //�� �߰�

//WndProc.cpp
DWORD WINAPI ThreadFunc(LPVOID); // ������ �Լ�
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //���� ������ ���ν���

//FindProc.cpp
BOOL CALLBACK FindDlgProc(HWND, UINT, WPARAM, LPARAM); //ã��&�ٲٱ� ���̾�α� ���ν���

//ModifyProc.cpp
BOOL CALLBACK ModifySzNumDlgProc(HWND, UINT, WPARAM, LPARAM); //�� ���� ���̾�α� ���ν���
BOOL CALLBACK ModifyMultiSzDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ModifyBinaryDlgProc(HWND, UINT, WPARAM, LPARAM);

//SubProc.cpp
LRESULT CALLBACK MainEditSubProc(HWND, UINT, WPARAM, LPARAM); //���� ������ EDIT ����Ŭ����
LRESULT CALLBACK DlgEditSubProc(HWND, UINT, WPARAM, LPARAM); //���̾�α� EDIT ����Ŭ����
LRESULT CALLBACK MultiSzEditSubProc(HWND, UINT, WPARAM, LPARAM); //MULTI_SZ EDIT ����Ŭ����

//Util.cpp
const TCHAR* getBasicKey(int idx); //�⺻Ű �̸� ����
TCHAR* getValidPath(TCHAR* path); //��� -> Ű �ּ�
int getBasicKey(TCHAR* path); //�⺻Ű ����
const TCHAR* getTypeName(int type); //������Ʈ�� Ÿ�� ���ڿ� ����
int getType(TCHAR* type); //������Ʈ�� Ÿ�� ���ڿ� -> ���ǵ� ��
void byteToString(BYTE* bytes, int size, TCHAR* dest); //Byte -> String
int splitMulSz(TCHAR* data, int size, TCHAR*** strings, int alloc); //MULTI_SZ �� ó��
void concatMulSz(TCHAR* strings, int len, TCHAR* ret); //MULTI_SZ �� NULL���� �������� �ٲ�
int is_number(TCHAR* string, int base); //���ڿ��� �������� üũ
int checkStringOverflow(TCHAR* string, int base, int type); //check string overflow / underflow
void getTime(TCHAR* ret);
void freeMemory(); //��� �Ҵ� �޸� ����

//WinControlUtil.cpp
int CALLBACK LVCompareFunc(LPARAM, LPARAM, LPARAM); //������ ����Ʈ�� �������� ����
int CALLBACK resultLVCompareFunc(LPARAM, LPARAM, LPARAM); //�˻� ��� ����Ʈ�� �׸� ID �� ����
void initWindow(); //��Ʈ�� ����
SPLIT getSplitter(POINT pt); //â ���� ����
HTREEITEM addTVitem(const TCHAR* text, HTREEITEM parent, int basicKey); //Ʈ���� ������ �߰�
void addLVitem(HWND hlv, TCHAR* name, TCHAR* type, TCHAR* value, int index, TCHAR* path, LPARAM lParam); //����Ʈ�� ������ �߰�
LVITEM getListViewItem(HWND handle, UINT mask, UINT index); //����Ʈ�� ������ �Ӽ� �� ����
void getPathfromItem(HTREEITEM item, TCHAR* retpath); //Ʈ���� ������ -> ���
HTREEITEM getItemfromPath(const TCHAR* path); //��� -> Ʈ���� ������
void setMarquee(int opt); //���α׷����� Marquee ����
void openPopupMenu(int x, int y); //������ ���콺 ��ư ���� �� �˾� �޴� ����
void processPopup(int id, int index, void* item); //�˾� �޴� ���ν���
void AcceleratorProcess(HWND hWnd, int id);
void openModifyDlg(int type);