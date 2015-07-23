/*
 * Gen5ImagerTester.h
 *
 *  Created on: Jun 27, 2013
 *      Author: fauthd
 */

#ifndef GEN5IMAGERTESTER_H_
#define GEN5IMAGERTESTER_H_

#include "PowerTester.h"

class Gen5ImagerTester : public CPowerTester
{
public:
	Gen5ImagerTester(IHwl* pHwl);
	virtual ~Gen5ImagerTester();

	virtual bool ConfigureForTestImage(int pattern);
	//	virtual void TriggerSensor();
	//	virtual void UnTriggerSensor();
	virtual void SetupSensor();
	virtual bool SensorTest();

protected:
	void SetupCenterWeight(bool on);
};



#endif /* GEN5IMAGERTESTER_H_ */
