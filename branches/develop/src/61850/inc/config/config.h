#ifndef _CONFIG_H_
#define _CONFIG_H_

// the version of linuxc
#define VERSION_MAJOR "1"
#define VERSION_MINOR "0"
#define VERSION_MICRO "0"

#define PLATFORM 1

#if PLATFORM == 0
#define PLATFORM_DESK
#elif PLATFORM == 1
#define PLATFORM_T113
#else
#define PLATFORM_T113
#endif

#define ROOTPASSWD "@adminiPDU123"
#define ESNFILE "/usr/share/esn"

#endif /* _CONFIG_H_ */
