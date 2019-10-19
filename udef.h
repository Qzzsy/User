#ifndef _UDEF_H_
#define _UDEF_H_

#include "uconfig.h"

#ifndef UNULL
#define UNULL     (void *)(0)
#endif

#ifndef __IO
#define __IO    (volatile)
#endif

#define UASSERT(EX)                                                         \
if (!(EX))                                                                    \
{                                                                             \
}

#ifdef USING_USER_BASE_TYPE
/* basic data type definitions */
typedef signed   char                   int8_t;      /**<  8bit integer type */
typedef signed   short                  int16_t;     /**< 16bit integer type */
typedef signed   int                    int32_t;     /**< 32bit integer type */
typedef unsigned char                   uint8_t;     /**<  8bit unsigned integer type */
typedef unsigned short                  uint16_t;    /**< 16bit unsigned integer type */
typedef unsigned int                    uint32_t;    /**< 32bit unsigned integer type */
typedef int                             bool_t;      /**< boolean type */

/* 32bit CPU */
typedef int                             base_t;      /**< Nbit CPU related date type */
typedef unsigned int                    ubase_t;     /**< Nbit unsigned CPU related data type */

typedef base_t                       err_t;       /**< Type for error number */
typedef uint32_t                     time_t;      /**< Type for time stamp */
typedef uint32_t                     tick_t;      /**< Type for tick count */
typedef base_t                       flag_t;      /**< Type for flags */
typedef ubase_t                      size_t;      /**< Type for size number */
typedef ubase_t                      dev_t;       /**< Type for device */
typedef base_t                       off_t;       /**< Type for offset */

#endif
// /**
//  * device (I/O) class type
//  */
// enum zu_device_class_type
// {
//     ZU_Device_Class_Char = 0,                           /**< character device */
//     ZU_Device_Class_Block,                              /**< block device */
//     ZU_Device_Class_NetIf,                              /**< net interface */
//     ZU_Device_Class_MTD,                                /**< memory device */
//     ZU_Device_Class_CAN,                                /**< CAN device */
//     ZU_Device_Class_RTC,                                /**< RTC device */
//     ZU_Device_Class_Sound,                              /**< Sound device */
//     ZU_Device_Class_Graphic,                            /**< Graphic device */
//     ZU_Device_Class_I2CBUS,                             /**< I2C bus device */
//     ZU_Device_Class_USBDevice,                          /**< USB slave device */
//     ZU_Device_Class_USBHost,                            /**< USB host bus */
//     ZU_Device_Class_SPIBUS,                             /**< SPI bus device */
//     ZU_Device_Class_SPIDevice,                          /**< SPI device */
//     ZU_Device_Class_SDIO,                               /**< SDIO bus device */
//     ZU_Device_Class_PM,                                 /**< PM pseudo device */
//     ZU_Device_Class_Pipe,                               /**< Pipe device */
//     ZU_Device_Class_Portal,                             /**< Portal device */
//     ZU_Device_Class_Timer,                              /**< Timer device */
//     ZU_Device_Class_Miscellaneous,                      /**< Miscellaneous device */
//     ZU_Device_Class_Unknown                             /**< unknown device */
// };

// /**
//  * device flags defitions
//  */
// #define ZU_DEVICE_FLAG_DEACTIVATE       0x000           /**< device is not not initialized */

// #define ZU_DEVICE_FLAG_RDONLY           0x001           /**< read only */
// #define ZU_DEVICE_FLAG_WRONLY           0x002           /**< write only */
// #define ZU_DEVICE_FLAG_RDWR             0x003           /**< read and write */

// #define ZU_DEVICE_FLAG_REMOVABLE        0x004           /**< removable device */
// #define ZU_DEVICE_FLAG_STANDALONE       0x008           /**< standalone device */
// #define ZU_DEVICE_FLAG_ACTIVATED        0x010           /**< device is activated */
// #define ZU_DEVICE_FLAG_SUSPENDED        0x020           /**< device is suspended */
// #define ZU_DEVICE_FLAG_STREAM           0x040           /**< stream mode */

// #define ZU_DEVICE_FLAG_INT_RX           0x100           /**< INT mode on Rx */
// #define ZU_DEVICE_FLAG_DMA_RX           0x200           /**< DMA mode on Rx */
// #define ZU_DEVICE_FLAG_INT_TX           0x400           /**< INT mode on Tx */
// #define ZU_DEVICE_FLAG_DMA_TX           0x800           /**< DMA mode on Tx */

