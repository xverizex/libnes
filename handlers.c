#include <stdint.h>
#include <stddef.h>
#include <cpunes.h>
#include <stdio.h>

#define CHECK_FLAGS_LD(flags, reg) \
{ \
	if (flags & STATUS_FLAG_NF) { \
		if (((int8_t) reg) < 0) { \
			cpu->P |= (STATUS_FLAG_NF); \
		} \
	} \
	if (flags & STATUS_FLAG_ZF) { \
		if (reg == 0) { \
			cpu->P |= (STATUS_FLAG_ZF); \
		} \
	} \
}

#define CHECK_FLAGS(flags, reg, ret) \
{ \
	if (flags & STATUS_FLAG_NF) { \
		if (((int8_t) ret) < 0) { \
			cpu->P |= (STATUS_FLAG_NF); \
		} \
	} \
	if (flags & STATUS_FLAG_ZF) { \
		if (ret == 0) { \
			cpu->P |= (STATUS_FLAG_ZF); \
		} \
	} \
	if (flags & STATUS_FLAG_CF) { \
		if (ret < reg) \
			cpu->P |= (STATUS_FLAG_CF); \
	} \
	if (((int8_t) ret < 0) && ((int8_t) reg > 0)) \
		cpu->P |= STATUS_FLAG_VF; \
	if (((int8_t) ret > 0) && ((int8_t) reg < 0)) \
		cpu->P |= STATUS_FLAG_VF; \
}

