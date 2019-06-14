/**
 ******************************************************************************
 * @file      zu_string.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2019-06-12
 * @brief     该文件提供了string实现方法
 * @History
 * Date           Author    version    		Notes
 * 2019-06-12       ZSY     V1.0.0      first version.
 */

/* Includes ------------------------------------------------------------------*/

#include "zu_string.h"
#include "stdarg.h"
#include "stdint.h"

#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */
#define INT_TYPE    (1 << 7)
#define FLO_TYPE    (1 << 8)
#define DOU_TYPE    (1 << 9)

static char PrintfSendBuf[PRINTF_SEND_BUF_SIZE] = {'\0'};

typedef int             bool_t;

/**
 * @func    zu_memset
 * @brief   This function will set the content of memory to specified value
 * @param   s the address of source memory
 * @param   c the value shall be set in content
 * @param   count the copied length
 * @return  the address of source memory
 */
void *zu_memset(void *s, zu_int32_t c, zu_uint32_t count)
{
#define LBLOCKSIZE (sizeof(zu_int32_t))
#define UNALIGNED(X) ((zu_int32_t)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < LBLOCKSIZE)

    int i;
    char *m = (char *)s;
    zu_uint32_t buffer;
    zu_uint32_t *aligned_addr;
    zu_uint32_t d = c & 0xff;

    if (!TOO_SMALL(count) && !UNALIGNED(s))
    {
        /* If we get this far, we know that n is large and m is word-aligned. */
        aligned_addr = (zu_uint32_t *)s;

        /* Store D into each char sized location in BUFFER so that
         * we can set large blocks quickly.
         */
        if (LBLOCKSIZE == 4)
        {
            buffer = (d << 8) | d;
            buffer |= (buffer << 16);
        }
        else
        {
            buffer = 0;
            for (i = 0; i < LBLOCKSIZE; i++)
                buffer = (buffer << 8) | d;
        }

        while (count >= LBLOCKSIZE * 4)
        {
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            count -= 4 * LBLOCKSIZE;
        }

        while (count >= LBLOCKSIZE)
        {
            *aligned_addr++ = buffer;
            count -= LBLOCKSIZE;
        }

        /* Pick up the remainder with a bytewise loop. */
        m = (char *)aligned_addr;
    }

    while (count--)
    {
        *m++ = (char)d;
    }

    return s;

#undef LBLOCKSIZE
#undef UNALIGNED
#undef TOO_SMALL
}

/**
 * @func    zu_memcpy
 * @brief   This function will copy memory content from source address to destination
 *          address.
 * @param   dst the address of destination memory
 * @param   src  the address of source memory
 * @param   count the copied length
 * @return  the address of destination memory
 */
void *zu_memcpy(void *dst, const void *src, zu_uint32_t count)
{
#define UNALIGNED(X, Y)                     \
    (((zu_int32_t)X & (sizeof(zu_int32_t) - 1)) | \
     ((zu_int32_t)Y & (sizeof(zu_int32_t) - 1)))
#define BIGBLOCKSIZE (sizeof(zu_int32_t) << 2)
#define LITTLEBLOCKSIZE (sizeof(zu_int32_t))
#define TOO_SMALL(LEN) ((LEN) < BIGBLOCKSIZE)

    char *dst_ptr = (char *)dst;
    char *src_ptr = (char *)src;
    zu_int32_t *aligned_dst;
    zu_int32_t *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!TOO_SMALL(len) && !UNALIGNED(src_ptr, dst_ptr))
    {
        aligned_dst = (zu_int32_t *)dst_ptr;
        aligned_src = (zu_int32_t *)src_ptr;

        /* Copy 4X long words at a time if possible. */
        while (len >= BIGBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible. */
        while (len >= LITTLEBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier. */
        dst_ptr = (char *)aligned_dst;
        src_ptr = (char *)aligned_src;
    }

    while (len--)
        *dst_ptr++ = *src_ptr++;

    return dst;
#undef UNALIGNED
#undef BIGBLOCKSIZE
#undef LITTLEBLOCKSIZE
#undef TOO_SMALL
}

/**
 * @func    zu_memmove
 * @brief   This function will move memory content from source address to destination
 *          address.
 * @param   dest the address of destination memory
 * @param   src  the address of source memory
 * @param   n the copied length
 * @return  the address of destination memory
 */
void *zu_memmove(void *dest, const void *src, zu_uint32_t n)
{
    char *tmp = (char *)dest, *s = (char *)src;

    if (s < tmp && tmp < s + n)
    {
        tmp += n;
        s += n;

        while (n--)
            *(--tmp) = *(--s);
    }
    else
    {
        while (n--)
            *tmp++ = *s++;
    }

    return dest;
}

/**
 * @func    zu_memcmp
 * @brief   This function will compare two areas of memory
 * @param   cs one area of memory
 * @param   ct znother area of memory
 * @param   count the size of the area
 * @return  the result
 */
char zu_memcmp(const void *cs, const void *ct, zu_uint32_t count)
{
    const unsigned char *su1, *su2;
    char res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
    {
        if ((res = *su1 - *su2) != 0)
            break;
    }

    return res;
}

/**
 * @func    zu_strnlen
 * @brief   The  strnlen()  function  returns the number of characters in the
 *          string pointed to by s, excluding the terminating null byte ('\0'),
 *          but at most maxlen.  In doing this, strnlen() looks only at the
 *          first maxlen characters in the string pointed to by s and never
 *          beyond s+maxlen.
 * @param   s the string
 * @param   maxlen the max size
 * @return  the length of string
 */
zu_uint32_t zu_strnlen(const char *s, zu_uint32_t maxlen)
{
    const char *sc;

    for (sc = s; *sc != '\0' && sc - s < maxlen; ++sc) /* nothing */
        ;

    return sc - s;
}

/**
 * @func    zu_strlen
 * @brief   This function will return the length of a string, which terminate will
 *          null character.
 * @param   s the string
 * @return  the length of string
 */
zu_uint32_t zu_strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc) /* nothing */
        ;

    return sc - s;
}

