#ifndef NES_DEBUGGER_H
#define NES_DEBUGGER_H

#include <cpunes.h>

void debug (struct NESEmu *emu);
char *debugger_print_regs (struct NESEmu *emu);

#endif
