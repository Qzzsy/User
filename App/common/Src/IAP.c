/**
 ******************************************************************************
 * @file      Iap.c
 * @author    ZSY
 * @version   V1.0.1
 * @date      2019-08-21
 * @brief     实现了IAP功能，通用框架，可移植性强
 * @History
 * Date           Author    version    		   Notes
 * 2018-10-25      ZSY      V1.0.0          first version.
 * 2019-08-21      ZSY      V1.0.1          完善通用的bootloader框架.
 */

/* Includes ------------------------------------------------------------------*/

#define LOG_TAG    "iap"

#include "Iap.h"
#include "stdbool.h"
#include "ustring.h"
#include "MD5.h"
#include "elog.h"

#define PACK_HEAD_SIZE      12

#define TRY_GET_FIRM_INFO           1
#define IAP_UPDATING_FLAG           (1 << 0)
#define IAP_UPDATED_FLAG            (1 << 1)
#define IAP_NEED_UPDATE_FLAG        (1 << 2)
#define IAP_NEED_WRITE_INFO         (1 << 3)

/* 数据缓存*/
uint8_t data_buf[FIRM_RECV_BUF_SIZE + 128] = {0};

/* 固件信息结构体 */
#pragma pack(4)
typedef struct
{
    uint8_t project_name[32];
    uint8_t version[4];
    uint8_t author[16];
    uint8_t company[64];
    uint8_t RTOS_name[32];
    uint8_t firm_level[16];
    uint8_t chip_name[24];
    uint8_t chip_core[16];
    uint8_t pack_time[24];
    uint8_t partitions_name[32];
    uint32_t partitions_size;
    uint32_t chip_ram_size;
    uint32_t chip_rom_size;
    uint32_t chip_ram_start_addr;
    uint32_t chip_rom_start_addr;
    uint32_t app_startup_addr;
    uint32_t firm_size;
    uint32_t per_pack_size;
    uint32_t pack_sum;
    uint8_t MD5[16];
    uint8_t encrypt[8];
    uint8_t Reserve[96];
    
    uint8_t is_updating[32];
    uint8_t is_updated[32];
    uint8_t is_need_update[32];
} firm_info_t;
#pragma pack()

/* 枚举升级状态 */
enum
{
    UPDATING = 1,
    UPDATED = 2,
    NEED_UPDATE = 3,
    NEED_WRITE_INFO = 4
};

struct _iap_api
{
    /* APIs，外部应提供这几个APIs */
    iap_err_t (*send_cmd)(const void *data, uint32_t size);
    iap_err_t (*read_firm_data)(uint32_t pack_num, uint32_t offiset, void *data, uint32_t size);
    iap_err_t (*write_onchip_flash)(uint32_t addr, const void *data, uint32_t size);
    iap_err_t (*read_onchip_flash)(uint32_t addr, void *data, uint32_t size);
    iap_err_t (*control_onchip_flash)(uint8_t cmd, void *args);
    void (*jump_app)(uint32_t startup_addr);
};

struct _iap_api iap_api = 
{
    .send_cmd = UNULL,
    .read_firm_data = UNULL,
    .write_onchip_flash = UNULL,
    .read_onchip_flash = UNULL,
    .control_onchip_flash = UNULL,
    .jump_app = UNULL,
};

/**
 * @func    SetBootloaderHooks
 * @brief   设置APIs
 * @param   SendData 发送数据的API
 * @param   GetDataSize 获取数据大小API
 * @param   ReadData 读取数据的API
 * @note
 * @retval  无
 */
void set_bootloader_hooks(iap_err_t (*_send_cmd)(const void *data, uint32_t Size),
                        iap_err_t (*_read_firm_data)(uint32_t pack_num, uint32_t offiset, void *data, uint32_t size),
                        iap_err_t (*_write_onchip_flash)(uint32_t addr, const void *data, uint32_t size),
                        iap_err_t (*_read_onchip_flash)(uint32_t addr, void *data, uint32_t size),
                        iap_err_t (*_control_onchip_flash)(uint8_t cmd, void *vargs),
                        void (*_jump_app)(uint32_t startup_addr))
{
    iap_api.send_cmd = _send_cmd;
    iap_api.read_firm_data = _read_firm_data;
    iap_api.read_onchip_flash = _read_onchip_flash;
    iap_api.write_onchip_flash = _write_onchip_flash;
    iap_api.control_onchip_flash = _control_onchip_flash;
    iap_api.jump_app = _jump_app;
}

