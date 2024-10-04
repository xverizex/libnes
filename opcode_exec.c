#include "opcode_exec.h"
#include "flags_checking.h"

static void wait_cycles (uint32_t cycles)
{

}


void adc_immediate (struct NESEmu *emu)
{
    flags_adc_imm (emu);
    emu->cpu.A += emu->buf[emu->cpu.PC + 1];
    emu->cpu.PC += 2;
    wait_cycles(2);
}

void adc_absolute (struct NESEmu *emu)
{
    flags_adc_absolute(emu);
    uint16_t addr = *(uint16_t *) &emu->buf[emu->cpu.PC + 1];
    emu->cpu.A += emu->buf[addr];
    emu->cpu.PC += 3;
    wait_cycles(4);
}