#define REPETITIVE_ACTS(_flags, reg, calc, eq, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t flags = _flags; \
	uint8_t ret = 0; \
	ret = calc; \
	cpu->P &= ~(flags); \
	CHECK_FLAGS (flags, reg, ret); \
	reg eq ret; \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define ASL_ACTS(_flags, mem, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t flags = _flags; \
	uint8_t ret = 0; \
	uint8_t bt = mem; \
	cpu->P &= ~(flags); \
	if (bt & 0x80) { \
		cpu->P |= STATUS_FLAG_CF; \
	} else { \
		cpu->P |= STATUS_FLAG_ZF; \
	} \
	if (bt & 0x40) { \
		cpu->P |= STATUS_FLAG_NF; \
	} \
	bt <<= 1; \
	mem = bt; \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define LSR_ACTS(_flags, mem, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t flags = _flags; \
	uint8_t ret = 0; \
	uint8_t bt = mem; \
	cpu->P &= ~(flags); \
	if (bt & 0x01) { \
		cpu->P |= STATUS_FLAG_CF; \
	} else { \
		cpu->P |= STATUS_FLAG_ZF; \
	} \
	bt >>= 1; \
	mem = bt; \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define ROL_ACTS(_flags, mem, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t flags = _flags; \
	uint8_t ret = 0; \
	uint8_t bt = mem; \
	cpu->P &= ~(flags); \
	if (bt & 0x80) { \
		cpu->P |= STATUS_FLAG_CF; \
	} else { \
		cpu->P |= STATUS_FLAG_ZF; \
	} \
	if (bt & 0x40) { \
		cpu->P |= STATUS_FLAG_NF; \
	} \
	bt <<= 1; \
	if (cpu->P & STATUS_FLAG_CF) \
		bt |= 0x1; \
	mem = bt; \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define ROR_ACTS(_flags, mem, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t flags = _flags; \
	uint8_t ret = 0; \
	uint8_t bt = mem; \
	cpu->P &= ~(flags); \
	if (bt & 0x01) { \
		cpu->P |= STATUS_FLAG_CF; \
	} else { \
		cpu->P |= STATUS_FLAG_ZF; \
	} \
	bt >>= 1; \
	if (cpu->P & STATUS_FLAG_CF) \
		bt |= 0x80; \
	mem = bt; \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define LD_ACTS(_flags, reg, get_addr, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t flags = _flags; \
	uint8_t ret = 0; \
	uint16_t addr = get_addr; \
	read_from_address (emu, addr, &reg); \
	ret = reg; \
	cpu->P &= ~(flags); \
	CHECK_FLAGS_LD (flags, reg); \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define ST_ACTS(reg, get_addr, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t ret = 0; \
	uint16_t addr = get_addr; \
	write_to_address (emu, addr, &reg); \
	wait_cycles (emu, cycles); \
	emu->cpu.PC += _bytes; \
}

#define BIT_ACTS(_flags, mem, cycles, is_off, _bytes) { \
	struct CPUNes *cpu = &emu->cpu; \
	uint8_t returned_reg = 0; \
	read_from_address (emu, mem, &returned_reg); \
	cpu->P &= ~(_flags); \
	CHECK_FLAGS (_flags, cpu->A, returned_reg); \
	cpu->P &= ~(STATUS_FLAG_ZF); \
	if ((cpu->A & returned_reg) == 0) { \
		cpu->P |= (STATUS_FLAG_ZF); \
	} \
	wait_cycles (emu, cycles); \
	cpu->PC += _bytes; \
}

uint16_t accumulator (struct NESEmu *emu);
uint16_t immediate (struct NESEmu *emu);
uint8_t immediate_val (struct NESEmu *emu);
uint16_t absolute (struct NESEmu *emu);
uint8_t zeropage (struct NESEmu *emu);
uint8_t zeropage_x (struct NESEmu *emu);
uint8_t zeropage_y (struct NESEmu *emu);
uint16_t absolute_x (struct NESEmu *emu);
uint16_t absolute_y (struct NESEmu *emu);
uint16_t indirect (struct NESEmu *emu);
uint16_t indirect_x (struct NESEmu *emu);
uint16_t indirect_y (struct NESEmu *emu);

static void write_to_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	if (addr == OAMADDR) {
		emu->addr = 0;
		emu->addr |= *r;
		return;
	}
	if (addr == OAMDMA) {
		emu->addr |= (*r << 8) & 0xff00;
		//printf ("dma: %04x\n", emu->addr);
		return;
	}

	if (addr >= 0 && addr <= 0x800) {
		if (emu->addr >= 0x200 && emu->addr <= 0x2ff) {
			emu->oam[emu->addr++ - 0x200] = *r;
		} else {
			emu->ram[addr] = *r;
		}
		return;
	}

	if (addr == PPUMASK) {
		printf ("ppumask\n");
		emu->mem[addr] = *r;
		if ((*r) & MASK_IS_BACKGROUND_RENDER) {
			printf ("clear screen\n");
			emu->is_return = 1;
			emu->cb->ppu_mask (emu, NULL);
		}

	} else if (addr == PPUADDR) {
		if (emu->addr_off == 0) {
			emu->addr = 0;
			emu->addr |= (*r << 8) & 0xff00;
			emu->addr_off++;
		} else {
			emu->addr |= *r & 0xff;
			emu->addr_off = 0;
		}
	} else if (addr == PPUDATA) {
		if (emu->addr >= 0x2000 && emu->addr < 0x4000) {
			emu->ppu[emu->addr++ - 0x2000] = *r; //screen on the 0x2000
		} else {
			emu->mem[emu->addr++] = *r;
		}
	} else {
		emu->mem[addr] = *r;
	}
}

static void read_from_address (struct NESEmu *emu, uint16_t addr, uint8_t *r)
{
	if (addr == 0x2002) {
		*r = 0x80;
		return;
	}
	if (addr >= 0 && addr <= 0x800) {
		*r = emu->ram[addr];
	} else {
		*r = emu->mem[addr];
	}
}

static void wait_cycles (struct NESEmu *emu, uint32_t cycles)
{
	emu->last_cycles_float = (float) cycles * 0.000601465f;
	emu->last_cycles_int64 = 16L;
	emu->last_cycles_int64 = cycles * 601465L;
}

static inline void set_ext_cycles (struct CPUNes *cpu, int8_t offset, uint16_t new_offset, uint32_t *ext_cycles)
{
	*ext_cycles = 1;

	if (offset < 0) {
		uint8_t c = cpu->PC;
		uint8_t n = new_offset;
		
		if (c < n) {
			*ext_cycles = 2;
		}
	} else if (offset > 0) {
		uint8_t c = cpu->PC;
		uint8_t n = new_offset;
		
		if (c > n) {
			*ext_cycles = 2;
		}
	}
}

uint16_t accumulator (struct NESEmu *emu)
{
    return 0;
}

uint16_t immediate (struct NESEmu *emu)
{
    return (emu->cpu.PC + 1);
}

uint8_t immediate_val (struct NESEmu *emu)
{
    return emu->mem[(emu->cpu.PC + 1)];
}

uint16_t absolute (struct NESEmu *emu)
{
    uint16_t addr = emu->mem[emu->cpu.PC + 1];
    addr |= ((emu->mem[emu->cpu.PC + 2] << 8) & 0xff00);
    return addr;
}

uint8_t zeropage (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    return addr;
}

uint8_t zeropage_x (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    uint8_t off = addr;
    addr += emu->cpu.X;
    return addr;
}

uint8_t zeropage_y (struct NESEmu *emu)
{
    uint8_t addr = emu->mem[emu->cpu.PC + 1];
    addr += emu->cpu.Y;
    return addr;
}

uint16_t absolute_x (struct NESEmu *emu)
{
    uint16_t addr = emu->mem[emu->cpu.PC + 1];
    addr |= ((emu->mem[emu->cpu.PC + 2] << 8) & 0xff00);
    addr += emu->cpu.X;
    addr += ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0);
    return addr;
}

uint16_t absolute_y (struct NESEmu *emu)
{
    uint16_t addr = emu->mem[emu->cpu.PC + 1];
    addr |= ((emu->mem[emu->cpu.PC + 2] << 8) & 0xff00);
    addr += emu->cpu.Y;
    addr += ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0);
    return addr;
}

uint16_t indirect (struct NESEmu *emu)
{
    	uint16_t addr = emu->mem[emu->cpu.PC + 1];
    	addr |= ((emu->mem[emu->cpu.PC + 2] << 8) & 0xff00);

	uint16_t new_addr = 0;
	if (addr >= 0x0 && addr <= 0x800) {
    		new_addr = emu->ram[addr + 0];
    		new_addr |= ((emu->ram[addr + 1] << 8) & 0xff00);
	} else {
    		new_addr = emu->mem[addr + 0];
    		new_addr |= ((emu->mem[addr + 1] << 8) & 0xff00);
	}

	return new_addr;
}

uint16_t indirect_x (struct NESEmu *emu)
{
	uint8_t addr = emu->mem[emu->cpu.PC + 1];
	uint16_t indirect = 0;

	indirect = *((uint16_t *) &emu->ram[addr + emu->cpu.X]);

	return indirect;
}

uint16_t indirect_y (struct NESEmu *emu)
{
	uint8_t zeroaddr = emu->mem[emu->cpu.PC + 1];
	uint16_t addr = 0;

	addr = *((uint16_t *) &emu->ram[zeroaddr]);

	return addr + emu->cpu.Y;// + ((emu->cpu.P & STATUS_FLAG_CF) ? 1: 0);
}

void invalid_opcode (struct NESEmu *emu) 
{
	//emu->cpu.PC++;
}

void brk_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

#if 0
	emu->stack[--cpu->S] = (uint8_t) (cpu->PC & 0xff);
	emu->stack[--cpu->S] = (uint8_t) ((cpu->PC >> 8) & 0xff);
	emu->stack[--cpu->S] = cpu->P;

	cpu->P |= (STATUS_FLAG_IF);

	cpu->PC = emu->irq_handler;
#endif

	wait_cycles (emu, 7);
	printf ("brk\n");
}