void iap_update_error(void)
{
    while(1)
    {

    }
}

/**
 * @func    GetFirmInfo
 * @brief   获取固件信息
 * @param   Info 固件信息指针
 * @note
 * @retval  无
 */
uint32_t get_firm_info(firm_info_t * info)
{
    return iap_api.read_onchip_flash(FIRM_INFO_BASE_ADDR, (uint8_t *)info, sizeof(firm_info_t));
}

bool_t is_empty(void * buf, uint32_t size)
{
    uint8_t *p_buf;

    p_buf = buf;

    for (uint32_t i = 0; i < size; i++)
    {
        if (p_buf[i] != 0xff)
        {
            return IAP_ERROR;
        }
    }

    return IAP_OK;
}

/**
 * @func    GetUpdateStatus
 * @brief   获取当前的升级状态
 * @note
 * @retval  当前的状态
 */
iap_err_t get_update_status(firm_info_t * info)
{
    uint32_t flag = 0;
    uint8_t is_update_flag = false, is_updated_flag = false, is_need_update_flag = false;

    if (info == UNULL)
    {
        /* 为了兼容M7内核 */
        uint8_t r_buf[32] = {'\0'};

        iap_api.read_onchip_flash(FIRM_INFO_BASE_ADDR + 512 - 96, (uint8_t *)r_buf, 32);
        if (umemcmp(r_buf, "updating", sizeof("updating")) == 0)
        {
            flag |= IAP_UPDATING_FLAG;
        }
        else
        {
            flag &= ~(IAP_UPDATING_FLAG);
        }
        
        iap_api.read_onchip_flash(FIRM_INFO_BASE_ADDR + 512 - 64, (uint8_t *)r_buf, 32);
        if (umemcmp(r_buf, "updated", sizeof("updated")) == 0)
        {
            flag |= IAP_UPDATED_FLAG;
        }
        else
        {
            flag &= ~(IAP_UPDATED_FLAG);
        }
        
        iap_api.read_onchip_flash(FIRM_INFO_BASE_ADDR + 512 - 32, (uint8_t *)r_buf, 32);
        if (umemcmp(r_buf, "need_update", sizeof("need_update")) == 0)
        {
            flag |= IAP_NEED_UPDATE_FLAG;
        }
        else
        {
            flag &= ~(IAP_NEED_UPDATE_FLAG);
        }
    }
    else
    {
        if (umemcmp(info->is_updating, "updating", sizeof("updating")) == 0)
        {
            flag |= IAP_UPDATING_FLAG;
        }
        else
        {
            flag &= ~(IAP_UPDATING_FLAG);
        }
        
        if (umemcmp(info->is_updated, "updated", sizeof("updated")) == 0)
        {
            flag |= IAP_UPDATED_FLAG;
        }
        else
        {
            flag &= ~(IAP_UPDATED_FLAG);
        }
        
        if (umemcmp(info->is_need_update, "need_update", sizeof("need_update")) == 0)
        {
            flag |= IAP_NEED_UPDATE_FLAG;
        }
        else
        {
            flag &= ~(IAP_NEED_UPDATE_FLAG);
        }
    }

    if (!(flag & IAP_UPDATING_FLAG) && !(flag & IAP_UPDATED_FLAG) && !(flag & IAP_NEED_UPDATE_FLAG))
    {
        /* 属于第一次上电，需要写入配置 */
        return NEED_WRITE_INFO;
    }
    else if ((flag & IAP_UPDATING_FLAG) && !(flag & IAP_UPDATED_FLAG) && !(flag & IAP_NEED_UPDATE_FLAG))
    {
        /* 系统正在更新中 */
        return UPDATING;
    }
    else if ((flag & IAP_UPDATING_FLAG) && (flag & IAP_UPDATED_FLAG) && !(flag & IAP_NEED_UPDATE_FLAG))
    {
        /* 系统更新完成 */
        return UPDATED;
    }
    else if ((flag & IAP_UPDATING_FLAG) && (flag & IAP_UPDATED_FLAG) && (flag & IAP_NEED_UPDATE_FLAG))
    {
        /* 系统需要更新 */
        return NEED_UPDATE;
    }
    return IAP_ERROR;
}

