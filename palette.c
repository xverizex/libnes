#include <stdint.h>
#include <cpunes.h>

static uint32_t colors_2c02[64] = {
  0x626262, // 00
  0xae2e00, // 01
  0xc32706, // 02
  0xae2447, // 03
  0x731d6b, // 04
  0x241679, // 05
  0x04186d, // 06
  0x052a4f, // 07
  0x09422b, // 08
  0x0e541f, // 09
  0x105921, // 0A
  0x28501c, // 0B
  0x743e07, // 0C
  0x000000, // 0D
  0x000000, // 0E
  0x000000, // 0F
  0xababab, // 10
  0xf96200, // 11
  0xf9473a, // 12
  0xf93e7c, // 13
  0xd13aaf, // 14
  0x6833c6, // 15
  0x0e3cbd, // 16
  0x125797, // 17
  0x1a7865, // 18
  0x219344, // 19
  0x249e41, // 1a
  0x49943b, // 1b
  0xb17d25, // 1c
  0x000000, // 1d
  0x000000, // 1e
  0x000000, // 1f
  0xffffff, // 20
  0xfbaf5e, // 21
  0xfa8c8a, // 22
  0xfa75c7, // 23
  0xfa6ef0, // 24
  0xcc6ff1, // 25
  0x5b7ff4, // 26
  0x28a0f2, // 27
  0x30c4c0, // 28
  0x38e192, // 29
  0x3fef76, // 2a
  0x86e967, // 2b
  0xf4d357, // 2c
  0x4e4e4e, // 2d
  0x000000, // 2e
  0x000000, // 2f
  0xffffff, // 30
  0xfde1bb, // 31
  0xfdd3cd, // 32
  0xfcc7e3, // 33
  0xfcc2f7, // 34
  0xf2c2f7, // 35
  0xc4c8f9, // 36
  0x9ed5fb, // 37
  0x89e4ea, // 38
  0x8af0d5, // 39
  0xa1f6c2, // 3a
  0xc6f6b7, // 3b
  0xf3eeb3, // 3c
  0xb8b8b8, // 3d
  0x000000, // 3e
  0x000000  // 3f
};

#include <stdio.h>
void nes_get_colors_background_clear (struct NESEmu *emu, float *r, float *g, float *b)
{
	uint8_t color = emu->mem[0x3f00];

	uint32_t c = colors_2c02[color];

	*r = ((c >> 16) & 0xff) / 255.f;
	*g = ((c >>  8) & 0xff) / 255.f;
	*b = ((c >>  0) & 0xff) / 255.f;
}
