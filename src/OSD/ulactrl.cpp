#include <Arduino.h>

extern QueueHandle_t vidQueue;
extern TaskHandle_t videoTaskHandle;
extern volatile bool videoTaskIsRunning;
extern uint16_t *param;

void stepULA() {
    xQueueSend(vidQueue, &param, portMAX_DELAY);
    // Wait while ULA loop is finishing
    delay(45);
}
