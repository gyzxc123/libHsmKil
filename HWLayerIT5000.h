#ifndef HWLAYER_IT5000_H
#define HWLAYER_IT5000_H

#include "HWLayer_Skel.h"

class HwlIT5000 : public IHWL_BASE_CLASS
{
public:
	HwlIT5000();
	virtual ~HwlIT5000();

	virtual const char * EngineType() { return "MT9V022"; };
	virtual bool WriteReg(unsigned subaddress, const unsigned short * uiBuffer, size_t nCount );
	virtual bool ReadReg(unsigned subaddress, unsigned short * uiBuffer, size_t nCount);
	virtual bool WriteIIC_PSOC(unsigned char address, const unsigned char * p, unsigned cnt );
	virtual bool ReadIIC_PSOC(unsigned char address, unsigned char * p, unsigned cnt);
};


#endif  // HWLAYER_IT5000_H