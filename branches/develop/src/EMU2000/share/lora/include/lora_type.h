#ifndef __TYPE_LORA_H__
#define __TYPE_LORA_H__
#ifndef bool
#define bool int
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#define PACK_LENGTH   255

#define USE_SX1276_RADIO 1
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;


    /* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;


#endif 