#include "BaseInfoPart.h"
#include "AdditionalFuncs.h"

BOOL ShowBaseInfo(LPCTSTR szFileName) {
	TCHAR szFullFileName[MAX_PATH] = { 0 };
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNtHeaders = NULL;
	PIMAGE_FILE_HEADER pFileHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDir = NULL;
	DWORD dwSizeOfImage = 0;
	LPCTSTR pSuffix = NULL;
	HANDLE hFile = NULL, hFileMapping = NULL;
	DWORD dwSize = 0;
	LPVOID lpReserved = NULL;
	BOOL fOk = FALSE;

	__try {
		hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;
		dwSize = GetFileSize(hFile, NULL);
		hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (NULL == hFileMapping)
			__leave;;
		lpReserved = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, dwSize);
		if (NULL == lpReserved)
			__leave;
		pDosHeader = (PIMAGE_DOS_HEADER)lpReserved;
		if (IMAGE_DOS_SIGNATURE != *(WORD*)pDosHeader) {
			_tprintf("不是有效的Win32 PE文件");
			__leave;
		}
		_tprintf("软件名称: %s\r\n", szFileName);
		_tprintf("----------------------------------------\r\n");
		_tprintf("IMAGE_DOS_HEADER重要字段:\r\n");
		_tprintf("\te_magic: 0x%04X\r\n", pDosHeader->e_magic);
		_tprintf("\te_lfanew: 0x%08X\r\n", pDosHeader->e_lfanew);
		pNtHeaders = (PIMAGE_NT_HEADERS)((LONG)lpReserved + pDosHeader->e_lfanew);
		if (IMAGE_NT_SIGNATURE != pNtHeaders->Signature) {
			_tprintf("不是有效的Win32 PE文件");
			__leave;
		}
		_tprintf("----------------------------------------\r\n");
		_tprintf("IMAGE_NT_HEADERS重要字段:\r\n");
		_tprintf("  IMAGE_FILE_HEADER重要字段:\r\n");
		pFileHeader = (PIMAGE_FILE_HEADER)((PBYTE)pNtHeaders + 4);
		_tprintf("\tMachine: 0x%04X\r\n", pFileHeader->Machine);
		_tprintf("\tNumberOfSections: 0x%04X\r\n", pFileHeader->NumberOfSections);
		_tprintf("\tSizeOfOptionalHeader: 0x%04X\r\n", pFileHeader->SizeOfOptionalHeader);
		_tprintf("\tCharacteristics: 0x%04X\r\n", pFileHeader->Characteristics);
		_tprintf("----------------------------------------\r\n");
		pOptHeader = (PIMAGE_OPTIONAL_HEADER)((LONG)pFileHeader + 20);
		_tprintf("  IMAGE_OPTIONAL_HEADER重要字段:\r\n");
		_tprintf("\tMagic: 0x%04X\r\n", pOptHeader->Magic);
		_tprintf("\tAddressOfEntryPoint: 0x%08X\r\n", pOptHeader->AddressOfEntryPoint);
		_tprintf("\tImageBase: 0x%08X\r\n", pOptHeader->ImageBase);
		_tprintf("\tSectionAlignment: 0x%08X\r\n", pOptHeader->SectionAlignment);
		_tprintf("\tFileAlignment: 0x%08X\r\n", pOptHeader->FileAlignment);
		dwSizeOfImage = pOptHeader->SizeOfImage;
		int nCount = 0;
		while (dwSizeOfImage / 1024) {
			dwSizeOfImage /= 1024;
			nCount++;
		}
		switch (nCount) {
			// B
		case 0:
			pSuffix = "B";
			break;
			// KB
		case 1:
			pSuffix = "KB";
			break;
			// MB
		case 2:
			pSuffix = "MB";
			break;
			// GB
		case 3:
			pSuffix = "GB";
			break;
		}
		_tprintf("\tSizeOfImage: %ld(%ld%s)\r\n", pOptHeader->SizeOfImage, dwSizeOfImage, pSuffix);
		_tprintf("\tSizeOfHeaders: %ldB\r\n", pOptHeader->SizeOfHeaders);
		_tprintf("\tSubsystem: %s", ((pOptHeader->Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) ? "Windows图形界面\r\n" : "Windows控制台界面或者未知\r\n"));
		_tprintf("\tDllCharacteristrics: 0x%04X\r\n", (pOptHeader->DllCharacteristics));
		_tprintf("\tNumberOfRvaAnsSizes: %ld\r\n", pOptHeader->NumberOfRvaAndSizes);
		pDataDir = (PIMAGE_DATA_DIRECTORY)((PBYTE)pNtHeaders + 120);
		_tprintf("----------------------------------------\r\n");
		_tprintf("  IMAGE_DATA_DIRECTORY重要字段:\r\n");
		for (int i = 0; i < pNtHeaders->OptionalHeader.NumberOfRvaAndSizes; ++i)
			_tprintf("\tVirtualAddress[%02d]: 0x%08X\tSize: 0x%08X\r\n", i, pDataDir[i].VirtualAddress, pDataDir[i].Size);
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