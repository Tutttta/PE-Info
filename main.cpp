#include "stdMain.h"
//int _tmain(INT argc, TCHAR** argv) {
//	DWORD dwRVA = 0;
//
//	RVAToOffset("C:\\Users\\Administrator\\Desktop\\HelloWorld.exe", 4500, &dwRVA);
//	_tprintf("%ld\r\n", dwRVA);
//	system("pause");
//
//	return(0);
//}

int _tmain(INT argc, TCHAR **argv) {
	/*
	 -b BaseInfo    x
	 -i ImportInfo  x
	 -e ExportInfo  x
	 -r RelocInfo   x
	 -res ResInfo   *
	 -a AllInfo     x
	 -o RVAToOffset x
	 -s SectionInfo x
	 -c CheckRVASec x
	 */
	CONST TCHAR* pszCmd[] = { "-b", "-i", "-e", "-r", "-res", "-a", "-s", "-o", "-c"};
	DWORD dwRAW = 0;
	TCHAR szSelfName[MAX_PATH] = {0};
	__try {
		if (!GetModuleFileName(NULL, szSelfName, _countof(szSelfName) * sizeof(TCHAR))) {
			ShowErrMsg();
			return(-1);
		}
		PathStripPath(szSelfName);
		// 首先判定完全不符合参数数目的命令形式
		if (argc < 2 || argc > 3) {
			if (argc == 4) {
				if (!_tcscmp(argv[1], "-c")) {
					if (!SectionInfo(argv[2], atol(argv[3]), TRUE))
						ShowErrMsg();
				}
				else if (!_tcscmp(argv[1], "-o")) {
					DWORD dwOffset = 0;
					if (!RVAToOffset(argv[2], atol(argv[3]), &dwOffset))
						ShowErrMsg();
					else
						_tprintf("RVA: %ld转换成FOA: 0x%08X\r\n", atol(argv[3]), dwOffset);
				}
			}
			else 
				_tprintf("参数用法: <%s> [-b/-i/-e/-r/-res/-a/-o/-s/-c] [文件名]\r\n帮助文档请加-h参数获取。\r\n版本信息加-v后缀\r\n", szSelfName);
		}
		// 判定符合两个参数的形式
		else if (argc == 2) {
			if (!_tcscmp(argv[1], "-v"))
				_tprintf("版本信息v1.0\r\n作者: Kiopler");
			else if (!_tcscmp(argv[1], "-h")) {
				Help(szSelfName);
			}
			else {
				_tprintf("参数用法: <%s> [-b/-i/-e/-r/-res/-a/-o/-s/-c] [文件名]\r\n帮助文档请加-h参数获取。\r\n版本信息加-v后缀\r\n", szSelfName);
			}
		}
		// 满足参数数目条件时，开始判定具体参数
		else {
			int n;
			for (n = 0; n < _countof(pszCmd); ++n) {
				if (!_tcscmp(argv[1], pszCmd[n])) 
					break;
			}
			switch (n) {
			case b:
				if (!ShowBaseInfo(argv[2]))
					ShowErrMsg();
				break;
			case i:
				if (!ShowIDT(argv[2]))
					ShowErrMsg();
				break;
			case e:
				if (!ShowEDT(argv[2]))
					ShowErrMsg();
				break;
			case r:
				if (!ShowReloc(argv[2]))
					ShowErrMsg();
				break;
			case s:
				if (!SectionInfo(argv[2], 0, FALSE))
					ShowErrMsg();
				break;
			case res:
				_tprintf("暂时没有编写资源表代码!\r\n");
				break;
			case a:
				if (!ShowBaseInfo(argv[2]))
					ShowErrMsg();
				if (!SectionInfo(argv[2], 0, FALSE))
					ShowErrMsg();
				if (!ShowIDT(argv[2]))
					ShowErrMsg();
				if (!ShowEDT(argv[2]))
					ShowErrMsg();
				if (!ShowReloc(argv[2]))
					ShowErrMsg();
				// 差资源表
				break;
			default:
				_tprintf("参数用法: <%s> [-b/-i/-e/-r/-res/-a/-o/-s/-c] [文件名]\r\n帮助文档请加-h参数获取。\r\n版本信息加-v后缀\r\n", szSelfName);
				break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		_tprintf("未知错误!\r\n");
	}

	return(0);
}

