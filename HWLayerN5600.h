#ifndef HWLAYER_N5600_H
#define HWLAYER_N5600_H

#include "HWLayer_Skel.h"

class HwlN5600 : public IHWL_BASE_CLASS
{
public:
	HwlN5600();
	virtual ~HwlN5600();

	virtual const char * EngineType() { return "JADE"; };
	virtual bool ImagerPowerUp(void);
	virtual bool WriteReg(unsigned subaddress, const unsigned short * uiBuffer, size_t nCount );
	virtual bool ReadReg(unsigned subaddress, unsigned short * uiBuffer, size_t nCount);
};


#endif  // HWLAYER_N5600_H