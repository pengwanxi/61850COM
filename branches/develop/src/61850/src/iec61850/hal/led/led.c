#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "err_def.h"
#include "led.h"

pthread_t g_thread_led;

typedef enum { LED_OFF = 0, LED_ON, LED_BLINK } LED_STATE;
/*  */
typedef enum _LED_CTL_STATE {
    LED_CTL_OFF = 0, // 关闭LED
    LED_CTL_ON, // 打开LED
} LED_CTL_STATE;

/*  */
typedef struct _LED_DATA {
    LED_IDX idx; // LED索引
    LED_STATE state; // LED状态

    int blink_interval_ms; // 闪烁间隔，单位毫秒

    LED_CTL_STATE current_blink_state; // 当前闪烁状态，用于控制闪烁
    int current_blink_time_ms; // 当前闪烁时间，用于控制闪烁
} LED_DATA;

static LED_DATA gs_led_data[] = { {
    .idx = LED_RUN,
    .state = LED_OFF,
    .blink_interval_ms = 1000,
} };

static int led_ctrl(LED_IDX idx, LED_CTL_STATE ctl_state)
{

    switch (idx) {
    case LED_RUN:

        // 这里添加实际控制LED的代码，例如写入文件描述符等
        printf("LED_RUN %s\n", ctl_state == LED_CTL_ON ? "ON" : "OFF");
        break;
    default:
        return ERR_NODEV;
    }

    return ERR_NODEV;
}

static void thread_led(void *arg)
{
    while (1) {
        for (int i = 0; i < sizeof(gs_led_data) / sizeof(LED_DATA); i++) {
            LED_DATA *p = &gs_led_data[i];
            switch (p->state) {
            case LED_BLINK:
                if (p->current_blink_time_ms >= p->blink_interval_ms) {
                    p->current_blink_time_ms = 0;
                    p->current_blink_state = !p->current_blink_state;

                    led_ctrl(p->idx,
                             p->current_blink_state ?
                                 LED_CTL_ON :
                                 LED_CTL_OFF); // 根据当前闪烁状态设置LED
                }
                else {
                    p->current_blink_time_ms += 100; // 每100ms增加闪烁时间
                }
                break;
            default:
                break;
            }
        }

        usleep(100 * 1000); // 每100ms检查一次LED状态
    }
}

int led_init()
{
    // 这里可以添加初始化LED的代码，例如打开文件描述符等
    int ret = pthread_create(&g_thread_led, NULL, (void *)thread_led, NULL);
    if (ret != 0) {
        return ERR_EXECFAIL;
    }

    return ERR_OK;
}

int led_exit()
{
    pthread_cancel(g_thread_led);
    // 这里可以添加清理LED资源的代码，例如关闭文件描述符等
    return ERR_OK;
}

int led_on(LED_IDX idx)
{
    for (int i = 0; i < sizeof(gs_led_data) / sizeof(LED_DATA); i++) {
        LED_DATA *p = &gs_led_data[i];
        if (p->idx == idx) {
            p->state = LED_ON;
            p->blink_interval_ms = 0;
            p->current_blink_time_ms = 0; // 重置闪烁时间
            p->current_blink_state = LED_CTL_ON; // 确保LED状态为OFF
            return ERR_OK;
        }
    }
    // 这里添加实际控制LED亮的代码，例如写入文件描述符等
    return led_ctrl(idx, LED_CTL_ON);
}

int led_off(LED_IDX idx)
{
    for (int i = 0; i < sizeof(gs_led_data) / sizeof(LED_DATA); i++) {
        LED_DATA *p = &gs_led_data[i];
        if (p->idx == idx) {
            p->state = LED_OFF;
            p->blink_interval_ms = 0;
            p->current_blink_time_ms = 0; // 重置闪烁时间
            p->current_blink_state = LED_CTL_OFF; // 确保LED状态为OFF
            return ERR_OK;
        }
    }
    // 这里添加实际控制LED灭的代码，例如写入文件描述符等
    return led_ctrl(idx, LED_CTL_OFF);
}

int led_blink(LED_IDX idx, int interval_ms)
{
    for (int i = 0; i < sizeof(gs_led_data) / sizeof(LED_DATA); i++) {
        LED_DATA *p = &gs_led_data[i];
        if (p->idx == idx) {
            p->state = LED_BLINK;
            p->blink_interval_ms = 200;
            if (interval_ms >= 200) {
                p->blink_interval_ms = interval_ms;
            }
            p->current_blink_time_ms = 0; // 重置闪烁时间
            return ERR_OK;
        }
    }

    return ERR_NODEV;
}