/**
 * @func    zu_strstr
 * @brief   This function will return the first occurrence of a string.
 * @param   s1 the source string
 * @param   s2 the find string
 * @return  the first occurrence of a s2 in s1, or RT_NULL if no found.
 */
char *zu_strstr(const char *s1, const char *s2)
{
    int l1, l2;

    l2 = zu_strlen(s2);
    if (!l2)
        return (char *)s1;
    l1 = zu_strlen(s1);
    while (l1 >= l2)
    {
        l1--;
        if (!zu_memcmp(s1, s2, l2))
            return (char *)s1;
        s1++;
    }

    return NULL;
}

/**
 * @func    zu_strstr_r
 * @brief   This function will return the first occurrence of a string.
 * @param   s1 the source string
 * @param   s2 the find string
 * @return  the first occurrence of a s2 in s1, or RT_NULL if no found.
 */
char *zu_strstr_r(const char *s1, const char *s2, zu_uint32_t n)
{
    int l1, l2;

    l2 = zu_strlen(s2);
    if (!l2)
        return (char *)s1;

    l1 = n;

    while (l1 >= l2)
    {
        l1--;
        if (!zu_memcmp(s1, s2, l2))
            return (char *)s1;
        s1++;
    }

    return NULL;
}

/**
 * @func    zu_strcasecmp
 * @brief   This function will compare two strings while ignoring differences in case
 * @param   a the string to be compared
 * @param   b the string to be compared
 * @return  the result
 */
zu_uint32_t zu_strcasecmp(const char *a, const char *b)
{
    int ca, cb;

    do
    {
        ca = *a++ & 0xff;
        cb = *b++ & 0xff;
        if (ca >= 'A' && ca <= 'Z')
            ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z')
            cb += 'a' - 'A';
    } while (ca == cb && ca != '\0');

    return ca - cb;
}

/**
 * @func    zu_strncpy
 * @brief   This function will copy string no more than n bytes.
 * @param   dst the string to copy
 * @param   src the string to be copied
 * @param   n the maximum copied length
 * @return  the result
 */
char *zu_strncpy(char *dst, const char *src, zu_uint32_t n)
{
    if (n != 0)
    {
        char *d = dst;
        const char *s = src;

        do
        {
            if ((*d++ = *s++) == 0)
            {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                    *d++ = 0;
                break;
            }
        } while (--n != 0);
    }

    return (dst);
}

/**
 * @func    zu_strncmp
 * @brief   This function will compare two strings with specified maximum length
 * @param   cs the string to be compared
 * @param   ct the string to be compared
 * @param   count the maximum compare length
 * @return  the result
 */
char zu_strncmp(const char *cs, const char *ct, zu_int32_t count)
{
    register signed char __res = 0;

    while (count)
    {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
        count--;
    }

    return __res;
}

/**
 * @func    zu_strcmp
 * @brief   This function will compare two strings without specified length
 * @param   cs the string to be compared
 * @param   ct the string to be compared
 * @return  the result
 */
long zu_strcmp(const char *cs, const char *ct)
{
    while (*cs && *cs == *ct)
        cs++, ct++;

    return (*cs - *ct);
}

