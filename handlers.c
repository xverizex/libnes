#include <stdint.h>
#include <cpunes.h>

uint16_t accumulator (struct NESEmu *emu);
uint16_t immediate (struct NESEmu *emu);
uint16_t absolute (struct NESEmu *emu);
uint16_t zeropage (struct NESEmu *emu);
uint16_t zeropage_x (struct NESEmu *emu);
uint16_t zeropage_y (struct NESEmu *emu);
uint16_t absolute_x (struct NESEmu *emu);
uint16_t absolute_y (struct NESEmu *emu);
uint16_t indirect (struct NESEmu *emu);
uint16_t indirect_x (struct NESEmu *emu);
uint16_t indirect_y (struct NESEmu *emu);

void ppu_ctrl (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

void ppu_mask (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

void ppu_status (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
	if (is_write)
		return;


}

void oam_addr (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

void oam_data (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

void ppu_scroll (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

void ppu_addr (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

void ppu_data (struct NESEmu *emu, uint8_t *r, uint8_t is_write)
{
}

static void internal_ram_addr (struct NESEmu *emu, uint8_t low, uint8_t high)
{
}

static void mirror_addr (struct NESEmu *emu, uint8_t low, uint8_t high)
{
}


static void nes_ppu_addr (struct NESEmu *emu, uint8_t low, uint8_t high, uint8_t *r, uint8_t is_write)
{
	if (low <= 7)
		emu->ppu_handler[low] (emu, r, is_write);
}

static void nes_apu_addr (struct NESEmu *emu, uint8_t low, uint8_t high)
{
}

static void apu_addr (struct NESEmu *emu, uint8_t low, uint8_t high)
{
}

static void cartridge_ram_addr (struct NESEmu *emu, uint8_t low, uint8_t high)
{
}

static void cartridge_rom_addr (struct NESEmu *emu, uint8_t low, uint8_t high)
{
}

static void work_addr (struct NESEmu *emu, uint8_t low, uint8_t high, uint8_t *r, uint8_t is_write)
{
	if (high < 0x40) {
		if (high < 0x20) {
			if (high < 0x08) {
				internal_ram_addr (emu, low, high);
			} else {
				mirror_addr (emu, low, high);
			}
		} else if (high == 0x20) {
			if (low < 0x08) {
				nes_ppu_addr (emu, low, high, r, is_write);
			} else {
				mirror_addr (emu, low, high);
			}
		}
	} else {
		if (high == 0x40) {
			if (low < 0x18) {
				nes_apu_addr (emu, low, high);
			} else if (low < 0x20) {
				apu_addr (emu, low, high);
			}
			/* unmapped. Available for cartidge use. */
		} else if (high < 80) {
			cartridge_ram_addr (emu, low, high);
		} else {
			cartridge_rom_addr (emu, low, high);
		}
	}
}

static void wait_cycles (struct NESEmu *emu, uint16_t addr, uint32_t cycles, uint32_t is_page)
{

}

uint16_t accumulator (struct NESEmu *emu)
{
    return 0;
}

uint16_t immediate (struct NESEmu *emu)
{
    return emu->mem[emu->cpu.PC + 1];
}

uint16_t absolute (struct NESEmu *emu)
{
    uint16_t addr = *(uint16_t *) &emu->mem[emu->cpu.PC + 1];
    return addr;
}

uint16_t zeropage (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    return addr;
}

uint16_t zeropage_x (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1] + emu->cpu.X;
    return addr;
}

uint16_t zeropage_y (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1] + emu->cpu.Y;
    return addr;
}

uint16_t absolute_x (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->mem[emu->cpu.PC + 1]) + emu->cpu.X;
    return addr;
}

uint16_t absolute_y (struct NESEmu *emu)
{
    uint16_t addr = *((uint16_t *) &emu->mem[emu->cpu.PC + 1]) + emu->cpu.Y;
    return addr;
}

uint16_t indirect (struct NESEmu *emu)
{
	return 0;
}

uint16_t indirect_x (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    uint16_t indirect = *((uint16_t *) &emu->mem[emu->cpu.X]);
    uint16_t fulladdr = addr + indirect;

    return fulladdr;
}

uint16_t indirect_y (struct NESEmu *emu)
{
    uint8_t zeroaddr = emu->mem[emu->cpu.PC + 1];
    uint16_t addr = *((uint16_t *) &emu->mem[zeroaddr]);
    uint16_t fulladdr = addr + emu->cpu.Y;

    return fulladdr;
}

void adc (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.A += emu->mem[addr];
}

void _and (struct NESEmu *emu, uint16_t addr)
{
    emu->cpu.A &= emu->mem[addr];
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
        emu->cpu.PC += (int8_t) emu->mem[emu->cpu.PC + 1];
    }
}

void bcs (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_CF) {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->mem[emu->cpu.PC + 1];
    }
}

void beq (struct NESEmu *emu, uint16_t addr)
{
    (void) addr;
    if (emu->cpu.P & STATUS_FLAG_ZF) {
    } else {
        emu->is_branch = 1;
        emu->cpu.PC += (int8_t) emu->mem[emu->cpu.PC + 1];
    }
}

void invalid_opcode (struct NESEmu *) {}
void brk_implied (struct NESEmu *) {}
void ora_indirect_x (struct NESEmu *) {}
void ora_zeropage (struct NESEmu *) {}
void asl_zeropage (struct NESEmu *) {}
void php_implied (struct NESEmu *) {}
void ora_immediate (struct NESEmu *) {}
void asl_accumulator (struct NESEmu *) {}
void ora_absolute (struct NESEmu *) {}
void asl_absolute (struct NESEmu *) {}
void bpl_relative (struct NESEmu *) {}
void ora_indirect_y (struct NESEmu *) {}
void ora_zeropage_x (struct NESEmu *) {}
void asl_zeropage_x (struct NESEmu *) {}
void clc_implied (struct NESEmu *) {}
void ora_absolute_y (struct NESEmu *) {}
void ora_absolute_x (struct NESEmu *) {}
void asl_absolute_x (struct NESEmu *) {}
void jsr_absolute (struct NESEmu *) {}
void and_indirect_x (struct NESEmu *) {}
void bit_zeropage (struct NESEmu *) {}
void and_zeropage (struct NESEmu *) {}
void rol_zeropage (struct NESEmu *) {}
void plp_implied (struct NESEmu *) {}
void and_immediate (struct NESEmu *) {}
void rol_accumulator (struct NESEmu *) {}

void bit_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t low = emu->mem[cpu->PC++];
	uint8_t high = emu->mem[cpu->PC++];

	work_addr (emu, low, high, NULL, 0);

	wait_cycles (emu, 0, 4, 0);
}

