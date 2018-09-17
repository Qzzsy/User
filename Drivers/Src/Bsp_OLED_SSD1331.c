/**
 ******************************************************************************
 * @file      Bsp_OLED_SSD1331.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-17
 * @brief     实现了对SSD1331的驱动
 * @History
 * Date           Author    version    		Notes
 * 2018-09-17       ZSY     V1.0.0      first version.
 */
	
/* Includes ------------------------------------------------------------------*/
#include "Bsp_OLED_SSD1331.h"

/* Private macro Definition --------------------------------------------------*/
#define OLED_WRITE_CS_H     HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
#define OLED_WRITE_CS_L     HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)

#define OLED_WRITE_DC_H     HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
#define OLED_WRITE_DC_L     HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)

#define OLED_WRITE_RD_H     HAL_GPIO_WritePin(OLED_RD_GPIO_Port, OLED_RD_Pin, GPIO_PIN_SET)
#define OLED_WRITE_RD_L     HAL_GPIO_WritePin(OLED_RD_GPIO_Port, OLED_RD_Pin, GPIO_PIN_RESET)

#define OLED_WRITE_WR_H     HAL_GPIO_WritePin(OLED_WR_GPIO_Port, OLED_WR_Pin, GPIO_PIN_SET)
#define OLED_WRITE_WR_L     HAL_GPIO_WritePin(OLED_WR_GPIO_Port, OLED_WR_Pin, GPIO_PIN_RESET)

#define OLED_WRITE_RESET_H  HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, GPIO_PIN_SET)
#define OLED_WRITE_RESET_L  HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, GPIO_PIN_RESET)

#define GPIO_WRITE(_data)   GPIOA->ODR = (GPIOA->ODR & 0xffffff00) | (_data & 0xff)

/* User functions -------------------------------------------------------------*/
/**
 * @func    _ssd1331WriteCmd
 * @brief   lcd写命令到寄存器
 * @param   _cmd 命令
 * @retval  无
 */
void _ssd1331WriteCmd(uint8_t _cmd)
{
    OLED_WRITE_DC_L;
    OLED_WRITE_RD_H;
    OLED_WRITE_WR_L;
    GPIO_WRITE(_cmd);
    OLED_WRITE_WR_H;
}

/**
 * @func    _ssd1331WriteData
 * @brief   lcd写数据到寄存器
 * @param   _data 数据
 * @retval  无
 */
void _ssd1331WriteData(uint8_t _data)
{
    OLED_WRITE_DC_H;
    OLED_WRITE_RD_H;
    OLED_WRITE_WR_L;
    GPIO_WRITE(_data);
    OLED_WRITE_WR_H;
}

/**
 * @func    OLED_SSD1331_Init
 * @brief   lcd初始化
 * @retval  无
 */
