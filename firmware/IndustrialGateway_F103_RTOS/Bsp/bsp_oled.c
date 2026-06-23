#include "bsp_oled.h"
#include "oled_font.h"

/*
 * This driver uses HAL I2C.
 * Default I2C handle: hi2c1 from CubeMX i2c.c
 */
extern I2C_HandleTypeDef hi2c1;

static HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2];

    buf[0] = 0x00;   /* control byte: command */
    buf[1] = cmd;

    return HAL_I2C_Master_Transmit(&hi2c1, BSP_OLED_I2C_ADDR, buf, 2, BSP_OLED_TIMEOUT);
}

static HAL_StatusTypeDef OLED_WriteData(uint8_t data)
{
    uint8_t buf[2];

    buf[0] = 0x40;   /* control byte: data */
    buf[1] = data;

    return HAL_I2C_Master_Transmit(&hi2c1, BSP_OLED_I2C_ADDR, buf, 2, BSP_OLED_TIMEOUT);
}

static void OLED_SetPosition(uint8_t page, uint8_t col)
{
    if (page >= BSP_OLED_PAGE_NUM || col >= BSP_OLED_WIDTH)
    {
        return;
    }

    OLED_WriteCmd((uint8_t)(0xB0 + page));
    OLED_WriteCmd((uint8_t)(0x00 + (col & 0x0F)));
    OLED_WriteCmd((uint8_t)(0x10 + ((col >> 4) & 0x0F)));
}

void BSP_OLED_Init(void)
{
    HAL_Delay(100);

    OLED_WriteCmd(0xAE); /* display off */

    OLED_WriteCmd(0x20); /* memory addressing mode */
    OLED_WriteCmd(0x02); /* page addressing mode */

    OLED_WriteCmd(0xB0); /* page start address */
    OLED_WriteCmd(0xC8); /* COM scan direction remapped */
    OLED_WriteCmd(0x00); /* low column address */
    OLED_WriteCmd(0x10); /* high column address */

    OLED_WriteCmd(0x40); /* start line address */
    OLED_WriteCmd(0x81); /* contrast */
    OLED_WriteCmd(0x7F);

    OLED_WriteCmd(0xA1); /* segment remap */
    OLED_WriteCmd(0xA6); /* normal display */

    OLED_WriteCmd(0xA8); /* multiplex ratio */
    OLED_WriteCmd(0x3F);

    OLED_WriteCmd(0xA4); /* output follows RAM content */
    OLED_WriteCmd(0xD3); /* display offset */
    OLED_WriteCmd(0x00);

    OLED_WriteCmd(0xD5); /* display clock divide */
    OLED_WriteCmd(0x80);

    OLED_WriteCmd(0xD9); /* pre-charge period */
    OLED_WriteCmd(0xF1);

    OLED_WriteCmd(0xDA); /* COM pins hardware configuration */
    OLED_WriteCmd(0x12);

    OLED_WriteCmd(0xDB); /* VCOMH deselect level */
    OLED_WriteCmd(0x40);

    OLED_WriteCmd(0x8D); /* charge pump */
    OLED_WriteCmd(0x14);

    OLED_WriteCmd(0xAF); /* display on */

    BSP_OLED_Clear();
}

void BSP_OLED_Clear(void)
{
    uint8_t page;
    uint8_t col;

    for (page = 0; page < BSP_OLED_PAGE_NUM; page++)
    {
        OLED_SetPosition(page, 0);
        for (col = 0; col < BSP_OLED_WIDTH; col++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void BSP_OLED_ClearLine(uint8_t y)
{
    uint8_t col;

    if (y >= BSP_OLED_PAGE_NUM)
    {
        return;
    }

    OLED_SetPosition(y, 0);
    for (col = 0; col < BSP_OLED_WIDTH; col++)
    {
        OLED_WriteData(0x00);
    }
}

/*
 * x: character column. One character uses 6 pixels: 5 font columns + 1 blank column.
 * y: page row.
 */
void BSP_OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
    const uint8_t *font;
    uint8_t i;
    uint8_t col;

    col = (uint8_t)(x * 6U);

    if (y >= BSP_OLED_PAGE_NUM || col >= BSP_OLED_WIDTH)
    {
        return;
    }

    font = OLED_Font_Get5x7(ch);

    OLED_SetPosition(y, col);

    for (i = 0; i < 5; i++)
    {
        OLED_WriteData(font[i]);
    }

    OLED_WriteData(0x00); /* blank column between characters */
}

void BSP_OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    if (str == 0)
    {
        return;
    }

    while (*str != '\0')
    {
        BSP_OLED_ShowChar(x, y, *str);
        x++;

        if (x >= 21)
        {
            x = 0;
            y++;
        }

        if (y >= BSP_OLED_PAGE_NUM)
        {
            break;
        }

        str++;
    }
}

void BSP_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    char buf[11];
    uint8_t i;

    if (len == 0)
    {
        return;
    }

    if (len > 10)
    {
        len = 10;
    }

    buf[len] = '\0';

    for (i = 0; i < len; i++)
    {
        buf[len - 1U - i] = (char)('0' + (num % 10U));
        num /= 10U;
    }

    BSP_OLED_ShowString(x, y, buf);
}
