#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "zu_config.h"
#include "zu_def.h"
zu_err_t zu_device_register(p_zu_device_t dev,
                            const char *name,
                            zu_uint16_t flags);
zu_err_t zu_device_unregister(p_zu_device_t dev);
p_zu_device_t zu_device_find(const char *name);
zu_err_t zu_device_init(p_zu_device_t dev);
zu_err_t zu_device_open(p_zu_device_t dev, zu_uint16_t oflag);
zu_err_t zu_device_close(p_zu_device_t dev);
zu_size_t zu_device_read(p_zu_device_t dev,
                         zu_off_t    pos,
                         void       *buffer,
                         zu_size_t   size);
zu_size_t zu_device_write(p_zu_device_t dev,
                          zu_off_t    pos,
                          const void *buffer,
                          zu_size_t   size);
zu_err_t zu_device_control(p_zu_device_t dev, int cmd, void *arg);
zu_err_t zu_device_set_rx_indicate(p_zu_device_t dev,
                zu_err_t (*rx_ind)(p_zu_device_t dev, zu_size_t size));
zu_err_t zu_device_set_tx_complete(p_zu_device_t dev,
                          zu_err_t (*tx_done)(p_zu_device_t dev, void *buffer));




#endif
