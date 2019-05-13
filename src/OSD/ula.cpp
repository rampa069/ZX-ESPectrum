#include "osd.h"

extern volatile boolean xULAStop;
extern volatile boolean xULAStopped;

// Stop ULA service
void stopULA() {
    return;
    xULAStop = true;
    while (!xULAStopped) {
        delay(5);
    }
}

// Start ULA service
void startULA() {
    return;
    xULAStop = false;
    while (xULAStopped) {
        delay(5);
    }
}

// Just one ULA step
void stepULA() {
    return;
    startULA();
    stopULA();
}
