#ifndef _BSP_HEF4021_H
#define _BSP_HEF4021_H

#include <stdint.h>

#define HEF4021_CASCADE_NUMBER  2

typedef struct {
    void (*hef4021_gpio_init)(void);
    void (*hef4021_pl)(uint8_t);
    void (*hef4021_cp)(uint8_t);
    uint8_t (*hef4021_q7)(void);
} hef4021_gpio_ctrl_t;


typedef struct bsp_hef4021_tag bsp_hef4021_t;

struct bsp_hef4021_tag {
    hef4021_gpio_ctrl_t *gpio_ctrl;
    void (*delay_us)(uint32_t us);
    uint8_t (*read_byte)(bsp_hef4021_t *handle);
    uint16_t (*read_2byte)(bsp_hef4021_t *handle);
    uint8_t (*read_nbyte)(bsp_hef4021_t *handle, uint8_t *data, uint8_t len);
    uint8_t (*read_bit)(bsp_hef4021_t *handle, uint8_t bit);
};

uint8_t hef4021_init(bsp_hef4021_t *handle,
                    void (*hef4021_gpio_init)(void),
                    void (*hef4021_pl)(uint8_t),
                    void (*hef4021_cp)(uint8_t),
                    uint8_t (*hef4021_q7)(void),
                    void (*delay_us)(uint32_t us));



#endif /* _BSP_HEF4021_H */
