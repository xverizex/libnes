#include "opcode_exec.h"
#include "flags_checking.h"

static void wait_cycles (struct NESEmu *emu, uint32_t start_pc, uint32_t cycles)
{

}

uint16_t immediate (struct NESEmu *emu)
{
    return emu->buf[emu->cpu.PC + 1];
}

uint16_t absolute (struct NESEmu *emu)
{
    uint16_t addr = *(uint16_t *) &emu->buf[emu->cpu.PC + 1];
    return addr;
}

uint16_t zeropage (struct NESEmu *emu)
{
    uint8_t addr = emu->buf[emu->cpu.PC + 1];
    return addr;
}

uint16_t zeropage_x (struct NESEmu *emu)
{
    uint8_t addr = emu->buf[emu->cpu.PC + 1] + emu->cpu.X;
    return addr;
}

uint16_t absolute_x (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->buf[emu->cpu.PC + 1]) + emu->cpu.X;
    return addr;
}

uint16_t absolute_y (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->buf[emu->cpu.PC + 1]) + emu->cpu.Y;
    return addr;
}

uint16_t indirect_x (struct NESEmu *emu)
{
    uint8_t addr = emu->buf[emu->cpu.PC + 1];
    uint16_t indirect = *((uint16_t *) &emu->buf[emu->cpu.X]);
    uint16_t fulladdr = addr + indirect;

    return fulladdr;
}

uint16_t indirect_y (struct NESEmu *emu)
{
    uint8_t zeroaddr = emu->buf[emu->cpu.PC + 1];
    uint16_t addr = *((uint16_t *) &emu->buf[zeroaddr]);
    uint16_t fulladdr = addr + emu->cpu.Y;

    return fulladdr;
}

void adc (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.A += emu->buf[addr];
}


void calc_addr (struct NESEmu *emu,
                uint16_t (*get_addr) (struct NESEmu *emu),
                void (*flags) (struct NESEmu *emu),
                void (*opcode_exec) (struct NESEmu *emu, uint16_t addr),
                uint16_t pc_offset,
                uint8_t cycles,
                uint8_t cross_page
                )
{
    flags (emu);
    uint16_t addr = get_addr (emu);
    opcode_exec (nes, addr);
    wait_cycles(emu, emu->cpu.PC, cycles);
    emu->cpu.PC += pc_offset;
}
