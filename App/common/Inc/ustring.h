/**
 ******************************************************************************
 * @file      zu_string.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2019-06-12
 * @brief     该文件提供了string方法相关的API
 * @History
 * Date           Author    version    		Notes
 * 2019-06-12       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _USTRING_H_
#define _USTRING_H_

#include "udef.h"
#include "uconfig.h"
#include "stdarg.h"
#include "stdint.h"

#ifndef NULL
#define NULL        0
#endif

void *umemset(void *s, int32_t c, uint32_t count);
void *umemcpy(void *dst, const void *src, uint32_t count);
void *umemmove(void *dest, const void *src, uint32_t n);
char umemcmp(const void *cs, const void *ct, uint32_t count);
uint32_t ustrnlen(const char *s, uint32_t maxlen);
uint32_t ustrlen(const char *s);
char *ustrstr(const char *s1, const char *s2);
char *ustrstr_r(const char *s1, const char *s2, uint32_t n);
uint32_t ustrcasecmp(const char *a, const char *b);
char *ustrncpy(char *dst, const char *src, uint32_t n);
char ustrncmp(const char *cs, const char *ct, base_t count);
int32_t ustrcmp(const char *cs, const char *ct);
void * ustrtok_r(char * str, const char * delimiters, char ** saveptr);
void * ustrtok(char * str, const char * delimiters, char * cnt);
int uatoi(const char *s);
long uatol(const char *s);
int uvsnprintf(char *buf, uint32_t size, const char *format, va_list arg_ptr);
int32_t usnprintf(char *buf, int32_t size, const char *fmt, ...);
int32_t usprintf(char *buf, const char *fmt, ...);
int32_t uprintf(const char *fmt, ...);
#ifdef USING_CONSOLE   
void set_console_device(void (*_console_device)(const char * buf, uint32_t zise));
#endif

#endif


