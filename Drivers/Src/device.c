#include "device.h"
#include "list.h"
#include "zu_string.h"

static zu_list_t device_list;
static p_zu_list_t p_device_list = &device_list;

/**
 * This function registers a device driver with specified name.
 *
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @param flags the flag of device
 *
 * @return the error code, RT_EOK on initialization successfully.
 */
zu_err_t zu_device_register(p_zu_device_t dev,
                            const char *name,
                            zu_uint16_t flags)
{
    if (dev == ZU_NULL)
        return -ZU_ERROR;

    if (zu_device_find(name) != ZU_NULL)
        return -ZU_ERROR;
    
    zu_strncpy(dev->parent.name, name, ZU_NAME_MAX);
    list_insert_after(p_device_list, &dev->parent.list);
    
    dev->flag = flags;
    
    dev->ref_count = 0;
    dev->open_flag = 0;

    return ZU_EOK;
}

/**
 * This function removes a previously registered device driver
 *
 * @param dev the pointer of device driver structure
 *
 * @return the error code, RT_EOK on successfully.
 */
zu_err_t zu_device_unregister(p_zu_device_t dev)
{
    ZU_ASSERT(dev != ZU_NULL);

    list_remove(&dev->parent.list);

    return ZU_EOK;
}


/**
 * This function finds a device driver by specified name.
 *
 * @param name the device driver's name
 *
 * @return the registered device driver on successful, or RT_NULL on failure.
 */
p_zu_device_t zu_device_find(const char *name)
{
    zu_object_t *object;
    zu_list_t *node;

    /* try to find device object */
    for (node  = p_device_list->next;
         node != p_device_list;
         node  = node->next)
    {
        object = ZU_LIST_ENTRY(node, zu_object_t, list);
        if (zu_strncmp(object->name, name, ZU_NAME_MAX) == 0)
        {

            return (p_zu_device_t)object;
        }
    }

    /* not found */
    return ZU_NULL;
}

/**
 * This function will initialize the specified device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
zu_err_t zu_device_init(p_zu_device_t dev)
{
    zu_err_t result = ZU_EOK;

    ZU_ASSERT(dev != ZU_NULL);

    /* get device init handler */
    if (dev->init != ZU_NULL)
    {
        if (!(dev->flag & ZU_DEVICE_FLAG_ACTIVATED))
        {
            result = dev->init(dev);
            if (result != ZU_EOK)
            {
                zu_printf("To initialize device:%s failed. The error code is %d\n",
                           dev->parent.name, result);
            }
            else
            {
                dev->flag |= ZU_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}

/**
 * This function will open a device
 *
 * @param dev the pointer of device driver structure
 * @param oflag the flags for device open
 *
 * @return the result
 */
zu_err_t zu_device_open(p_zu_device_t dev, zu_uint16_t oflag)
{
    zu_err_t result = ZU_EOK;

    ZU_ASSERT(dev != ZU_NULL);

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & ZU_DEVICE_FLAG_ACTIVATED))
    {
        if (dev->init != ZU_NULL)
        {
            result = dev->init(dev);
            if (result != ZU_EOK)
            {
                zu_printf("To initialize device:%s failed. The error code is %d\n",
                           dev->parent.name, result);

                return result;
            }
        }

        dev->flag |= ZU_DEVICE_FLAG_ACTIVATED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & ZU_DEVICE_FLAG_STANDALONE) &&
        (dev->open_flag & ZU_DEVICE_OFLAG_OPEN))
    {
        return -ZU_EBUSY;
    }

    /* call device open interface */
    if (dev->open != ZU_NULL)
    {
        result = dev->open(dev, oflag);
    }
    else
    {
        /* set open flag */
        dev->open_flag = (oflag & ZU_DEVICE_OFLAG_MASK);
    }

    /* set open flag */
    if (result == ZU_EOK || result == -ZU_ENOSYS)
    {
        dev->open_flag |= ZU_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        /* don't let bad things happen silently. If you are bitten by this assert,
         * please set the ref_count to a bigger type. */
        ZU_ASSERT(dev->ref_count != 0);
    }

    return result;
}

