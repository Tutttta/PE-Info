#include "AdditionalFuncs.h"

DWORD RVAToOffset(LPVOID lpFile, DWORD dwRVA) {
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_SECTION_HEADER pSecHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptHeader = NULL;
	PIMAGE_FILE_HEADER pFileHeader = NULL;
	PIMAGE_NT_HEADERS pNtHeaders = NULL;
	DWORD dwOffset = 0;
	int i = 0;

	if (NULL == lpFile)
		return(-1);
	pDosHeader = (PIMAGE_DOS_HEADER)lpFile;
	if (IMAGE_DOS_SIGNATURE != (WORD)pDosHeader->e_magic) {
		_tprintf("不是有效的PE文件!\r\n");
		return(-1);
	}
		
	pNtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)lpFile + (DWORD)pDosHeader->e_lfanew);
	pFileHeader = (PIMAGE_FILE_HEADER)((PBYTE)lpFile + (DWORD)pDosHeader->e_lfanew + 4);
	pSecHeader = (PIMAGE_SECTION_HEADER)((PBYTE)pNtHeaders + pFileHeader->SizeOfOptionalHeader + 24);
	while (i < pFileHeader->NumberOfSections) {
		if ((dwRVA >= pSecHeader->VirtualAddress) && (dwRVA < (pSecHeader->VirtualAddress + pSecHeader->SizeOfRawData))) {
			dwOffset = dwRVA - pSecHeader->VirtualAddress + pSecHeader->PointerToRawData;
			return(dwOffset);
		}
		pSecHeader++;
		++i;
	}
	_tprintf("不在任何一个节区之内!\r\n");

	return(-1);
}

BOOL RVAToOffset(LPCTSTR szFileName, DWORD dwRVA, PDWORD dwRAW) {
	HANDLE hFile = NULL, hFileMapping = NULL;
	LPVOID lpFile = NULL;
	DWORD dwRet = -1;
	__try {
		hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;
		hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (NULL == hFileMapping)
			__leave;
		lpFile = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, GetFileSize(hFile, NULL));
		if (NULL == lpFile)
			__leave;
		dwRet = RVAToOffset(lpFile, dwRVA);
	}
	__finally {
		if (lpFile)
			UnmapViewOfFile(lpFile);
		if (hFileMapping)
			CloseHandle(hFileMapping);
		if (hFile)
			CloseHandle(hFile);
		if (-1 == dwRet) {
			*dwRAW = NULL;
			return(FALSE);
		}
	}
	*dwRAW = dwRet;
	return(TRUE);
}

BOOL SectionInfo(LPCTSTR szFileName, DWORD dwRVA, BOOL bCheck) {
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_SECTION_HEADER pSecHeader = NULL;
	PIMAGE_FILE_HEADER pFileHeader = NULL;
	PBYTE lpReserved = NULL;
	HANDLE hFile = NULL, hFileMapping = NULL;
	LPVOID lpFile = NULL;
	BOOL fOk = FALSE;

	__try {
		hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;
		hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (NULL == hFileMapping)
			__leave;
		lpFile = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, GetFileSize(hFile, NULL));
		if (NULL == lpFile)
			__leave;
		lpReserved = (PBYTE)lpFile;
		pDosHeader = (PIMAGE_DOS_HEADER)lpFile;
		if (IMAGE_DOS_SIGNATURE != (WORD)pDosHeader->e_magic)
			__leave;
		lpReserved += (DWORD)lpReserved[0x3C];
		if (IMAGE_NT_SIGNATURE != *(DWORD*)lpReserved)
			__leave;
		pFileHeader = (PIMAGE_FILE_HEADER)(lpReserved + 4);
		if (NULL == pFileHeader)
			__leave;
		pSecHeader = (PIMAGE_SECTION_HEADER)((PBYTE)pFileHeader + (DWORD)pFileHeader->SizeOfOptionalHeader + 20);
		int i = 0;
		_tprintf("节区名称\t\tVirtualAddress\t\tPointerToRawData\t\tSizeOfRawData\r\n");

		while (i < pFileHeader->NumberOfSections) {
			if (!bCheck) {
				_tprintf("%-15s\t\t0x%08X\t\t0x%08X\t\t\t0x%08X\r\n", pSecHeader->Name,
					pSecHeader->VirtualAddress, pSecHeader->PointerToRawData, pSecHeader->SizeOfRawData);
			}
			else if (bCheck && dwRVA) {
				if (dwRVA >= pSecHeader->VirtualAddress && dwRVA < pSecHeader->VirtualAddress + pSecHeader->SizeOfRawData) {
					_tprintf("0x%08X位于%s节区，其RAW为: 0x%08X",
						dwRVA, pSecHeader->Name, (dwRVA - pSecHeader->VirtualAddress + pSecHeader->PointerToRawData));
					fOk = TRUE;
					__leave;
				}
			}
			else
				__leave;
			++pSecHeader;
			++i;
		}
	}
	__finally {
		if (lpFile)
			UnmapViewOfFile(lpFile);
		if (hFileMapping)
			CloseHandle(hFileMapping);
		if (hFile)
			CloseHandle(hFile);
		if (!fOk)
			return(FALSE);
	}
	
	return(TRUE);
}

VOID Help(LPCTSTR pszFileName) {
	_tprintf("参数使用方法: \r\n<%s> [-b/-i/-e/-r/-res/-a/-s] [文件名]\r\n", pszFileName);
	_tprintf("-b: 该参数显示基础信息，位于IMAGE_DOS_HEADER与IMAGE_NT_HEADERS中的重要字段.\r\n");
	_tprintf("-i: 该参数显示导入表信息.\r\n");
	_tprintf("-e: 该参数显示导出表信息.\r\n");
	_tprintf("-r: 该参数显示重定位表信息.\r\n");
	_tprintf("-res: 该参数显示资源表信息.\r\n");
	_tprintf("-a: 该参数显示所有重要信息.\r\n");
	_tprintf("-s: 该参数显示节表重要信息.\r\n");
	_tprintf("<%s> [-o/-c] [文件名] RVA\r\n", pszFileName);
	_tprintf("-o: 该参数将RVA转成文件内偏移.\r\n");
	_tprintf("-c: 该参数确认指定RVA所在节区及其相关信息.\r\n");
	return;
}

VOID ShowErrMsg() {
	HLOCAL hlocal = NULL;
	DWORD dwError = 0;
	DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

	dwError = GetLastError();
	if (!dwError)
		return;
	BOOL fOk = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, systemLocale, (PTSTR)& hlocal, 0, NULL);
	if (!fOk)
		_tprintf("未知错误!\r\n");
	if (fOk && (NULL != hlocal)) {
		_tprintf("%s\r\n", (LPCTSTR)LocalLock(hlocal));
		LocalFree(hlocal);
	}
	else
		_tprintf("未知错误!\r\n");
	return;
}
