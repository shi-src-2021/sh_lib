#include <stdio.h>

#include "sh_button.h"

/* 定义按键ID枚举 */
enum sh_button_id {
    SH_BUTTON_ID_1 = 0,
    SH_BUTTON_ID_2,
};

/* 定义按键控制对象，可初始化为自定义值 */
sh_button_ctrl_t sh_button_ctrl;

/* 定义按键对象 */
sh_button_t sh_button;

/* 这里实现自己的gpio读取函数 */
uint8_t sh_get_button_level(uint8_t id)
{
    switch (id) {
    case SH_BUTTON_ID_1:
        // return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);

    case SH_BUTTON_ID_2:
        // return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);

    default:
        return 0;
    }
}

/* 定义按键事件回调函数 */
void board_button_cb_fn(sh_button_t *button, enum sh_button_event event)
{
    switch (event) {
    case SH_BUTTON_EVENT_PRESS:
        printf("%s [press]", button->name);
        break;

    case SH_BUTTON_EVENT_RELEASE:
        printf("%s [release]", button->name);
        break;

    case SH_BUTTON_EVENT_SINGLE_CLICK:
        printf("%s [single click]", button->name);
        break;

    case SH_BUTTON_EVENT_DOUBLE_CLICK:
        printf("%s [double click]", button->name);
        break;

    case SH_BUTTON_EVENT_MULTI_CLICK:
        printf("%s [multi click] %d", button->name, button->repeat);
        break;

    case SH_BUTTON_EVENT_LONG_PRESS_START:
        printf("%s [long press start]", button->name);
        break;

    case SH_BUTTON_EVENT_LONG_PRESS_REPEAT:
        printf("%s [long press repeat]", button->name);
        break;

    default:
        ULOG_ERROR("%s [wrong event id] %d", button->name, event);
        break;
    }
}

int main(int argc, char **argv)
{
    /* 定义按键配置结构体 */
    sh_button_config_t config_1 = {
        .name               = "button_1",
        .id                 = SH_BUTTON_ID_1,
        .active_level       = SH_BUTTON_ACTIVE_LOW,
        .get_button_level   = sh_get_button_level,
    };

    /* 根据按键配置初始化按键 */
    sh_button_init(&sh_button, &config_1);

    /* 按键绑定事件回调函数 */
    sh_button_attach_cbs(&sh_button, board_button_cb_fn);


    /* 初始化按键控制对象 */
    sh_button_ctrl_init(&sh_button_ctrl);

    /* 向按键控制对象中添加按键 */
    sh_button_ctrl_add(&sh_button_ctrl, &sh_button);

    while (1) {
        /* 定时器或者RTOS线程中周期性调用 */
        sh_button_handler(&sh_button_ctrl);

        /* 延时 */
        rt_thread_mdelay(1);
    }
}





