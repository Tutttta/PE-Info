#include "ImportTableDescriptor.h"
#include "AdditionalFuncs.h"

BOOL ShowIDT(LPCTSTR szFileName) {
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNtHeaders = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = NULL;
	PBYTE lpByte = NULL;
	HANDLE hFile = NULL, hFileMapping = NULL;
	DWORD dwSize = 0, dwOffset = 0;
	LPVOID lpReserved = NULL;
	int iCount = 0;
	BOOL fOk = FALSE;
	TCHAR szCurrPath[MAX_PATH] = {0};
	__try {
		// 改进1：判定是何种类型PE文件并加上合适后缀
		if ((NULL == _tcsrchr(szFileName, '/')) && (NULL == _tcsrchr(szFileName, '\\'))) {
			GetModuleFileName(NULL, szCurrPath, _countof(szCurrPath) * sizeof(TCHAR));
			LPTSTR pStr = _tcsrchr(szCurrPath, '\\');
			if (NULL != pStr) {
				++pStr;
				_tcscpy_s(pStr, _tcslen(pStr) * sizeof(TCHAR), szFileName);
			}
			else
				__leave;
		}
		else
			_tcscpy_s(szCurrPath, _countof(szCurrPath) * sizeof(TCHAR), szFileName);
		hFile = CreateFile(szCurrPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;
		dwSize = GetFileSize(hFile, NULL);
		hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (NULL == hFileMapping)
			__leave;
		lpReserved = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, dwSize);
		if (NULL == lpReserved)
			__leave;
		
		pDosHeader = (PIMAGE_DOS_HEADER)lpReserved;
		if (IMAGE_DOS_SIGNATURE != (WORD)pDosHeader->e_magic) {
			_tprintf("Not PE format!\r\n");
			__leave;
		}
		lpByte = (PBYTE)pDosHeader;
		lpByte += (DWORD)lpByte[0x3C];
		pNtHeaders = (PIMAGE_NT_HEADERS)lpByte;
		//lpByte += 0x80; // NTͷ 4 + 20 + 96 + 8 == 0x80
		PIMAGE_OPTIONAL_HEADER pOptHeader = (PIMAGE_OPTIONAL_HEADER)(lpByte + 24);
		if (IMAGE_NT_SIGNATURE != (DWORD)pNtHeaders->Signature) {
			_tprintf("Not PE format!\r\n");
			__leave;
		}
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)lpReserved +
			RVAToOffset(lpReserved, (pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)));
		
		while (0 != pImportDesc->Name) {
			LPCTSTR pDllName = (LPCTSTR)((PBYTE)lpReserved + RVAToOffset(lpReserved, pImportDesc->Name));
			_tprintf("----------------------------------------\r\n");
			_tprintf("IMAGE_IMPORT_DESCRIPTOR%d:\r\n\tDllName: %s\r\n", iCount + 1, pDllName);
			_tprintf("----------------------------------------\r\n");
			PIMAGE_THUNK_DATA stThunk = (PIMAGE_THUNK_DATA)((PBYTE)lpReserved + RVAToOffset(lpReserved, pImportDesc->FirstThunk));
			int iCount = 0;
			while (stThunk->u1.AddressOfData) {
				PIMAGE_IMPORT_BY_NAME pSt = (PIMAGE_IMPORT_BY_NAME)((PBYTE)lpReserved + RVAToOffset(lpReserved, stThunk->u1.Function));
				if (!(stThunk->u1.Ordinal & 0x80000000))
					_tprintf("%d: %s\r\n", iCount + 1, pSt->Name);
				else
					_tprintf("%d: %d\r\n", iCount + 1, pSt->Hint);
				stThunk++;
				iCount++;
			}

			pImportDesc++;
			iCount++;
		}
		_tprintf("----------------------------------------\r\n");
		fOk = TRUE;
	}
	__finally {
		if (lpReserved)
			UnmapViewOfFile(lpReserved);
		if (hFileMapping)
			CloseHandle(hFileMapping);
		if (hFile)
			CloseHandle(hFile);
		if (!fOk)
			return(FALSE);
	}

	return(TRUE);
}