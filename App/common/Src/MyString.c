#include "MyString.h"

#ifdef __cplusplus
#define _ADDRESSOF(v) (&reinterpret_cast<const char &>(v))
#else
#define _ADDRESSOF(v) (&(v))
#endif

#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define _crt_va_start(ap, v)    (ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v))
#define _crt_va_arg(ap, t)      (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define _crt_va_end(ap)         (ap = (va_list)0)

#define _va_start   _crt_va_start /* windows stdarg.h */
#define _va_arg     _crt_va_arg
#define _va_end     _crt_va_end

#define ZEROPAD (1 << 0)    /* pad with zero */
#define SIGN    (1 << 1)    /* unsigned/signed long */
#define PLUS    (1 << 2)    /* show plus */
#define SPACE   (1 << 3)    /* space if plus */
#define LEFT    (1 << 4)    /* left justified */
#define SPECIAL (1 << 5)    /* 0x */
#define LARGE   (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed long     int32_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned long   uint32_t;
typedef int             bool_t;

/* 32bit CPU */
typedef long            base_t;
typedef unsigned long   ubase_t;

typedef char *va_list;

/**
 * This function will set the content of memory to specified value
 *
 * @param s the address of source memory
 * @param c the value shall be set in content
 * @param count the copied length
 *
 * @return the address of source memory
 */