void ora_indirect_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->ram[indirect_x(emu)], =, 6, 0, 2);
}

void ora_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->ram[zeropage(emu)], =, 3, 0, 2);
}

void asl_zeropage (struct NESEmu *emu) 
{
	ASL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage (emu)], 5, 0, 2);
}

void php_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	emu->stack[--cpu->S] = cpu->P;

	wait_cycles (emu, 3);

	cpu->PC++;
}

void ora_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A | immediate_val (emu), =, 2, 0, 2);
}
void asl_accumulator (struct NESEmu *emu) 
{
	ASL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, 2, 0, 1);
}

void ora_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->mem[absolute(emu)], =, 4, 0, 3);
}

void asl_absolute (struct NESEmu *emu) 
{
	ASL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute (emu)], 6, 0, 3);
}

static uint8_t hex_number_str (uint8_t n)
{
	if (n >= 0 && n <= 9) {
		return n + '0';
	}

	if (n >= 10 && n <= 15) {
		return (n - 10) + 'A';
	}
}

static uint8_t hex_up_half (uint8_t n)
{
	n = n >> 4;

	return hex_number_str (n);
}

static uint8_t hex_down_half (uint8_t n)
{
	n = n & 0xf;

	return hex_number_str (n);
}

static uint8_t *show_debug_info (struct NESEmu *emu, uint32_t count, char *op)
{
	uint32_t tcount = 0;
	struct CPUNes *cpu = &emu->cpu;

	uint8_t *pl = emu->line;
	if (emu->is_show_address) {
		*pl++ = hex_up_half (cpu->PC >> 8);
		*pl++ = hex_down_half (cpu->PC >> 8);
		*pl++ = hex_up_half (cpu->PC & 0xff);
		*pl++ = hex_down_half (cpu->PC & 0xff);
		*pl++ = ':';
		*pl++ = ' ';
	}
	if (emu->is_show_bytes) {
		for (uint16_t i = 0; i < count; i++) {
			tcount++;
			*pl++ = hex_up_half (emu->mem[emu->cpu.PC + i]);
			*pl++ = hex_down_half (emu->mem[emu->cpu.PC + i]);
			*pl++ = ' ';
		}

	}

	while (tcount < 3) {
		*pl++ = ' ';
		*pl++ = ' ';
		*pl++ = ' ';
		tcount++;
	}
	while (*op != 0) {
		*pl++ = *op++;
	}

	*pl++ = ' ' ;

	return pl;
}
#if 0
	/* lst */
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];
		uint16_t new_offset = cpu->PC + offset;

		uint8_t *pl = show_debug_info (emu, 2, "BPL");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xf);
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		return;
	}