/**
 * @func    SetUpdateStatus
 * @brief   设置当前的升级状态
 * @note
 * @retval  无
 */
void set_update_status(uint8_t status)
{
    uint8_t w_buf[2][32] = {{"updating"}, {"updated"}};
    switch (status)
    {
        case UPDATING: iap_api.write_onchip_flash(FIRM_INFO_BASE_ADDR + 512 - 96, ((uint8_t *)&w_buf[0]), 32);break;
        case UPDATED: iap_api.write_onchip_flash(FIRM_INFO_BASE_ADDR + 512 - 64, ((uint8_t *)&w_buf[1]), 32);break;
        default : break;
    }
}

/**
 * @func    PackBuild
 * @brief   打包数据，往下一层发送数据
 * @param   cmd 命令
 * @param   packNum 数据包编号
 * @param   Buf 数据缓存
 * #param   Len 数据大小
 * @note
 * @retval  无
 */
void pack_build(uint8_t cmd, uint16_t pack_num, void *buf, uint16_t len)
{
    firm_pack_t pack;
    uint8_t data_buf[30] = {0};
    pack.cmd = cmd;
    pack.pack_num = pack_num;
    pack.data = buf;
    pack.size = len;

    umemcpy(data_buf, &pack, PACK_HEAD_SIZE);
    if (len > 0)
    {
        umemcpy(data_buf + PACK_HEAD_SIZE, pack.data, pack.size);
    }

    iap_api.send_cmd(data_buf, pack.size + PACK_HEAD_SIZE);
}

static void print_firm_info(firm_info_t * info)
{
    uprintf("\033[35;22m           Project name is: %s\033[0m\r\n", info->project_name);
    uprintf("\033[35;22m       software version is: %d.%d.%d\033[0m\r\n", info->version[0], info->version[1], info->version[2]);
    uprintf("\033[35;22m                   company: %s\033[0m\r\n", info->company);
    uprintf("\033[35;22m             The chip name: %s\033[0m\r\n", info->chip_name);
    uprintf("\033[35;22m             The chip core: %s\033[0m\r\n", info->chip_core);
    uprintf("\033[35;22mThe chip rom start address: 0x%08x, rom size: 0x%08x\033[0m\r\n", info->chip_rom_start_addr, info->chip_rom_size);
    uprintf("\033[35;22mThe chip ram start address: 0x%08x, ram size: 0x%08x\033[0m\r\n", info->chip_ram_start_addr, info->chip_ram_size);
    uprintf("\033[35;22m            firmware level: %s\033[0m\r\n", info->firm_level);
    if (is_empty(info->RTOS_name, sizeof(info->RTOS_name)) == IAP_ERROR)
    {
        uprintf("\033[35;22m               system RTOS: %s\033[0m\r\n", info->RTOS_name);
    }
    uprintf("\033[35;22mappliction startup address: 0x%08x\033[0m\r\n", info->app_startup_addr);
}

