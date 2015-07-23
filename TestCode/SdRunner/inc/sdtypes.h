#ifndef TYPES_H_be5397cc_b4a0_4504_8b27_11b05decc741
#define TYPES_H_be5397cc_b4a0_4504_8b27_11b05decc741

#include <stdint.h>
#include <string.h>

typedef uint32_t DWORD;
typedef DWORD * PDWORD;
typedef unsigned int UINT;
typedef int  BOOL;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef BYTE * PBYTE;
typedef unsigned short WORD;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef long LONG;

#define TRUE  (1)
#define FALSE (0)

#define PASTE(a,b) a##b
#define ASCII_LIT(s) s
#define UNICODE_LIT(s) PASTE(L,s)



#ifdef _UNICODE
typedef wchar_t TCHAR;
#define TEXT(s) (PASTE(L,s))
#else
typedef char TCHAR;
#define _T(string) string
inline TCHAR * _tcscpy(TCHAR * d, const TCHAR * s) { return strcpy(d,s); }
#define TEXT(s) (s)
#endif

#endif // #ifndef TYPES_H_be5397cc_b4a0_4504_8b27_11b05decc741