/**
 * This function will close a device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
zu_err_t zu_device_close(p_zu_device_t dev)
{
    zu_err_t result = ZU_EOK;

    ZU_ASSERT(dev != ZU_NULL);

    if (dev->ref_count == 0)
        return -ZU_ERROR;

    dev->ref_count--;

    if (dev->ref_count != 0)
        return ZU_EOK;

    /* call device close interface */
    if (dev->close != ZU_NULL)
    {
        result = dev->close(dev);
    }

    /* set open flag */
    if (result == ZU_EOK || result == -ZU_ENOSYS)
        dev->open_flag = ZU_DEVICE_OFLAG_CLOSE;

    return result;
}

/**
 * This function will read some data from a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of reading
 * @param buffer the data buffer to save read data
 * @param size the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 *
 * @note
 */
zu_size_t zu_device_read(p_zu_device_t dev,
                         zu_off_t    pos,
                         void       *buffer,
                         zu_size_t   size)
{
    ZU_ASSERT(dev != ZU_NULL);

    if (dev->ref_count == 0)
    {
        //zu_set_errno(-ZU_ERROR);
        return 0;
    }

    /* call device read interface */
    if (dev->read != ZU_NULL)
    {
        return dev->read(dev, pos, buffer, size);
    }

    /* set error code */
    //zu_set_errno(-ZU_ENOSYS);

    return 0;
}

/**
 * This function will write some data to a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of written
 * @param buffer the data buffer to be written to device
 * @param size the size of buffer
 *
 * @return the actually written size on successful, otherwise negative returned.
 *
 * @note
 */
zu_size_t zu_device_write(p_zu_device_t dev,
                          zu_off_t    pos,
                          const void *buffer,
                          zu_size_t   size)
{
    ZU_ASSERT(dev != ZU_NULL);

    if (dev->ref_count == 0)
    {
        //zu_set_errno(-ZU_ERROR);
        return 0;
    }

    /* call device write interface */
    if (dev->write != ZU_NULL)
    {
        return dev->write(dev, pos, buffer, size);
    }

    /* set error code */
    //zu_set_errno(-ZU_ENOSYS);

    return 0;
}

/**
 * This function will perform a variety of control functions on devices.
 *
 * @param dev the pointer of device driver structure
 * @param cmd the command sent to device
 * @param arg the argument of command
 *
 * @return the result
 */
zu_err_t zu_device_control(p_zu_device_t dev, int cmd, void *arg)
{
    ZU_ASSERT(dev != ZU_NULL);

    /* call device write interface */
    if (dev->control != ZU_NULL)
    {
        return dev->control(dev, cmd, arg);
    }

    return -ZU_ENOSYS;
}

/**
 * This function will set the reception indication callback function. This callback function
 * is invoked when this device receives data.
 *
 * @param dev the pointer of device driver structure
 * @param rx_ind the indication callback function
 *
 * @return ZU_EOK
 */
zu_err_t zu_device_set_rx_indicate(p_zu_device_t dev,
                zu_err_t (*rx_ind)(p_zu_device_t dev, zu_size_t size))
{
    ZU_ASSERT(dev != ZU_NULL);

    dev->rx_indicate = rx_ind;

    return ZU_EOK;
}

/**
 * This function will set the indication callback function when device has
 * written data to physical hardware.
 *
 * @param dev the pointer of device driver structure
 * @param tx_done the indication callback function
 *
 * @return ZU_EOK
 */
zu_err_t zu_device_set_tx_complete(p_zu_device_t dev,
                          zu_err_t (*tx_done)(p_zu_device_t dev, void *buffer))
{
    ZU_ASSERT(dev != ZU_NULL);

    dev->tx_complete = tx_done;

    return ZU_EOK;
}