iap_err_t check_firm_validity(firm_info_t * info, uint8_t flag);
iap_err_t try_get_new_firm_info(firm_info_t *info)
{
    firm_pack_t pack;
    firm_info_t new_firm_info;
    uint32_t size, new_version_size, version_size;
    pack_build(IAP_CMD_GET_FIRM_INFO, 0, NULL, 0);
    
    umemset(&new_firm_info, 0, sizeof(firm_info_t));

    log_i("try get new firm info.");

    /* 获取第0包，即获取新固件信息 */
    size = iap_api.read_firm_data(0, 0, data_buf, FIRM_HEAD_INFO_SIZE);

    if (size != 0)
    {
        pack = *(firm_pack_t *)data_buf;
        if (pack.cmd == IAP_CMD_GET_FIRM_INFO)
        {
            new_firm_info = *((firm_info_t *)(data_buf + PACK_HEAD_SIZE));
            if (umemcmp(new_firm_info.firm_level, DEVICE_FIRM_LEVEL, sizeof(DEVICE_FIRM_LEVEL)) != 0)
            {
                log_w("no firmware in flash and needn`t update.");

                return IAP_ERROR;
            }

            if (umemcmp(new_firm_info.project_name, PROJECT_NAME, sizeof(PROJECT_NAME)) != 0)
            {
                log_w("no firmware in flash and needn`t update.");

                return IAP_ERROR;
            }

            if (umemcmp(new_firm_info.chip_name, DEVICE_CHIP_NAME, sizeof(DEVICE_CHIP_NAME)) != 0)
            {
                log_w("no firmware in flash and needn`t update.");

                return IAP_ERROR;
            }

            print_firm_info(&new_firm_info);

            new_version_size = new_firm_info.version[0];
            new_version_size <<= 8;
            new_version_size |= new_firm_info.version[1];
            new_version_size <<= 8;
            new_version_size |= new_firm_info.version[2];

            version_size = info->version[0];
            version_size <<= 8;
            version_size |= info->version[1];
            version_size <<= 8;
            version_size |= info->version[2];
                
            /* 判断新固件的版本号是否比当前的大 */
            if (new_version_size > version_size)
            {
                return IAP_OK;
            }
        }
    }
    return IAP_ERROR;
}

/**
 * @brief 检验固件的合法性
 * 
 * @param info 固件参数表
 */
iap_err_t check_firm_validity(firm_info_t * info, uint8_t flag)
{
    if (umemcmp(info->firm_level, DEVICE_FIRM_LEVEL, sizeof(DEVICE_FIRM_LEVEL)) != 0)
    {
        log_e("this firmware does not match this system, error code: %s, firmware level: %s",
                "level error", info->firm_level);
        log_e("Please contact the administrator!");
        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }

    if (umemcmp(info->project_name, PROJECT_NAME, sizeof(PROJECT_NAME)) != 0)
    {
        log_e("this firmware does not match this system, error code: %s, project name: %s",
                "project name error", info->project_name);
        log_e("Please contact the administrator!");

        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }

    if (umemcmp(info->chip_name, DEVICE_CHIP_NAME, sizeof(DEVICE_CHIP_NAME)) != 0)
    {
        log_e("this firmware does not match this system, error code: %s, chip name: %s",
                "target chip error", info->chip_name);
        log_e("Please contact the administrator!");

        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }

    if (info->chip_rom_start_addr != ONCHIP_FLASH_START_ADDRESS)
    {
        log_e("this firmware does not match this system, error code: %s, firmware flash address: 0x%08x", 
        "chip flash start address discrepancy", info->chip_rom_start_addr);
        log_e("Please contact the administrator!");

        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }

    if (info->chip_rom_size != ONCHIP_FLASH_SIZE)
    {
        log_e("this firmware does not match this system, error code: %s, firmware flash size: %d", 
        "chip flash size discrepancy", info->chip_rom_size);
        log_e("Please contact the administrator!");

        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }
    
    if (info->chip_ram_start_addr != ONCHIP_RAM_START_ADDRESS)
    {
        log_e("this firmware does not match this system, error code: %s, firmware ram address: 0x%08x", 
        "chip ram start address discrepancy", info->chip_ram_start_addr);
        log_e("Please contact the administrator!");

        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }

    if (info->chip_ram_size != ONCHIP_RAM_SIZE)
    {
        log_e("this firmware does not match this system, error code: %s, firmware ram size: %d", 
        "chip ram size discrepancy", info->chip_ram_size);
        log_e("Please contact the administrator!");

        if (try_get_new_firm_info(info) == IAP_OK && flag == TRY_GET_FIRM_INFO)
        {
            return IAP_OK;
        }

        return IAP_ERROR;
    }

    return IAP_OK;
}

/**
 * @func    UpdateFirm
 * @brief   升级固件
 * @param   JumpUserAppFlag 跳转标志
 * @note
 * @retval  无
 */
