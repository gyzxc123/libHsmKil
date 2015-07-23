/*
 * PowerTester.cpp
 *
 *  Created on: Oct 15, 2013
 *      Author: fauthd
 */

#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#include <types.h>
#include <Os.h>
#include "PowerTester.h"

#undef DBG_FUNC
#define DBG_FUNC()

CPowerTester::CPowerTester(IHwl* pHwl)
: ImagerTester(pHwl)
, m_ExtraDelay(0)
, m_VsyncCnt(0)
, m_ResetCounter(0)
{
}

CPowerTester::~CPowerTester()
{
}

// Waits until n vsyncs are received.
void CPowerTester::WaitForVsyncs(size_t n)
{
	ASSERT(isPowered()); // Imager must be powered up to produce vsyncs

	unsigned long last_vsync_cnt = m_VsyncCnt;
	unsigned long last_vsync_time = GetTickCount();
	while(true)
	{
		usleep(1*1000);

		if(sem_trywait(&m_semGotVsync)==0)
		{
			m_VsyncCnt++;
		}

		if (m_VsyncCnt >= n)
			break;

		if (last_vsync_cnt == m_VsyncCnt)
		{
			// If more than 200ms without a vsync, try to reset the sensor.
			if (GetTickCount() - last_vsync_time > 200)
			{
				m_ResetCounter++;
				m_bStore=true;
				printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ "
					   "200ms without vsync, do permanent standby recovery Count=%i"
						" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n", m_ResetCounter);

				ResetSensor();
				last_vsync_time = GetTickCount();
			}
		}
		else
		{
			last_vsync_cnt = m_VsyncCnt;
			last_vsync_time = GetTickCount();
		}
	}
}


void CPowerTester::DoStartScanning()
{
//	printf("StartScanning\n");
	PowerUp();
	StartScanning();
	GetHWL()->AimOff();
	GetHWL()->IllumOff();
	SetPsocLights(false, true);
	TriggerSensor();

	GetHWL()->LoadCaptureInfo();
	GetHWL()->EnableVsyncProcessing();
}

// Stop scanning.
void CPowerTester::DoStopScanning()
{

//	printf("StopScanning\n");
	GetHWL()->AimOff();
	GetHWL()->IllumOff();
	SetPsocLights(false, false);

//	GetHWL()->AimOn();
	StopScanning();
	UnTriggerSensor();

//	GetHWL()->AimOff();
	m_ExtraDelay += 5;
	usleep(m_ExtraDelay+(28*1000));
//	printf("ExtraDelay=%i\n",m_ExtraDelay);
	if(m_ExtraDelay>(3*1000))
	{
		m_ExtraDelay=0;
		printf("Reset ExtraDelay\n");
	}

	PowerDown();
}


void CPowerTester::TestPowerDownUp()
{
	DBG_FUNC();
	const int ImagesPerRound=5;
	SetImageDebugQuiet(true);
	m_bStore=false;
	m_Rounds=0;
	m_ExtraDelay=0;
	SetupSensor();
	GetHWL()->RegisterVsyncNotification(StaticEofCallback, this);
	while(!kbhit())
	{
		if (m_Rounds % 200 == 0)
		{
			printf("Rounds so far = %u\n", m_Rounds);
			m_bStore=true;
		}
		m_Rounds++;
		m_VsyncCnt = 0;
		DoStartScanning();
		WaitForVsyncs(ImagesPerRound);
		DoStopScanning();
		ReleaseAllBuffers();
		if(m_bStore)
		{
			m_bStore=false;
			WriteFileImageDebug(PGM);
			ResetImageDebug();
		}
		usleep(200*1000);
	};
	printf("Exit with %u rounds\n", m_Rounds);
}




