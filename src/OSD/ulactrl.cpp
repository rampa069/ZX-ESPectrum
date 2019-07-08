#include <Arduino.h>
#include "Emulator/Sound/AY-emulator.h"
extern QueueHandle_t vidQueue;
extern TaskHandle_t videoTaskHandle;
extern volatile bool videoTaskIsRunning;
extern uint16_t *param;

void stepULA() {
    xQueueSend(vidQueue, &param, portMAX_DELAY);
    // Wait while ULA loop is finishing
    ay_reset(false);
    delay(45);
}
