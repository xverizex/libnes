#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cpunes.h"
#include <glad/gl.h>

int scanline_delay (struct NESEmu *emu);
uint32_t platform_and_scanline_delay (struct NESEmu *emu);
void nes_copy_texture_surface (struct NESEmu *emu, uint32_t *tex);

#ifdef __cplusplus
}
#endif
#endif
