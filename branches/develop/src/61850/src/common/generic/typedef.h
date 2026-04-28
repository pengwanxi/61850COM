/**
 *   \file datatype.h
 *   \brief 提供类型相关头文件或类型
 */
#ifndef _typedef_H_
#define _typedef_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "config_macro.h"

//定义数据类型
typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef bool BOOL;
typedef int HANDLE;
typedef void *LPVOID;

#define UINT8 BYTE
#define UINT16 WORD

#ifndef FALSE
#define FALSE false
#endif
#ifndef TRUE
#define TRUE true
#endif

#define OK 0
#define ERROR -1

////////////////////////////////////////////////////////////////////////////////
// 全局数据类型及数据结构定义
typedef signed char s08_t;
typedef signed short s16_t;
typedef signed int s32_t;
typedef unsigned char u08_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef long long int s64_t;
typedef unsigned long long int u64_t;
typedef float f32_t;
typedef double f64_t;

typedef signed char s8;
typedef signed short s16;
/* typedef signed int s32_t; */
typedef unsigned char u8;
typedef unsigned short u16;
/* typedef unsigned int u32_t; */
typedef long long int s64;
typedef unsigned long long int u64;
typedef float f32;
typedef double f64;

#include "maths.h"

#endif /* _typedef_H_ */
