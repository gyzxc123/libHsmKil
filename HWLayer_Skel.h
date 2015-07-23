#ifndef HWLAYER_SKEL_H
#define HWLAYER_SKEL_H

#include "IHwl.h"

#define IHWL_BASE_CLASS	HwlBase

// HwlBase is a implementation of the abstract interface defined in IHwl.
class HwlBase : public IHwl
{
public:
	HwlBase();
	virtual ~HwlBase();

	virtual const char * EngineType();
	virtual const Image_buffer_pointers * GetImageBuffers(size_t & n);
	virtual void RegisterVsyncNotification(void(*pf)(buffer_handle_t, void *), 
										   void * data);
	virtual unsigned GetScanHeight(void);
	virtual unsigned GetScanWidth(void);
	virtual void AimOn(void);
	virtual void AimOff(void);
	virtual void IllumConfig(unsigned p);
	virtual void IllumOn(void);
	virtual void IllumOff(void);
	virtual bool ImagerPowerUp(void);
	virtual bool  ImagerPowerDown(void);
	virtual bool ImagerPowerOn();
	virtual bool ImagerPowerOff();
	virtual void  ResetTransferHardware(void);
	virtual bool WriteReg(unsigned subaddress, const unsigned short * uiBuffer, size_t nCount ) { return false; }
	virtual bool ReadReg(unsigned subaddress, unsigned short * uiBuffer, size_t nCount) { return false; }

	virtual bool EnableVsyncProcessing();

	virtual bool IsPsocSupported();
	virtual bool WriteIIC_PSOC(unsigned char address, const unsigned char * p, unsigned cnt );
	virtual bool ReadIIC_PSOC(unsigned char address, unsigned char * p, unsigned cnt);

	virtual void GetConfig(Config * p_config);
	virtual void StartSnapshot();
	virtual void ReleaseBuffer(buffer_handle_t handle);

	virtual bool InitCapture(buffer_handle_t handle);
	virtual bool StartScanning();
	virtual bool StopScanning();


	#if 1
	virtual bool RequestStopScanning(StopScanningCallback_t fnCallback, void * closure);
	virtual bool OpenSession();
	virtual void CloseSession();
	#endif

	#if 1
	virtual void AimOnZltd(void);
	virtual void AimOffZltd(void);
	#endif
};

#endif //HWLAYER_SKEL_H
