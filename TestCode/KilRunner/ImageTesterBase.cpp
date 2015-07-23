/*
 * ImageTesterBase.cpp
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
#include "ImageTester.h"


ImagerTesterBase *ImagerTesterBase::ms_this=NULL;

ImagerTesterBase::ImagerTesterBase(IHwl* pHwl)
: m_pHwl(pHwl)
, m_bPowered(false)
, m_LightsMode(0)
, m_GpioAim(false)
, m_GpioIllu(false)
, m_bTriggered(false)
, m_iTestPattern(TEST_PATTERN_NONE)
, m_Rounds(0)
, m_pRegEntries(NULL)
, m_cRegEntries(0)
, m_iRegister_for_i2c_loop(0)
{
	pthread_mutex_init(&m_mutPowered, NULL);
	pthread_mutex_init(&m_mutDeferredStop, NULL);
	ms_this = this;
}

ImagerTesterBase::~ImagerTesterBase()
{
	pthread_mutex_unlock(&m_mutPowered);
	pthread_mutex_destroy(&m_mutPowered);
	pthread_mutex_unlock(&m_mutDeferredStop);
	pthread_mutex_destroy(&m_mutDeferredStop);
}

///////////////////////////////////////////////////////////////////////////////////////////
#define AIMERBIT	1
#define ILUMINATIONBIT	2
#define PSOC_MODE_REG	0x32

void ImagerTesterBase::SetPsocAimer(bool bOn)
{
	AssertPowered();
	m_LightsMode = bOn ? m_LightsMode|AIMERBIT : m_LightsMode&~AIMERBIT;
	Set_PSOC_Mode(m_LightsMode);
}

void ImagerTesterBase::SetPsocIllumination(bool bOn)
{
	AssertPowered();
	m_LightsMode = bOn ? m_LightsMode|ILUMINATIONBIT : m_LightsMode&~ILUMINATIONBIT;
	Set_PSOC_Mode(m_LightsMode);
}

void ImagerTesterBase::SetPsocLights(bool bAimer, bool bIll)
{
	AssertPowered();
	m_LightsMode &= ~(AIMERBIT|ILUMINATIONBIT);
	m_LightsMode |= bAimer ? AIMERBIT : 0;
	m_LightsMode |= bIll ? ILUMINATIONBIT : 0;
	Set_PSOC_Mode(m_LightsMode);
}

void ImagerTesterBase::LightsOff()
{
	AssertPowered();
	m_LightsMode &= ~(AIMERBIT|ILUMINATIONBIT);
//	printf("mode set=%X\n", m_LightsMode);
	Set_PSOC_Mode(m_LightsMode);
	GetHWL()->AimOff();
	GetHWL()->IllumOff();
}

void ImagerTesterBase::TogglePsocAimer()
{
	AssertPowered();
	m_LightsMode ^= AIMERBIT;
//	printf("mode set=%X\n", m_LightsMode);
	Set_PSOC_Mode(m_LightsMode);
}

void ImagerTesterBase::TogglePsocIllumination()
{
	AssertPowered();
	m_LightsMode ^= ILUMINATIONBIT;
//	printf("mode set=%X\n", m_LightsMode);
	Set_PSOC_Mode(m_LightsMode);
}

bool ImagerTesterBase::IsPsocAimer()
{
	AssertPowered();
	return (Get_PSOC_Mode() & AIMERBIT) ? true : false;
}

bool ImagerTesterBase::IsPsocIllumination()
{
	AssertPowered();
	return (Get_PSOC_Mode() & ILUMINATIONBIT) ? true : false;
}

void ImagerTesterBase::ToggleGpioAimer()
{
	AssertPowered();
	m_GpioAim ^= true;
	if(m_GpioAim)
		GetHWL()->AimOn();
	else
		GetHWL()->AimOff();
}

void ImagerTesterBase::ToggleGpioIllumination()
{
	AssertPowered();
	m_GpioIllu ^= true;
	if(m_GpioIllu)
	{
		WriteBoost();
		GetHWL()->IllumOn();
	}
	else
	{
		GetHWL()->IllumOff();
	}
}


//*****************************************************************************
// Function: Set_PSOC_Mode
// Description:	Sets the PSoC mode for manual usage (for HWLayerTester)
// Arguments: None
// Returns: None
//*****************************************************************************
bool ImagerTesterBase::Set_PSOC_Mode(BYTE mode)
{
	AssertPowered();
	bool ret;

	if( mode&ILUMINATIONBIT )
		WriteBoost();

	ret = GetHWL()->WriteIIC_PSOC(PSOC_MODE_REG,&mode,1);
	if (!ret)
	{
		printf("Error PSOC I2C write, Reg=%02X, Value=%04X\n", PSOC_MODE_REG, (UINT)mode);
	}

	return ret;
}

BYTE ImagerTesterBase::Get_PSOC_Mode()
{
	AssertPowered();
	BYTE mode=0;
	BOOL status = FALSE;
	status = GetHWL()->ReadIIC_PSOC(PSOC_MODE_REG, &mode, 1);
	if(!status)
	{
		printf("Error PSOC I2C read, Reg=%02X, Value=%04X\n", PSOC_MODE_REG, (UINT)mode);
	}
	return mode;
}

#define PSOC_MAJOR_REV_REG 0x38
#define PSOC_MINOR_REV_REG 0x39

//*****************************************************************************
// Function: QueryPsocRev
// Description:	Retrieves the PSoC revision
// Arguments: None
// Returns:	None
//*****************************************************************************
bool ImagerTesterBase::QueryPsocRev(void)
{
	AssertPowered();
	BYTE bytePsocMajorRev = 0x00;
	BYTE bytePsocMinorRev = 0x00;
	BOOL status = FALSE;

	status = GetHWL()->ReadIIC_PSOC(PSOC_MAJOR_REV_REG, &bytePsocMajorRev, 1);
	DBGMSG(ZONE_PSOC,(_T("GetHWL()->ReadIIC_PSOC status = %d\r\n"), status));

	status = GetHWL()->ReadIIC_PSOC(PSOC_MINOR_REV_REG, &bytePsocMinorRev, 1);
	DBGMSG(ZONE_PSOC,(_T("GetHWL()->ReadIIC_PSOC status = %d\r\n"), status));

	if( status )
	{
		DBGMSG(ZONE_PSOC,(_T("Psoc Revision = %d.%d\r\n"), bytePsocMajorRev, bytePsocMinorRev));
		printf("Psoc Revision = %d.%d\r\n", bytePsocMajorRev, bytePsocMinorRev);
	}

	return status;
}

void ImagerTesterBase::WriteRegisters(RegisterEntry_t *pRegs, size_t NumRegs)
{
	if(IsForeignCamera())
	{
		printf("Foreign Camera: Ignoring register writes\n");
	}
	else
	{
		for(size_t i=0; i<NumRegs; i++)
		{
			if( !GetHWL()->WriteReg(pRegs[i].nRegister, &pRegs[i].nValue, 1) )
			{
				printf("Sensor I2C write, Reg=%02X, Value=%04X, Err=%d %s\n", (UINT)pRegs[i].nRegister, (UINT)pRegs[i].nValue, errno, strerror (errno));
			}
		}
	}
}

void ImagerTesterBase::DumpRegisters(RegisterEntry_t *pRegs, size_t NumRegs)
{
	for(size_t i=0; i<NumRegs; i++)
	{
		unsigned short	value = 0;
			
		if( GetHWL()->ReadReg(pRegs[i].nRegister, &value, 1) )
		{
			printf("Reg[0x%02X] = 0x%04X (0x%04X) %s\r\n", (UINT)pRegs[i].nRegister, (UINT)value, (UINT)pRegs[i].nValue, (value==pRegs[i].nValue)? "OK" : "FAIL");
		}
		else
		{
			printf("Sensor I2C read, Reg=%02X, Err=%d %s\n", (UINT)pRegs[i].nRegister, errno, strerror (errno));
		}
	}
}

void ImagerTesterBase::DumpPSOC()
{
	CAutoPower(this);
	const int NumRegs=64;
	const int Count=NumRegs;
	BYTE Regs[NumRegs+1];
	BOOL status = FALSE;
	status = GetHWL()->ReadIIC_PSOC(0,Regs,Count);
	if(!status)
		MSC_ERROR(MSCGEN_APP_INTERFACE, "Error: Could not read PSOC registers");

	for(int i=0; i<NumRegs; i+=8)
	{
		printf("Reg %02X (%02d): ", i,i);
		for(int j=0; j<8; j++)
		{
			printf("%02X,", Regs[i+j]);
		}
		printf("\n");
	}
}

//#define DBG_UNIMPLEMENTED DBG_OUT1
#define DBG_UNIMPLEMENTED(...)

void ImagerTesterBase::SetCurrentTestPattern(int new_test_pattern)
{
	m_iTestPattern = new_test_pattern;
}

void ImagerTesterBase::TriggerSensor(bool)
{
	DBG_UNIMPLEMENTED("%s not implemented\n", __func__);
}

void ImagerTesterBase::UnTriggerSensor()
{
	DBG_UNIMPLEMENTED("%s not implemented\n", __func__);
}

void ImagerTesterBase::SetupSensor()
{
	if( m_pRegEntries )
	{
		AssertPowered();
		WriteRegisters(m_pRegEntries, m_cRegEntries);
	}

	ConfigureForTestImage(m_iTestPattern);
}

bool ImagerTesterBase::SensorTest()
{
	if( !m_pRegEntries ) return true;

	printf("+++++ Executing SensorTest +++++\n");

	AssertPowered();
	WriteRegisters(m_pRegEntries, m_cRegEntries);
	DumpRegisters(m_pRegEntries, m_cRegEntries);

	printf("----- SensorTest Complete -----\n");

	return true;
}

//#define PROOF_RESET_WORKS 1
void ImagerTesterBase::ResetSensor()
{
	AssertPowered();
	bool status;

#ifdef PROOF_RESET_WORKS
	unsigned short Data=0;
	unsigned char Register=0x02;	// Row start
	Data=0xF;
	status = GetHWL()->WriteReg(Register,&Data,1);
	if(!status)
		printf("Write sensor I2C failed\n");
	usleep(50 * 1000);	// wait one frame
	status = GetHWL()->ReadReg(Register,&Data,1);
	if(!status)
		printf("Read sensor I2C failed\n");
	printf("Before reset reg%i=%04X\n", Register, Data);

	BYTE PsocRegRead = 0;
	status = GetHWL()->ReadIIC_PSOC(0, &PsocRegRead,1);
	if(!status)
		printf("Read PSOC I2C failed\n");
#endif

	// Issue reset, wait 100ms, then reload all IIC registers.
	unsigned char data = 0;
	status = GetHWL()->WriteIIC_PSOC(0x82, &data, 1);
	if(!status)
		printf("Write PSOC I2C failed\n");
	usleep(100 * 1000);

#ifdef PROOF_RESET_WORKS
	status = GetHWL()->ReadReg(Register,&Data,1);
	if(!status)
		printf("Read sensor I2C failed\n");
	printf("After reset reg%i=%04X\n", Register, Data);
#endif

	SetupSensor();
}

bool ImagerTesterBase::PsocTest()
{
	return QueryPsocRev();
}

bool ImagerTesterBase::ClockTest()
{
	DBG_UNIMPLEMENTED("%s not implemented\n", __func__);
	return true;
}

bool ImagerTesterBase::WriteBoost()
{
	DBG_UNIMPLEMENTED("%s not implemented\n", __func__);
	return true;
}

void ImagerTesterBase::PowerUp()
{
	if (!GetHWL()->ImagerPowerUp())
	{
		puts("ERROR: ImagerPowerUp failed.");
//		assert(0);
	}
//	printf("power_up");
	pthread_mutex_lock(&m_mutPowered);
	m_bPowered = true;
	pthread_mutex_unlock(&m_mutPowered);
}

void ImagerTesterBase::PowerDown()
{
	if (!GetHWL()->ImagerPowerDown())
	{
		puts("ERROR: ImagerPowerDown failed.");
//		assert(0);
	}
//	printf("power_down");
	pthread_mutex_lock(&m_mutPowered);
	m_bPowered = false;
	pthread_mutex_unlock(&m_mutPowered);

}

bool ImagerTesterBase::isPowered()
{
	bool RetVal=false;
	pthread_mutex_lock(&m_mutPowered);
	RetVal = m_bPowered;
	pthread_mutex_unlock(&m_mutPowered);
	return RetVal;
}

void ImagerTesterBase::AssertPowered()
{
	if (!isPowered())
	{
		puts("ERROR: Imager is not PowerUp. @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	}
}

bool ImagerTesterBase::IsOurInstance(void *pData)
{
	return ((pData != NULL) && (ms_this == pData));
}


void ImagerTesterBase::StaticStopScanningCallback(void *pData)
{
	ASSERT(pData!=NULL);
	if (IsOurInstance(pData))
	{
		ImagerTester *pThis = (ImagerTester*) pData;
		pThis->StopScanningCallback();
	}
	else
	{
		MSC_ERROR(MSCGEN_APP_INTERFACE, "KIL returned the wrong pointer in StopScanningCallback");
	}
}

void ImagerTesterBase::StopScanningCallback()
{
	int RetVal = GetHWL()->StopScanning();
	if(!RetVal)
	{
		MSC_ERROR(MSCGEN_APP_INTERFACE, "StopScanning failed");
	}
	pthread_mutex_unlock(&m_mutDeferredStop);
}

bool ImagerTesterBase::StopScanning()
{
	// code below emulates the behavior of the SD
	pthread_mutex_lock(&m_mutDeferredStop);	// do not overrun stop
	int RetVal = GetHWL()->RequestStopScanning(StaticStopScanningCallback, this);
	if(RetVal)
	{	// no deferred Stop
		StopScanningCallback();
	}
	pthread_mutex_lock(&m_mutDeferredStop);		// now wait until the callback did its work
	pthread_mutex_unlock(&m_mutDeferredStop); 	// clean
	return true;
}

bool ImagerTesterBase::StartScanning()
{
	int RetVal = GetHWL()->StartScanning();
	if (!RetVal)
	{
		MSC_ERROR(MSCGEN_APP_INTERFACE, "StartScanning failed");
	}
	return RetVal ? true : false;
}

// helps in analyzing the I2C with the oscilloscope
void ImagerTesterBase::ReadWriteI2c(bool bSensor, bool bPsoc, int LoopCount)
{
	AssertPowered();
	unsigned short Data = 0;
	unsigned char Register = m_iRegister_for_i2c_loop;0x02;	// Row start
	bool status;

	for (int i = 0; i < LoopCount; i++)
	{
		if (bSensor)
		{
			status = GetHWL()->ReadReg(Register, &Data, 1);
			if (!status)
				printf("Read sensor I2C failed\n");
			status = GetHWL()->WriteReg(Register, &Data, 1);
			if (!status)
				printf("Write sensor I2C failed\n");
		}
		
		if(bSensor&bPsoc)
		{
			usleep(200);
		}
		
		if (bPsoc)
		{
			BYTE PsocRegRead = 0;
			status = GetHWL()->ReadIIC_PSOC(PSOC_MODE_REG, &PsocRegRead, 1);
			if (!status)
				printf("Read PSOC I2C failed\n");
			status = GetHWL()->WriteIIC_PSOC(PSOC_MODE_REG, &PsocRegRead, 1);
			if (!status)
				printf("Write PSOC I2C failed\n");
		}

		if(bSensor&bPsoc)
		{
			usleep(200);
		}
		
		if(AbortOnKbHit())
		{
			break;
		}
	}
}
