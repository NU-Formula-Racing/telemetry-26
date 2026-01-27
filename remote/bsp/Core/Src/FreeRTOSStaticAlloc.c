#include "FreeRTOS.h"
#include "task.h"

static StaticTask_t idle_tcb;
static StackType_t idle_stack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize) {
  *ppxIdleTaskTCBBuffer = &idle_tcb;
  *ppxIdleTaskStackBuffer = idle_stack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if (configUSE_TIMERS == 1)
static StaticTask_t timer_tcb;
static StackType_t timer_stack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint32_t* pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &timer_tcb;
  *ppxTimerTaskStackBuffer = timer_stack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif