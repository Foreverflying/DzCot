
#include <stdio.h>
#include <string.h>

#if defined( _WIN32 )
#include <malloc.h>
#elif defined( __linux__ )
#include <alloca.h>
#endif

#ifdef UNICODE
#define _TCHAR      wchar_t
#define _T( str )   L##str
#else
#define _TCHAR      char
#define _T( str )   str
#endif

#include "../DzCot/Inc_DzCot.h"
#include "DzTrace.h"