// #define ZU_DEVICE_OFLAG_CLOSE           0x000           /**< device is closed */
// #define ZU_DEVICE_OFLAG_RDONLY          0x001           /**< read only access */
// #define ZU_DEVICE_OFLAG_WRONLY          0x002           /**< write only access */
// #define ZU_DEVICE_OFLAG_RDWR            0x003           /**< read and write */
// #define ZU_DEVICE_OFLAG_OPEN            0x008           /**< device is opened */
// #define ZU_DEVICE_OFLAG_MASK            0xf0f           /**< mask of open flag */

// /**
//  * general device commands
//  */
// #define ZU_DEVICE_CTRL_RESUME           0x01            /**< resume device */
// #define ZU_DEVICE_CTRL_SUSPEND          0x02            /**< suspend device */
// #define ZU_DEVICE_CTRL_CONFIG           0x03            /**< configure device */

// #define ZU_DEVICE_CTRL_SET_INT          0x10            /**< set interrupt */
// #define ZU_DEVICE_CTRL_CLR_INT          0x11            /**< clear interrupt */
// #define ZU_DEVICE_CTRL_GET_INT          0x12            /**< get interrupt status */

// /**
//  * special device commands
//  */
// #define ZU_DEVICE_CTRL_CHAR_STREAM      0x10            /**< stream mode on char device */
// #define ZU_DEVICE_CTRL_BLK_GETGEOME     0x10            /**< get geometry information   */
// #define ZU_DEVICE_CTRL_BLK_SYNC         0x11            /**< flush data to block device */
// #define ZU_DEVICE_CTRL_BLK_ERASE        0x12            /**< erase block on block device */
// #define ZU_DEVICE_CTRL_BLK_AUTOREFRESH  0x13            /**< block device : enter/exit auto refresh mode */
// #define ZU_DEVICE_CTRL_NETIF_GETMAC     0x10            /**< get mac address */
// #define ZU_DEVICE_CTRL_MTD_FORMAT       0x10            /**< format a MTD device */
// #define ZU_DEVICE_CTRL_RTC_GET_TIME     0x10            /**< get time */
// #define ZU_DEVICE_CTRL_RTC_SET_TIME     0x11            /**< set time */
// #define ZU_DEVICE_CTRL_RTC_GET_ALARM    0x12            /**< get alarm */
// #define ZU_DEVICE_CTRL_RTC_SET_ALARM    0x13            /**< set alarm */

// /* error code definitions */
#define UEOK                          0               /**< There is no error */
#define UERROR                        1               /**< A generic error happens */
// #define ZU_ETIMEOUT                     2               /**< Timed out */
// #define ZU_EFULL                        3               /**< The resource is full */
// #define ZU_EEMPTY                       4               /**< The resource is empty */
// #define ZU_ENOMEM                       5               /**< No memory */
// #define ZU_ENOSYS                       6               /**< No system */
// #define ZU_EBUSY                        7               /**< Busy */
// #define ZU_EIO                          8               /**< IO error */
// #define ZU_EINTR                        9               /**< Interrupted system call */
 #define UEINVAL                       10              /**< Invalid argument */

#define UENABLE                       (1 << 0)
#define UDISABLE                      (1 << 1)
// /**
//  * Base structure of Kernel object
//  */
// struct zu_object
// {
//     char            name[ZU_NAME_MAX];          /**< name of kernel object */
//     zu_uint8_t      type;                       /**< type of kernel object */
//     zu_uint8_t      flag;                       /**< flag of kernel object */

//     zu_list_t     list;                         /**< list node of kernel object */
// };

// typedef struct zu_object zu_object_t;
// typedef struct zu_object *p_zu_object_t;

// typedef struct zu_device zu_device_t;
// typedef struct zu_device *p_zu_device_t;
// /**
//  * Device structure
//  */
// struct zu_device
// {
//     struct zu_object          parent;                   /**< inherit from rt_object */

//     enum zu_device_class_type type;                     /**< device type */
//     zu_uint16_t               flag;                     /**< device flag */
//     zu_uint16_t               open_flag;                /**< device open flag */

