#pragma once

//#define UNICODE
//����� - UTF-8 ��� ASCII

#ifdef UNICODE
using TCHAR = wchar_t;
#define _tcscmp wcscmp
#define _tcslen wcslen
#define _tcscpy wcscpy
#define _tcstoll wcstoll
#define _tcstof wcstof
#define _T(str) L##str
#else
using TCHAR = char;
#define _tcscmp strcmp
#define _tcslen strlen
#define _tcscpy strcpy
#define _tcstoll strtoll
#define _tcstof strtof
#define _T(str) str
#endif