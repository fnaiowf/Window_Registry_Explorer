/*
MULTI_SZ 수정
바이너리 수정
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
enum SPLIT { SP_NONE, SP_VERT, SP_HORZ}; //창 분할 정보
enum THREAD_TYPE{REFRESH, FIND, CHANGE, LOAD};

typedef struct DATA { //쓰레드 매개변수
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
HKEY _RegOpenKeyEx(int bKeyIndex, TCHAR* path); //레지스트리 오픈 함수 래퍼
int enumRegistry(DATA* data); //기본키 enum
void enumKeys(HKEY hkey, HTREEITEM parent, TCHAR* subkeystr, DATA* data, int bkey); //기본키의 서브키 enum
void enumValue(HKEY hkey, DATA* data); //값 enum
void changeValue(HKEY hkey, TCHAR* name, TCHAR* value, DATA* data, DWORD pos); //값 변경
void changeValue(int iItem, DATA *data); //하나씩 바꾸기
void loadValue(TCHAR* path, HKEY basickey); //listview에 값 로드
void deleteAllSubkey(TCHAR* path, HTREEITEM item); //서브키 모두 삭제
void deleteAllSubkey(HKEY hkey, HTREEITEM item); //서브키 모두 삭제
void createValue(int type, HTREEITEM hitem); //값 추가
int _RegSetValueEx(HKEY hkey, TCHAR* name, int type, BYTE* value, int base); //SetVlaue 래퍼

//Proc.cpp
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //메인 윈도우 프로시저
LRESULT CALLBACK MainEditSubProc(HWND, UINT, WPARAM, LPARAM); //메인 윈도우 EDIT 서브클래싱
BOOL CALLBACK FindDlgProc(HWND, UINT, WPARAM, LPARAM); //찾기&바꾸기 다이얼로그 프로시저
LRESULT CALLBACK DlgEditSubProc(HWND, UINT, WPARAM, LPARAM); //다이얼로그 EDIT 서브클래싱
BOOL CALLBACK ModifySzNumDlgProc(HWND, UINT, WPARAM, LPARAM); //값 수정 다이얼로그 프로시저
BOOL CALLBACK ModifyBinaryDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ModifyMultiSzDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK CompareFunc(LPARAM, LPARAM, LPARAM); //검색 결과 리스트뷰 정렬 함수
DWORD WINAPI ThreadFunc(LPVOID); // 쓰레드 함수

//Util.cpp
const TCHAR* getBasicKey(int idx); //기본키 이름 리턴
int getBasicKey(TCHAR* path); //기본키 리턴
TCHAR* getValidPath(TCHAR* path); //경로 -> 키 주소
const TCHAR* getTypeName(int type); //레지스트리 타입 문자열 리턴
void openPopupMenu(HMENU menu, int x, int y); //오른쪽 마우스 버튼 누를 때 팝업 메뉴 열기
void processPopup(int id, int index, void* item); //팝업 메뉴 프로시저
int getType(TCHAR* type); //레지스트리 타입 문자열 -> 정의된 값
void byteToString(BYTE* bytes, int size, TCHAR* dest); //Byte -> String
int is_number(TCHAR* string, int base); //문자열이 숫자인지 체크
int splitMulSz(TCHAR* data, int size, TCHAR*** strings); //MULTI_SZ 값 처리
void concatMulSz(TCHAR* strings, int size, TCHAR* ret); //MULTI_SZ 값 NULL문자 공백으로 바꿈
void freeMemory(); //모든 할당 메모리 해제

void initWindow(); //컨트롤 생성
SPLIT getSplitter(POINT pt); //창 분할 정보

HTREEITEM addTVitem(const TCHAR* text, HTREEITEM parent, int basicKey); //트리뷰 아이템 추가
void getPathfromItem(HTREEITEM item, TCHAR* retpath); //트리뷰 아이템 -> 경로
HTREEITEM getItemfromPath(const TCHAR* path); //경로 -> 트리뷰 아이템
void addLVitem(HWND hlv, TCHAR* name, TCHAR* type, TCHAR* value, int index, TCHAR* opt, LPARAM lParam); //리스트뷰 아이템 추가
LVITEM getListViewItem(HWND handle, UINT mask, UINT index); //리스트뷰 아이템 속성 값 리턴
void setMarquee(int opt); //프로그레스바 Marquee 설정