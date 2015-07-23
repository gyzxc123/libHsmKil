/*
 * PowerTester.h
 *
 *  Created on: Oct 15, 2013
 *      Author: fauthd
 */

#pragma once

#include "ImageTester.h"

class CPowerTester : public ImagerTester
{
public:
	CPowerTester(IHwl* pHwl);
	virtual ~CPowerTester();
	virtual void TestPowerDownUp();
protected:
	void WaitForVsyncs(size_t n);
	void DoStartScanning();
	void DoStopScanning();

protected:

	unsigned long m_ExtraDelay;
	unsigned long m_VsyncCnt;
	unsigned long m_ResetCounter;
};

