#ifndef __LORA_H__
#define __LORA_H__
extern "C" int lora_init(void);
extern "C"  int lora_write (char *buf, int count);
extern "C" int lora_read (char *buf, int count);
extern "C" int lora_set_power ( int count);
#endif 