/**
 * @func    zu_strtok_r
 * @brief   根据分割符号分割字符串
 * @param   str 源字符串
 * @param   delimiters 分割符
 * @param   saveptr 剩余字符串
 * @note  	
 * @retval  返回第一段字符串的首地址，返回RT_NULL为没有分割字符串
 */
void *zu_strtok_r(char *str, const char *delimiters, char **saveptr)
{
    char *start_str = str;

    zu_uint16_t cnt = 0, offiset = 0, len = 0;

    /* 计算分割符的长度 */
    len = zu_strlen(delimiters);

    while (*(str + cnt) != '\0')
    {
        if (*(str + cnt) == *(delimiters + offiset))
        {
            offiset++;

            if (offiset == len)
            {
                /* 分割 */
                start_str[++cnt - offiset] = 0;

                *saveptr = start_str + cnt;

                /* 分割成功返回第一段的首地址 */
                return start_str;
            }
        }
        else
        {
            offiset = 0;
        }
        cnt++;
    }

    return NULL;
}

/**
 * @func    zu_strtok
 * @brief   根据分割符号分割字符串
 * @param   str 源字符串
 * @param   delimiters 分割符
 * @param   cnt 分割后字符段的个数
 * @note  	需要释放内存
 * @retval  字符串第一段的首地址的指针的指针，返回RT_NULL为没有分割字符串
 */
void *zu_strtok(char *str, const char *delimiters, char *cnt)
{
    static char *s;
    char *result = NULL;

    if (str != NULL)
    {
        s = str;
    }

    result = (char *)zu_strtok_r(s, delimiters, &s);

    /* 返回分割后的数组指针 */
    return result;
}

/**
 * @func    _div
 * @brief   分割整型为单个数字
 * @param   n 数字
 * @param   base 单位
 * @retval  结果
 */
static inline int _div(long *n, unsigned base)
{
    int __res;
    __res = ((unsigned long)*n) % (unsigned)base;
    *n = ((unsigned long)*n) / (unsigned)base;
    return __res;
}

#define do_div(n, base) _div(&n, base)

/**
 * @func    isdigit
 * @brief   判断是否为数字
 * @param   ch 输入的字符
 * @retval  1 是； 0 不是
 */
static inline int isdigit(int ch)
{
    return (ch >= '0') && (ch <= '9');
}

/**
 * @func    zu_atol
 * @brief   字符串转长整型
 * @param   s 输入的字符串
 * @retval  转换结果
 */
static int skip_atoi(const char **s)
{
    register int i = 0;

    while (isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

/**
 * @func    zu_atoi
 * @brief   字符串转整型
 * @param   s 输入的字符串
 * @retval  转换结果
 */
static long skip_atol(const char **s)
{
    register long i = 0;

    while (isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

/**
 * @func    zu_atoi
 * @brief   字符串转整型
 * @param   s 输入的字符串
 * @retval  转换结果
 */
int zu_atoi(const char *s)
{
    return skip_atoi(&s);
}

/**
 * @func    zu_atol
 * @brief   字符串转长整型
 * @param   s 输入的字符串
 * @retval  转换结果
 */
long zu_atol(const char *s)
{
    return skip_atol(&s);
}

/**
 * @func    print_number
 * @brief   字符串构建函数（具体实现，浮点打印）
 * @param   buf 输出缓存
 * @param   end 输出缓存的结尾
 * @param   num 需要转换的数字
 * @param   base 基本单位（典型是10）
 * @param   size 显示的区域（最小）
 * @param   precision 精度
 * @param   type flags
 * @retval  处理完成的字符串
 */
static char *print_flt(char *buf, 
                 char *end, 
                 double num,
                 int base,
                 int size,
                 int precision,
                 int type)
{
    char sign, tmp[66], flo_tmp[66];
    const char *digits = "0123456789";
    int i, j;

    if (base != 10)
    {
        return 0;
    }

    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
            size--;
        }
        else if (type & PLUS)
        {
            sign = '+';
            size--;
        }
        else if (type & SPACE)
        {
            sign = ' ';
            size--;
        }
    }

    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
    {
        long _tmp = num;
        tmp[i] = '0';
        while (_tmp != 0)
            tmp[i++] = digits[do_div(_tmp, base)];
        if (_tmp == 0)
            i++;
        flo_tmp[0] = '.';
        if (type & DOU_TYPE)
        {
            for (j = 1; j <= 12; j++)
            {
                num *= 10;
                flo_tmp[j] = digits[(uint32_t)num % 10];
            }
            
            if (precision > 12)
                precision = 12;
        }
        else if (type & FLO_TYPE)
        {
            for (j = 1; j <= 6; j++)
            {
                num *= 10;
                flo_tmp[j] = digits[(uint32_t)num % 10];
            }
            
            if (precision > 6)
                precision = 6;
        }
    }

    if (sign)
    {
        if (buf <= end)
        {
            *buf = sign;
            --size;
        }
        ++buf;
    }

    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf <= end)
                *buf = ' ';
            ++buf;
        }
    }
    
    while (i-- > 0)
    {
        if (buf <= end)
            *buf = tmp[i];
        ++buf;
        size--;
    }
    
    i = 0;
    while(i < (precision + 1))
    {
        if (buf <= end)
            *buf = flo_tmp[i++];
        ++buf;
        size--;
    }

    while (size-- > 0)
    {
        if (buf <= end)
            *buf = ' ';
        ++buf;
    }
    return buf;
}

