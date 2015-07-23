#pragma once

#include <IHwl.h>

#define TEST_PATTERN_NONE	0x0000 // No test pattern: video is output
#define TEST_PATTERN_MOVING	0x0001 // Moving test pattern
#define TEST_PATTERN_FIXED	0x0002 // Fixed test pattern
#define TEST_PATTERN_FUNCTIONAL 0x0003 // Functional test pattern

struct RegisterEntry_t
{
	unsigned char nRegister;
	unsigned short nValue;
};

class ImagerTesterBase
{
public:
	ImagerTesterBase(IHwl* pHwl);
	virtual ~ImagerTesterBase();

	virtual void TestPowerDownUp()=0;

	virtual bool Set_PSOC_Mode(BYTE mode);
	virtual BYTE Get_PSOC_Mode();
	virtual bool QueryPsocRev(void);

	virtual void SetupSensor();
	virtual void ResetSensor();
	virtual void WriteRegisters(RegisterEntry_t *pRegs, size_t NumRegs);
	virtual void DumpRegisters(RegisterEntry_t *pRegs, size_t NumRegs);

	virtual bool IsForeignCamera() { return false; }

	virtual void TriggerSensor(bool bTriggerOnce=false);
	virtual void UnTriggerSensor();
	virtual bool SensorTest();
	virtual bool PsocTest();
	virtual bool ClockTest();
	virtual bool WriteBoost();
	virtual void SetCurrentTestPattern(int new_test_pattern);
	virtual bool ConfigureForTestImage(int pattern)=0;
	virtual void DumpPSOC();
	virtual bool StartScanning();
	virtual bool StopScanning();
	virtual void ReadWriteI2c(bool bSensor=1, bool bPsoc=1, int LoopCount=1);

	static void StaticStopScanningCallback(void *pData);
	void StopScanningCallback();
	static bool IsOurInstance(void *pData);

	void PowerUp();
	void PowerDown();
	bool isPowered();
	void AssertPowered();

	void SetPsocAimer(bool bOn);
	void SetPsocIllumination(bool bOn);
	void SetPsocLights(bool bAimer, bool bIll);
	void TogglePsocAimer();
	void TogglePsocIllumination();
	bool IsPsocAimer();
	bool IsPsocIllumination();
	void LightsOff();
	void ToggleGpioAimer();
	void ToggleGpioIllumination();

	IHwl* GetHWL()
	{
		ASSERT(m_pHwl!=NULL);
		return m_pHwl;
	}

protected:
	IHwl*m_pHwl;
	bool m_bPowered;
	pthread_mutex_t m_mutPowered;
	pthread_mutex_t m_mutDeferredStop;		//!< protect deferred StopScanning
	static ImagerTesterBase *ms_this;		//!< Used to check the callback code.
	BYTE m_LightsMode;
	bool m_GpioAim;
	bool m_GpioIllu;
	bool m_bTriggered;
	int m_iTestPattern;
	unsigned long m_Rounds;

	RegisterEntry_t		*m_pRegEntries;
	int			m_cRegEntries;
	unsigned char		m_iRegister_for_i2c_loop;
};

class CAutoPower
{
public:
	CAutoPower(ImagerTesterBase *pTester)
	: m_pTester(pTester)
	, m_bSavedPower(false)
	{
		ASSERT(m_pTester!=NULL);
		m_bSavedPower=m_pTester->isPowered();
		m_pTester->PowerUp();
	}
	~CAutoPower()
	{
		ASSERT(m_pTester!=NULL);
		if(!m_bSavedPower)
			m_pTester->PowerDown();
	}
protected:
	ImagerTesterBase *m_pTester;
	bool m_bSavedPower;
};

