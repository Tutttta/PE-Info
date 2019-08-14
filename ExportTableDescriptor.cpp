#include "ExportTableDecriptor.h"
#include "AdditionalFuncs.h"

BOOL ShowEDT(LPCTSTR szFileName) {
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNtHeaders = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDir = NULL;
	HANDLE hFile = NULL, hFileMapping = NULL;
	LPVOID lpReserved = NULL;
	PBYTE pByte = NULL;
	BOOL fOk = FALSE;
	TCHAR szCurrPath[MAX_PATH] = { 0 };

	__try {
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
		hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;
		hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (NULL == hFileMapping)
			__leave;
		lpReserved = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, GetFileSize(hFile, NULL));
		if (NULL == lpReserved)
			__leave;
		pDosHeader = (PIMAGE_DOS_HEADER)lpReserved;
		pByte = (PBYTE)lpReserved;
		if (NULL == pByte)
			__leave;
		if (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
			__leave;
		//pByte += RVAToOffset(lpReserved, (DWORD)pByte[0x3C]);
		pByte += (DWORD)pByte[0x3C];
		if (IMAGE_NT_SIGNATURE != (DWORD)(((PIMAGE_NT_HEADERS)pByte)->Signature))
			__leave;
		pNtHeaders = (PIMAGE_NT_HEADERS)pByte;
		pExportDir = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)lpReserved +
			RVAToOffset(lpReserved, pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress));
		LPCTSTR pFileName = (LPCTSTR)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->Name));
		_tprintf("�����ļ���: %s\r\n", pFileName);
		_tprintf("NumberOfFunctions: %ld\r\n", pExportDir->NumberOfFunctions);
		_tprintf("NumberOfNames: %ld\r\n", pExportDir->NumberOfNames);
		_tprintf("Base: 0x%08X\r\n", pExportDir->Base);
		_tprintf("�����������Ʊ�: \r\n");
		if (!pExportDir->NumberOfFunctions) {
			_tprintf("\t�޵�����������\r\n");
			__leave;
		}
		// �������Ʊ����Ŀ�뵼����ַ�����Ŀ���
		if ((pExportDir->NumberOfNames == pExportDir->NumberOfFunctions) && (pExportDir->NumberOfFunctions != 0)) {
			for (int i = 0; i < pExportDir->NumberOfNames; ++i) {
				LPCTSTR pStr = (LPCTSTR)((PBYTE)lpReserved + RVAToOffset(lpReserved,
					((PDWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfNames)))[i]));
				WORD wNum = ((PWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfNameOrdinals)))[i];
				_tprintf("\t%d: %-40s\tOrdinal: 0x%04X\t������ַ: 0x%08X\r\n", i + 1, pStr, wNum,
					((PDWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfFunctions)))[i]);
			}
		}
		// ������ŵ��������к�������
		else if (pExportDir->NumberOfNames < pExportDir->NumberOfFunctions) {
			for (int i = 0; i < pExportDir->NumberOfFunctions; ++i) {
				WORD wNum = ((PWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfNameOrdinals)))[i];
				if (i == wNum) { // ������i�����Ӧ�������Ʊ���������ú�������
					LPCTSTR pStr = (LPCTSTR)((PBYTE)lpReserved + RVAToOffset(lpReserved,
						((PDWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfFunctions)))[i]));
					_tprintf("\t%d: %-40s\tOrdinal: 0x%04X\t������ַ: 0x%08X\r\n", i + 1, pStr, wNum,
						((PDWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfFunctions)))[i]);
				}
				else			// û�к������������
					_tprintf("\t%d\tBase+Ordinal: 0x%04X\r\n", i + 1, wNum + pExportDir->Base);
			}
		}
		// ȫ��������ŵ��� 
		else {
			for (int i = 0; i < pExportDir->NumberOfFunctions; ++i) {
				WORD wNum = ((PWORD)((PBYTE)lpReserved + RVAToOffset(lpReserved, pExportDir->AddressOfNameOrdinals)))[i];
				_tprintf("\t%d\tBase+Ordinal: 0x%04X\r\n", i + 1, wNum + pExportDir->Base);
			}
		}
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