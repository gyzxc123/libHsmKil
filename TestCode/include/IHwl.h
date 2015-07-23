/** @file

	Defines the interface to the HW Layer.  The scan driver will
	retrieve an object that implements this interface by calling @c
	InitHWLayer.
*/

#ifndef IHWL_H_5323F569_0318_4534_AEC2_FCE08233D186
#define IHWL_H_5323F569_0318_4534_AEC2_FCE08233D186

#ifdef STDINT_H_AVAILABLE
#   include <stdint.h>
    typedef intptr_t buffer_handle_t
#else
	#include <stddef.h>
	typedef int buffer_handle_t;
#endif

struct Image_buffer_pointers
{
	/**
		Normally, @c IHwl::GetImageBuffers will initialize @c p_cached
	    to contain a cached pointer to the image data.

		The scan driver will not access an image buffer using the @c
	    p_cached pointer from the time that InitCapture is called to
	    capture an image into a buffer until the time the Vsync
	    callback registered with @c RegisterVsyncNotification is
	    called to indicate that image capture to a buffer is complete.
	    This HW Layer should insure the cache is consitent before
	    calling the Vsync callback.
	*/

	unsigned char * p_cached;

	/** Normally, @c IHwl::GetImageBuffers will initialize @c
		p_uncached to contain an uncached point to the image data.


		The scan driver can use p_uncached to access an image buffer
		anytime.  In particular, while the image is being captured
		between the call to @c InitCapture to start capturing into a
		buffer and the Vsync callback that inidates that capture to
		the buffer is complete, the scan driver will use p_uncached to
		monitor the progress of the image capture and to sample pixels
		for exposure control.
	*/

	unsigned char * p_uncached;

	/**
		@c handle is a unique identifier for a buffer.  The @c handle
		is passed to InitCapture to indicate into which buffer the
		next image from the camera should be placed.  The value of
		handle should never be zero.
	*/

	buffer_handle_t           handle;
};

class IHwl;

extern "C"
{

/** Initializes the HW Layer.

	This is called by the scan driver to detect and initialize the
	hardware layer.  If an image engine is detected and any necessary
	initialization is successful, then this method returns a pointer
	to an @c IHwl.  If an image engine is not detected, this method
	returns @c null. If @c null is returned the HW Layer should free
	any resources it allocated.

	@param[in] context This is a platform specific driver
	                   initializtion information.  Under WinCE this is
	                   is a pointer to a string containing registery
	                   information.  See documentation for XXX_Init.

	@returns A pointer to an @c IHwl, the HW Layer was initialzed.
	         @c null, the HW Layer initialization failed.
*/

IHwl * InitHWLayer(void * context);

/** Deinitializes the HW Layer.

	@param[in] p_hwlayer A pointer to the @c IHwl object that is to be
                         released.

	Deallocats the object pointed to by @c p_hwlayer and frees any
    resources used by that object.
*/


void DeinitHWLayer(IHwl * p_hwlayer);
}

class IHwl_psoc
{
public:
	virtual ~IHwl_psoc()
		{}

	/** Determines if the engine has a PSOC.

		@returns true if the engine has a PSOC.  Otherwise, returns
		         false.
	*/
	virtual bool IsPsocSupported()
		{ return false; }

	/** Writes values to the PSOC.

		@param[in] address Address of first register to write.
		@param[in] p An array of values to write to the
                    @c address
		@param[in] cnt The number of values in @c p to write.

		@returns true if there is no error. Otherwise, false.

		This writes @c cnt values in @c p to @c address of the PSOC.
		The scan driver will not call this function if @c
		IsPsocSUpported returns @c false.
	*/

	virtual bool WriteIIC_PSOC(unsigned char address,
	 								 const unsigned char * p, unsigned cnt )
		{ return false; }


	/** Reads values from the PSOC.

		@param[in] address Address of first register to read.
		@param[out] p An array of values that are read from
		                     @c address
		@param[in] cnt The number of values to read into @c p.

		@returns true if there is no error. Otherwise, false.

		This reads @c cnt values into @c p from @c address of the
		PSOC.  The scan driver will not call this function if
		@c IsPsocSUpported returns @c false.
	*/
	virtual bool ReadIIC_PSOC(unsigned char address, unsigned char * p,
	 								unsigned cnt)
		{ return false; }
};

class IHwl : public IHwl_psoc
{
public:
	typedef void (*StopScanningCallback_t)(void * closure);
	typedef void (*VsyncNotifationCallback_t)(buffer_handle_t, void *);
	virtual ~IHwl() {}


	/** Retrieves the engine type from the HW Layer.



		@returns A string representing the engine type detected by the
		         HW Layer.
	*/


