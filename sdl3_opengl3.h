#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cpunes.h"
#include <GLES3/gl3.h>

int scanline_delay (struct NESEmu *emu);
uint32_t platform_and_scanline_delay (struct NESEmu *emu);

#ifdef __cplusplus
}
#endif
#endif