#endif

#include <stdlib.h>
void bpl_relative (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];
		uint16_t new_offset;

		if (offset < 0) {
			uint8_t off = 0xff - offset - 1;
			new_offset = cpu->PC - off;
		} else {
			new_offset = cpu->PC + offset;
		}

		uint8_t *pl = show_debug_info (emu, 2, "BPL");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 2;
		return;
	}
#endif

	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_NF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void ora_indirect_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->ram[indirect_y(emu)], =, 5, 1, 2);
}

void ora_zeropage_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->ram[zeropage_x(emu)], =, 4, 0, 2);
}

void asl_zeropage_x (struct NESEmu *emu) 
{
	ASL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage_x (emu)], 6, 0, 2);
}
void clc_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P &= ~(STATUS_FLAG_CF);

	wait_cycles (emu, 2);

	cpu->PC++;
}

void ora_absolute_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->mem[absolute_y(emu)], =, 4, 1, 3);
}

void ora_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A | emu->mem[absolute_x(emu)], =, 4, 1, 3);
}

void asl_absolute_x (struct NESEmu *emu) 
{
	ASL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute_x (emu)], 7, 0, 3);
}

void jsr_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	uint16_t pc = cpu->PC + 3;

	pc--;
	printf ("stacked %04x\n", pc);

	emu->stack[--cpu->S] = (uint8_t) ((pc >> 8) & 0xff);
	emu->stack[--cpu->S] = (uint8_t) (pc & 0xff);

#if 0
	uint16_t new_pc = 0;
	printf ("cur pc: %x\n", cpu->PC);
	new_pc = emu->mem[cpu->PC + 1];
	new_pc |= (( emu->mem[cpu->PC + 2] << 8) & 0xff00);
	cpu->PC = new_pc;
	printf ("called pc: %x\n", cpu->PC);
	printf ("returned pc: %x\n", pc);
#else
	uint16_t new_pc = 0;
	new_pc = emu->mem[cpu->PC + 1];
	new_pc |= (( emu->mem[cpu->PC + 2] << 8) & 0xff00);
	cpu->PC = new_pc;
	printf ("called: %04x\n", new_pc);
#endif

	wait_cycles (emu, 6);
}

void and_indirect_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->ram[indirect_x (emu)], =, 6, 0, 2);
}

void bit_zeropage (struct NESEmu *emu) 
{
	BIT_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_VF, zeropage (emu), 3, 0, 2);
}

void and_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->ram[zeropage (emu)], =, 3, 0, 2);
}

void rol_zeropage (struct NESEmu *emu) 
{
	ROL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage (emu)], 5, 0, 2);
}

void plp_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->P = emu->stack[cpu->S++];

	wait_cycles (emu, 4);

	cpu->PC++;
}



void and_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & immediate_val (emu), =, 2, 0, 2);
}
void rol_accumulator (struct NESEmu *emu) 
{
	ROL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, 2, 0, 1);
}

void bit_absolute (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "BIT");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	BIT_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_VF, absolute (emu), 4, 0, 3);
}

void and_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->mem[absolute (emu)], =, 4, 0, 3);
}

void rol_absolute (struct NESEmu *emu) 
{
	ROL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute (emu)], 6, 0, 3);
}

void bmi_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_NF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void and_indirect_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->ram[indirect_y (emu)], =, 5, 1, 2);
}

void and_zeropage_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->ram[zeropage_x (emu)], =, 4, 0, 2);
}

void rol_zeropage_x (struct NESEmu *emu) 
{
	ROL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage_x (emu)], 6, 0, 2);
}

void sec_implied (struct NESEmu *) {}

