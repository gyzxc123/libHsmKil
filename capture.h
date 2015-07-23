#ifndef CAPTURE_H
#define CAPTURE_H

#include "CommonInclude.h"

//! Capture mode
/*

+------------------------------------+---------------------------+
|                                    | **Buffer Management**     |
|                                    +-------------+-------------+
|                                    |   HWLayer   | Scan Driver |
|                                    |             |             |
+----------+-----------+-------------+-------------+-------------+
| **Capture| Streaming | Progress -  | Mode 1      | Mode 2      |
| Mode**   |           | HWLayer     |             |             |
|          |           +-------------+-------------+-------------+
|          |           | Progress -  | Unsupported | Mode 3      |
|          |           | Scan Driver |             |             |
|          |           +-------------+-------------+-------------+
|          |           | Post Vsync  | Mode 4      | Mode 5      |
|          +-----------+-------------+-------------+-------------+
|          | Snapshot  | Post Vsync  | Mode 6      | Mode 7      | ** !! ONLY MODES 6 & 7 CURRENTLY SUPPORTED !! **
+----------+-----------+-------------+-------------+-------------+

*/

#define DEF_CAPTURE_MODE	6 // SD passes buffer management to the KIL

//! Amount of video buffers. 
#ifndef NUM_BUFFERS
#define NUM_BUFFERS             3	// Total number of 'system' buffers (this is where the HWlayer has control of the buffers)
#endif

int capture_init(int capture_mode);
int capture_deinit();
void *capture_get_buffer_pointer(int index);
int capture_set_buffer_handle(int index, int handle);
int capture_request_buffer(void);
int capture_request_buffer(int index);
void capture_release_buffer(int index);
bool capture_start_scanning();
bool capture_stop_scanning();

bool capture_stop_preview();


typedef void (*vsync_notify_t)(int, void *);

unsigned long capture_register_vsync_notification(vsync_notify_t pfNotification, void * data);

#endif /*CAPTURE_H*/
