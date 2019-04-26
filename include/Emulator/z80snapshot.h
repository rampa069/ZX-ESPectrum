#ifndef __Z80SNAPSHOT_INCLUDED__
#define __Z80SNAPSHOT_INCLUDED__

#include <stdint.h>
#include "SPIFFS.h"

namespace zx
{

bool LoadZ80Snapshot(File* file, uint8_t buffer1[0x4000], uint8_t buffer2[0x4000]);
bool LoadScreenFromZ80Snapshot(File* file, uint8_t buffer1[0x4000]);
bool LoadScreenshot(File* file, uint8_t buffer1[0x4000]);
bool SaveZ80Snapshot(File* file, uint8_t buffer1[0x4000], uint8_t buffer2[0x4000]);

}

#endif
