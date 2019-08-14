#ifndef _STDMAIN_H_
#define _STDMAIN_H_
#include <shlwapi.h>
#include "BaseInfoPart.h"
#include "AdditionalFuncs.h"
#include "ImportTableDescriptor.h"
#include "ExportTableDescriptor.h"
#include "RelocTableDescriptor.h"

#pragma comment(lib, "shlwapi.lib")

enum Param {
	b, i, e, r, res, a, s, o, c
} _Param;

#endif