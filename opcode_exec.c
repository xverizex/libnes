#include "opcode_exec.h"
#include "flags_checking.h"

static void wait_cycles (struct NESEmu *emu, uint32_t addr, uint32_t cycles)
{

}

uint16_t accumulator (struct NESEmu *emu)
{
    return 0;
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

void _and (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.A &= emu->buf[addr];
}

void asl (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    emu->cpu.A <<= 1;
}

void bcc (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_CF) {
    } else {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->buf[emu->cpu.PC + 1];
    }
}

void bcs (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_CF) {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->buf[emu->cpu.PC + 1];
    }
}

void beq (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_ZF) {
    } else {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->buf[emu->cpu.PC + 1];
    }
}

void bit (struct NESEmu *emu, uint16_t addr)
{

}

void calc_addr (struct NESEmu *emu,
                uint16_t (*get_addr) (struct NESEmu *emu),
                void (*flags) (struct NESEmu *emu, uint16_t addr),
                void (*opcode_exec) (struct NESEmu *emu, uint16_t addr),
                uint16_t pc_offset,
                uint8_t cycles,
                uint8_t cross_page
                )
{
    emu->is_branch = 0;
    uint16_t addr = 0;
    if (get_addr) addr = get_addr (emu);
    if (flags) flags (emu, addr);
    if (opcode_exec) opcode_exec (nes, addr);
    wait_cycles(emu, addr, cycles);
    if (emu->is_branch == 0)
        emu->cpu.PC += pc_offset;
}