void and_absolute (struct NESEmu *) {}
void rol_absolute (struct NESEmu *) {}
void bmi_relative (struct NESEmu *) {}
void and_indirect_y (struct NESEmu *) {}
void and_zeropage_x (struct NESEmu *) {}
void rol_zeropage_x (struct NESEmu *) {}
void sec_implied (struct NESEmu *) {}
void and_absolute_y (struct NESEmu *) {}
void and_absolute_x (struct NESEmu *) {}
void rol_absolute_x (struct NESEmu *) {}
void rti_implied (struct NESEmu *) {}
void eor_indirect_x (struct NESEmu *) {}
void eor_zeropage (struct NESEmu *) {}
void lsr_zeropage (struct NESEmu *) {}
void pha_implied (struct NESEmu *) {}
void eor_immediate (struct NESEmu *) {}
void lsr_accumulator (struct NESEmu *) {}
void jmp_absolute (struct NESEmu *) {}
void eor_absolute (struct NESEmu *) {}
void lsr_absolute (struct NESEmu *) {}
void bvc_relative (struct NESEmu *) {}
void eor_indirect_y (struct NESEmu *) {}
void eor_zeropage_x (struct NESEmu *) {}
void lsr_zeropage_x (struct NESEmu *) {}
void cli_implied (struct NESEmu *) {}
void eor_absolute_y (struct NESEmu *) {}
void eor_absolute_x (struct NESEmu *) {}
void lsr_absolute_x (struct NESEmu *) {}
void rts_implied (struct NESEmu *) {}
void adc_indirect_x (struct NESEmu *) {}
void adc_zeropage (struct NESEmu *) {}
void ror_zeropage (struct NESEmu *) {}
void pla_implied (struct NESEmu *) {}
void adc_immediate (struct NESEmu *) {}
void ror_accumulator (struct NESEmu *) {}
void jmp_indirect (struct NESEmu *) {}
void adc_absolute (struct NESEmu *) {}
void ror_absolute (struct NESEmu *) {}
void bvs_relative (struct NESEmu *) {}
void adc_indirect_y (struct NESEmu *) {}
void adc_zeropage_x (struct NESEmu *) {}
void ror_zeropage_x (struct NESEmu *) {}

void sei_implied (struct NESEmu *emu) 
{
	emu->cpu.P &= (STATUS_FLAG_IF);
	emu->cpu.PC++;

	wait_cycles (emu, 0, 2, 0);
}

void adc_absolute_y (struct NESEmu *) {}
void adc_absolute_x (struct NESEmu *) {}
void ror_absolute_x (struct NESEmu *) {}
void sta_indirect_x (struct NESEmu *) {}
void sty_zeropage (struct NESEmu *) {}
void sta_zeropage (struct NESEmu *) {}
void stx_zeropage (struct NESEmu *) {}
void dey_implied (struct NESEmu *) {}
void txa_implied (struct NESEmu *) {}

void sty_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t low = emu->mem[cpu->PC++];
	uint8_t high = emu->mem[cpu->PC++];

	uint8_t *r = &cpu->Y;

	work_addr (emu, low, high, r, 1);

	wait_cycles (emu, 0, 4, 0);
}

void sta_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t low = emu->mem[cpu->PC++];
	uint8_t high = emu->mem[cpu->PC++];

	uint8_t *r = &cpu->A;

	work_addr (emu, low, high, r, 1);

	wait_cycles (emu, 0, 4, 0);
}



