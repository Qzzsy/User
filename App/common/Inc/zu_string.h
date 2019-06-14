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

#ifndef _ZU_STRING_H_
#define _ZU_STRING_H_

#include "zu_def.h"
#include "zu_config.h"

#ifndef NULL
#define NULL        0
#endif

void *zu_memset(void *s, zu_int32_t c, zu_uint32_t count);
void *zu_memcpy(void *dst, const void *src, zu_uint32_t count);
void *zu_memmove(void *dest, const void *src, zu_uint32_t n);
char zu_memcmp(const void *cs, const void *ct, zu_uint32_t count);
zu_uint32_t zu_strnlen(const char *s, zu_uint32_t maxlen);
zu_uint32_t zu_strlen(const char *s);
char *zu_strstr(const char *s1, const char *s2);
char *zu_strstr_r(const char *s1, const char *s2, zu_uint32_t n);
zu_uint32_t zu_strcasecmp(const char *a, const char *b);
char *zu_strncpy(char *dst, const char *src, zu_uint32_t n);
char zu_strncmp(const char *cs, const char *ct, zu_base_t count);
zu_int32_t zu_strcmp(const char *cs, const char *ct);
void * zu_strtok_r(char * str, const char * delimiters, char ** saveptr);
void * zu_strtok(char * str, const char * delimiters, char * cnt);
int zu_atoi(const char *s);
zu_int32_t zu_atol(const char *s);
zu_int32_t zu_snprintf(char *buf, zu_int32_t size, const char *fmt, ...);
zu_int32_t zu_sprintf(char *buf, const char *fmt, ...);
void SetConsoleOutFunc(void (* ConsoleOut)(const char *buf, zu_uint32_t Length));
zu_int32_t zu_printf(const char *fmt, ...);
void zu_set_console_device(void (*_console_device)(const char * buf, zu_uint32_t zise));

#endif