void OLED_SSD1331_Init(void)
{
    HAL_GPIO_WritePin(SX1308_EN_GPIO_Port, SX1308_EN_Pin, GPIO_PIN_SET);
    
    OLED_WRITE_RESET_L;
    for (uint16_t i = UINT16_MAX; i > 0; i--);
    OLED_WRITE_RESET_H;

    OLED_WRITE_CS_L;

    _ssd1331WriteCmd(0x15);
    _ssd1331WriteCmd(0x00);
    _ssd1331WriteCmd(0x81);
    _ssd1331WriteCmd(0xdf);
    _ssd1331WriteCmd(0x82);
    _ssd1331WriteCmd(0x1f);
    _ssd1331WriteCmd(0x83);
    _ssd1331WriteCmd(0xff);
    _ssd1331WriteCmd(0x87);
    _ssd1331WriteCmd(0x0f);

    _ssd1331WriteCmd(0xa0);
    _ssd1331WriteCmd(0x60);
    _ssd1331WriteCmd(0x71);

    _ssd1331WriteCmd(0xa4);

    _ssd1331WriteCmd(0xa8);

    _ssd1331WriteCmd(0x3f);

    _ssd1331WriteCmd(0xa9);
    _ssd1331WriteCmd(0x03);
    _ssd1331WriteCmd(0xaf);

    _ssd1331WriteCmd(0xb8);
    _ssd1331WriteCmd(0x01);
    _ssd1331WriteCmd(0x05);
    _ssd1331WriteCmd(0x09);
    _ssd1331WriteCmd(0x0d);
    _ssd1331WriteCmd(0x11);
    _ssd1331WriteCmd(0x15);
    _ssd1331WriteCmd(0x19);
    _ssd1331WriteCmd(0x1d);
    _ssd1331WriteCmd(0x21);
    _ssd1331WriteCmd(0x25);
    _ssd1331WriteCmd(0x29);
    _ssd1331WriteCmd(0x2d);
    _ssd1331WriteCmd(0x31);
    _ssd1331WriteCmd(0x35);
    _ssd1331WriteCmd(0x39);
    _ssd1331WriteCmd(0x3d);
    _ssd1331WriteCmd(0x41);
    _ssd1331WriteCmd(0x45);
    _ssd1331WriteCmd(0x49);
    _ssd1331WriteCmd(0x4d);
    _ssd1331WriteCmd(0x51);

    _ssd1331WriteCmd(0x55);
    _ssd1331WriteCmd(0x59);
    _ssd1331WriteCmd(0x5d);
    _ssd1331WriteCmd(0x61);
    _ssd1331WriteCmd(0x65);
    _ssd1331WriteCmd(0x69);
    _ssd1331WriteCmd(0x6d);
    _ssd1331WriteCmd(0x71);
    _ssd1331WriteCmd(0x75);
    _ssd1331WriteCmd(0x79);
    _ssd1331WriteCmd(0x7d);
    
    OLED_WRITE_CS_H;
}

/**
 * @func    ssd1331PutPixel
 * @brief   lcd在屏幕上打点
 * @param   _xPos x坐标
 * @param   _yPos y坐标
 * @param   _hwColor 点的颜色
 * @retval  无
 */
void ssd1331PutPixel(uint16_t _xPos, uint16_t _yPos, uint16_t _hwColor)
{
    OLED_WRITE_CS_L;
    
    _ssd1331WriteCmd(0x15);
    _ssd1331WriteCmd(_xPos);
    _ssd1331WriteCmd(_xPos);

    _ssd1331WriteCmd(0x75);
    _ssd1331WriteCmd(_yPos);
    _ssd1331WriteCmd(_yPos);

    _ssd1331WriteData(_hwColor >> 8);
    _ssd1331WriteData(_hwColor);
    
    OLED_WRITE_CS_H;
}

/**
 * @func    ssd1331ClrScreen
 * @brief   lcd清屏方法
 * @param   Color 清屏的颜色
 * @retval  无
 */
void ssd1331ClrScreen(uint16_t Color)
{
    uint16_t i, j;

    for (i = 0; i < 96; i++)
    {
        for (j = 0; j < 64; j++)
        {
            ssd1331PutPixel(i, j, Color);
        }
    }
}

/**
 * @func    ssd1331SetWin
 * @brief   lcd设置窗口坐标
 * @param   _xPos x的起点坐标
 * @param   _yPos y的起点坐标
 * @param   _Width 窗口的宽
 * @param   _Height 窗口的高
 * @retval  无
 */
void ssd1331SetWin(uint16_t _xPos, uint16_t _yPos, uint16_t _Width, uint16_t _Height)
{
    OLED_WRITE_CS_L;

    _ssd1331WriteCmd(0x15);
    _ssd1331WriteCmd(_xPos);
    _ssd1331WriteCmd(_xPos + _Width - 1);

    _ssd1331WriteCmd(0x75);
    _ssd1331WriteCmd(_yPos);
    _ssd1331WriteCmd(_yPos + _Height - 1);

    OLED_WRITE_CS_H;
}

/**
 * @func    ssd1331PutPixelNoPos
 * @brief   lcd在屏幕上打点
 * @param   _hwColor 点的颜色
 * @retval  无
 */
void ssd1331PutPixelNoPos(uint16_t _hwColor)
{
    OLED_WRITE_CS_L;

    _ssd1331WriteData(_hwColor >> 8);
    _ssd1331WriteData(_hwColor);

    OLED_WRITE_CS_H;
}
