#include "exec.h"
#include "flags.h"

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

uint16_t zeropage_y (struct NESEmu *emu)
{
    uint8_t addr = emu->buf[emu->cpu.PC + 1] + emu->cpu.Y;
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

uint16_t indirect (struct NESEmu *emu)
{
	return 0;
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

void bmi (struct NESEmu *emu, uint16_t addr)
{

}

void bne (struct NESEmu *emu, uint16_t addr)
{

}

void bpl (struct NESEmu *emu, uint16_t addr)
{

}

void brk (struct NESEmu *emu, uint16_t addr)
{

}

void bvc (struct NESEmu *emu, uint16_t addr)
{

}

void bvs (struct NESEmu *emu, uint16_t addr)
{

}

void clc (struct NESEmu *emu, uint16_t addr)
{

}

void cld (struct NESEmu *emu, uint16_t addr)
{

}

void cli (struct NESEmu *emu, uint16_t addr)
{

}

void clv (struct NESEmu *emu, uint16_t addr)
{

}

void cmp (struct NESEmu *emu, uint16_t addr)
{

}

void cpx (struct NESEmu *emu, uint16_t addr)
{

}

void cpy (struct NESEmu *emu, uint16_t addr)
{

}

void dec (struct NESEmu *emu, uint16_t addr)
{

}

void dex (struct NESEmu *emu, uint16_t addr)
{

}

void dey (struct NESEmu *emu, uint16_t addr)
{

}

void eor (struct NESEmu *emu, uint16_t addr)
{

}

void inc (struct NESEmu *emu, uint16_t addr)
{

}

void inx (struct NESEmu *emu, uint16_t addr)
{

}

void iny (struct NESEmu *emu, uint16_t addr)
{

}

void jmp (struct NESEmu *emu, uint16_t addr)
{

}

void jsr (struct NESEmu *emu, uint16_t addr)
{

}

void lda (struct NESEmu *emu, uint16_t addr)
{

}

void ldx (struct NESEmu *emu, uint16_t addr)
{

}

void ldy (struct NESEmu *emu, uint16_t addr)
{

}

void lsr (struct NESEmu *emu, uint16_t addr)
{

}

void nop (struct NESEmu *emu, uint16_t addr)
{

}

void ora (struct NESEmu *emu, uint16_t addr)
{

}

void pha (struct NESEmu *emu, uint16_t addr)
{

}

void php (struct NESEmu *emu, uint16_t addr)
{

}

void pla (struct NESEmu *emu, uint16_t addr)
{

}

void plp (struct NESEmu *emu, uint16_t addr)
{

}

void rol (struct NESEmu *emu, uint16_t addr)
{

}

void ror (struct NESEmu *emu, uint16_t addr)
{

}

void rti (struct NESEmu *emu, uint16_t addr)
{

}

void rts (struct NESEmu *emu, uint16_t addr)
{

}

void sbc (struct NESEmu *emu, uint16_t addr)
{

}

void sec (struct NESEmu *emu, uint16_t addr)
{

}

void sed (struct NESEmu *emu, uint16_t addr)
{

}

void sei (struct NESEmu *emu, uint16_t addr)
{

}

void sta (struct NESEmu *emu, uint16_t addr)
{

}

void stx (struct NESEmu *emu, uint16_t addr)
{

}

void sty (struct NESEmu *emu, uint16_t addr)
{

}

void tax (struct NESEmu *emu, uint16_t addr)
{

}

void tay (struct NESEmu *emu, uint16_t addr)
{

}

void tsx (struct NESEmu *emu, uint16_t addr)
{

}

void txa (struct NESEmu *emu, uint16_t addr)
{

}

void txs (struct NESEmu *emu, uint16_t addr)
{

}

void tya (struct NESEmu *emu, uint16_t addr)
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
    if (opcode_exec) opcode_exec (emu, addr);
    wait_cycles(emu, addr, cycles);
    if (emu->is_branch == 0)
        emu->cpu.PC += pc_offset;
}
