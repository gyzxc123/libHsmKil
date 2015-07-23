/*
 * ImageTester.h
 *
 *  Created on: Jun 27, 2013
 *      Author: fauthd
 */

#pragma once

#include <semaphore.h>
#include <queue>

#include "ImageTesterBase.h"

using namespace std;

class ImagerTester : public ImagerTesterBase
{
public:
	ImagerTester(IHwl* pHwl);
	virtual ~ImagerTester();

	virtual void StreamImagesToFile(int NumImages);

	void SetResolution(int x, int y)			{ m_Width=x; m_Height=y; };

protected:
	void StreamImagesToFileWithMode(int NumImages);
	static void StaticEofCallback(buffer_handle_t handle, void *pData);
	void EofCallback(buffer_handle_t handle);
	void ReleaseBufferDelayed(int handle);
	void ReleaseAllBuffers();
	buffer_handle_t MakeHandle(int index) 	{ return index+1; }
	int MakeIndex(buffer_handle_t handle)	{ return handle-1; }

protected:
	int m_Width;										//!< Columns in pixels
	int m_Height;										//!< Rows in pixels

	bool m_bQueueBuffers;

	int m_HwlMode;
	size_t m_NumBuffers;
	const Image_buffer_pointers *m_pBufferPointers;
	sem_t m_semGotEof;
	sem_t m_semGotVsync;
	int m_RemainingImages;
	bool m_bStore;

	std::queue<int> m_BufferQueue;	//!< Used to emulate the behavior of SD/Decoder in regards of buffers
};