	virtual const char * EngineType() = 0;

	/** Retieves available image buffers.

		@param[out] n  This will be set to the number of image buffers
		that are available.

		@returns A pointer to an array of @c Image_buffer_pointers.
		The array should have @c n elements.

		This returns an array of all available image buffers allocated
		by the HW Layer.  Information about the image buffers are
		returned in the @c Image_buffer_pointers structure.

	*/
	virtual const Image_buffer_pointers * GetImageBuffers(size_t & n) = 0;

	/** When the scandriver no longer needs an image buffer.  It calls
		ReleaseBuffer with a handle to that buffer.  This tells the HW
		Layer that it can reuse the buffer to capture a new image.

		In capture modes where the HW Layer controls the capture
		buffer when a buffer handle is passed to the Vsync
		Notification callback, ownership of that before belong to the
		scandriver.  While the scandriver owns a buffer the HW Layer
		may not overwrite any data in the buffer. Later, the
		scandriver calls ReleaseBuffer to release its ownership back
		to the HW Layer so the buffer can be reused. */
	virtual void ReleaseBuffer(buffer_handle_t handle) { }

	/** This is called to register a vsync notification.

	   @param[in] pf A pointer to a function that is called when the
	                 camera produces an end of frame (or Vsync
	                 signal).  The first parameter passed to pf is the
	                 buffer handle where the image last image from the
	                 camera was placed.  If the camera produces an end
	                 of frame but the image was not captured to a
	                 buffer, then @c pf should still be called but
	                 handle, (the first paramter) should be zero.  The
	                 second parameter to pf is simply a copy of the
	                 void pointer, @c data, that is passed into @c
	                 RegisterVsyncNotification.

	   @param[in] data Is a void pointer that is passed to as the
	                   second paramter to @c pf.

	  This method registers a callback function that is called from
	  the HW Layer at the end of frame (or Vsync signal).

	*/
	virtual void RegisterVsyncNotification(VsyncNotifationCallback_t pf,
										   void * data) =0;

	/**
		Retrieves the maximum image height.

		@returns The maximum image height for a sensor.
	*/
	virtual unsigned GetScanHeight(void) = 0;

	/**
		Retrieves the maximum image width.

		@returns The maximum image width for a sensor.
	*/
	virtual unsigned GetScanWidth(void) = 0;

	/** Powers up the sensor.

		@returns true if there is no error. Otherwise, false.

		Normally, this asserts the enable pin (if there is one) on the
		sensor.
	*/
	virtual bool  ImagerPowerUp(void) = 0;

	/** Powers down the sensor.

		@returns true if there is no error. Otherwise, false.

		Normally, this deasserts the enable pin (if there is one) on
		the sensor.
	*/
	virtual bool  ImagerPowerDown(void) = 0;

	/** Powers on the sensor.

		@returns true if there is no error. Otherwise, false.

		Normally, this applies VCC to the sensor.
	*/
	virtual bool ImagerPowerOn() = 0;

	/** Powers off the sensor.

		@returns true if there is no error. Otherwise, false.

		Normally, this removes VCC from the sensor.
	*/
	virtual bool ImagerPowerOff() = 0;

	/** Initializes image capture.

		@param[in] handle A handle to the buffer where the next image
		will be placed.

		@returns true if there is no error. Otherwise, false.

		This instructs the HW Layer to place the next image from the
		sensor into the buffer indicated by @c handle.

		The scan driver expects InitCapture should configure the DMA
		or camera port to capture the next image produced by the
		sensor then return immediately.

		The HW Layer should be prepared to queue up one InitCapture
		for the next image while it is capturing an image.  When
		streaming images the scan driver will call @c InitCapture to
		tell the HW Layer where the next image should go while the HW
		Layer is capturing an image.  Many camera ports support an
		automatic pointer flipping mechanism that can be used to queue
		up the next buffer.
	*/
	virtual bool InitCapture(buffer_handle_t handle) { return 0; }


	/** In snapshot mode when the HW Layer controls the capture
		buffer, this function is instead of InitCapture to tell
		the HW Layer to capture an image.
	*/
	virtual void StartSnapshot() { }

	/**
		Resets image transfer.

		If image capture is in progress this will stop the DMA or
		camera port from capturing the image.
	*/
	virtual void  ResetTransferHardware(void) = 0;

	/** Turns the aimer on by setting the aimer pin on the sensor. */
	virtual void AimOn(void) = 0;

	/** Turns the aimer off by clearing the aimer pin on the sensor. */
	virtual void AimOff(void) = 0;

