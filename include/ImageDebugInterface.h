
#pragma once

//! Interface export.
#ifndef IMGDBGAPI
  #define IMGDBGAPI
#endif	// IMGDBGAPI

enum Imageformats_t
{
	RAW=0, BMP, PNG, PGM
};

// for the extrainfo
const int DBG_STAMP_DEFAULT = 0;
const int DBG_STAMP_HWL = 1;
const int DBG_STAMP_REJECT = 3;
const int DBG_STAMP_SD = 4;
const int DBG_STAMP_DECODER = 6;
const int DBG_STAMP_APP = 9;

IMGDBGAPI void InitImageDebug(int columns, int rows);
IMGDBGAPI void DestroyImageDebug();

IMGDBGAPI void ResetImageDebug();
IMGDBGAPI void StoreImageDebug(unsigned char *pAppBuffer, int extrainfo=0, int V4l2Index=-1);
IMGDBGAPI inline void StoreImageDebug(void *pAppBuffer, int extrainfo=0, int V4l2Index=-1)
{
	StoreImageDebug((unsigned char *)pAppBuffer, extrainfo, V4l2Index);
};
IMGDBGAPI void StampImageDebug(unsigned char *pAppBuffer, int extrainfo, int V4l2Index, int y_pos=-1);
IMGDBGAPI inline void StampImageDebug(void *pAppBuffer, int extrainfo, int V4l2Index, int y_pos=-1)
{
	StampImageDebug((unsigned char *)pAppBuffer, extrainfo, V4l2Index, y_pos);
};
IMGDBGAPI void WriteFileImageDebug(Imageformats_t format);

IMGDBGAPI bool WriteImageToFile(const char *pName, unsigned char* pImage, unsigned long Size);
IMGDBGAPI void WriteImageToBmpFile(unsigned char* pBuffer, unsigned short nCols, unsigned short nRows);
IMGDBGAPI int GetNumDebugImages();
IMGDBGAPI int GetMaxDebugImages();
IMGDBGAPI void SetImageDebugQuiet(bool bQuiet=true);

#ifdef USE_IMAGEDEBUG

inline void DBG_INIT_IMAGE(int columns, int rows)
{
	InitImageDebug(columns, rows);
}
inline void DBG_DESTROY_IMAGE()
{
	DestroyImageDebug();
}
inline void DBG_RESET_IMAGE()
{
	ResetImageDebug();
}
inline void DBG_STORE_IMAGE(unsigned char *pAppBuffer, int extrainfo, int V4l2Index=-1)
{
	StoreImageDebug(pAppBuffer, extrainfo, V4l2Index);
}
inline void DBG_STORE_IMAGE(void *pAppBuffer, int extrainfo, int V4l2Index=-1)
{
	StoreImageDebug(pAppBuffer, extrainfo, V4l2Index);
}
inline void DBG_STAMP_IMAGE(unsigned char *pAppBuffer, int extrainfo, int V4l2Index, int y_pos=-1)
{
	StampImageDebug(pAppBuffer, extrainfo, V4l2Index, y_pos);
}
inline void DBG_STAMP_IMAGE(void *pAppBuffer, int extrainfo, int V4l2Index, int y_pos=-1)
{
	StampImageDebug(pAppBuffer, extrainfo, V4l2Index, y_pos);
}
inline void DBG_WRITE_IMAGE_FILES(Imageformats_t format=BMP)
{
	WriteFileImageDebug(format);
}
#else // USE_IMAGEDEBUG
inline void DBG_INIT_IMAGE(int, int)	{}
inline void DBG_DESTROY_IMAGE()			{}
inline void DBG_RESET_IMAGE()			{}
inline void DBG_STORE_IMAGE(unsigned char*, int, int index=-1) {}
inline void DBG_STORE_IMAGE(void*, int, int index=-1) {}
inline void DBG_STAMP_IMAGE(unsigned char*, int, int, int y_pos=-1) {}
inline void DBG_STAMP_IMAGE(void*, int, int, int y_pos=-1) {}

inline void DBG_WRITE_IMAGE_FILES(Imageformats_t format=BMP)	{ puts("Compiled without ImageDebug"); }

#endif // USE_IMAGEDEBUG

