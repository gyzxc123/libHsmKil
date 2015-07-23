#ifndef _MEDIA_H_
#define _MEDIA_H_

#define MEDIA_ENTITY_ID_FLAG_NEXT		(1 << 31)

struct media_entity_desc {
	__u32 id;
	char name[32];
	__u32 type;
	__u32 revision;
	__u32 flags;
	__u32 group_id;
	__u16 pads;
	__u16 links;

	__u32 reserved[4];

	union {
		/* Node specifications */
		struct {
			__u32 major;
			__u32 minor;
		} v4l;
		struct {
			__u32 major;
			__u32 minor;
		} fb;
		struct {
			__u32 card;
			__u32 device;
			__u32 subdevice;
		} alsa;
		int dvb;

		/* Sub-device specifications */
		/* Nothing needed yet */
		__u8 raw[184];
	};
};

#define MEDIA_PAD_FLAG_INPUT			(1 << 0)
#define MEDIA_PAD_FLAG_OUTPUT			(1 << 1)

struct media_pad_desc {
	__u32 entity;		/* entity ID */
	__u16 index;		/* pad index */
	__u32 flags;		/* pad flags */
	__u32 reserved[2];
};

#define MEDIA_LINK_FLAG_ENABLED			(1 << 0)
#define MEDIA_LINK_FLAG_IMMUTABLE		(1 << 1)
#define MEDIA_LINK_FLAG_DYNAMIC			(1 << 2)

struct media_link_desc {
	struct media_pad_desc source;
	struct media_pad_desc sink;
	__u32 flags;
	__u32 reserved[2];
};

struct media_links_enum {
	__u32 entity;
	/* Should have enough room for pads elements */
	struct media_pad_desc  *pads;
	/* Should have enough room for links elements */
	struct media_link_desc  *links;
	__u32 reserved[4];
};

#define MEDIA_IOC_DEVICE_INFO		_IOWR('M', 1, struct media_device_info)
#define MEDIA_IOC_ENUM_ENTITIES		_IOWR('M', 2, struct media_entity_desc)
#define MEDIA_IOC_ENUM_LINKS		_IOWR('M', 3, struct media_links_enum)
#define MEDIA_IOC_SETUP_LINK		_IOWR('M', 4, struct media_link_desc)

#endif