//     zu_uint8_t                ref_count;                /**< reference count */
//     zu_uint8_t                device_id;                /**< 0 - 255 */

//     /* device call back */
//     zu_err_t (*rx_indicate)(p_zu_device_t dev, zu_size_t size);
//     zu_err_t (*tx_complete)(p_zu_device_t dev, void *buffer);

//     /* common device interface */
//     zu_err_t  (*init)   (p_zu_device_t dev);
//     zu_err_t  (*open)   (p_zu_device_t dev, zu_uint16_t oflag);
//     zu_err_t  (*close)  (p_zu_device_t dev);
//     zu_size_t (*read)   (p_zu_device_t dev, zu_off_t pos, void *buffer, zu_size_t size);
//     zu_size_t (*write)  (p_zu_device_t dev, zu_off_t pos, const void *buffer, zu_size_t size);
//     zu_err_t  (*control)(p_zu_device_t dev, int cmd, void *args);

//     void                     *user_data;                /**< device private data */
// };


// /* Compiler Related Definitions */
// #ifdef __CC_ARM                         /* ARM Compiler */
//     #include <stdarg.h>
//     #define SECTION(x)                  __attribute__((section(x)))
//     #define ZU_UNUSED                   __attribute__((unused))
//     #define ZU_USED                     __attribute__((used))
//     #define ALIGN(n)                    __attribute__((aligned(n)))
//     #define ZU_WEAK                     __weak
//     #define zu_inline                   static __inline

// //    #define RTT_API                     __declspec(dllexport)

// #elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
//     #include <stdarg.h>
//     #define SECTION(x)                  @ x
//     #define ZU_UNUSED
//     #define ZU_USED                     __root
//     #define PRAGMA(x)                   _Pragma(#x)
//     #define ALIGN(n)                    PRAGMA(data_alignment=n)
//     #define ZU_WEAK                     __weak
//     #define zu_inline                   static inline
// //    #define RTT_API

// #elif defined (__GNUC__)                /* GNU GCC Compiler */
//     #ifdef ZU_USING_NEWLIB
//         #include <stdarg.h>
//     #else
//         /* the version of GNU GCC must be greater than 4.x */
//         typedef __builtin_va_list   __gnuc_va_list;
//         typedef __gnuc_va_list      va_list;
//         #define va_start(v,l)       __builtin_va_start(v,l)
//         #define va_end(v)           __builtin_va_end(v)
//         #define va_arg(v,l)         __builtin_va_arg(v,l)
//     #endif

//     #define SECTION(x)                  __attribute__((section(x)))
//     #define ZU_UNUSED                   __attribute__((unused))
//     #define ZU_USED                     __attribute__((used))
//     #define ALIGN(n)                    __attribute__((aligned(n)))
//     #define ZU_WEAK                     __attribute__((weak))
//     #define zu_inline                   static __inline
// //    #define RTT_API
// #elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
//     #include <stdarg.h>
//     #define SECTION(x)                  __attribute__((section(x)))
//     #define ZU_UNUSED                   __attribute__((unused))
//     #define ZU_USED                     __attribute__((used))
//     #define ALIGN(n)                    __attribute__((aligned(n)))
//     #define ZU_WEAK                     __attribute__((weak))
//     #define zu_inline                   static inline
// //    #define RTT_API
// #elif defined (_MSC_VER)
//     #include <stdarg.h>
//     #define SECTION(x)
//     #define ZU_UNUSED
//     #define ZU_USED
//     #define ALIGN(n)                    __declspec(align(n))
//     #define ZU_WEAK
//     #define zu_inline                   static __inline
// //    #define RTT_API
// #elif defined (__TI_COMPILER_VERSION__)
//     #include <stdarg.h>
//     /* The way that TI compiler set section is different from other(at least
//      * GCC and MDK) compilers. See ARM Optimizing C/C++ Compiler 5.9.3 for more
//      * details. */
//     #define SECTION(x)
//     #define ZU_UNUSED
//     #define ZU_USED
//     #define PRAGMA(x)                   _Pragma(#x)
//     #define ALIGN(n)
//     #define ZU_WEAK
//     #define zu_inline                   static inline
// //    #define RTT_API
// #else
//     #error not supported tool chain
// #endif

#endif