void and_absolute_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->mem[absolute_y (emu)], =, 4, 1, 3);
}

void and_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A & emu->mem[absolute_x (emu)], =, 4, 1, 3);
}

void rol_absolute_x (struct NESEmu *emu) 
{
	ROL_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute_x (emu)], 7, 0, 3);
}

void rti_implied (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 1, "RTI");
		*pl = 0;
		cpu->PC++;
		return;
	}
#endif

	struct CPUNes *cpu = &emu->cpu;

	cpu->P = emu->stack[cpu->S++];

	uint16_t new_pc = 0;
	new_pc = emu->stack[cpu->S++];
	new_pc |= ((emu->stack[cpu->S++] << 8) & 0xff00);

	cpu->PC = new_pc;

	wait_cycles (emu, 6);
}

void eor_indirect_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->ram[indirect_x (emu)], =, 5, 1, 2);
}

void eor_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->ram[zeropage (emu)], =, 3, 0, 2);
}

void lsr_zeropage (struct NESEmu *emu) 
{
	LSR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage (emu)], 5, 0, 2);
}

void pha_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	emu->stack[--cpu->S] = cpu->A;

	wait_cycles (emu, 3);

	cpu->PC++;
}

void eor_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ immediate_val (emu), =, 2, 0, 2);
}
void lsr_accumulator (struct NESEmu *emu) 
{
	LSR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, 2, 0, 1);
}

void jmp_absolute (struct NESEmu *emu)
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "JMP");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	struct CPUNes *cpu = &emu->cpu;

	cpu->PC++;

	uint16_t new_offset = emu->mem[cpu->PC + 0];
	new_offset |= ((emu->mem[cpu->PC + 1] << 8) & 0xff00);

	cpu->PC = new_offset;

	wait_cycles (emu, 3);
}

void eor_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->mem[absolute (emu)], =, 4, 0, 3);
}

void lsr_absolute (struct NESEmu *emu) 
{
	LSR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute (emu)], 6, 0, 3);
}

void bvc_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_VF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void eor_indirect_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->ram[indirect_y (emu)], =, 6, 0, 2);
}

void eor_zeropage_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->ram[zeropage_x (emu)], =, 4, 0, 2);
}

void lsr_zeropage_x (struct NESEmu *emu) 
{
	LSR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage_x (emu)], 6, 0, 2);
}

void cli_implied (struct NESEmu *) {}

void eor_absolute_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->mem[absolute_y (emu)], =, 4, 1, 3);
}

void eor_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->A, cpu->A ^ emu->mem[absolute_x (emu)], =, 4, 1, 3);
}

void lsr_absolute_x (struct NESEmu *emu) 
{
	LSR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute_x (emu)], 7, 0, 3);
}

void rts_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->PC = emu->stack[cpu->S++];
	cpu->PC |= ((emu->stack[cpu->S++] << 8) & 0xff00);

	cpu->PC++;
	printf ("rts %04x\n", cpu->PC);

	wait_cycles (emu, 6);
}

void adc_indirect_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF);
	cpu->P &= ~(flags);

	uint16_t addr = indirect_x (emu);
	uint8_t ret = cpu->A + emu->ram[addr];
	CHECK_FLAGS (flags, cpu->A, ret);

	cpu->A = ret;

	wait_cycles (emu, 6);

	emu->cpu.PC += 2;

}

void adc_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A + emu->ram[zeropage(emu)], =, 3, 0, 2);
}

void ror_zeropage (struct NESEmu *emu) 
{
	ROR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage (emu)], 5, 0, 2);
}

void pla_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->A = emu->stack[cpu->S++];

	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF);
	cpu->P &= ~(flags);

	if (cpu->A >= 0x80) {
		cpu->P |= STATUS_FLAG_NF;
	} else if (cpu->A == 0x0) {
		cpu->P |= STATUS_FLAG_ZF;
	}

	wait_cycles (emu, 4);

	cpu->PC++;
}



void adc_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A + immediate_val (emu), =, 2, 0, 2);
}

void ror_accumulator (struct NESEmu *emu) 
{
	ROR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, 2, 0, 1);
}

void jmp_indirect (struct NESEmu *emu) 
{
	emu->cpu.PC = indirect (emu);

	wait_cycles (emu, 5);
}

void adc_absolute (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF);
	cpu->P &= ~(flags);

	uint16_t addr = absolute (emu);
	uint8_t ret = cpu->A + emu->mem[addr];
	CHECK_FLAGS (flags, cpu->A, ret);

	cpu->A = ret;

	wait_cycles (emu, 4);

	cpu->PC += 3;
}

