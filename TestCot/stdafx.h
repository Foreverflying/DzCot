// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>

#ifdef UNICODE
#define _TCHAR      wchar_t
#define _T( str )   L##str
#else
#define _TCHAR      char
#define _T( str )   str
#endif

// TODO: reference additional headers your program requires here
