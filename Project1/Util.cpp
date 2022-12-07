#include"header.h"

TCHAR* getValidPath(TCHAR* path)
{
	TCHAR* pos = path + 4;

	if (*(pos - 1) == L'\0') //컴퓨터 일 경우
		return NULL;

	while (*pos != L'\\')
	{
		pos++;
		if (*pos == L'\0') //컴퓨터\HKEY_~ 일 경우
			return NULL;
	}

	return pos + 1;
}

const TCHAR* getBasicKey(int idx)
{
	switch (idx)
	{
	case 0:
		return TEXT("HKEY_CLASSES_ROOT");
	case 1:
		return TEXT("HKEY_LOCAL_MACHINE");
	case 2:
		return TEXT("HKEY_CURRENT_USER");
	case 3:
		return TEXT("HKEY_USERS");
	case 4:
		return TEXT("HKEY_CURRENT_CONFIG");
	default:
		return NULL;
	}
}

int getBasicKey(TCHAR* path)
{
	TCHAR temp[MAX_PATH_LENGTH], *pos;

	memcpy(temp, path, sizeof(temp));
	pos = getValidPath(temp);

	if (pos == NULL)
		pos = temp + 4;

	*(pos-1) = NULL; //getVlidPath가 NULL이 아닐 경우 pos를 출력하면 HKEY~\경로 에서 경로 부분이므로 여기서 \를 NULL로 바꿔 wcscmp를 사용할 수 있게 함

	if (wcscmp(temp + 4, L"HKEY_CLASSES_ROOT") == 0)
		return 0;
	else if (wcscmp(temp + 4, L"HKEY_LOCAL_MACHINE") == 0)
		return 1;
	else if (wcscmp(temp + 4, L"HKEY_CURRENT_USER") == 0)
		return 2;
	else if (wcscmp(temp + 4, L"HKEY_USERS") == 0)
		return 3;
	else if (wcscmp(temp + 4, L"HKEY_CURRENT_CONFIG") == 0)
		return 4;
	else
		return -1;
}

int getType(TCHAR* type)
{
	if (type == NULL)
		return  -1;

	if (wcscmp(type, L"REG_DWORD") == 0)
		return 0;
	else if (wcscmp(type, L"REG_QWORD") == 0)
		return 1;
	else if (wcscmp(type, L"REG_SZ") == 0)
		return 2;
	else if (wcscmp(type, L"REG_EXPAND_SZ") == 0)
		return 3;
	else if (wcscmp(type, L"REG_MULTI_SZ") == 0)
		return 4;
	else if (wcscmp(type, L"REG_BINARY") == 0)
		return 5;

	else return -1;
}

const TCHAR* getTypeName(int type)
{
	switch (type)
	{
	case REG_SZ:
		return L"REG_SZ";
	case REG_DWORD:
		return L"REG_DWORD";
	case REG_BINARY:
		return L"REG_BINARY";
	case REG_EXPAND_SZ:
		return L"REG_EXPAND_SZ";
	case REG_MULTI_SZ:
		return L"REG_MULTI_SZ";
	case REG_NONE:
		return L"REG_NONE";
	case REG_LINK:
		return L"REG_LINK";
	case REG_QWORD:
		return L"REG_QWORD";
	default:
		wprintf(L"%d", type);
		return L"";
	}
}

void byteToString(BYTE* bytes, int size, TCHAR* dest)
{
	wsprintf(dest, L"%02X", bytes[0]);
	for (int i = 1; i < size; ++i)
	{
		TCHAR hex[4];
		wsprintf(hex, L" %02X", bytes[i]);
		wcscat(dest, hex);
	}
}

int splitMulSz(TCHAR* data, int size, TCHAR*** strings, int alloc) //alloc이 0이면 메모리 할당 필요 없다는 뜻
{
	int t = 0, count = 0, len;
	TCHAR* adr = data;

	while (1)
	{
		len = wcslen(adr);

		if(alloc)
			(*strings)[count] = (TCHAR*)malloc(sizeof(TCHAR) * (len + 1));

		wsprintf((*strings)[count], adr);

		t += len + 1;
		adr += len + 1;
		count++;

		if (t >= size / sizeof(TCHAR) - 1)
			break;

		if(alloc)
			*strings = (TCHAR**)realloc(*strings, sizeof(TCHAR*) * (count + 1));
	}

	return count;
}

void concatMulSz(TCHAR* strings, int len, TCHAR* ret)
{
	memset(ret, 0, (len + 1) * 2);
	for (int i = 0; i < len; i++)
	{
		if (strings[i] == 0) ret[i] = ' ';
		else ret[i] = strings[i];
	}
}

int is_number(TCHAR* string, int base)
{
	if (*string == 0) return 0;

	TCHAR* pos = string;

	while (*pos != NULL)
	{
		if (base) //10진수
		{
			if (!(*pos <= '9' && *pos >= '0'))
				return 0;
		}
		else //16진수
		{
			if (!(*pos <= '9' && *pos >= '0') && !(*pos <= 'F' && *pos >= 'A') && !(*pos <= 'f' && *pos >= 'a'))
				return 0;
		}

		pos++;
	}

	return 1;
}

int checkStringOverflow(TCHAR* string, int base, int type)
{
	int len = wcslen(string);

	if (base) //10진수
	{
		if (type == 0) //DWORD
		{
			if (len < 10)
				return 0;
			else if (len == 10 && wcscmp(string, L"2147483647") <= 0) //10진수 int 최대값 길이 10
				return 0;
			else
				return 1;
		}
		else //QWORD
		{
			if (len < 19)
				return 0;
			else if (len == 19 && wcscmp(string, L"9223372036854775807") <= 0) //10진수 llong 최대값 길이 19
				return 0;
			else
				return 1;
		}
	}
	else //16진수
	{
		if (type == 0) //DWORD
		{
			if (len < 8)
				return 0;
			else if (len == 8 && wcscmp(string, L"ffffffff") <= 0) //16진수 int 최대값 길이 8
				return 0;
			else
				return 1;
		}
		else //QWORD
		{
			if (len < 16)
				return 0;
			else if (len == 16 && wcscmp(string, L"ffffffffffffffff") <= 0) //16진수 llong 최대값 길이 16
				return 0;
			else
				return 1;
		}
	}
}

void freeMemory()
{
	if (lvData.byteData != NULL)
	{
		for (int i = 0; i < lvData.nByte; i++)
			free(lvData.byteData[i].bytes);
		free(lvData.byteData);
	}

	if (lvData.mulstrData != NULL)
	{
		for (int i = 0; i < lvData.nMul; i++)
		{
			for (int j = 0; j < lvData.mulstrData[i].nString; j++)
				free((lvData.mulstrData[i]).strings[j]);

			free((lvData.mulstrData[i]).strings);
		}

		free(lvData.mulstrData);
	}
}

void getTime(TCHAR* ret)
{
	time_t timer;
	timer = time(NULL);
	struct tm* t = localtime(&timer);

	wsprintf(ret, L"[%d/%d/%d %d:%d:%d]", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}