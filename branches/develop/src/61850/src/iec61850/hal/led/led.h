/**
 *   \file led.h
 *   \brief 灯的实际控微
 */
#ifndef _LED_H_
#define _LED_H_

/*  */
typedef enum _LED_IDX {
    LED_RUN = 0, // 运行指示灯
} LED_IDX;

int led_init();
int led_exit();

int led_on(LED_IDX idx);
int led_off(LED_IDX idx);
int led_blink(LED_IDX idx, int interval_ms);

#endif /* _LED_H_ */