/**
 * @func    print_number
 * @brief   字符串构建函数（具体实现，整型打印）
 * @param   buf 输出缓存
 * @param   end 输出缓存的结尾
 * @param   num 需要转换的数字
 * @param   base 基本单位（典型是10）
 * @param   size 显示的区域（最小）
 * @param   precision 精度
 * @param   type flags
 * @retval  处理完成的字符串
 */
static char *print_number(char *buf,
                          char *end,
                          long num,
                          int base,
                          int size,
                          int precision,
                          int type)
{
    char c, sign, tmp[66];
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
    {
        return 0;
    }

    c = (type & ZEROPAD) ? '0' : ' ';

    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
            size--;
        }
        else if (type & PLUS)
        {
            sign = '+';
            size--;
        }
        else if (type & SPACE)
        {
            sign = ' ';
            size--;
        }
    }
    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
    {
        while (num != 0)
            tmp[i++] = digits[do_div(num, base)];
    }

    if (i > precision)
        precision = i;

    size -= precision;

    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
            size--;

        while (size-- > 0)
            *buf++ = ' ';
    }

    if (sign)
    {
        if (buf <= end)
        {
            *buf = sign;
            --size;
        }
        ++buf;
    }

    if (type & SPECIAL)
    {
        if (base == 8)
        {
            if (buf <= end)
                *buf = '0';
            ++buf;
        }
        else if (base == 16)
        {
            if (buf <= end)
                *buf = '0';
            ++buf;
            if (buf <= end)
                *buf = type & LARGE ? 'X' : 'x';
            ++buf;
        }
    }

    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf <= end)
                *buf = c;
            ++buf;
        }
    }

    while (i < precision--)
    {
        if (buf <= end)
            *buf = '0';
        ++buf;
    }

    while (i-- > 0)
    {
        if (buf <= end)
            *buf = tmp[i];
        ++buf;
    }

    while (size-- > 0)
    {
        if (buf <= end)
            *buf = ' ';
        ++buf;
    }
    return buf;
}

/**
 * @func    _vsnprintf
 * @brief   字符串构建函数（具体实现）
 * @param   buf 输出缓存
 * @param   size 输出缓存的大小
 * @param   fmt 输入的字符串
 * @param   args 可变参数的参数列表
 * @retval  字符串的长度
 */
