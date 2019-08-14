#include "ExportTableDescriptor.h"

BOOL ShowEDT(LPCTSTR szFileName) {
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	HANDLE hFile = NULL, hFileMapping = NULL;
	LPVOID lpReserved = NULL;
	PBYTE pByte = NULL;
	hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return(FALSE);
	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (NULL == hFileMapping)
		return(FALSE);
	lpReserved = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, GetFileSize(hFile, NULL));
	if (NULL == lpReserved)
		return(FALSE);
	pDosHeader = (PIMAGE_DOS_HEADER)lpReserved;
	pByte = (PBYTE)lpReserved;
	if (NULL == pByte)
		return(FALSE);
	if (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
		return(FALSE);
	pByte += RVAToOffset(lpReserved, (DWORD)pByte[0x3C]);
	if (IMAGE_NT_SIGNATURE != (DWORD)(((PIMAGE_NT_HEADERS)pByte)->Signature))
		return(FALSE);


	UnmapViewOfFile(lpReserved);
	CloseHandle(hFile);


	return(TRUE);
}