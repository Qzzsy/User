#include "bsp_lcd.h"
#include "lv_conf.h"
#include "lv_tft.h"
#include "lv_core/lv_vdb.h"
#include "lv_hal/lv_hal.h"
#if USE_LV_GPU == 1
#define TFT_USE_GPU         1
#else
#define TFT_USE_GPU         0
#endif

#if TFT_USE_GPU != 0
static DMA2D_HandleTypeDef * DMA2D_Handler;

/**
 * Copy pixels to destination memory using opacity
 * @param dest a memory address. Copy 'src' here.
 * @param src pointer to pixel map. Copy it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, OPA_TRANSP: transparent ... 255, OPA_COVER, fully cover)
 */
static void gpu_mem_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    uint32_t Timeout = 0;
    /*Wait for the previous operation*/
    HAL_DMA2D_PollForTransfer(DMA2D_Handler, 100);
    DMA2D_Handler->Init.Mode         = DMA2D_M2M_BLEND;
    /* DMA2D Initialization */
    if(HAL_DMA2D_Init(DMA2D_Handler) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }

    DMA2D_Handler->LayerCfg[1].InputAlpha = opa;
    HAL_DMA2D_ConfigLayer(DMA2D_Handler, 1);
    HAL_DMA2D_BlendingStart_IT(DMA2D_Handler, (uint32_t) src, (uint32_t) dest, (uint32_t)dest, length, 1);    
    while ((DMA2D->ISR & (DMA2D_FLAG_TC)) == 0)	//等待传输完成
    {
        Timeout++;
        if (Timeout > 0X1FFFFF)
            break;	//超时退出
    }
    DMA2D->IFCR |= DMA2D_FLAG_TC;		//清除传输完成标志
    lv_flush_ready();
}


/**
 * Copy pixels to destination memory using opacity
 * @param dest a memory address. Copy 'src' here.
 * @param src pointer to pixel map. Copy it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, OPA_TRANSP: transparent ... 255, OPA_COVER, fully cover)
 */
static void gpu_mem_fill(lv_color_t * dest, uint32_t length, lv_color_t color)
{
    uint32_t Timeout = 0;
    /*Wait for the previous operation*/
    HAL_DMA2D_PollForTransfer(DMA2D_Handler, 100);

    DMA2D_Handler->Init.Mode         = DMA2D_R2M;
    /* DMA2D Initialization */
    if(HAL_DMA2D_Init(DMA2D_Handler) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }

    DMA2D_Handler->LayerCfg[1].InputAlpha = 0xff;
    HAL_DMA2D_ConfigLayer(DMA2D_Handler, 1);
    HAL_DMA2D_BlendingStart_IT(DMA2D_Handler, (uint32_t) lv_color_to24(color), (uint32_t) dest, (uint32_t)dest, length, 1);
    
    while ((DMA2D->ISR & (DMA2D_FLAG_TC)) == 0)	//等待传输完成
    {
        Timeout++;
        if (Timeout > 0X1FFFFF)
            break;	//超时退出
    }
    DMA2D->IFCR |= DMA2D_FLAG_TC;		//清除传输完成标志
    lv_flush_ready();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
    while(1)
    {
    }
}

static void DMA2D_TransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
}
static void DMA2D_TransferError(DMA2D_HandleTypeDef *hdma2d)
{

}
/**
  * @brief DMA2D configuration.
  * @note  This function Configure the DMA2D peripheral :
  *        1) Configure the Transfer mode as memory to memory with blending.
  *        2) Configure the output color mode as RGB565 pixel format.
  *        3) Configure the foreground
  *          - first image loaded from FLASH memory
  *          - constant alpha value (decreased to see the background)
  *          - color mode as RGB565 pixel format
  *        4) Configure the background
  *          - second image loaded from FLASH memory
  *          - color mode as RGB565 pixel format
  * @retval None
  */
static void DMA2D_Config(void)
{
    /* Configure the DMA2D Mode, Color Mode and output offset */
    DMA2D_Handler->Init.Mode         = DMA2D_M2M_BLEND;
    DMA2D_Handler->Init.ColorMode    = DMA2D_RGB565;
    DMA2D_Handler->Init.OutputOffset = 0x0;

    /* DMA2D Callbacks Configuration */
    DMA2D_Handler->XferCpltCallback  = DMA2D_TransferComplete;
    DMA2D_Handler->XferErrorCallback = DMA2D_TransferError;

    /* Foreground Configuration */
    DMA2D_Handler->LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    DMA2D_Handler->LayerCfg[1].InputAlpha = 0xFF;
    DMA2D_Handler->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
    DMA2D_Handler->LayerCfg[1].InputOffset = 0x0;

    /* Background Configuration */
    DMA2D_Handler->LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
    DMA2D_Handler->LayerCfg[0].InputAlpha = 0xFF;
    DMA2D_Handler->LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
    DMA2D_Handler->LayerCfg[0].InputOffset = 0x0;

    DMA2D_Handler->Instance   = DMA2D;

    /* DMA2D Initialization */
    if(HAL_DMA2D_Init(DMA2D_Handler) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    HAL_DMA2D_ConfigLayer(DMA2D_Handler, 0);
    HAL_DMA2D_ConfigLayer(DMA2D_Handler, 1);
}
#endif


void LCD_LvglInit(void)
{
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.disp_fill = BspLCD.Fill;
    disp_drv.disp_map = BspLCD.FillColor;
    disp_drv.disp_flush = BspLCD.FillColor;

#if TFT_USE_GPU != 0
    DMA2D_Handler = GetDMA2D_Handle();
    DMA2D_Config();
    disp_drv.mem_blend = gpu_mem_blend;
    disp_drv.mem_fill = gpu_mem_fill;
#endif
    lv_disp_drv_register(&disp_drv);
}
