#ifndef TYPES_H
#define TYPES_H

#ifndef BOOL
typedef int                 	BOOL;
#endif
#ifndef PBOOL
typedef int*                 	PBOOL;
#endif

#ifndef UCHAR
typedef	unsigned char 	UCHAR;
#endif

#ifndef CHAR
typedef	unsigned char 	CHAR;
#endif

#ifndef WCHAR
typedef unsigned short	WCHAR;
#endif

#ifndef TCHAR
#ifdef UNICODE
typedef	WCHAR 	TCHAR;
#else
typedef	CHAR 	TCHAR;
#endif
#endif

#ifndef PTCHAR
typedef	TCHAR*	PTCHAR;
#endif

#ifndef BYTE
typedef	unsigned char 	BYTE;
#endif
#ifndef PBYTE
typedef	unsigned char* 	PBYTE;
#endif

#ifndef UINT
typedef unsigned int	UINT;
#endif

#ifndef WORD
typedef 	unsigned short WORD;
#endif
#ifndef PWORD
typedef 	unsigned short* PWORD;
#endif

#ifndef DWORD
typedef 	unsigned long DWORD;
#endif
#ifndef PDWORD
typedef 	unsigned long* PDWORD;
#endif

#ifndef LONG
typedef 	unsigned long LONG;
#endif

#ifndef PVOID
typedef void*	PVOID;
#endif

#ifndef LPVOID
typedef void*	LPVOID;
#endif

#ifndef FAR
#define	FAR
#endif

#ifndef HANDLE
typedef 	void* HANDLE;
#endif

#ifndef RECT
typedef struct RECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT;
#endif 

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef __cplusplus
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

#endif // TYPES_H