void *my_memset(void *s, int c, ubase_t count)
{
#define LBLOCKSIZE (sizeof(int32_t))
#define UNALIGNED(X) ((int32_t)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < LBLOCKSIZE)

    int i;
    char *m = (char *)s;
    uint32_t buffer;
    uint32_t *aligned_addr;
    uint32_t d = c & 0xff;

    if (!TOO_SMALL(count) && !UNALIGNED(s))
    {
        /* If we get this far, we know that n is large and m is word-aligned. */
        aligned_addr = (uint32_t *)s;

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
 * This function will copy memory content from source address to destination
 * address.
 *
 * @param dst the address of destination memory
 * @param src  the address of source memory
 * @param count the copied length
 *
 * @return the address of destination memory
 */
void *my_memcpy(void *dst, const void *src, ubase_t count)
{
#define UNALIGNED(X, Y)                     \
    (((int32_t)X & (sizeof(int32_t) - 1)) | \
     ((int32_t)Y & (sizeof(int32_t) - 1)))
#define BIGBLOCKSIZE (sizeof(int32_t) << 2)
#define LITTLEBLOCKSIZE (sizeof(int32_t))
#define TOO_SMALL(LEN) ((LEN) < BIGBLOCKSIZE)

    char *dst_ptr = (char *)dst;
    char *src_ptr = (char *)src;
    int32_t *aligned_dst;
    int32_t *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!TOO_SMALL(len) && !UNALIGNED(src_ptr, dst_ptr))
    {
        aligned_dst = (int32_t *)dst_ptr;
        aligned_src = (int32_t *)src_ptr;

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
 * This function will move memory content from source address to destination
 * address.
 *
 * @param dest the address of destination memory
 * @param src  the address of source memory
 * @param n the copied length
 *
 * @return the address of destination memory
 */
void *my_memmove(void *dest, const void *src, ubase_t n)
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
 * This function will compare two areas of memory
 *
 * @param cs one area of memory
 * @param ct znother area of memory
 * @param count the size of the area
 *
 * @return the result
 */
char my_memcmp(const void *cs, const void *ct, ubase_t count)
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
 * The  strnlen()  function  returns the number of characters in the
 * string pointed to by s, excluding the terminating null byte ('\0'),
 * but at most maxlen.  In doing this, strnlen() looks only at the
 * first maxlen characters in the string pointed to by s and never
 * beyond s+maxlen.
 *
 * @param s the string
 * @param maxlen the max size
 * @return the length of string
 */
uint32_t my_strnlen(const char *s, uint32_t maxlen)
{
    const char *sc;

    for (sc = s; *sc != '\0' && sc - s < maxlen; ++sc) /* nothing */
        ;

    return sc - s;
}

/**
 * This function will return the length of a string, which terminate will
 * null character.
 *
 * @param s the string
 *
 * @return the length of string
 */
uint32_t my_strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc) /* nothing */
        ;

    return sc - s;
}

/**
 * This function will return the first occurrence of a string.
 *
 * @param s1 the source string
 * @param s2 the find string
 *
 * @return the first occurrence of a s2 in s1, or RT_NULL if no found.
 */
char *my_strstr(const char *s1, const char *s2)
{
    int l1, l2;

    l2 = my_strlen(s2);
    if (!l2)
        return (char *)s1;
    l1 = my_strlen(s1);
    while (l1 >= l2)
    {
        l1--;
        if (!my_memcmp(s1, s2, l2))
            return (char *)s1;
        s1++;
    }

    return NULL;
}

char *my_strstr_r(const char *s1, const char *s2, uint32_t n)
{
    int l1, l2;

    l2 = my_strlen(s2);
    if (!l2)
        return (char *)s1;

    l1 = n;

    while (l1 >= l2)
    {
        l1--;
        if (!my_memcmp(s1, s2, l2))
            return (char *)s1;
        s1++;
    }

    return NULL;
}
/**
 * This function will compare two strings while ignoring differences in case
 *
 * @param a the string to be compared
 * @param b the string to be compared
 *
 * @return the result
 */
uint32_t my_strcasecmp(const char *a, const char *b)
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
 * This function will copy string no more than n bytes.
 *
 * @param dst the string to copy
 * @param src the string to be copied
 * @param n the maximum copied length
 *
 * @return the result
 */
char *my_strncpy(char *dst, const char *src, ubase_t n)
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
 * This function will compare two strings with specified maximum length
 *
 * @param cs the string to be compared
 * @param ct the string to be compared
 * @param count the maximum compare length
 *
 * @return the result
 */
char my_strncmp(const char *cs, const char *ct, long count)
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
 * This function will compare two strings without specified length
 *
 * @param cs the string to be compared
 * @param ct the string to be compared
 *
 * @return the result
 */
long my_strcmp(const char *cs, const char *ct)
{
    while (*cs && *cs == *ct)
        cs++, ct++;

    return (*cs - *ct);
}

/**
 * @func    my_strtok_r
 * @brief   根据分割符号分割字符串
 * @param   str 源字符串
 * @param   delimiters 分割符
 * @param   saveptr 剩余字符串
 * @note  	
 * @retval  返回第一段字符串的首地址，返回RT_NULL为没有分割字符串
 */
void *my_strtok_r(char *str, const char *delimiters, char **saveptr)
{
    char *start_str = str;

    uint16_t cnt = 0, offiset = 0, len = 0;

    /* 计算分割符的长度 */
    len = my_strlen(delimiters);

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
 * @func    my_strtok
 * @brief   根据分割符号分割字符串
 * @param   str 源字符串
 * @param   delimiters 分割符
 * @param   cnt 分割后字符段的个数
 * @note  	需要释放内存
 * @retval  字符串第一段的首地址的指针的指针，返回RT_NULL为没有分割字符串
 */
void *my_strtok(char *str, const char *delimiters, char *cnt)
{
    static char *s;
    char *result = NULL;

    if (str != NULL)
    {
        s = str;
    }

    result = (char *)my_strtok_r(s, delimiters, &s);

    /* 返回分割后的数组指针 */
    return result;
}

static inline int _div(long *n, unsigned base)
{
    int __res;
    __res = ((unsigned long)*n) % (unsigned)base;
    *n = ((unsigned long)*n) / (unsigned)base;
    return __res;
}

#define do_div(n, base) _div(&n, base)

static inline int isdigit(int ch)
{
    return (ch >= '0') && (ch <= '9');
}

static int skip_atoi(const char **s)
{
    register int i = 0;

    while (isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

static long skip_atol(const char **s)
{
    register long i = 0;

    while (isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

int my_atoi(const char *s)
{
    return skip_atoi(&s);
}

long my_atol(const char *s)
{
    return skip_atol(&s);
}

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

static long _vsnprintf(char *buf, uint32_t size, const char *fmt, va_list args)
{
    int len;
    unsigned long num;
    int i, base;
    char *str, *end, c;
    const char *s;

    int flags; /* flags to Test_number() */

    int field_width; /* width of output field */
    int precision;   /* min. # of digits for integers; max
                  Test_number of chars for from string */
    int qualifier;   /* 'h', 'l', or 'L' for integer fields */

    str = buf;
    end = buf + size - 1;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end = ((char *)-1);
        size = end - buf;
    }

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
            field_width = _va_arg(args, int);
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
                precision = _va_arg(args, int);
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
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str <= end)
                        *str = ' ';
                    ++str;
                }
            }
            *str++ = (unsigned char)_va_arg(args, int);

            if (str <= end)
                *str = c;
            ++str;

            while (--field_width > 0)
            {
                if (str <= end)
                    *str = ' ';
                ++str;
            }
            continue;

        case 's':
            s = _va_arg(args, char *);

            if (!s)
                s = "(NULL)";

            len = my_strlen(s);

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
                               (unsigned long)_va_arg(args, void *), 16,
                               field_width, precision, flags);
            continue;

        case 'n':
            if (qualifier == 'l')
            {
                long *ip = _va_arg(args, long *);
                *ip = (str - buf);
            }
            else
            {
                int *ip = _va_arg(args, int *);
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
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
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
        if (qualifier == 'l')
        {
            num = _va_arg(args, unsigned long);
            if (flags & SIGN)
                num = (int)num;
        }
        else if (qualifier == 'h')
        {
            num = (unsigned short)_va_arg(args, int);
            if (flags & SIGN)
                num = (short)num;
        }
        else if (flags & SIGN)
            num = _va_arg(args, int);
        else
        {
            num = _va_arg(args, unsigned int);
            if (flags & SIGN)
                num = (int)num;
        }
        str = print_number(str, end, num, base, field_width, precision, flags);
    }

    if (str <= end)
        *str = '\0';
    else
        *end = '\0';

    return str - buf;
}

static int _vsprintf(char *buf, const char *format, va_list arg_ptr)
{
    return _vsnprintf(buf, (int)-1, format, arg_ptr);
}

long my_snprintf(char *buf, long size, const char *fmt, ...)
{
    int32_t n;
    va_list args;

    _va_start(args, fmt);
    n = _vsnprintf(buf, size, fmt, args);
    _va_end(args);

    return n;
}

long my_sprintf(char *buf, const char *fmt, ...)
{
    long n;

    //记录fmt对应的地址
    va_list args;

    //得到首个%对应的字符地址
    _va_start(args, fmt);
    n = _vsprintf(buf, fmt, args);
    _va_end(args);

    return n;
}

static void (*ConsoleOutFunc)(const char *buf, uint32_t Length);
void SetConsoleOutFunc(void (*ConsoleOut)(const char *buf, uint32_t Length))
{
    ConsoleOutFunc = ConsoleOut;
}

static char PrintfSendBuf[PRINTF_SEND_BUF_SIZE] = {'\0'};
long my_printf(const char *fmt, ...)
{
    long n;
    //记录fmt对应的地址
    va_list args;

    if (ConsoleOutFunc == NULL)
    {
        return NULL;
    }

    //得到首个%对应的字符地址
    _va_start(args, fmt);
    n = _vsprintf(PrintfSendBuf, fmt, args);
    _va_end(args);

    ConsoleOutFunc(PrintfSendBuf, n);

    return n;
}