void update_firm(firm_info_t *firm_info)
{
    firm_pack_t pack;
    firm_info_t new_firm_info;
    uint16_t read_size, pack_num, last_pack_num = 0;
    uint8_t read_data_cnt = 0, repack_cnt = 0, update_status, send_data;
    uint32_t size = 0, offiset = 0, version_size, new_version_size;
    iap_err_t retval;

    /* 获取固件信息 */
    log_i("send cmd to get new firmware information, system will update firmware.");
    pack_build(IAP_CMD_GET_FIRM_INFO, 0, NULL, 0);
    
    umemset(&new_firm_info, 0, sizeof(firm_info_t));

    /* 获取第0包，即获取新固件信息， 允许阻塞 */
    size = iap_api.read_firm_data(0, 0, data_buf, FIRM_HEAD_INFO_SIZE);
    if (size != IAP_ERROR)
    {
        /* 解析新固件的信息 */
        pack = *(firm_pack_t *)data_buf;
        if (pack.cmd == IAP_CMD_GET_FIRM_INFO)
        {
            update_status = get_update_status(firm_info);

            new_firm_info = *((firm_info_t *)(data_buf + PACK_HEAD_SIZE));
            
            /* 检验固件的合法性 */
            retval = check_firm_validity(&new_firm_info, 0);
            if (retval == IAP_ERROR)
            {
                send_data = IAP_ERR_UNKNOW_FIRM;
                pack_build(IAP_CMD_WARNING, 0, &send_data, 1);
                
                log_i("appliction firmware needn`t update.");
                
                /* 因为新固件不合法，直接启动原有的固件 */
                if (iap_api.jump_app != UNULL)
                {
                    iap_api.jump_app(firm_info->app_startup_addr);
                }
            }

            /* 打印新固件的信息 */
            print_firm_info(&new_firm_info);

            new_version_size = new_firm_info.version[0];
            new_version_size <<= 8;
            new_version_size |= new_firm_info.version[1];
            new_version_size <<= 8;
            new_version_size |= new_firm_info.version[2];

            version_size = firm_info->version[0];
            version_size <<= 8;
            version_size |= firm_info->version[1];
            version_size <<= 8;
            version_size |= firm_info->version[2];
                
            /* 判断新固件的版本号是否比当前的大 */
            if (new_version_size > version_size)
            {
                log_i("firmware need update!");

                uint32_t param[2];
                param[0] = new_firm_info.app_startup_addr - 512;
                param[1] = new_firm_info.firm_size + 512;

                /* 清除FLASH空间 */
                retval = iap_api.control_onchip_flash(IAP_CMD_ERASE, param);
                if (retval == IAP_ERROR)
                {
                    log_e("erase onchip flash fault!");
                    send_data = IAP_ERR_ERASE_FAULT;
                    pack_build(IAP_CMD_ERR, 0, &send_data, 1);
                    
                    iap_update_error();
                }

                /* 检查擦除结果 */
                log_i("checking flash is empty...");
                
                read_size = 512;
                retval = iap_api.read_onchip_flash(new_firm_info.app_startup_addr - 512, data_buf, read_size);
                if (retval == IAP_ERROR)
                {
                    send_data = IAP_ERR_GET_ONCHIP_DATA_FAULT;
                    pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                    iap_update_error();
                }
                if (is_empty(data_buf, read_size) == IAP_ERROR)
                {
                    log_e("flash onchip erase fault, address: 0x%08x", new_firm_info.app_startup_addr - 512);
                    send_data = IAP_ERR_CHECK_IS_EMPTY_FAULT;
                    pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                    iap_update_error();
                }
                
                if (new_firm_info.firm_size < new_firm_info.per_pack_size)
                {
                    read_size = new_firm_info.firm_size;
                }
                else
                {
                    read_size = new_firm_info.per_pack_size;
                }

                offiset = 0;
                while (offiset != new_firm_info.firm_size)
                {
                    retval = iap_api.read_onchip_flash(new_firm_info.app_startup_addr + offiset, data_buf, read_size);
                    if (retval == IAP_ERROR)
                    {
                        send_data = IAP_ERR_GET_ONCHIP_DATA_FAULT;
                        pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                        iap_update_error();
                    }

                    if (is_empty(data_buf, read_size) == IAP_ERROR)
                    {
                        log_e("flash onchip erase fault, address: 0x%08x", new_firm_info.app_startup_addr + offiset);
                        
                        send_data = IAP_ERR_CHECK_IS_EMPTY_FAULT;
                        pack_build(IAP_CMD_ERR, 0, &send_data, 1);
                        
                        iap_update_error();
                    }

                    offiset += read_size;

                    if ((offiset + new_firm_info.per_pack_size) < new_firm_info.firm_size)
                    {
                        read_size = new_firm_info.per_pack_size;
                    }
                    else
                    {
                        read_size = new_firm_info.firm_size - offiset;
                    }
                }

                set_update_status(UPDATING);
                log_i("checking flash onchip is empty OK!");
                log_i("system will update firmware, upgrade process ensures that power supply is ok!");
                
                last_pack_num = 0;
                pack_num = 1;
            }
            else if (update_status == UPDATED)
            {
                uint8_t tmp = IAP_ERR_FIRM_ALREADY;
                pack_build(IAP_CMD_WARNING, 0, &tmp, 1);

                log_i("appliction firmware needn`t update.");
                
                /* 两个固件的版本号相同，不执行升级 */
                if (iap_api.jump_app != UNULL)
                {
                    iap_api.jump_app(firm_info->app_startup_addr);
                }
            }
        }
        else
        {
            log_e("cmd error!");
            uint8_t tmp = IAP_ERR_CMD_ERROR;
            pack_build(IAP_CMD_WARNING, 0, &tmp, 1);
            
            iap_update_error();
        }
    }
    else
    {
        log_w("get data from ext data fault.");
        send_data = IAP_ERR_GET_EXT_DATA_FAULT;
        pack_build(IAP_CMD_ERR, 0, &send_data, 1);

        iap_update_error();
    }


    /* 开始升级固件 */
    pack_build(IAP_CMD_GET_FIRM_DATA, pack_num, NULL, 0);
    offiset = 0;
    read_data_cnt = 0;
    repack_cnt = 0;
    log_i("iap start get firmware data.");
    
    if (new_firm_info.firm_size < new_firm_info.per_pack_size)
    {
        read_size = new_firm_info.firm_size;
    }
    else
    {
        read_size = new_firm_info.per_pack_size;
    }

    while (1)
    {
        log_d("will read data pack num: %d", pack_num);
        retval = iap_api.read_firm_data(pack_num, (pack_num - 1) * new_firm_info.per_pack_size + 512, data_buf, read_size);
        if (retval == IAP_ERROR)
        {
            read_data_cnt++;
            /* 数据读取失败 */
            log_e("Read firmware data error");
            
            if (read_data_cnt >= 3)
            {
                log_e("System update firmware fault!");
                send_data = IAP_ERR_UPDATE_FAULT;
                pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                iap_update_error();
            }
            send_data = IAP_ERR_GET_EXT_DATA_FAULT;
            pack_build(IAP_CMD_WARNING, 0, &send_data, 1);
            continue;
        }
        
        /* 解析数据包 */
        pack = *(firm_pack_t *)data_buf;
        if (pack.cmd == IAP_CMD_GET_FIRM_DATA)
        {
            /* 重包检测 */
            if (last_pack_num == pack.pack_num)
            {
                repack_cnt++;
                log_e("The firmware package has been received, pack num:%d, cnt: %d", pack.pack_num, repack_cnt);
                
                if (repack_cnt >= 3)
                {
                    log_e("System update firmware fault!");
                    send_data = IAP_ERR_UPDATE_FAULT;
                    pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                    iap_update_error();
                }

                send_data = IAP_ERR_GET_EXT_DATA_FAULT;
                pack_build(IAP_CMD_WARNING, 0, &send_data, 1);
                continue;
            }
            else
            {
                last_pack_num = pack_num;
            }

            if (iap_api.write_onchip_flash(new_firm_info.app_startup_addr + (pack_num - 1) * new_firm_info.per_pack_size,
                                        (uint8_t *)data_buf + PACK_HEAD_SIZE, pack.size) == IAP_ERROR)
            {
                log_e("Write firmware data error!");

                log_e("System update firmware fault!");

                send_data = IAP_ERR_UPDATE_FAULT;
                pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                iap_update_error();
            }

            offiset += pack.size;

            log_i("write pack num: %d is OK, pack size: %d, write/offiset size: %d.", pack.pack_num, pack.size, offiset);

            pack_num++;

            if (offiset > firm_info->partitions_size)
            {
                log_e("size over user appliction define size!");

                send_data = IAP_ERR_FLASH_IS_FULL;
                pack_build(IAP_CMD_ERR, 0, &send_data, 1);

                iap_update_error();
            }
            
            if (offiset == new_firm_info.firm_size)
            {
                /* 程序烧录完成，下一步做校验计算 */
                offiset = 0;
                break ;
            }

            if ((offiset + new_firm_info.per_pack_size) < new_firm_info.firm_size)
            {
                read_size = new_firm_info.per_pack_size;
            }
            else
            {
                read_size = new_firm_info.firm_size - offiset;
            }
        }
        
    }

    uint8_t data_MD5[16];

    mbedtls_md5_context md5_ctx;

    log_i("update ok, checking firm....");
    
    umemset(data_MD5, 0, 16);
    umemset(&md5_ctx, 0, sizeof(mbedtls_md5_context));
    
    offiset = 0;

    if (new_firm_info.firm_size < new_firm_info.per_pack_size)
    {
        read_size = new_firm_info.firm_size;
    }
    else
    {
        read_size = new_firm_info.per_pack_size;
    }

    /* 计算启动区域的程序MD5码 */
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    while (offiset != new_firm_info.firm_size)
    {
        retval = iap_api.read_onchip_flash(new_firm_info.app_startup_addr + offiset, data_buf, read_size);
        if (retval == IAP_ERROR)
        {
            send_data = IAP_ERR_GET_ONCHIP_DATA_FAULT;
            pack_build(IAP_CMD_ERR, 0, &send_data, 1);

            iap_update_error();
        }

        mbedtls_md5_update(&md5_ctx, data_buf, read_size);
        offiset += read_size;

        if ((offiset + new_firm_info.per_pack_size) < new_firm_info.firm_size)
        {
            read_size = new_firm_info.per_pack_size;
        }
        else
        {
            read_size = new_firm_info.firm_size - offiset;
        }
    }

    mbedtls_md5_finish(&md5_ctx, data_MD5);
    
    log_i("new firmware md5:");
    for (uint16_t ii = 0; ii < 16; ii++)
        uprintf("0x%02x ", new_firm_info.MD5[ii]);
    uprintf("\r\n");
    
    log_i("check result md5:");
    for (uint16_t ii = 0; ii < 16; ii++)
        uprintf("0x%02x ", data_MD5[ii]);
    uprintf("\r\n");

    if (umemcmp(new_firm_info.MD5, data_MD5, 16) != 0)
    {
        log_e("Firmware update fault! please restart machine!");

        send_data = IAP_ERR_CHECK_FAULT;
        pack_build(IAP_CMD_ERR, 0, &send_data, 1);

        iap_update_error();
    }

    log_i("checked firm ok! jump to new appliction!");
    /* 设置标志，表示已经升级并校验完成 */
    iap_api.write_onchip_flash(FIRM_INFO_BASE_ADDR, &new_firm_info, sizeof(firm_info_t) - 96);
    set_update_status(UPDATED);
    iap_api.jump_app(new_firm_info.app_startup_addr);
}

