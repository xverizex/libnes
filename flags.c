#include "flags.h"
#include <cpunes.h>

void flags_adc_imm (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_absolute (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_zeropage (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_zeropage_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_absolute_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_absolute_y (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_indirect_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_adc_indirect_y (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_imm (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_absolute (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_zeropage (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_zeropage_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_absolute_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_absolute_y (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_indirect_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_and_indirect_y (struct NESEmu *emu, uint16_t addr)
{

}

void flags_asl_accumulator (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.P &= ~(STATUS_FLAG_CF);
    (void) addr;
    if (emu->cpu.A & 0x80) {
        emu->cpu.P |= STATUS_FLAG_CF;
    }
}

void flags_asl_zeropage (struct NESEmu *emu, uint16_t addr)
{

}

void flags_asl_zeropage_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_asl_absolute_x (struct NESEmu *emu, uint16_t addr)
{

}

void flags_asl_absolute (struct NESEmu *emu, uint16_t addr)
{

}