void ror_absolute (struct NESEmu *emu) 
{
	ROR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute (emu)], 6, 0, 3);
}

void bvs_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_VF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void adc_indirect_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF);
	cpu->P &= ~(flags);

	uint16_t addr = indirect_y (emu);
	uint8_t ret = cpu->A + emu->ram[addr];
	CHECK_FLAGS (flags, cpu->A, ret);

	cpu->A = ret;

	uint8_t off = 0;
	/*
	 * TODO: cross
	 */
	wait_cycles (emu, 5 + off);

	cpu->PC += 2;
}

void adc_zeropage_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF);
	cpu->P &= ~(flags);

	uint8_t addr = zeropage_x (emu);
	uint8_t ret = cpu->A + emu->ram[addr];
	CHECK_FLAGS (flags, cpu->A, ret);

	cpu->A = ret;

	wait_cycles (emu, 4);

	cpu->PC += 2;
}

void ror_zeropage_x (struct NESEmu *emu) 
{
	ROR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->ram[zeropage_x (emu)], 6, 0, 2);
}

void sei_implied (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;

		uint8_t *pl = show_debug_info (emu, 1, "SEI");
		*pl = 0;
		cpu->PC++;
		return;
	}
#endif

	emu->cpu.P &= (STATUS_FLAG_IF);
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void adc_absolute_y (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF);
	cpu->P &= ~(flags);

	uint8_t addr = absolute_x (emu);
	uint8_t ret = cpu->A + emu->mem[addr];
	CHECK_FLAGS (flags, cpu->A, ret);

	cpu->A = ret;

	uint8_t off = 0;
	/*
	 * TODO: cross page
	 */
	wait_cycles (emu, 4 + off);

	cpu->PC += 3;
}

void adc_absolute_x (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;
	uint8_t flags = (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF);
	cpu->P &= ~(flags);

	uint8_t addr = absolute_x (emu);
	uint8_t ret = cpu->A + emu->ram[addr];
	CHECK_FLAGS (flags, cpu->A, ret);

	cpu->A = ret;

	uint8_t off = 0;
	/*
	 * TODO: cross page
	 */
	wait_cycles (emu, 4 + off);

	cpu->PC += 3;
}

void ror_absolute_x (struct NESEmu *emu) 
{
	ROR_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, emu->mem[absolute_x (emu)], 7, 0, 3);
}

void sta_indirect_x (struct NESEmu *emu) 
{
	ST_ACTS(cpu->A, indirect_x (emu), 6, 0, 2);
}

void sty_zeropage (struct NESEmu *emu) 
{
	ST_ACTS(cpu->Y, zeropage (emu), 3, 0, 2);
}

void sta_zeropage (struct NESEmu *emu) 
{
	ST_ACTS(cpu->A, zeropage (emu), 3, 0, 2);
}
void stx_zeropage (struct NESEmu *emu) 
{
	ST_ACTS(cpu->X, zeropage (emu), 3, 0, 2);
}

void dey_implied (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->Y, --cpu->Y, =, 2, 0, 1);
}

void txa_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->A = cpu->X;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sty_absolute (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "STY");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 2;
		return;
	}
#endif

	ST_ACTS(cpu->Y, absolute (emu), 4, 0, 3);
}

void sta_absolute (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "STA");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	ST_ACTS(cpu->A, absolute (emu), 4, 0, 3);
}



void stx_absolute (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "STX");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	ST_ACTS(cpu->X, absolute (emu), 4, 0, 3);
}

void bcc_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_CF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void sta_indirect_y (struct NESEmu *emu) 
{
	ST_ACTS(cpu->A, indirect_y (emu), 6, 0, 2);
}

void sty_zeropage_x (struct NESEmu *emu) 
{
	ST_ACTS(cpu->Y, zeropage_x (emu), 4, 0, 2);
}

void sta_zeropage_x (struct NESEmu *emu) 
{
	ST_ACTS(cpu->X, zeropage_x (emu), 4, 0, 2);
}
void stx_zeropage_y (struct NESEmu *emu) 
{
	ST_ACTS(cpu->X, zeropage_y (emu), 4, 0, 2);
}

void tya_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->A = cpu->Y;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_y (struct NESEmu *emu) 
{
	ST_ACTS(cpu->A, absolute_y (emu), 5, 0, 3);
}

