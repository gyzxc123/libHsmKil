
#include "CommonInclude.h"
#if LINUX
#include <iostream>
#endif
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

//  declared here to avoid inclusion of many files with even more dependencies
#ifndef BOOL
typedef int                 	BOOL;
#endif
#ifndef PBOOL
typedef int*                 	PBOOL;
#endif

#ifndef UCHAR
typedef	unsigned char 	UCHAR;
#endif

#ifndef TCHAR
typedef	char 	TCHAR;
#endif
#ifndef PTCHAR
typedef	char*	PTCHAR;
#endif

#ifndef BYTE
typedef	unsigned char 	BYTE;
#endif
#ifndef PBYTE
typedef	unsigned char* 	PBYTE;
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

#ifndef LPVOID
typedef 	void* LPVOID;
#endif

#ifndef FAR
#define	FAR
#endif

class RECT
 {
public:
     long left;
     long top;
     long right;
     long bottom;

	RECT(int _left, int _top, int _right, int _bottom)
	: left(_left), top(_top), right(_right), bottom(_bottom)
	{};
};

typedef RECT* PRECT;

#define SIZEOF_RGBQUAD                  4


/////////////////////////////////////////////////////////////////////////
#include "bitmap.h"

//----------------------------------------------------------------------
//	This will take the input image and convert it to a bitmap.
//----------------------------------------------------------------------
Result_t ConvertToBitmap(unsigned char* pImg, unsigned short nRows, unsigned short nCols, unsigned long* pdwSize, unsigned short nBits)
//int ConvertToBitmap(BYTE* pImg, WORD nRows, WORD nCols, DWORD* pdwSize, WORD nBits)
{
	// -----------------------------------
   // Local Variables
	DWORD	dwIndex = 0;
	DWORD	x, y;
	BYTE*	pBuff;
	BITMAPINFO			BMI;
	BITMAPFILEHEADER	BMFH;
	DWORD		dwWidthBytes;
	DWORD		dwPixelDataSize;
	DWORD		dwPaletteSize;
	DWORD		dw;
	WORD		nColors;
	DWORD		dwSrcIndex;
	DWORD		dwDestIndex;
	// -----------------------------------

	// Calculate size in bytes of image (with proper BMP padding)
	if(nBits == 1)
		dw = (DWORD)((nCols+7)/8);
	else if(nBits == 8)
		dw = nCols;
	else
		return RESULT_ERR_PARAMETER;

	dwWidthBytes = (DWORD)(((dw+3)/4)*4);
	dwPixelDataSize = dwWidthBytes * (DWORD)nRows;
	nColors	= (1<<nBits);
	dwPaletteSize = nColors * SIZEOF_RGBQUAD;

	// Create & Fill in fields of the File Header
   BMFH.bfType = ((WORD) ('M' << 8) | 'B'); // Marker for DIB headers
	BMFH.bfSize = dwPixelDataSize + dwPaletteSize + sizeof(BITMAPFILEHEADER);
   BMFH.bfReserved1 = 0;
   BMFH.bfReserved2 = 0;
   BMFH.bfOffBits   = (DWORD)sizeof(BITMAPFILEHEADER);
	BMFH.bfOffBits	 += sizeof(BITMAPINFOHEADER);
	BMFH.bfOffBits  += dwPaletteSize;

	// Allocate memory for the BM Info
	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   BMI.bmiHeader.biWidth = nCols;
   BMI.bmiHeader.biHeight = nRows;
	BMI.bmiHeader.biPlanes = 1;
   BMI.bmiHeader.biBitCount = nBits;
   BMI.bmiHeader.biCompression = BI_RGB;
   BMI.bmiHeader.biSizeImage = dwPixelDataSize;
   BMI.bmiHeader.biXPelsPerMeter = 0;
   BMI.bmiHeader.biYPelsPerMeter = 0;
   BMI.bmiHeader.biClrUsed = nColors;
   BMI.bmiHeader.biClrImportant = nColors;

	// First move the raw image data into a temporary buffer
	pBuff = (BYTE*)malloc(*pdwSize);
	if(pBuff == NULL)
		return RESULT_ERR_MEMORY;
	memcpy(pBuff, pImg, *pdwSize);

	// Write the Bitmap file header
	memcpy(&pImg[dwIndex], &BMFH, sizeof(BITMAPFILEHEADER));
	dwIndex += sizeof(BITMAPFILEHEADER);

	// Write the Bitmap info header
	memcpy(&pImg[dwIndex], &BMI, sizeof(BITMAPINFOHEADER));
	dwIndex += sizeof(BITMAPINFOHEADER);

	// Write the Bitmap palette info
	//lint -e414
	for(x=0; x<nColors; x++)
	{
		pImg[dwIndex++] = (BYTE)(x*(255/(nColors-1)));
		pImg[dwIndex++] = (BYTE)(x*(255/(nColors-1)));
		pImg[dwIndex++] = (BYTE)(x*(255/(nColors-1)));
		pImg[dwIndex++] = 0;
	}
	//lint +e414

	// And finally, write the pixel data.  Remember that
	// the raw bytes are in top-down format and a bitmap
	// stores data in a bottom-up format.
	if(nBits == 8)
	{
		for(y=0; y<nRows; y++)
		{
			for(x=0; x<dwWidthBytes; x++)
			{
				if(x >= nCols)
					pImg[dwIndex++] = 0;
				else
					pImg[dwIndex++] = pBuff[((nRows-y-1)*dw)+x];
			}
		}
	}
	else if(nBits == 1)
	{
		dwDestIndex = 0;
		pImg[dwIndex+dwDestIndex] = 0;

		for(y=0; y<nRows; y++)
		{
			for(x=0; x<(dwWidthBytes*8); x++)
			{
				if(dwDestIndex != (y*dwWidthBytes) + (x/8))
				{
					dwDestIndex = (y*dwWidthBytes) + (x/8);
					pImg[dwIndex+dwDestIndex] = 0;
				}

				if(x < nCols)
				{
					dwSrcIndex = ((nRows-y-1)*nCols) + x;
					if(dwSrcIndex < *pdwSize)
						pImg[dwIndex+dwDestIndex]  += (pBuff[dwSrcIndex] & 0x01) << (7-(x % 8));
				}
			}
		}
		dwIndex += dwDestIndex;
	}
	free(pBuff);

	// Report how many bytes of the image buffer we filled
	*pdwSize = dwIndex;

	return RESULT_SUCCESS;
}
