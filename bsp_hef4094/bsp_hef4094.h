#ifndef _BSP_HEF4094_H
#define _BSP_HEF4094_H

#include <stdint.h>

#define HEF4094_CASCADE_NUMBER  2

typedef struct {
    void (*hef4094_gpio_init)(void);
    void (*hef4094_str)(uint8_t);
    void (*hef4094_cp)(uint8_t);
    void (*hef4094_data)(uint8_t);
    void (*hef4094_oe)(uint8_t);
} hef4094_gpio_ctrl_t;


typedef struct bsp_hef4094_tag bsp_hef4094_t;

struct bsp_hef4094_tag {
    // uint8_t cascade_number;
    // uint8_t *register_cache;
    hef4094_gpio_ctrl_t *gpio_ctrl;
    void (*delay_us)(uint32_t us);
    void (*output_enable)(bsp_hef4094_t *handle);
    void (*output_disable)(bsp_hef4094_t *handle);
    uint8_t (*write_byte)(bsp_hef4094_t *handle, uint8_t data);
    uint8_t (*write_2byte)(bsp_hef4094_t *handle, uint16_t data);
    uint8_t (*write_nbyte)(bsp_hef4094_t *handle, uint8_t *data, uint8_t len);
    uint8_t (*write_bit)(bsp_hef4094_t *handle, uint8_t bit, uint8_t sta);
};



uint8_t hef4094_init(bsp_hef4094_t *handle,
                    void (*hef4094_gpio_init)(void),
                    void (*hef4094_str)(uint8_t),
                    void (*hef4094_cp)(uint8_t),
                    void (*hef4094_data)(uint8_t),
                    void (*hef4094_oe)(uint8_t),
                    void (*delay_us)(uint32_t us));

#endif /* _BSP_HEF4094_H */
