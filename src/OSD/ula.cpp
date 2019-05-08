#include "osd.h"

extern volatile boolean xULAStop;
extern volatile boolean xULAStopped;

// Stop ULA service
void stopULA() {
    xULAStop = true;
    while (!xULAStopped) {
        delay(5);
    }
}

// Start ULA service
void startULA() {
    xULAStop = false;
    while (xULAStopped) {
        delay(5);
    }
}

// Just one ULA step
void stepULA() {
    startULA();
    stopULA();
}
