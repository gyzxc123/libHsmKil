/*
 * ImageTester.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: fauthd
 */

#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#include <types.h>
#include "ImageTester.h"

ImagerTester::ImagerTester(IHwl* pHwl)
: ImagerTesterBase(pHwl)
, m_Width(0)
, m_Height(0)
, m_bQueueBuffers(true)
, m_HwlMode(0)
, m_NumBuffers(0)
, m_RemainingImages(0)
, m_bStore(true)
{
	sem_init(&m_semGotEof,0,0);
	sem_init(&m_semGotVsync,0,0);

	m_Width = GetHWL()->GetScanWidth();
	m_Height = GetHWL()->GetScanHeight();
	IHwl::Config config;
	GetHWL()->GetConfig(&config);
	m_HwlMode = config.captureMode;

	m_pBufferPointers = GetHWL()->GetImageBuffers(m_NumBuffers);
}

ImagerTester::~ImagerTester()
{
	sem_destroy(&m_semGotVsync);
	sem_destroy(&m_semGotEof);
}

// Here we try to emulate the behavior of the SD/Decoder that keeps the buffers busy for some time.

//#define DBG_BUFFER_QUEUE DBG_OUT1
#define DBG_BUFFER_QUEUE(...)

#define MAX_BUFFER_DELAY 3

void ImagerTester::ReleaseBufferDelayed(int handle)
{
	m_BufferQueue.push(handle);
	DBG_BUFFER_QUEUE("push handle=%i\n", handle);
	if(m_BufferQueue.size() > MAX_BUFFER_DELAY)
	{
		handle = m_BufferQueue.front();
		m_BufferQueue.pop();
		GetHWL()->ReleaseBuffer(handle);
		DBG_BUFFER_QUEUE("pop handle=%i\n", handle);
	}
}

void ImagerTester::ReleaseAllBuffers()
{
	while (!m_BufferQueue.empty())
	{
		int handle = m_BufferQueue.front();
		m_BufferQueue.pop();
		GetHWL()->ReleaseBuffer(handle);
		DBG_BUFFER_QUEUE("pop handle=%i\n", handle);
	}
}


void ImagerTester::StaticEofCallback(buffer_handle_t handle, void *pData)
{
	ASSERT(pData!=NULL);
	if (IsOurInstance(pData))
	{
		ImagerTester *pThis=(ImagerTester*)pData;
		pThis->EofCallback(handle);
	}
	else
	{
		MSC_ERROR(MSCGEN_APP_INTERFACE, "KIL returned the wrong pointer in EofCallback");
	}
}

// We try be be similar in behavior as the SD (but simpler).
// That implies that we call StartSnapshot from here.
// Unfortunately this makes the code more complicated.
void ImagerTester::EofCallback(buffer_handle_t handle)
{
//	DBG_FUNC();
	int index = MakeIndex(handle);
	ASSERT(m_pBufferPointers[index].handle == handle);
	if(m_pBufferPointers[index].handle == handle)
	{
		sem_post(&m_semGotVsync);	// took one image
		unsigned char *pImage = m_pBufferPointers[index].p_cached;
		//DBG_OUT1("store new image. handle=%i, index=%i\n", handle, index);
		if(m_bStore)
		{
			StoreImageDebug(pImage, DBG_STAMP_APP, index);
		}
		if(m_HwlMode==6)
		{
			if(--m_RemainingImages > 0)
			{
				GetHWL()->StartSnapshot();	// do same as SD: call Snapshot from the Callback
				ReleaseBufferDelayed(handle);
			}
			else
			{
				sem_post(&m_semGotEof);	// done taking images
			}
		}
		else if(m_HwlMode==4)
		{
			ReleaseBufferDelayed(handle);
			sem_post(&m_semGotEof);	// took one image
		}
		else	// other modes are untested right now again
		{
			ReleaseBufferDelayed(handle);
			sem_post(&m_semGotEof);	// took one image
		}
	}
	else
	{
		printf("Handle is broken\n");
	}
}

void ImagerTester::StreamImagesToFileWithMode(int NumImages)
{
	switch(m_HwlMode)
	{
	case 4:
	{
		MSC_NOTE(MSCGEN_APP_INTERFACE, "Capture mode 4");
		for(int i=0; i<NumImages; i++)
		{
			MSC_NOTE(MSCGEN_APP_INTERFACE, "Wait for EOF");
			sem_wait(&m_semGotVsync);
		}
		break;
	}
	case 6:
		MSC_NOTE(MSCGEN_APP_INTERFACE, "Capture mode 6");
		m_RemainingImages = NumImages;
		GetHWL()->StartSnapshot();
//		TriggerSensor(true);

		sem_wait(&m_semGotEof);
		break;
	default:
		MSC_ERROR(MSCGEN_APP_INTERFACE, "Mode %i is not supported by this tool", m_HwlMode);
		break;
	}

	ReleaseAllBuffers();
	MSC_NOTE(MSCGEN_APP_INTERFACE, "Done taking images");
}

void ImagerTester::StreamImagesToFile(int NumImages)
{
	MSC_NOTE(MSCGEN_APP_INTERFACE, "Stream %i images to array. Mode=%i", NumImages, m_HwlMode);
	SetImageDebugQuiet(true);

	GetHWL()->RegisterVsyncNotification(StaticEofCallback, this);
	StartScanning();

	PowerUp();
	WriteBoost();
	SetupSensor();
	//Set_PSOC_Mode(0x03);

	m_pBufferPointers = GetHWL()->GetImageBuffers(m_NumBuffers);
	TriggerSensor(false);

	StreamImagesToFileWithMode(NumImages);

	StopScanning();

	//Set_PSOC_Mode(m_LightsMode);
	UnTriggerSensor();

	PowerDown();
}

