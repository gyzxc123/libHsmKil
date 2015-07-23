//! \file
#define LOG_TAG "ImgDbg"

#include "CommonInclude.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "ImageDebug.h"
#include "ImageDebugInterface.h"
#include "DebugHelpers.h"

//#define DEBUG_IMAGE_BUFFERS_INTERNAL 1
#ifdef DEBUG_IMAGE_BUFFERS_INTERNAL
#define DBG_OUT		puts
#define DBG_OUT1 		printf
#else
#undef DBG_OUT
#undef DBG_OUT1
#define DBG_OUT(...)	((void)0)
#define DBG_OUT1(...)	((void)0)
#endif

void ShowHSMError(Result_t errnum)
{
//	static char pErrorText[ENGINE_API_RESPONSE_LEN];
	if(errnum!=RESULT_SUCCESS)
	{
//		oemGetErrorMessage(pErrorText, errnum);
//		puts(pErrorText);
		DbgOut1(PRINT_LEVEL, LOG_TAG, "ERROR=%i\n", errnum);
	}
}

#define DEFAULT_Y 3
#define RESET_Y 50

CImageDebug::CImageDebug(int columns, int rows)
: m_Size(columns*rows)
, m_Current(0)
, m_Latest(0)
, m_View(-1)
, m_Columns(columns)
, m_Rows(rows)
, m_NameCounter(0)
, m_ImageCounter(0)
, m_StampCounter(0)
, m_Quiet(false)
{
	DBG_OUT1("CImageDebug(columns=%i, rows=%i)\n",m_Columns, m_Rows);
	for(int i=0; i<NUM_DBG_BUFFERS; i++)
	{
		m_pBuffer[i] = new unsigned char[m_Size+100];
		memset(m_pBuffer[i], 128, m_Size);
		StampInternalImage(i, DBG_STAMP_DEFAULT, -1, RESET_Y);
//		DBG_OUT1("CImageDebug() m_pBuffer[%i]=%p\n", i, m_pBuffer[i]);
	}
	m_StampCounter=0;
	pthread_mutex_init(&m_lock, NULL);

//	DBG_OUT("CImageDebug()-\n");
}

CImageDebug::~CImageDebug()
{
//	DBG_OUT("~CImageDebug()+\n");
	for(int i=0; i<NUM_DBG_BUFFERS; i++)
	{
//		DBG_OUT1("~CImageDebug() m_pBuffer[%i]=%p\n", i, m_pBuffer[i]);
		delete [] m_pBuffer[i];
		m_pBuffer[i] = NULL;
	}
	pthread_mutex_destroy(&m_lock);
//	DBG_OUT("~CImageDebug()-\n");
}


void CImageDebug::ResetBuffers()
{
	pthread_mutex_lock(&m_lock);
	int Save_StampCounter = m_StampCounter;
	m_ImageCounter=0;
	m_Current=0;
	m_Latest=0;
	m_View=-1;
	for(int i=0; i<NUM_DBG_BUFFERS; i++)
	{
		memset(m_pBuffer[i], 0, m_Columns*20);
		StampInternalImage(i, DBG_STAMP_DEFAULT, -1, RESET_Y);
	}
	m_StampCounter=Save_StampCounter;
	pthread_mutex_unlock(&m_lock);
}

bool CImageDebug::CheckMemory(unsigned char *pAppBuffer, size_t length)
{
	long pagesize = sysconf(_SC_PAGESIZE);
	pAppBuffer = (unsigned char *)(((size_t)pAppBuffer) & ~(pagesize-1));		// make it aligned
	length = (length+pagesize-1) & ~(pagesize-1);	// correct the length for aligned
	size_t vec_size = (length+pagesize-1) / pagesize;
	unsigned char *vec = new unsigned char[vec_size+1];

	int ret = mincore(pAppBuffer, length, vec);
	if(ret)
	{
		if(errno!=ENOMEM)
		{
			ErrOut1("@@@@@@@@@@ CImageDebug::CheckMemory failed: %i, %s\n", errno, strerror (errno));
			ErrOut1("@@@@@@@@@@ CImageDebug::CheckMemory pAppBuffer=%p, length= %x\n", pAppBuffer, length);
		}
	}

	delete [] vec;
	return ret ? false : true;
}