void txs_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->S &= 0xff00;
	cpu->S |= cpu->X;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void sta_absolute_x (struct NESEmu *emu) 
{
	ST_ACTS(cpu->A, absolute_x (emu), 5, 0, 3);
}

void ldy_immediate (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->Y, immediate (emu), 2, 0, 2);
}

void lda_indirect_x (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, indirect_x (emu), 6, 0, 2);
}

void ldx_immediate (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 2, "LDX");
		uint8_t number = (offset & 0xff);
		*pl++ = '#';
		*pl++ = '$';
		*pl++ = hex_up_half (number);
		*pl++ = hex_down_half (number);
		*pl = 0;
		cpu->PC += 2;
		return;
	}
#endif

	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->X, immediate (emu), 2, 0, 2);
}

void ldy_zeropage (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->Y, zeropage (emu), 3, 0, 2);
}

void lda_zeropage (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, zeropage (emu), 3, 0, 2);
}

void ldx_zeropage (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->X, zeropage (emu), 3, 0, 2);
}

void tay_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->Y = cpu->A;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void lda_immediate (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		int8_t offset = emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 2, "LDA");
		uint8_t number = (offset & 0xff);
		*pl++ = '#';
		*pl++ = '$';
		*pl++ = hex_up_half (number);
		*pl++ = hex_down_half (number);
		*pl = 0;
		cpu->PC += 2;
		return;
	}
#endif

	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, immediate (emu), 2, 0, 2);
}

void tax_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->X = cpu->A;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void ldy_absolute (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "LDY");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->Y, absolute (emu), 4, 0, 3);
}

void lda_absolute (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "LDA");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, absolute (emu), 4, 0, 3);
}

void ldx_absolute (struct NESEmu *emu)
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint16_t new_offset = *(uint16_t *) &emu->mem[cpu->PC + 1];

		uint8_t *pl = show_debug_info (emu, 3, "LDX");
		uint8_t up = (new_offset >> 8) & 0xff;
		uint8_t down = (new_offset & 0xff);
		*pl++ = '$';
		*pl++ = hex_up_half (up);
		*pl++ = hex_down_half (up);
		*pl++ = hex_up_half (down);
		*pl++ = hex_down_half (down);
		*pl = 0;
		cpu->PC += 3;
		return;
	}
#endif

	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->X, absolute (emu), 4, 0, 3);
}

void bcs_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_CF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void lda_indirect_y (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, indirect_y (emu), 5, 1, 2);
}
void ldy_zeropage_x (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->Y, zeropage_x (emu), 4, 0, 2);
}

void lda_zeropage_x (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, zeropage_x (emu), 4, 0, 2);
}

void ldx_zeropage_y (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->X, zeropage_y (emu), 4, 0, 2);
}

void clv_implied (struct NESEmu *emu) 
{
	emu->cpu.P &= ~(STATUS_FLAG_VF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void lda_absolute_y (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, absolute_y (emu), 4, 1, 3);
}

void tsx_implied (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	cpu->X = cpu->S;

	emu->cpu.PC += 1;

	wait_cycles (emu, 2);
}

void ldy_absolute_x (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->Y, absolute_x (emu), 4, 1, 3);
}

void lda_absolute_x (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, absolute_x (emu), 4, 1, 3);
}

void ldx_absolute_y (struct NESEmu *emu) 
{
	LD_ACTS (STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->X, absolute_y (emu), 4, 1, 3);
}

void cpy_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->Y, cpu->Y - immediate_val (emu), |, 2, 0, 2);
}

void cmp_indirect_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->X, cpu->X - emu->ram[indirect_x (emu)], |, 6, 0, 2);
}

void cpy_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->Y, cpu->Y - emu->ram[zeropage (emu)], |, 3, 0, 2);
}

void cmp_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A - emu->ram[zeropage (emu)], |, 3, 0, 2);
}

void dec_zeropage (struct NESEmu *emu) 
{
	uint16_t pg = zeropage (emu);
#if 0
	printf ("\tvalue on zeropage %04x: ", pg);
#endif
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->ram[zeropage(emu)], --emu->ram[zeropage (emu)], =, 5, 0, 2);
#if 0
	printf ("%02x\n", emu->ram[pg]);
#endif
}

void iny_implied (struct NESEmu *emu) 
{
	emu->cpu.Y++;

	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void cmp_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_CF|STATUS_FLAG_ZF|STATUS_FLAG_NF, cpu->A, cpu->A - immediate_val (emu), |, 2, 0, 2);
}

void dex_implied (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, cpu->X, --cpu->X, =, 2, 0, 1);
}

