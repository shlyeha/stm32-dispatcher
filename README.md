# STM32 process dispatcher

The dispatcher designed to execute several tasks simultaneously. Every SysTick tick it changes task in task table and execute it. Also it is possible to lock task from changing to execute some critical code and then unlock it.

## Usage example

```C
#include "dispatcher.h"

void task2(){
  setupPin(BUZZER_PORT, BUZZER_PIN, OUTPUT_MODE_PP);
  buzzer_play_startup_song();
}

bool check_mode_select(void){
  return State.mode_selected;
}

void task3(){
  while(1){
    if (State.mode_selected){
      MODE_LED_ON
    }
    else{
      TOGGLE_MODE_LED
      delay_ms_until_event(500, check_mode_select);
    }
  }
}

void task4(){
  while(1){
    State.modes[State.mode_number]();
  }
}

void add_tasks(){
  add_task("Buzzer startup song", task2, 0);
  add_task("Leds routine", task3, 0);
  add_task("Main loop", task4, 0);
}

int main(void){
  gpio_init();
  dispatcher_init();
  timer_init();
  state_init();
  add_tasks();

  SysTick_Config(SYSCLK_FREQ / 1000);
  __enable_irq();
  dispatcher_start();
  while(1);
}
```

## Locking process

If you need to lock critical section of code you execute, you will use `CRITICAL_LOCK` and `CRITICAL_UNLOCK` macroses

```C
void store_settings(void){
  CRITICAL_LOCK

    flash_unlock();
    flash_erase_page(GET_SETTINGS_PAGE_NUM);
    flash_write_word(GET_SETTINGS_START_ADDR, State.volume_level);
    flash_write_word(GET_SETTINGS_START_ADDR + 0x04, State.brightness_level);
    flash_lock();

  CRITICAL_UNLOCK
}
```

## Create and kill process by process

You can create/kill processes from another process

```C
void motors_routine(){
  while(1){
    DO_SOMETHING_WITH_MOTORS;
    delay_ms(1000);
    DO_ANOTHER_THING_WITH_MOTORS;
    delay_ms(1000);
  }
}

void some_task(){
  MOTORS_ENABLE
  add_task("Motors routine", motors_routine, 0);

  while(State.mode_selected);

  kill_task_by_name("Motors routine");
  MOTORS_DISABLE
}
```
