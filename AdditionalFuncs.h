#ifndef _ADDITIONALFUNCS_H_
#define _ADDITIONALFUNCS_H_
#include <windows.h>
#include <tchar.h>
#include <cstdio>
#include <strsafe.h>

DWORD RVAToOffset(LPVOID, DWORD);
BOOL RVAToOffset(LPCTSTR szFileName, DWORD dwRVA, PDWORD dwRAW);
BOOL SectionInfo(LPCTSTR szFileName, DWORD dwRVA, BOOL bCheck);
VOID Help(LPCTSTR pszFileName);
VOID ShowErrMsg();

#endif