void cpy_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->Y, cpu->Y - emu->mem[absolute (emu)], |, 4, 0, 3);
}

void cmp_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A - emu->mem[absolute (emu)], |, 4, 0, 3);
}

void dec_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->mem[absolute(emu)], --emu->mem[absolute (emu)], =, 6, 0, 3);
}

void bne_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_ZF) {
		cpu->PC += 2;
	} else {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void cmp_indirect_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A - emu->ram[indirect_y (emu)], |, 5, 1, 2);
}

void cmp_zeropage_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A - emu->mem[zeropage_x (emu)], |, 4, 0, 2);
}

void dec_zeropage_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->ram[zeropage_x(emu)], --emu->ram[zeropage_x (emu)], =, 6, 0, 2);
}

void cld_implied (struct NESEmu *emu) 
{
#if 0
	if (emu->is_debug_list) {
		struct CPUNes *cpu = &emu->cpu;
		uint8_t *pl = show_debug_info (emu, 1, "CLD");
		*pl = 0;
		cpu->PC++;
		return;
	}
#endif

	emu->cpu.P &= ~(STATUS_FLAG_DF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void cmp_absolute_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A - emu->mem[absolute_y (emu)], |, 4, 1, 3);
}

void cmp_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->A, cpu->A - emu->mem[absolute_x (emu)], |, 4, 1, 3);
}

void dec_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->mem[absolute_x(emu)], --emu->mem[absolute_x (emu)], =, 7, 0, 3);
}

void cpx_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->X, cpu->X - immediate_val (emu), |, 2, 0, 2);
}

void sbc_indirect_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->ram[indirect_x (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 6, 0, 2);
}

void cpx_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->X, cpu->X - emu->ram[zeropage (emu)], |, 3, 0, 2);
}

void sbc_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->ram[zeropage (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 3, 0, 2);
}

void inc_zeropage (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->ram[zeropage(emu)], ++emu->ram[zeropage (emu)], =, 5, 0, 2);
}

void inx_implied (struct NESEmu *emu)
{
	emu->cpu.X++;

	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void sbc_immediate (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - immediate_val (emu) - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 2, 0, 2);
}

void nop_implied (struct NESEmu *emu) 
{
	emu->cpu.PC++;

	wait_cycles (emu, 2);
}

void cpx_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF, cpu->X, cpu->X - emu->mem[absolute (emu)], |, 4, 0, 3);
}
void sbc_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->mem[absolute (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 4, 0, 3);
}

void inc_absolute (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->mem[absolute(emu)], ++emu->mem[absolute (emu)], =, 6, 0, 3);
}

void beq_relative (struct NESEmu *emu) 
{
	struct CPUNes *cpu = &emu->cpu;

	int8_t offset = emu->mem[cpu->PC + 1];

	uint16_t new_offset;

	if (offset < 0) {
		uint8_t off = 0xff - offset - 1;
		new_offset = cpu->PC - off;
	} else {
		new_offset = cpu->PC + offset + 2;
	}

	uint32_t ext_cycles;

	if (emu->cpu.P & STATUS_FLAG_ZF) {
		set_ext_cycles (cpu, offset, new_offset, &ext_cycles);

		cpu->PC = new_offset;
	} else {
		cpu->PC += 2;
	}

	wait_cycles (emu, 2 + ext_cycles);
}

void sbc_indirect_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->ram[indirect_y (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 5, 1, 2);
}

void sbc_zeropage_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->ram[zeropage_x (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 4, 0, 2);
}

void inc_zeropage_x (struct NESEmu *emu) {
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->ram[zeropage_x(emu)], ++emu->ram[zeropage_x (emu)], =, 6, 0, 2);
}

void sed_implied (struct NESEmu *emu) 
{
	emu->cpu.P |= (STATUS_FLAG_DF);

	wait_cycles (emu, 2);

	emu->cpu.PC++;
}

void sbc_absolute_y (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->mem[absolute_y (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 4, 1, 3);
}
void sbc_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF|STATUS_FLAG_CF|STATUS_FLAG_VF, cpu->A, cpu->A - emu->mem[absolute_x (emu)] - ((emu->cpu.P & STATUS_FLAG_CF)? 1: 0), =, 4, 1, 3);
}

void inc_absolute_x (struct NESEmu *emu) 
{
	REPETITIVE_ACTS (STATUS_FLAG_NF|STATUS_FLAG_ZF, emu->mem[absolute_x(emu)], ++emu->mem[absolute_x (emu)], =, 7, 0, 3);
}