void CImageDebug::StoreImage(unsigned char *pAppBuffer, int extrainfo, int V4l2Index)
{
	pthread_mutex_lock(&m_lock);
	m_ImageCounter++;
	TextOut1("CImageDebug::StoreImage Buffer=%p, extra=%i, index=%i, stamp=%i, num=%i\n",
			pAppBuffer, extrainfo, V4l2Index, m_StampCounter, m_ImageCounter);
	ASSERT(m_Current>=0);
	ASSERT(m_Current<NUM_DBG_BUFFERS);
	if(pAppBuffer != NULL)
	{
		if(CheckMemory(pAppBuffer, m_Size))
		{
			memcpy(m_pBuffer[m_Current], pAppBuffer, m_Size);
			StampInternalImage(m_Current, extrainfo, V4l2Index, DEFAULT_Y);
			m_Latest=m_Current;
			if (++m_Current>=NUM_DBG_BUFFERS)
			{
				m_Current=0;
			}
		}
		else
		{
			ErrOut1("CImageDebug::StoreImage found invalid pointer !!! Check image size and init !!!!!!!!!!!!!!!\n");
		}
	}
	else
	{
		ErrOut1("CImageDebug::StoreImage NULL pointer !!!!!!!!!!!!!!!!!!!\n");
	}
	pthread_mutex_unlock(&m_lock);
}

const int FONTX = 7;
const int FONTY = 11;
const int NUMDIGITS=10+6+1+1+1;
static const char font[NUMDIGITS][FONTY][FONTX] =
{
	{
		"******",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"******"
	},
	{
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *"
	},
	{
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"******",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"******"
	},
	{
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"******"
	},
	{
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *"
	},
	{
		"******",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"******"
	},
	{
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"******",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"******"
	},
	{
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *"
	},
	{
		"******",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"******",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"******"
	},
	{
		"******",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"******",
		"     *",
		"     *",
		"     *",
		"     *",
		"     *"
	},
	{
		"      ",
		" **** ",
		"*    *",
		"*    *",
		"*    *",
		"******",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"      "
	},
	{
		"      ",
		"***** ",
		"*    *",
		"*    *",
		"*    *",
		"******",
		"*    *",
		"*    *",
		"*    *",
		"***** ",
		"      "
	},
	{
		"      ",
		" *****",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		" *****",
		"      "
	},
	{
		"      ",
		"***** ",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"*    *",
		"***** ",
		"      "
	},
	{
		"      ",
		"******",
		"*     ",
		"*     ",
		"*     ",
		"***** ",
		"*     ",
		"*     ",
		"*     ",
		"******",
		"      "
	},
	{
		"      ",
		"******",
		"*     ",
		"*     ",
		"*     ",
		"***** ",
		"*     ",
		"*     ",
		"*     ",
		"*     ",
		"      "
	},
	{
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      "
	},
	{
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"    **",
		"    **",
	},
	{
		"******",
		"******",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"      ",
		"******",
		"******",
	},
};

#define SPAC	10+6
#define DOT	SPAC+1
#define NONE	DOT+1

const char xlate[] =
{
//	  0 ,  1 ,  2 ,  3 ,  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  A ,  B ,  C ,  D ,  E ,  F ,
	NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,
	NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,
	SPAC,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE, DOT,NONE,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,NONE,NONE,NONE,NONE,NONE,NONE,
	NONE,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,
	NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,
	NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,
	NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,
};

void CImageDebug::WriteDigit(unsigned char* pBuf, unsigned int x_pos, unsigned int y_pos, unsigned int digit)
{
	if (digit >= NUMDIGITS)
		return;

	const char (*c_font)[FONTX] = font[digit];
	unsigned int stride = m_Columns;
	unsigned char * p = pBuf;
	p += x_pos + y_pos * stride;
#if 0
	for (int x = 0; x < FONTX; ++x)
	{
		unsigned char * p2 = p + x;
		*p2 = 0xff-50;
		p2[(FONTY+1) * stride] = 0xff-50;
	}
#endif
	for (int y = 1; y <= FONTY; ++y)
	{
		for (int x = 0; x < FONTX; ++x)
		{
			unsigned char * p2 = p + x + y * stride;
			char c = c_font[y-1][x];
			if (c == '*')
				*p2 = 0;
			else
				*p2 = 0xff;
		}
	}
}

