#include "RelocTableDescriptor.h"
#include "AdditionalFuncs.h"

BOOL ShowReloc(LPCTSTR szFileName) {
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNtHeaders = NULL;
	PIMAGE_BASE_RELOCATION pReloc = NULL;
	HANDLE hFile = NULL, hFileMapping = NULL;
	LPVOID lpFile = NULL;
	PBYTE lpReserved = NULL;

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
		pDosHeader = (PIMAGE_DOS_HEADER)lpFile;
		if (IMAGE_DOS_SIGNATURE != (WORD)pDosHeader->e_magic)
			__leave;
		pNtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader + pDosHeader->e_lfanew);
		if (IMAGE_NT_SIGNATURE != (DWORD)pNtHeaders->Signature)
			__leave;
		if (!pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress ||
			!pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			_tprintf("没有重定位表!\r\n");
			__leave;
		}
			
		pReloc = (PIMAGE_BASE_RELOCATION)((PBYTE)lpFile +
			RVAToOffset(lpFile, pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress));
		int iCount = 0;
		while (pReloc->VirtualAddress) {
			_tprintf("重定位块%d:", iCount + 1);
			_tprintf("\tSizeOfBlock: 0x%08X\t\tVirtualAddress: 0x%08X\r\n", (DWORD)pReloc->SizeOfBlock, (DWORD)pReloc->VirtualAddress);
			PWORD pBlock = (PWORD)((PBYTE)pReloc + 8);
			for (DWORD i = 0; i < (pReloc->SizeOfBlock - 8) / 2; ++i) {
				if (pBlock[i] >> 0xC == 3) {
					if (i % 3 == 0)
						putchar('\n');
					_tprintf("\t%4d块内偏移0x%04X\t\t", i + 1, pBlock[i] & 0x0FFF);
				}
			}
			putchar('\n');
			++iCount;
			pReloc = (PIMAGE_BASE_RELOCATION)((PBYTE)pReloc + pReloc->SizeOfBlock);
		}
	} 
	__finally {
		if (lpFile)
			UnmapViewOfFile(lpFile);
		if (hFileMapping)
			CloseHandle(hFileMapping);
		if (hFile)
			CloseHandle(hFile);
	}

	return(TRUE);
}