void stx_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint8_t low = emu->mem[cpu->PC++];
	uint8_t high = emu->mem[cpu->PC++];

	uint8_t *r = &cpu->X;

	work_addr (emu, low, high, r, 1);

	wait_cycles (emu, 0, 4, 0);
}

void bcc_relative (struct NESEmu *) {}
void sta_indirect_y (struct NESEmu *) {}
void sty_zeropage_x (struct NESEmu *) {}
void sta_zeropage_x (struct NESEmu *) {}
void stx_zeropage_y (struct NESEmu *) {}
void tya_implied (struct NESEmu *) {}
void sta_absolute_y (struct NESEmu *) {}
void txs_implied (struct NESEmu *) {}
void sta_absolute_x (struct NESEmu *) {}
void ldy_immediate (struct NESEmu *) {}
void lda_indirect_x (struct NESEmu *) {}

void ldx_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;
	uint8_t imm_byte = emu->mem[emu->cpu.PC];
	if (imm_byte == 0) {
		cpu->P |= (STATUS_FLAG_ZF);
	}
	if (imm_byte >= 0x80) {
		cpu->P |= (STATUS_FLAG_NF);
	}
	cpu->X = imm_byte;

	wait_cycles (emu, 0, 2, 0);

	cpu->PC++;
}

void ldy_zeropage (struct NESEmu *) {}
void lda_zeropage (struct NESEmu *) {}
void ldx_zeropage (struct NESEmu *) {}
void tay_implied (struct NESEmu *) {}

void lda_immediate (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;
	uint8_t imm_byte = emu->mem[emu->cpu.PC];
	if (imm_byte == 0) {
		cpu->P |= (STATUS_FLAG_ZF);
	}
	if (imm_byte >= 0x80) {
		cpu->P |= (STATUS_FLAG_NF);
	}
	cpu->A = imm_byte;

	wait_cycles (emu, 0, 2, 0);

	cpu->PC++;
}

void tax_implied (struct NESEmu *) {}
void ldy_absolute (struct NESEmu *) {}
void lda_absolute (struct NESEmu *) {}
void ldx_absolute (struct NESEmu *) {}
void bcs_relative (struct NESEmu *) {}
void lda_indirect_y (struct NESEmu *) {}
void ldy_zeropage_x (struct NESEmu *) {}
void lda_zeropage_x (struct NESEmu *) {}
void ldx_zeropage_y (struct NESEmu *) {}
void clv_implied (struct NESEmu *) {}
void lda_absolute_y (struct NESEmu *) {}
void tsx_implied (struct NESEmu *) {}
void ldy_absolute_x (struct NESEmu *) {}
void lda_absolute_x (struct NESEmu *) {}
void ldx_absolute_y (struct NESEmu *) {}
void cpy_immediate (struct NESEmu *) {}
void cmp_indirect_x (struct NESEmu *) {}
void cpy_zeropage (struct NESEmu *) {}
void cmp_zeropage (struct NESEmu *) {}
void dec_zeropage (struct NESEmu *) {}
void iny_implied (struct NESEmu *) {}
void cmp_immediate (struct NESEmu *) {}
void dex_implied (struct NESEmu *) {}
void cpy_absolute (struct NESEmu *) {}
void cmp_absolute (struct NESEmu *) {}
void dec_absolute (struct NESEmu *) {}
void bne_relative (struct NESEmu *) {}
void cmp_indirect_y (struct NESEmu *) {}
void cmp_zeropage_x (struct NESEmu *) {}
void dec_zeropage_x (struct NESEmu *) {}

void cld_implied (struct NESEmu *emu) 
{
	emu->cpu.P &= ~(STATUS_FLAG_DF);

	wait_cycles (emu, 0, 2, 0);

	emu->cpu.PC++;
}

void cmp_absolute_y (struct NESEmu *) {}
void cmp_absolute_x (struct NESEmu *) {}
void dec_absolute_x (struct NESEmu *) {}
void cpx_immediate (struct NESEmu *) {}
void sbc_indirect_x (struct NESEmu *) {}
void cpx_zeropage (struct NESEmu *) {}
void sbc_zeropage (struct NESEmu *) {}
void inc_zeropage (struct NESEmu *) {}
void inx_implied (struct NESEmu *) {}
void sbc_immediate (struct NESEmu *) {}
void nop_implied (struct NESEmu *) {}
void cpx_absolute (struct NESEmu *) {}
void sbc_absolute (struct NESEmu *) {}
void inc_absolute (struct NESEmu *) {}
void beq_relative (struct NESEmu *) {}
void sbc_indirect_y (struct NESEmu *) {}
void sbc_zeropage_x (struct NESEmu *) {}
void inc_zeropage_x (struct NESEmu *) {}
void sed_implied (struct NESEmu *) {}
void sbc_absolute_y (struct NESEmu *) {}
void sbc_absolute_x (struct NESEmu *) {}
void inc_absolute_x (struct NESEmu *) {}

#if 0
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
#endif