void CImageDebug::WriteText(unsigned char* pBuf, unsigned int x_pos, unsigned int y_pos, char *szText)
{
	unsigned int Xoffset=0;
	while(*szText!=0)
	{
		char cTemp = *szText++;
		if (cTemp < 0)
			cTemp = 0;
		cTemp = xlate[(int)cTemp];

		WriteDigit(pBuf, x_pos+Xoffset, y_pos, cTemp);
		Xoffset+=FONTX;
	}
}

void CImageDebug::WriteTime(unsigned char* pBuf, int extrainfo, int V4l2Index, unsigned int x_pos, unsigned int y_pos, bool external)
{
	const int BufSize=20;
 	char Buf[BufSize];
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
	snprintf(Buf, BufSize, "%lu.%06lu", ts.tv_sec, ts.tv_nsec / 1000);
	WriteText(pBuf, x_pos, y_pos, Buf);
//	Uncomment line below to see the time in the logs
//	DbgOut1("Write%s: e=%i, i=%i, c=%i, %s\n", external ? "Stamp":"Time", extrainfo, V4l2Index, m_StampCounter-1, Buf);
}

void CImageDebug::WriteCounter(unsigned char* pBuf, unsigned int x_pos, unsigned int y_pos)
{
	const int BufSize=20;
 	char Buf[BufSize];

	snprintf(Buf, BufSize, "%02i", m_StampCounter);
	WriteText(pBuf, x_pos, y_pos, Buf);
	m_StampCounter++;
}

#define POS_EXTRA	130
#define POS_INDEX	150
#define POS_COUNT	200
#define POS_TIME	250

void CImageDebug::StampInternalImage(unsigned int BufferIndex, int extrainfo, int V4l2Index, int y_pos)
{
	unsigned char* pBuf = m_pBuffer[BufferIndex];

	if(extrainfo>=0)
		WriteDigit(pBuf,  POS_EXTRA + FONTX, y_pos, extrainfo);

	if(V4l2Index>=0)
		WriteDigit(pBuf,  POS_INDEX + FONTX, y_pos, V4l2Index);

	WriteCounter(pBuf, POS_COUNT, y_pos);
	WriteTime(pBuf, extrainfo, V4l2Index, POS_TIME + FONTX, y_pos, false);

	m_V4L2Index[BufferIndex] = V4l2Index;
	m_ExtraInfo[BufferIndex] = extrainfo;
}

void CImageDebug::StampExternalImage(unsigned char *pBuf, int extrainfo, int V4l2Index, int y_pos)
{
	pthread_mutex_lock(&m_lock);
	TextOut1("CImageDebug::Stamp Buffer=%p, extra=%i, index=%i, stamp=%i\n",
			pBuf, extrainfo, V4l2Index, m_StampCounter);

	if(CheckMemory(pBuf, m_Size))
	{
		if (y_pos < 0)	// make a nice default position
			y_pos = DEFAULT_Y+FONTY+DEFAULT_Y;

		if(extrainfo>=0)
			WriteDigit(pBuf, POS_EXTRA + FONTX, y_pos, extrainfo);

		if(V4l2Index>=0)
			WriteDigit(pBuf, POS_INDEX + FONTX, y_pos, V4l2Index);

		WriteCounter(pBuf, POS_COUNT, y_pos);
		WriteTime(pBuf, extrainfo, V4l2Index, POS_TIME + FONTX, y_pos, true);
	}
	else
	{
		ErrOut1("CImageDebug::StampExternalImage found invalid pointer !!! Check image size and init !!!!!!!!!!!!!!!\n");
	}
	pthread_mutex_unlock(&m_lock);
}

bool CImageDebug::WriteImageToFile(const char *pName, unsigned char* pImage, unsigned long Size)
{
	bool bStatus=false;
	FILE * hFile;
	hFile = fopen( pName, "w-");
	if (hFile != NULL)
	{
		unsigned int written = fwrite(pImage, sizeof(unsigned char), Size, hFile);
		if(written==Size)
			bStatus=true;

		fclose(hFile);
	}

	return bStatus;
};

