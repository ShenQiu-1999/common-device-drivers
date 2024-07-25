#include <stddef.h>
#include <string.h>
#include "bsp_hef4021.h"

#if !(defined(HEF4021_CASCADE_NUMBER) && (HEF4021_CASCADE_NUMBER > 0))
    #define HEF4021_CASCADE_NUMBER  1
#endif

static uint8_t read_byte(bsp_hef4021_t *handle);
static uint16_t read_2byte(bsp_hef4021_t *handle);
static uint8_t read_nbyte(bsp_hef4021_t *handle, uint8_t *data, uint8_t len);
static uint8_t read_bit(bsp_hef4021_t *handle, uint8_t bit);

/**
 * @brief   
 * @param   
 * @retval  0: success; 1: param error
 */
uint8_t hef4021_init(bsp_hef4021_t *handle,
                    void (*hef4021_gpio_init)(void),
                    void (*hef4021_pl)(uint8_t),
                    void (*hef4021_cp)(uint8_t),
                    uint8_t (*hef4021_q7)(void),
                    void (*delay_us)(uint32_t us))
{
    static hef4021_gpio_ctrl_t hef4021_gpio_ctrl;
    if(NULL == handle || NULL == hef4021_gpio_init || NULL == hef4021_pl || NULL == hef4021_cp || NULL == hef4021_q7 || NULL == delay_us) {
        return 1;
    }

    hef4021_gpio_ctrl.hef4021_gpio_init = hef4021_gpio_init;
    hef4021_gpio_ctrl.hef4021_pl = hef4021_pl;
    hef4021_gpio_ctrl.hef4021_cp = hef4021_cp;
    hef4021_gpio_ctrl.hef4021_q7 = hef4021_q7;

    handle->gpio_ctrl = &hef4021_gpio_ctrl;

    handle->delay_us = delay_us;
    handle->read_byte = read_byte;
    handle->read_2byte = read_2byte;
    handle->read_nbyte = read_nbyte;
     handle->read_bit = read_bit;

    handle->gpio_ctrl->hef4021_gpio_init();
    handle->gpio_ctrl->hef4021_cp(0);
    handle->gpio_ctrl->hef4021_pl(0);

    return 0;
}

static void hef4021_clk(bsp_hef4021_t *handle)
{
    handle->gpio_ctrl->hef4021_cp(0);
    handle->delay_us(1);
    handle->gpio_ctrl->hef4021_cp(1);
    handle->delay_us(1);
}

/**
 * @brief   Function to load parallel data into the HEF4021B
 * @param   
 * @retval  
 */
static void hef4021_load_parallel_data(bsp_hef4021_t *handle) {
    // Pull the load pin high to latch the inputs into the shift register
    handle->gpio_ctrl->hef4021_pl(1);
    // Small delay to ensure PL is recognized
    // for (volatile int i = 0; i < 10; i++);
    handle->delay_us(1);
    // Pull the load pin low to end the latch phase
    handle->gpio_ctrl->hef4021_pl(0);
    handle->delay_us(1);
}

/**
 * @brief   Function to read a single byte from the HEF4021B
 * @param   
 * @retval  
 */
static uint8_t read_byte(bsp_hef4021_t *handle) {

    uint8_t i;
    uint8_t data = 0;
    if (HEF4021_CASCADE_NUMBER == 1) {

        hef4021_load_parallel_data(handle);

        for (i = 0; i < 8; i++) {
            // Read the data bit first
            if (handle->gpio_ctrl->hef4021_q7()) {
                data |= (1 << (7 - i));
            }
            // Then generate a clock pulse to shift the next bit in
            hef4021_clk(handle);
        }
        return data;
    } else {
        return data; // 0
    }
}

/**
 * @brief	
 *  @note   The first chip is in low byte and the second chip is in high byte
 * @param	
 * @retval	Returns all input states, one bit for one input state
 */
static uint16_t read_2byte(bsp_hef4021_t *handle)
{
    uint8_t i,j;
    uint16_t data = 0;
    if (HEF4021_CASCADE_NUMBER == 2) {

        hef4021_load_parallel_data(handle);

        for (i = 0; i < HEF4021_CASCADE_NUMBER; i++) {
            for (j = 0; j < 8; j++) {
                // Read the data bit first
                if (handle->gpio_ctrl->hef4021_q7()) {
                    data |= (1 << (7 + (i * 8) - j));
                }
                // Then generate a clock pulse to shift the next bit in
                hef4021_clk(handle);
            }
        }
        return data;
    } else {
        return data; // 0
    }
}

/**
 * @brief   
 * @param   len: The length of bytes read must be equal to the number of cascaded chips
 * @retval  0: success; 1: param error
 */
static uint8_t read_nbyte(bsp_hef4021_t *handle, uint8_t *data, uint8_t len)
{
    uint8_t i,j;

    if(data == NULL) return 1;
    
    if (HEF4021_CASCADE_NUMBER == len) {
        memset(data, 0, len);
        hef4021_load_parallel_data(handle);
        
        for (i = 0; i < HEF4021_CASCADE_NUMBER; i++) {
            for (j = 0; j < 8; j++) {
                // Read the data bit first
                if (handle->gpio_ctrl->hef4021_q7()) {
                    data[i] |= (1 << (7 - j));
                }
                // Then generate a clock pulse to shift the next bit in
                hef4021_clk(handle);
            }
        }
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief   
 * @param   bit: 0 ~ (HEF4021_CASCADE_NUMBER * 8 -1)
 * @retval  0/1: bit_state; 0xFF: param error
 */
static uint8_t read_bit(bsp_hef4021_t *handle, uint8_t bit)
{
    uint8_t i, j;
    uint8_t chip_index, bit_index;
    uint8_t bit_state = 0xFF;
    // uint8_t buffer[HEF4021_CASCADE_NUMBER] = {0};

    if (bit >= HEF4021_CASCADE_NUMBER * 8) {
        return 0xFF; // param error
    }

    chip_index = bit / 8;
    bit_index = bit % 8;

    hef4021_load_parallel_data(handle);
    
    // 逐位读取数据
    for (i = 0; i < HEF4021_CASCADE_NUMBER; i++) {
        for (j = 0; j < 8; j++) {
            // if (handle->gpio_ctrl->hef4021_q7()) {
            //     buffer[i] |= (1 << (7 - j));
            // }
            // hef4021_clk(handle);

            // 读取当前位状态
            if (i == chip_index && j == (7 - bit_index)) {
                bit_state = handle->gpio_ctrl->hef4021_q7() ? 1 : 0;
                return bit_state;
            }
            // 生成时钟脉冲以移位下一位数据
            hef4021_clk(handle);
        }
    }
    // 从缓冲区读取目标位的状态
    // bit_state = (buffer[chip_index] & (1 << bit_index)) ? 1 : 0;

    return bit_state;
}