static long _vsnprintf(char *buf, uint32_t size, const char *fmt, va_list args)
{
    int len;
    unsigned long num;
    double fd_num;
    int i, base;
    char *str, *end, c;
    const char *s;

    int flags; /* flags to print_number() */

    int field_width; /* width of output field */
    int precision;   /* min. # of digits for integers; max
                        print_number of chars for from string */
    int qualifier;   /* 'h', 'l', or 'L' for integer fields */

    str = buf;
    end = buf + size - 1;
    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end = ((char *)-1);
        size = end - buf;
    }
    
    zu_memset(buf, 0, size);

    for (; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str <= end)
                *str = *fmt;
            ++str;
            continue;
        }

        /* process flags */
        flags = 0;
    repeat:
        ++fmt; /* this also skips first '%' */
        switch (*fmt)
        {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++fmt;
            if (isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }

            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
        {
            qualifier = *fmt;
            ++fmt;
        }

        /* default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))            /* 右对齐 */
            {
                while (--field_width > 0)
                {
                    if (str <= end)
                        *str = ' ';
                    ++str;
                }
            }
            c = (unsigned char)va_arg(args, int);

            if (str <= end)                 /* 安全检测 */
                *str = c;
            ++str;

            while (--field_width > 0)
            {
                if (str <= end)             /* 安全检测 */
                    *str = ' ';
                ++str;
            }
            continue;

        case 's':
            s = va_arg(args, char *);

            if (!s)
                s = "(NULL)";

            len = zu_strlen(s);

            if (precision > 0 && len > precision)
                len = precision;

            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str <= end)
                        *str = ' ';
                    ++str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str <= end)
                    *str = *s;
                ++str;
                ++s;
            }
            while (len < field_width--)
            {
                if (str <= end)
                    *str = ' ';
                ++str;
            }
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
                flags |= ZEROPAD;
            }
            str = print_number(str, end,
                               (unsigned long)va_arg(args, void *), 16,
                               field_width, precision, flags);
            continue;

        case 'n':
            if (qualifier == 'l')
            {
                long *ip = va_arg(args, long *);
                *ip = (str - buf);
            }
            else
            {
                int *ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            continue;

        case '%':
            if (str <= end)
                *str = '%';
            ++str;
            continue;

        /* integer Test_number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            flags |= INT_TYPE;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            flags |= INT_TYPE;
            break;
        case 'E':
        case 'G':
        case 'e':
        case 'f':
        case 'g':
            flags |= FLO_TYPE;
            break;
        default:
            if (str <= end)
                *str = '%';
            ++str;

            if (*fmt)
            {
                if (str <= end)
                    *str = *fmt;
                ++str;
            }
            else
            {
                --fmt;
            }
            continue;
        }
        if (flags & INT_TYPE)
        {
            if (qualifier == 'l')
            {
                num = va_arg(args, unsigned long);
                if (flags & SIGN)
                    num = (int)num;
            }
            else if (qualifier == 'h')
            {
                num = (unsigned short)va_arg(args, int);
                if (flags & SIGN)
                    num = (short)num;
            }
            else if (flags & SIGN)
                num = va_arg(args, int);
            else
            {
                num = va_arg(args, unsigned int);
                if (flags & SIGN)
                    num = (int)num;
            }
            str = print_number(str, end, num, base, field_width, precision, flags);
        }
        else if (flags & FLO_TYPE)
        {
            if (qualifier == 'l')
            {
                flags |= DOU_TYPE;
                if (precision <= 0)
                    precision = 12;
            }
            else
            {
                if (precision <= 0)
                    precision = 6;
            }
            fd_num = va_arg(args, double);
            str = print_flt(str, end, fd_num, base, field_width, precision, flags);
        }
    }

    if (str <= end)
        *str = '\0';
    else
        *end = '\0';

    return str - buf;
}

/**
 * @func    _vsprintf
 * @brief   字符串构建函数
 * @param   buf 输出缓存
 * @param   format 输入的字符串
 * @param   arg_ptr 可变参数的参数列表
 * @note    不安全，不建议使用
 * @retval  字符串的长度
 */
static int _vsprintf(char *buf, const char *format, va_list arg_ptr)
{
    return _vsnprintf(buf, (int)-1, format, arg_ptr);
}

/**
 * @func    zu_snprintf
 * @brief   字符串构建函数
 * @param   buf 输出缓存
 * @param   size 输出缓存最大的大小
 * @param   fmt 输入的字符串
 * @retval  字符串的长度
 */
zu_int32_t zu_snprintf(char *buf, zu_int32_t size, const char *fmt, ...)
{
    int32_t n;
    va_list args;

    va_start(args, fmt);
    n = _vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}

/**
 * @func    zu_sprintf
 * @brief   字符串构建函数
 * @param   buf 输出缓存
 * @param   fmt 输入的字符串
 * @note    不安全，不建议使用
 * @retval  字符串的长度
 */
long zu_sprintf(char *buf, const char *fmt, ...)
{
    long n;

    //记录fmt对应的地址
    va_list args;

    //得到首个%对应的字符地址
    va_start(args, fmt);
    n = _vsprintf(buf, fmt, args);
    va_end(args);

    return n;
}

void (*console_device)(void * buf, zu_uint32_t zise);
/**
 * @func    zu_printf
 * @brief   print函数
 * @param   fmt 输入的字符串
 * @retval  字符串的长度
 */
long zu_printf(const char *fmt, ...)
{
    long n;
    //记录fmt对应的地址
    va_list args;

    //得到首个%对应的字符地址
    va_start(args, fmt);
    n = _vsnprintf(PrintfSendBuf, sizeof(PrintfSendBuf) - 1, fmt, args);
    va_end(args);
#ifdef USE_CONSOLE    
    if (console_device != ZU_NULL)
    {
        console_device(PrintfSendBuf, n);
    }
#endif
    return n;
}


void zu_set_console_device(void (*_console_device)(const char * buf, zu_uint32_t zise))
{
    console_device = _console_device;
}