void CImageDebug::WriteAsRawImageToFile(const char *pName, unsigned char* pImage, unsigned long Size)
{
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];

	snprintf(Name, MaxNameLen, "%s.raw", pName);
	TextOut1("Writing %s\n", Name);
	if (!WriteImageToFile(Name, pImage, Size))
	{
		ErrOut1("CImageDebug::WriteImageToFile failed!!");
	}
}

void CImageDebug::WriteAsBmpImageToFile(const char *pName, unsigned char* pImage, unsigned long Size)
{
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];
	int BmPSize = (m_Columns*m_Rows)*2;
	unsigned char *pBmpBuffer = new unsigned char[BmPSize+100];

	snprintf(Name, MaxNameLen, "%s.bmp", pName);
	TextOut1("Writing %s\n", Name);
	memcpy(pBmpBuffer, pImage, m_Size);
	unsigned long pdwSize=m_Size;
	Result_t RetVal = ConvertToBitmap(pBmpBuffer, m_Rows, m_Columns, &pdwSize, 8);
	ShowHSMError(RetVal);

	if(RetVal==RESULT_SUCCESS)
	{
		if (!WriteImageToFile(Name, pBmpBuffer, pdwSize))
		{
			ErrOut1("CImageDebug::WriteImageToFile failed!!");
		}
	}
	delete [] pBmpBuffer;
}

void CImageDebug::WriteAsPngImageToFile(const char *pName, unsigned char* pImage, unsigned long Size)
{
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];
	int PngSize = (m_Columns*m_Rows)*2;
	unsigned char *pPngBuffer = new unsigned char[PngSize+100];

	snprintf(Name, MaxNameLen, "%s.png", pName);
	ErrOut1("CImageDebug::WriteImageToFile failed!! because .png is not implemented yet");
	delete [] pPngBuffer;
}

void CImageDebug::WriteAsPgmImageToFile(const char *pName, unsigned char* pImage, unsigned long Size)
{
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];

	snprintf(Name, MaxNameLen, "%s.pgm", pName);
	TextOut1("Writing %s\n", Name);

	bool bStatus=false;
	FILE * hFile;
	hFile = fopen( Name, "w-");
	if (hFile != NULL)
	{
		fprintf(hFile, "P5\n%u %u\n255\n", m_Columns, m_Rows);
		unsigned int written = fwrite(pImage, sizeof(unsigned char), Size, hFile);
		if(written!=Size)
		{
			ErrOut1("CImageDebug::WriteImageToFile failed!!");
		}
		fclose(hFile);
	}
}

void CImageDebug::WriteAllImagesToFile(Imageformats_t format)
{
	pthread_mutex_lock(&m_lock);
//	DbgOut("WriteAllImagesToFile+");
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];

	int BmPSize = (m_Columns*m_Rows)*2;
	unsigned char *pBmpBuffer = new unsigned char[BmPSize+100];
	for (int i=m_Latest; i>=0; i--)
	{
		snprintf(Name, MaxNameLen, "DbgImage_%dx%d_%02d_%02d_%1d", m_Columns, m_Rows, m_NameCounter, i+1, m_ExtraInfo[i]);
		switch (format)
		{
		case RAW:
			WriteAsRawImageToFile(Name, m_pBuffer[i], m_Size);
			break;
		case BMP:
			WriteAsBmpImageToFile(Name, m_pBuffer[i], m_Size);
			break;
		case PNG:
			WriteAsPngImageToFile(Name, m_pBuffer[i], m_Size);
			break;
		case PGM:
			WriteAsPgmImageToFile(Name, m_pBuffer[i], m_Size);
			break;
		default:
			ErrOut1("CImageDebug::WriteImageToFile failed!! Unsupported format");
			break;
		}
	}
	m_NameCounter++;
	pthread_mutex_unlock(&m_lock);
}

void CImageDebug::TextOut(const char *szLog)
{
	if(!m_Quiet)
	{
		::DbgOut(PRINT_LEVEL, LOG_TAG, szLog);
	}
}