	/** Sets the illumination intensity.

		@param[in] intensity is between 0 and 100% inclusive.  100%
		indicates the illumination should be as bright as possible.
		0% indicates the illumination should be as dark as possible.

		This is often used in cases where the illumination pin on the
		sensor is connected to a pulse width modulation circuit.  If
		the sensor lighting is controlled through a PSOC, the HW Layer
		should ignore this method.
	*/

	virtual void IllumConfig(unsigned intensity) = 0;

	/** Turns the illumination on by setting the illumination pin on
	    the sensor. */
	virtual void IllumOn(void) = 0;

	/** Turns the illumination off by clearing the illumination pin on
	    the sensor. */
	virtual void IllumOff(void) = 0;

	/** Write values to registers in the sensor.

		@param[in] reg_address Address of first register to write.
		@param[in] uiBuffer An array of values to write to the
                            @c reg_address.
		@param[in] nCount The number of values in @c uiBuffer to write.

		@returns true if there is no error. Otherwise, false.

		This writes @c nCount values in @c uiBuffer to @c reg_address
		in the sensor.
	*/

	virtual bool WriteReg(  unsigned reg_address,
							const unsigned short * uiBuffer,
							size_t nCount                     )  = 0;

	/** Reads values from registers in the sensor.

		@param[in] reg_address Address of first register to read.
		@param[out] uiBuffer An array of values that are read from
		                     @c reg_address.
		@param[in] nCount The number of values to read into @c uiBuffer.

		@returns true if there is no error. Otherwise, false.

		This reads @c nCount values into @c uiBuffer from @c
		reg_address in the sensor.
	*/
	virtual bool ReadReg(  unsigned reg_address,
						   unsigned short * uiBuffer,
						   size_t nCount               ) = 0;

	struct Config
	{
		size_t size;
		unsigned captureMode;
		/** Specifies scan engine orientation.
			0   - right side up
			90  - rotated 90 degrees clockwise
			180 - up side down
			270 - rotated 270 degress clockwise.
		*/
		unsigned orientation;  
		
	};

	/** Retrieves configuration parameters. At the moment, I've only
		defined the captureMode as a parameter. */
	virtual void GetConfig(Config * p_config) { p_config->captureMode = 3; }

	/** Obsolete, new HW Layers should not implement this method

		@returns true.
	 */
	virtual bool LoadCaptureInfo() { return true; }

	/** Obsolete, new HW Layers should not implement this method

		@returns true.
	 */
	virtual bool EnableVsyncProcessing() { return true; }

	/** This is called before the scan driver does any operations
	    to start scanning.  This HWLayer may override this method to
	    perform operations to stop scanning. */
	virtual bool StartScanning() { return true; }

	/** This is called after the scan driver does any operations
	    to stop scanning.  This HWLayer may override this method to
	    perform operations needed to stop scanning . */
	virtual bool StopScanning() { return true; }

	/** OpenSession is called when the scan driver's client attempts
	    to open a session with the scan driver.  There can be multiple
	    open sessions at any given time.  This is only called when
	    there are no open sessions and a client tries to open a
	    session.  This is an optional method that may be implemented
	    by the HW Layer.

		@returns @c false indicates there is a problem with the sensor
		and the client should not be permitted to open a session.  @c
		true indicates the sensor is okay and the session open should
		succeed.

		If this method returns false, the session will not be opened.
		Instead, a NULL session handle will be returned to the scan
		driver's client.  This may method may verify the sesnor
		hardware is working correctly when a session is opened and
		return an error to the client if it is not working.

		On devices where the camera port is muliplexed between
		multiple cameras, @c OpenSession may return @c false if the
		camera port is unavaialible because it is accessing a
		different camera.
	*/
	virtual bool OpenSession() { return true; };

	/** CloseSession is called when the scan driver's client closes a
	    session with the scan driver.  There can be multiple open
	    sessions at any given time.  This is only called when the last
	    open session is closed.  This is an optional method that may
	    be implemented by the HW Layer.
	*/
	virtual void CloseSession() { };

	/** RequestStopScanning is called before the scan driver tries to
		stop scanning images. If this method returns true, the scan
		driver will immediately perform its stop scanning sequence.
		If this method returns false, the scan driver will wait and
		perform the stop scanning sequence when fnCallback is called.
		The closure parameter must be passed to fnCallback.  This is
		an optional method that may be implemented by the HW Layer.
		The default implementation of this method returns true.

		@returns true to begin stop scanning sequence immediately.
		         false to begin the stop scanning sequnce when
		         fnCallback is called.
	*/
	virtual bool RequestStopScanning(StopScanningCallback_t fnCallback,
							 void * closure)
	{
		return true;
	}
};


#endif // #ifndef IHWL_H_5323F569_0318_4534_AEC2_FCE08233D186
