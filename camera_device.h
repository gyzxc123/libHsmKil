#ifndef CAMERA_DEVICE_H
#define CAMERA_DEVICE_H

#include <sys/ioctl.h>
#include "CommonInclude.h"
#include <hsm_imager.h>

extern struct hsm_engine_properties	ImagerProps;





// Prototypes:
int camera_init(void);
int camera_ioctl(int request, void *parg);
int camera_allocate_buffers(int num_buffers);
void* camera_query_buffer(int index);
int camera_discard_buffer(int index, void *data_ptr);
int camera_queue_buffer(int index);
int camera_streamon(void);
int camera_streamoff(void);
void camera_deinit(void);

int camera_stoppreview();

#endif /*CAMERA_DEVICE_H*/