void CImageDebug::TextOut1(const char *psz, ...)
{
	if(!m_Quiet)
	{
		va_list	pArgs;
		va_start( pArgs, psz );
		vDbgOut1( PRINT_LEVEL, LOG_TAG, psz, pArgs );
	}
}

void CImageDebug::ErrOut(const char *szLog)
{
	::DbgOut(PRINT_LEVEL, LOG_TAG, szLog);
}

void CImageDebug::ErrOut1(const char *psz, ...)
{
	va_list	pArgs;
	va_start( pArgs, psz );
	vDbgOut1( PRINT_LEVEL, LOG_TAG, psz, pArgs );
}

/////////////////////////////////////////////////////////////////////////////
CImageDebug *gImages=NULL;

IMGDBGAPI void InitImageDebug(int columns, int rows)
{
	if(gImages==NULL)
	{
		gImages = new CImageDebug(columns, rows);
	}
}

IMGDBGAPI void DestroyImageDebug()
{
	delete gImages;
	gImages = NULL;
}

IMGDBGAPI void StoreImageDebug(unsigned char *pAppBuffer, int extrainfo, int V4l2Index)
{
	if(gImages!=NULL)
	{
		gImages->StoreImage(pAppBuffer, extrainfo, V4l2Index);
	}
}

IMGDBGAPI void StampImageDebug(unsigned char *pAppBuffer, int extrainfo, int V4l2Index, int y_pos)
{
	if(gImages!=NULL)
	{
		gImages->StampExternalImage(pAppBuffer, extrainfo, V4l2Index, y_pos);
	}
}

IMGDBGAPI void SetImageDebugQuiet(bool bQuiet)
{
	if(gImages!=NULL)
	{
		gImages->SetQuiet(bQuiet);
	}
}

IMGDBGAPI void ResetImageDebug()
{
	if(gImages!=NULL)
	{
		gImages->ResetBuffers();
	}
}

IMGDBGAPI int GetNumDebugImages()
{
	int ret=0;
	if(gImages!=NULL)
	{
		ret = gImages->GetNumImages();
	}
	return ret;
}

IMGDBGAPI int GetMaxDebugImages()
{
	int ret=0;
	if(gImages!=NULL)
	{
		ret = gImages->GetMaxImages();
	}
	return ret;
}

IMGDBGAPI CImageDebug* GetImageDebugObject()
{
	return gImages;
}

IMGDBGAPI void WriteFileImageDebug(Imageformats_t format)
{
	if(gImages!=NULL)
	{
		gImages->WriteAllImagesToFile(format);
	}
}

#if 0 // was not used anymore
IMGDBGAPI bool WriteImageToFile(const char *pName, unsigned char* pImage, unsigned long Size)
{
	bool bStatus=false;
	FILE * hFile;
	hFile = fopen( pName, "w-");
	if (hFile != NULL)
	{
		unsigned int written = fwrite(pImage, sizeof(unsigned char), Size, hFile);
		if(written==Size)
			bStatus=true;

		fclose(hFile);
	}

	return bStatus;
};

static int gsFileNumber=0;

IMGDBGAPI void WriteImageToBmpFile(unsigned char* pBuffer, unsigned short nCols, unsigned short nRows)
{
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];

	int RawSize = nCols*nRows;
	int BmPSize = (nCols*nRows)*2;
	unsigned char *pBmpBuffer = new unsigned char[BmPSize+100];
	snprintf(Name, MaxNameLen, "Image_%03d.bmp", gsFileNumber);
	DbgOut1("Writing %s\n", Name);
	memcpy(pBmpBuffer, pBuffer, RawSize);
	unsigned long pdwSize=RawSize;
	Result_t RetVal = ConvertToBitmap(pBmpBuffer, nRows, nCols, &pdwSize, 8);
	ShowHSMError(RetVal);

	if(RetVal==RESULT_SUCCESS)
	{
		if (!WriteImageToFile(Name, pBmpBuffer, pdwSize))
		{
			DbgOut("WriteImageToFile failed!!");
		}
	}
	delete [] pBmpBuffer;
	gsFileNumber++;
}
#endif