/**
 * @func    Bootloader
 * @brief   程序启动引导
 * @note
 * @retval  无
 */
void Bootloader(void)
{
    uint32_t retval;
    uint8_t send_data;
    if ((iap_api.control_onchip_flash == UNULL) ||
        (iap_api.jump_app == UNULL) ||
        (iap_api.read_firm_data == UNULL) ||
        (iap_api.read_onchip_flash == UNULL) ||
        (iap_api.send_cmd == UNULL) ||
        (iap_api.write_onchip_flash == UNULL))
    {
        log_e("apis error!");
        
        /* 发出错误的命令 */
        send_data = IAP_ERR_APIS_ERROR;
        pack_build(IAP_CMD_ERR, 0, &send_data, 1);
        
        iap_update_error();
    }

    firm_info_t firm_info;
    
    uint8_t flag;
    uint8_t data_MD5[16];
        
    /* 获取当前的固件信息 */
    retval = get_firm_info(&firm_info);
    if (retval == IAP_ERROR)
    {
        /* 发出错误的命令 */
        send_data = IAP_ERR_GET_ONCHIP_DATA_FAULT;
        pack_build(IAP_CMD_ERR, 0, &send_data, 1);
        
        iap_update_error();
    }
    
    /* 固件信息表为空，则可能为上次升级失败，尝试重新升级 */
    if (is_empty(&firm_info, sizeof(firm_info_t) - 96) == true)
    {
        goto _update;
    }

    /* 检验固件的合法性，固件不合法，尝试重新升级 */
    if (check_firm_validity(&firm_info, TRY_GET_FIRM_INFO) == IAP_ERROR)
    {
        goto _update;
    }

    /* 判断是否是第一次出厂上电 */
    flag = get_update_status(&firm_info);
    if (flag == NEED_WRITE_INFO)
    {
        log_d("this is first startup, code: %d", flag);
        set_update_status(UPDATING);
        set_update_status(UPDATED);

        flag = UPDATED;
    }

    print_firm_info(&firm_info);

    /* 判断是否需要升级 */
    if (flag == UPDATED)
    {
        /* 程序不需要升级，进入校验阶段 */
        mbedtls_md5_context md5_ctx;
        uint32_t offiset;
        uint16_t read_size;
        
        /* 尝试去获取新固件， 获取失败则进入启动环节 */
        if (try_get_new_firm_info(&firm_info) == IAP_OK)
        {
            log_i("try get new firmware info OK!");
            goto _update;
        }

        log_i("checking program Integrity......");

        umemset(data_MD5, 0, 16);
        umemset(&md5_ctx, 0, sizeof(mbedtls_md5_context));

        offiset = 0;

        if (firm_info.firm_size < firm_info.per_pack_size)
        {
            read_size = firm_info.firm_size;
        }
        else
        {
            read_size = firm_info.per_pack_size;
        }

        /* 计算启动区域的程序MD5码 */
        mbedtls_md5_init(&md5_ctx);
        mbedtls_md5_starts(&md5_ctx);
        while (offiset != firm_info.firm_size)
        {
            retval = iap_api.read_onchip_flash(firm_info.app_startup_addr + offiset, data_buf, read_size);
            if (retval == IAP_ERROR)
            {
                send_data = IAP_ERR_GET_ONCHIP_DATA_FAULT;
                pack_build(IAP_CMD_ERR, 0, &send_data, 1);
                iap_update_error();
            }

            mbedtls_md5_update(&md5_ctx, data_buf, read_size);
            offiset += read_size;

            if ((offiset + firm_info.per_pack_size) < firm_info.firm_size)
            {
                read_size = firm_info.per_pack_size;
            }
            else
            {
                read_size = firm_info.firm_size - offiset;
            }
        }

        mbedtls_md5_finish(&md5_ctx, data_MD5);
        
        log_i("firmware file md5:");
        for (uint16_t ii = 0; ii < 16; ii++)
            uprintf("0x%02x ", firm_info.MD5[ii]);
        uprintf("\r\n");
        
        log_i("check result md5:");
        for (uint16_t ii = 0; ii < 16; ii++)
            uprintf("0x%02x ", data_MD5[ii]);
        uprintf("\r\n");
        
        /* 判断程序的完整性 */
        if (umemcmp(firm_info.MD5, data_MD5, 16) == 0)
        {
            log_i("check firmware complete! startup appliction!");
            
            /* 校验成功直接启动用户程序 */
            if (iap_api.jump_app != NULL)
            {
                iap_api.jump_app(firm_info.app_startup_addr);
            }
        }

        /* 程序校验失败，程序已被更改 */
        log_e("Incomplete procedure!!!");
        log_e("Please contact the administrator");

        send_data = IAP_ERR_FIRM_ERROR;
        pack_build(IAP_CMD_ERR, 0, &send_data, 1);

        iap_update_error();
        return;
    }

    log_d("system need update");
_update:
    /* 升级固件 */
    update_firm(&firm_info);
}
