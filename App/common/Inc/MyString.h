#ifndef _MY_STRING_H_
#define _MY_STRING_H_

/* 开辟4K的空间用于发送字符串 */
#define PRINTF_SEND_BUF_SIZE        4096

#ifndef NULL
#define NULL        0
#endif

/* 32bit CPU */
typedef long                            base_t;  
typedef unsigned long                   ubase_t; 

void *my_memset(void *s, int c, ubase_t count);
void *my_memcpy(void *dst, const void *src, ubase_t count);
void *my_memmove(void *dest, const void *src, ubase_t n);
char my_memcmp(const void *cs, const void *ct, ubase_t count);
unsigned long my_strnlen(const char *s, unsigned long  maxlen);
unsigned long my_strlen(const char *s);
char *my_strstr(const char *s1, const char *s2);
char *my_strstr_r(const char *s1, const char *s2, unsigned long n);
unsigned long  my_strcasecmp(const char *a, const char *b);
char *my_strncpy(char *dst, const char *src, ubase_t n);
char my_strncmp(const char *cs, const char *ct, long count);
long my_strcmp(const char *cs, const char *ct);
void * my_strtok_r(char * str, const char * delimiters, char ** saveptr);
void * my_strtok(char * str, const char * delimiters, char * cnt);
int my_atoi(const char *s);
long my_atol(const char *s);
long my_snprintf(char *buf, long size, const char *fmt, ...);
long my_sprintf(char *buf, const char *fmt, ...);
void SetConsoleOutFunc(void (* ConsoleOut)(const char *buf, unsigned long Length));
long my_printf(const char *fmt, ...);

#endif


