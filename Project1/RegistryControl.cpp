#include"header.h"

LONG kcount, vcount, fcount, ccount;
FILE* fp;
TCHAR path[MAX_PATH_LENGTH] = TEXT(""), * msg;
const HKEY BASIC_KEY_HANDLE[5] = { HKEY_CLASSES_ROOT, HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG };
const unsigned int REG_TYPE[6] = { REG_DWORD, REG_QWORD, REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ, REG_BINARY };
LV_DATA_MANAGE lvData;

HKEY _RegOpenKeyEx(int bKeyIndex, TCHAR* path)
{
	HKEY hkey;

	if (*(path + 3) == L'\0')
		return NULL;

	if (getValidPath(path) == NULL)
		return BASIC_KEY_HANDLE[bKeyIndex];

	if (RegOpenKeyEx(BASIC_KEY_HANDLE[bKeyIndex], getValidPath(path), 0, KEY_READ | KEY_WRITE, &hkey) == ERROR_SUCCESS)
		return hkey;
	else
		return NULL;
}

int enumRegistry(DATA* data)
{
	HKEY hkey;
	TCHAR* key = (TCHAR*)malloc(sizeof(TCHAR) * MAX_KEY_LENGTH);
	if (key == NULL)
	{
		//fwprintf(fp, TEXT("In main : variable \"key1\" memory alloction failed.\n"));
		fclose(fp);
		return -1;
	}
	DWORD i = 0, len = MAX_KEY_LENGTH;
	HTREEITEM root = 0, item = 0;
	DATA* dt;
	kcount = vcount = fcount = ccount =  0;

	if (data == NULL) dt = NULL;
	else if (data->t_type == REFRESH) dt = NULL;
	else dt = data;

	if(dt == NULL)
		root = addTVitem(TEXT("컴퓨터"), 0, 0);

	for (int k = 3; k < 5; k++) //BASIC KEY
	{
		wsprintf(path, TEXT("컴퓨터\\%ws"), getBasicKey(k));
		if(dt == NULL)
			item = addTVitem(path+4, root, k);
		kcount++;
		
		if(dt == NULL)
			wsprintf(msg, L"%d key  %d value loading...", kcount, vcount);
		else
			wsprintf(msg, L"%d finding...", fcount);
		SetWindowText(hStatic, msg);
		
		len = MAX_KEY_LENGTH;;
		i = 0;
		while (RegEnumKeyEx(BASIC_KEY_HANDLE[k], i, key, &len, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS)
		{
			len = MAX_KEY_LENGTH;
			wsprintf(path, TEXT("%ws\\%ws"), path, key);
			if (RegOpenKeyEx(BASIC_KEY_HANDLE[k], key, 0, KEY_READ | KEY_WRITE, &hkey) == ERROR_SUCCESS)
			{
				kcount++;
				
				if (dt == NULL)
				{
					if (kcount % 100 == 0)
					{
						wsprintf(msg, L"%d key loading...", kcount);
						SetWindowText(hStatic, msg);
					}
					enumKeys(hkey, addTVitem(key, item, k), key, data, k);
				}
				else
					enumKeys(hkey, NULL, key, data, k);
					
				RegCloseKey(hkey);
			}
			else //접근 권한 X
			{
				wsprintf(msg, TEXT("%ws open failed\n"), path);
				//fwprintf(fp, msg);

				for (int j = lstrlen(path) - 1;; j--)
				{
					path[j] = '\0';
					if (path[j - 1] == '\\')
					{
						path[j - 1] = '\0';
						break;
					}
				}
			}
			i++;
		}
	}

	free(key);
	TreeView_Expand(hTV, TreeView_GetRoot(hTV), TVE_EXPAND);
	if(dt == NULL)
		wsprintf(msg, L"%d key loaded", kcount);
	else if(dt->t_type == CHANGE)
		wsprintf(msg, L"%d value changed", ccount);
	else
		wsprintf(msg, L"%d value found", fcount);

	SetWindowText(hStatic, msg);

	return ccount;
}

void enumKeys(HKEY hkey, HTREEITEM parent,TCHAR* keystr, DATA* data, int bkey)
{
	HKEY hkey2;
	DWORD i = 0, len = MAX_KEY_LENGTH;
	TCHAR* key = (TCHAR*)malloc(sizeof(TCHAR) * MAX_KEY_LENGTH);
	if (key == NULL)
	{
		//fwprintf(fp, TEXT("In func : variable \"key1\" memory alloction failed.\n"));
		return;
	}

	while (RegEnumKeyEx(hkey, i, key, &len, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS)
	{
		len = MAX_KEY_LENGTH;
		wsprintf(path, TEXT("%ws\\%ws"), path, key);
		
		if (RegOpenKeyEx(hkey, key, 0, KEY_READ | KEY_WRITE, &hkey2) == ERROR_SUCCESS)
		{
			kcount++;
			
			if (parent != NULL)
			{
				if (kcount % (100 + rand() % 150) == 0)
				{
					wsprintf(msg, L"%d key loading...", kcount);
					SetWindowText(hStatic, msg);
				}
				enumKeys(hkey2, addTVitem(key, parent, bkey), key, data, bkey);
			}
			else
				enumKeys(hkey2, NULL, key, data, bkey);
				
			RegCloseKey(hkey2);
		}
		else
		{
			wsprintf(msg, TEXT("%ws open failed\n"), path);
			//fwprintf(fp, msg);

			for (int j = lstrlen(path) - 1;; j--)
			{
				path[j] = '\0';
				if (path[j - 1] == '\\')
				{
					path[j - 1] = '\0';
					break;
				}
			}
		}
		i++;
	}

	free(key);
	if (data != NULL && data->t_type != REFRESH)
		enumValue(hkey, data);

	for (int j = lstrlen(path) - 1;; j--)
	{
		path[j] = '\0';
		if (path[j - 1] == '\\')
		{
			path[j - 1] = '\0';
			break;
		}
	}
}

void enumValue(HKEY hkey, DATA* data)
{
	DWORD i = 0, len = MAX_KEY_LENGTH, len2 = MAX_VALUE_LENGTH, type, dvalue;
	long long dvalue64;
	TCHAR name[MAX_VALUE_LENGTH], * value = NULL, * adr, stype[30];
	BYTE* byte;
	int j, t, targetLen;
	LVITEM li;

	while (RegEnumValue(hkey, i, name, &len, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS)
	{
		len = MAX_KEY_LENGTH;

		if (RegQueryValueEx(hkey, name, NULL, &type, NULL, &len2) == ERROR_SUCCESS)
		{
			vcount++;
			wsprintf(stype, getTypeName(type));
			switch (type)
			{
			case REG_QWORD:
				len2 = sizeof(long long);
				RegQueryValueEx(hkey, name, NULL, NULL, (LPBYTE)&dvalue64, &len2);

				value = (TCHAR*)malloc(sizeof(TCHAR) * 40);
				wsprintf(value, L"0x%08I64x (%I64d)", dvalue64, dvalue64);
				break;
			case REG_DWORD:
				len2 = sizeof(DWORD);
				RegQueryValueEx(hkey, name, NULL, NULL, (LPBYTE)&dvalue, &len2);

				value = (TCHAR*)malloc(sizeof(TCHAR)*40);
				wsprintf(value, L"0x%08x (%d)", dvalue, dvalue);
				break;
			case REG_SZ:
			case REG_EXPAND_SZ:
				len2 += (len2 % 2); //TCAHR이 2byte인데 len2가 홀수인 경우가 있음. 이유는 모름
				value = (TCHAR*)malloc(len2);
				if (value != NULL)
				{
					if (len2 <= 2)
						value[0] = 0;
					else
					{
						RegQueryValueEx(hkey, name, NULL, NULL, (LPBYTE)value, &len2);
						value[len2 / 2 - 1] = 0;
					}
						
				}
				break;
			case REG_MULTI_SZ:
				value = (TCHAR*)calloc(len2, 1);
				if (len2 <= 2)
				{
					if (data == NULL)
					{
						lvData.mulstrData[lvData.nMul].size = len2;
						lvData.mulstrData[lvData.nMul].nString = 0;
					}

					value[0] = 0;
				}
				else
				{
					RegQueryValueEx(hkey, name, NULL, NULL, (LPBYTE)value, &len2);
					if (data == NULL)
					{
						lvData.mulstrData[lvData.nMul].strings = (TCHAR**)calloc(sizeof(TCHAR*), 1);

						int c = splitMulSz(value, len2, &(lvData.mulstrData[lvData.nMul].strings));
						lvData.mulstrData[lvData.nMul].size = len2;
						lvData.mulstrData[lvData.nMul].nString = c;
					}
				}
				break;
			case REG_NONE:
				value = (TCHAR*)malloc(sizeof(TCHAR) * 13);
				wsprintf(value, L"(길이가 0인 이진값)");
				break;
			case REG_BINARY:
				if (len2 == 0)
				{
					if (data == NULL)
					{
						lvData.byteData[lvData.nByte].bytes = (BYTE*)malloc(sizeof(BYTE));
						lvData.byteData[lvData.nByte].size = len2;
					}

					value = (TCHAR*)malloc(13 * sizeof(TCHAR));
					wsprintf(value, L"(길이가 0인 이진값)");
				}
				else
				{
					if (data == NULL)
					{
						lvData.byteData[lvData.nByte].bytes = (BYTE*)malloc(sizeof(BYTE) * len2);
						lvData.byteData[lvData.nByte].size = len2;

						RegQueryValueEx(hkey, name, NULL, NULL, lvData.byteData[lvData.nByte].bytes, &len2);
					}
				}
				break;
			default:
				break;
			}

			if (data!=NULL && data->t_type == FIND)
			{
				if ((type == REG_EXPAND_SZ || type == REG_MULTI_SZ || type == REG_SZ) && data->type == REG_SZ) //FIND
				{
					targetLen = wcslen(data->targetValue);

					if (type == REG_MULTI_SZ && len2 > 2)
					{
						TCHAR* temp = (TCHAR*)malloc(len2);

						concatMulSz(value, len2, temp);
						wsprintf(value, temp);

						free(temp);
					}

					if (len2 / 2 - 1 >= targetLen)
					{
						adr = wcsstr(value, data->targetValue);
						if (adr != NULL)
						{
							TCHAR temp[MAX_PATH_LENGTH];
							if (wcslen(name) == 0)
								wsprintf(name, L"(기본값)");

							if(data->t_type == FIND)
								addLVitem(hresultLV, path, stype, value, fcount++, name, (int)(adr - value));

							wsprintf(msg, L"%d finding...", fcount);
							SetWindowText(hStatic, msg);
						}
					}
				}
				else if ((type == REG_DWORD || type == REG_QWORD) && type == data->type)
				{
					int cmp = type == REG_DWORD ? (wcstol(data->targetValue, NULL, data->base ? 10 : 16) == dvalue) : (wcstoll(data->targetValue, NULL, data->base ? 10 : 16) == dvalue64);
					
					if (is_number(data->targetValue, data->base) && cmp)
					{
						if(data->t_type == FIND)
							addLVitem(hresultLV, path, stype, value, fcount++, name, 0);

						wsprintf(msg, L"%d finding...", fcount);
						SetWindowText(hStatic, msg);
					}
				}
			}
			else
			{
				if (wcslen(name) == 0)
				{
					li.mask = LVIF_TEXT;
					li.iItem = 0;
					li.iSubItem = 2;
					li.pszText = value;
					ListView_SetItem(hLV, &li);
				}
				else
					addLVitem(hLV, name, stype, value, i + 1, NULL, 0);
			}

			if(!value) free(value);
		}
		i++;
	}
}

void changeValue(HKEY hkey, TCHAR* name, TCHAR* value, DATA* data, DWORD pos)
{
	TCHAR temp[MAX_VALUE_LENGTH];
	//fwprintf(fp, L"%ws   %ws:%ws", path, name, value);

	if (data->type == REG_DWORD || data->type == REG_QWORD)
		wsprintf(temp, data->newValue);
	else
	{
		value[pos] = '\0';
		wsprintf(temp, L"%ws%ws%ws", value, data->newValue, value + (pos + wcslen(data->targetValue)));
	}

	//fwprintf(fp, L" -> %ws\n", temp);

	if (_RegSetValueEx(hkey, name, data->type, (LPBYTE)temp, data->base))
	{
		ccount++;
		wsprintf(value, temp);
	}
}

void changeValue(int n, DATA* tarData)
{
	HTREEITEM item;
	HKEY hkey;
	TVITEM ti;
	LVITEM li;
	LVFINDINFO lvi;
	TCHAR temp[MAX_PATH_LENGTH], name[MAX_KEY_LENGTH], value[MAX_KEY_LENGTH], * context = NULL;
	DWORD toi;
	long long toi64;

	ListView_GetItemText(hresultLV, n, 0, temp, sizeof(temp));
	ListView_GetItemText(hresultLV, n, 1, name, sizeof(name));
	ListView_GetItemText(hresultLV, n, 3, value, sizeof(value));

	item = getItemfromPath(temp);
	ti.mask = TVIF_PARAM;
	ti.hItem = item;
	TreeView_GetItem(hTV, &ti);

	li.mask = LVIF_PARAM;
	li.iItem = n;
	ListView_GetItem(hresultLV, &li);

	if ((hkey = _RegOpenKeyEx(ti.lParam, temp)) != NULL)
		changeValue(hkey, name, value, tarData, li.lParam);

	
	if (tarData->type == REG_DWORD)
	{
		toi = wcstol(value, NULL, tarData->base ? 10 : 16);
		wsprintf(value, L"0x%08x (%d)", toi, toi);
	}
	else if (tarData->type == REG_QWORD)
	{
		toi64 = wcstoll(value, NULL, tarData->base ? 10 : 16);
		wsprintf(value, L"0x%08I64x (%I64d)", toi64, toi64);
	}

	ListView_SetItemText(hresultLV, n, 3, value);
	if (TreeView_GetSelection(hTV) == item)
	{
		TreeView_SelectItem(hTV, TreeView_GetRoot(hTV));
		TreeView_SelectItem(hTV, item);
	}
}

void loadValue(TCHAR* mpath, HKEY bkeyH)
{
	HKEY hkey;
	TCHAR temp[3][10] = { L"(기본값)", L"REG_SZ", L"(값 설정 안됨)" };

	addLVitem(hLV, temp[0], temp[1], temp[2], 0, NULL, -1);

	if (RegOpenKeyEx(bkeyH, mpath, 0, KEY_READ | KEY_WRITE, &hkey) == ERROR_SUCCESS)
	{
		enumValue(hkey, NULL);
		RegCloseKey(hkey);
	}
}

void deleteAllSubkey(TCHAR* path, HTREEITEM item)
{
	HKEY hkey;
	TVITEM ti;
	int i = 0, t = 0;

	ti.mask = TVIF_PARAM;
	ti.hItem = item;
	TreeView_GetItem(hTV, &ti);
	
	if ((hkey = _RegOpenKeyEx(ti.lParam, path)) != NULL)
	{
		deleteAllSubkey(hkey, item);
		RegCloseKey(hkey);

		while (path[i] != '\0')
		{
			if (path[i] == '\\')
				t = i;

			i++;
		}
		path[t] = '\0';

		if ((hkey = _RegOpenKeyEx(ti.lParam, path)) != NULL)
		{
			if(RegDeleteKey(hkey, path + t + 1) == ERROR_SUCCESS)
				TreeView_DeleteItem(hTV, item);
			RegCloseKey(hkey);
		}
	}
}

void deleteAllSubkey(HKEY hkey, HTREEITEM item)
{
	HKEY skey;
	HTREEITEM citem;
	int i = 0;
	DWORD len = MAX_KEY_LENGTH;
	TCHAR key[MAX_KEY_LENGTH];

	while (RegEnumKeyEx(hkey, 0, key, &len, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS)
	{
		len = MAX_KEY_LENGTH;
		if (RegOpenKeyEx(hkey, key, 0, KEY_READ | KEY_WRITE, &skey) == ERROR_SUCCESS)
		{
			citem = TreeView_GetChild(hTV, item);
			deleteAllSubkey(skey, citem);
			RegCloseKey(skey);

			if(RegDeleteKey(hkey, key) == ERROR_SUCCESS)
				TreeView_DeleteItem(hTV, citem);
		}
	}
}

void createValue(int type, HTREEITEM hitem)
{
	HKEY hkey;
	int index = 0, data = -1;
	TCHAR tstr[100] = L"새 값 #1", typeName[20], ivalue[15] = L"0x00000000 (0)";
	LVFINDINFO lfi;
	LVITEM li;

	lfi.flags = LVFI_STRING | LVFI_WRAP;
	lfi.psz = tstr;

	while(1)
	{
		if (ListView_FindItem(hLV, index++, &lfi) != -1)
			wsprintf(tstr, L"새 값 #%d", index + 1);
		else
			break;
	}

	GetWindowText(hEdit, path, sizeof(path));

	if (type < 2) data = 0;

	wsprintf(typeName, L"%ws", getTypeName(REG_TYPE[type]));

	addLVitem(hLV, tstr, typeName, type < 2 ? ivalue : NULL, ListView_GetItemCount(hLV), NULL, 0);

	SetFocus(hLV);
	ListView_SetItemState(hLV, -1, LVIF_STATE, LVIS_SELECTED);
	ListView_EditLabel(hLV, ListView_GetItemCount(hLV) - 1);
}

int _RegSetValueEx(HKEY key, TCHAR* name, int type, BYTE* value, int base)
{
	int dword, res;
	long long qword;

	if (type == REG_DWORD)
	{
		dword = wcstol((TCHAR*)value, NULL, base ? 10 : 16);
		res = RegSetValueEx(key, name, 0, type, (BYTE*)&dword, sizeof(dword));
	}
	else if (type == REG_QWORD)
	{
		qword = wcstoll((TCHAR*)value, NULL, base ? 10 : 16);
		res = RegSetValueEx(key, name, 0, type, (BYTE*)&qword, sizeof(qword));
	}
	else
	{
		int size = value == NULL ? 0 : wcslen((TCHAR*)value) * sizeof(TCHAR);
		res = RegSetValueEx(key, name, 0, type, value, size);
	}

	if (res == ERROR_SUCCESS)
		return 1;
	else
		return 0;
}