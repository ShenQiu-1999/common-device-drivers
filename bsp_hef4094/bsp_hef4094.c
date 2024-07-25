#include <stddef.h>
#include <string.h>
#include "bsp_hef4094.h"

#if !(defined(HEF4094_CASCADE_NUMBER) && (HEF4094_CASCADE_NUMBER > 0))
    #define HEF4094_CASCADE_NUMBER  1
#endif

static uint8_t register_cache[HEF4094_CASCADE_NUMBER];

static void hef4094_clk(bsp_hef4094_t *handle);
static void hef4094_latch_data(bsp_hef4094_t *handle);
static void write(bsp_hef4094_t *handle);

static void output_enable(bsp_hef4094_t *handle);
static void output_disable(bsp_hef4094_t *handle);
static uint8_t write_byte(bsp_hef4094_t *handle, uint8_t data);
static uint8_t write_2byte(bsp_hef4094_t *handle, uint16_t data);
static uint8_t write_nbyte(bsp_hef4094_t *handle, uint8_t *data, uint8_t len);
static uint8_t write_bit(bsp_hef4094_t *handle, uint8_t bit, uint8_t sta);

/**
 * @brief   
 * @param   
 * @retval  
 */
uint8_t hef4094_init(bsp_hef4094_t *handle,
                    void (*hef4094_gpio_init)(void),
                    void (*hef4094_str)(uint8_t),
                    void (*hef4094_cp)(uint8_t),
                    void (*hef4094_data)(uint8_t),
                    void (*hef4094_oe)(uint8_t),
                    void (*delay_us)(uint32_t us))
{
    static hef4094_gpio_ctrl_t hef4094_gpio_ctrl;

    if(NULL == handle || NULL == hef4094_gpio_init || NULL == hef4094_str || NULL == hef4094_cp \
        || NULL == hef4094_data || NULL == hef4094_oe || NULL == delay_us) {
        return 1;
    }
    
    hef4094_gpio_ctrl.hef4094_gpio_init = hef4094_gpio_init;
    hef4094_gpio_ctrl.hef4094_str = hef4094_str;
    hef4094_gpio_ctrl.hef4094_cp = hef4094_cp;
    hef4094_gpio_ctrl.hef4094_data = hef4094_data;
    hef4094_gpio_ctrl.hef4094_oe = hef4094_oe;

    handle->gpio_ctrl = &hef4094_gpio_ctrl;

    handle->delay_us = delay_us;
    handle->output_enable = output_enable;
    handle->output_disable = output_disable;
    handle->write_byte = write_byte;
    handle->write_2byte = write_2byte;
    handle->write_nbyte = write_nbyte;
    handle->write_bit = write_bit;

    handle->gpio_ctrl->hef4094_gpio_init();
    handle->gpio_ctrl->hef4094_str(0);
    handle->gpio_ctrl->hef4094_cp(0);
    handle->gpio_ctrl->hef4094_data(0);
    handle->gpio_ctrl->hef4094_oe(0); // 1: enable; 0: Z-state. default: output disable

    memset(register_cache, 0, HEF4094_CASCADE_NUMBER); // default: output low
    write(handle);

    return 0;
}

uint8_t hef4094_deinit(bsp_hef4094_t *handle)
{
    handle->gpio_ctrl->hef4094_oe(0); // 1: enable; 0: Z-state
    return 0;
}

static void output_enable(bsp_hef4094_t *handle)
{
    handle->gpio_ctrl->hef4094_oe(1);
}

static void output_disable(bsp_hef4094_t *handle)
{
    handle->gpio_ctrl->hef4094_oe(0); // Z-state
}

static void hef4094_clk(bsp_hef4094_t *handle)
{
    handle->gpio_ctrl->hef4094_cp(0);
    handle->delay_us(1);
    handle->gpio_ctrl->hef4094_cp(1); // HEF4094B在时钟信号的上升沿（从低到高的过渡）捕获数据输入
    handle->delay_us(1);
}

static void hef4094_latch_data(bsp_hef4094_t *handle)
{
    handle->gpio_ctrl->hef4094_str(1); // STR=H,将数据从移位寄存器锁存到存储寄存器
    handle->delay_us(1);
    handle->gpio_ctrl->hef4094_str(0); // STR=L,禁止数据锁存,以防输出干扰
    handle->delay_us(1);
}

static void write(bsp_hef4094_t *handle)
{
    uint8_t i,j;
    uint8_t temp_data;

    for(i = HEF4094_CASCADE_NUMBER; i > 0; i--)
    {
        temp_data = register_cache[i - 1];
        for(j = 0; j < 8; j++)
        {
            handle->gpio_ctrl->hef4094_data((temp_data & 0x80) >> 7); // 数据在时钟信号的上升沿被移位到内部寄存器中。
            temp_data <<= 1;
            hef4094_clk(handle);
        }
    }
    hef4094_latch_data(handle); // 当锁存输入信号从低到高的过渡时, 移位寄存器的内容被锁存到存储寄存器, 并可供输出.
}

/**
 * @brief   
 * @param   
 * @retval  0: success; 1: param error
 */
static uint8_t write_byte(bsp_hef4094_t *handle, uint8_t data)
{
    if (HEF4094_CASCADE_NUMBER == 1) {
        memcpy(register_cache, &data, 1);
        write(handle);
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief   
 *  @note   The first chip is in low byte and the second chip is in high byte
 * @param   
 * @retval  0: success; 1: param error
 */
static uint8_t write_2byte(bsp_hef4094_t *handle, uint16_t data)
{
    if (HEF4094_CASCADE_NUMBER ==  2) {
        memcpy(register_cache, &data, 2);
        write(handle);
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief   Function to write multiple bytes to the HEF4094
 *  @note   
 * @param   
 * @retval  0: sucess; 1: param error
 */
static uint8_t write_nbyte(bsp_hef4094_t *handle, uint8_t *data, uint8_t len)
{
    if (HEF4094_CASCADE_NUMBER == len) {
        memcpy(register_cache, data, len);
        write(handle);
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief   设置HEF4094芯片输出的某个位状态，如果状态没有变化则不进行通信
 *  @note   
 * @param   handle - 指向HEF4094控制结构体的指针
 * @param   bit - 需要设置的位的索引（0表示最低位）
 * @param   sta - 该位的新状态（0或1）
 * @retval  0: success; 1: param error; 2: status unchanged
 */
static uint8_t write_bit(bsp_hef4094_t *handle, uint8_t bit, uint8_t sta)
{
    uint8_t chip_index = bit / 8;
    uint8_t bit_index =  bit % 8;

    if (chip_index < HEF4094_CASCADE_NUMBER) {
        // 检查当前位的状态
        uint8_t current_sta = (register_cache[chip_index] >> bit_index) & 1;
        if (current_sta == sta) {
            return 2; // 状态未改变
        }
        if (sta) {
            register_cache[chip_index] |= (1 << bit_index);
        } else {
            register_cache[chip_index] &= ~(1 << bit_index);
        }
        write(handle);
        return 0;
    } else {
        return 1;
    }
}
