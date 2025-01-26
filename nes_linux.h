#ifndef NES_LINUX_H
#define NES_LINUX_H
#include "cpunes.h"

void linux_print_debug (struct NESEmu *emu, void *_other_data);

void linux_init_callbacks (struct NESCallbacks *cb);

#endif
