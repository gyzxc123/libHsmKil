
#ifndef CIMAGE_DEBUG_H
#define CIMAGE_DEBUG_H

#include <ImageDebugInterface.h>

// This class object will be instantiated in the HWL
class CImageDebug
{
public:
	CImageDebug(int columns, int rows);
	~CImageDebug();
	void ResetBuffers();
	void StoreImage(unsigned char *pAppBuffer, int extrainfo, int V4l2Index);
	void StampExternalImage(unsigned char *pAppBuffer, int extrainfo, int V4l2Index, int y_pos);

	bool WriteImageToFile(const char  *pName, unsigned char* pImage, unsigned long Size);
	void WriteAsRawImageToFile(const char  *pName, unsigned char* pImage, unsigned long Size);
	void WriteAsBmpImageToFile(const char  *pName, unsigned char* pImage, unsigned long Size);
	void WriteAsPngImageToFile(const char  *pName, unsigned char* pImage, unsigned long Size);
	void WriteAsPgmImageToFile(const char  *pName, unsigned char* pImage, unsigned long Size);
	void WriteAllImagesToFile(Imageformats_t format=BMP);
	int GetNumImages()					{ return m_ImageCounter;		}
	int GetMaxImages()					{ return NUM_DBG_BUFFERS-2;		}
	void SetQuiet(bool bQuiet=true)	{ m_Quiet=bQuiet; 				}

protected:
	void StampInternalImage(unsigned int BufferIndex, int extrainfo, int V4l2Index, int y_pos);
	void WriteDigit(unsigned char* pBuf, unsigned int x_pos, unsigned int y_pos, unsigned int digit);
	void WriteText(unsigned char* pBuf, unsigned int x_pos, unsigned int y_pos, char *szText);
	void WriteTime(unsigned char* pBuf, int extrainfo, int V4l2Index, unsigned int x_pos, unsigned int y_pos, bool external);
	void WriteCounter(unsigned char* pBuf, unsigned int x_pos, unsigned int y_pos);
	bool CheckMemory(unsigned char *pAppBuffer, size_t);
	void TextOut(const char *szLog);
	void TextOut1(const char *psz, ...);
	void ErrOut(const char *szLog);
	void ErrOut1(const char *psz, ...);

	enum
	{
		NUM_DBG_BUFFERS=30*4
	};
	unsigned char *m_pBuffer[NUM_DBG_BUFFERS+1];
	int m_ExtraInfo[NUM_DBG_BUFFERS+1];
	int m_V4L2Index[NUM_DBG_BUFFERS+1];
	unsigned int m_Size;
	int m_Current;
	int m_Latest;
	int m_View;
	int m_Columns;
	int m_Rows;
	int m_NameCounter;
	int m_ImageCounter;								//!< Count all images. In opposite to m_Current it is not reset after NUM_DBG_BUFFERS
	unsigned int m_StampCounter;					//!< Count external stamped images, so we can see which ones are identical.
	pthread_mutex_t m_lock;
	bool m_Quiet;									//!< no output (only error output is shown)
};

#endif // CIMAGE_DEBUG_H
