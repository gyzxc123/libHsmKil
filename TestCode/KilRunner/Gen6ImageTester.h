
#ifndef CIMAGE_TESTER_H
#define CIMAGE_TESTER_H

#include "PowerTester.h"

class Gen6ImagerTester : public CPowerTester
{
public:
	Gen6ImagerTester(IHwl* pHwl);
	virtual ~Gen6ImagerTester();

	virtual bool ConfigureForTestImage(int pattern);
	virtual void TriggerSensor(bool bTriggerOnce=false);
	virtual void UnTriggerSensor();
	virtual bool SensorTest();
	virtual bool PsocTest();
	virtual bool ClockTest();
	virtual bool WriteBoost();
	virtual void ResetSensor();

protected:
	virtual void WriteRegisters(RegisterEntry_t *pRegs, size_t NumRegs);

protected:

};


#endif // CIMAGE_TESTER_H
