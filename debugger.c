#include <cpunes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

static void memory_map (struct NESEmu *emu, uint16_t from, uint16_t to)
{
	uint16_t addr = from;
	printf ("xxxx: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");
	printf ("-----------------------------------------------------\n");
	printf ("%04x:", addr);
	uint8_t *mem = NULL;
	uint16_t off = 0;
	if (from < RAM_MAX) {
		mem = &emu->ram[from];
	} else {
		mem = &emu->mem[from - 0x8000];
	}
	int i = 0;
	int end = to - from;
	for (; i <= end; i++) {
		if ((i > 0) && ((i % 16) == 0)) {
			printf ("\n");
			addr += 16;
			printf ("%04x: ", addr);
		}
		printf ("%02x ", (mem[i]));
	}
	printf ("\n");
}

static void debug_map (struct NESEmu *emu, char *buf)
{
	char *s0 = buf;
	char *s1 = NULL;
	s0 += 3;
	while (*s0 == ' ') s0++;
	s1 = strchr (s0, ' ');
	if (!s1) {
		printf ("error\n");
		return;
	}
	char *e = s1;
	*e = 0;
	s1++;
	e = strchr (s1, '\n');
	if (!e) {
		printf ("error\n");
		return;
	}
	*e = 0;
	uint16_t from = strtol (s0, NULL, 16);
	uint16_t to = strtol (s1, NULL, 16);

	memory_map (emu, from, to);
}

static void print_help ()
{
	printf ("brk 0xXXXX, A == 0xXX, X == 0xXX - breakpoint\n");
	printf ("cnt - continue\n");
	printf ("map 0xXXXX 0xXXXX - show memory map\n");
	printf ("dr - show registers\n");
	printf ("list - list breakpoints\n");
	printf ("enable [indx] - enable breakpoint\n");
	printf ("disable [indx] - disable breakpoint\n");
	printf ("step - step trace\n");
	printf ("pc - current pc\n");
	printf ("stack - show stack\n");
}

static void debug_breakpoint (struct NESEmu *emu, uint8_t *b)
{
	uint32_t cnt = emu->debug_brk_cnt;

	char *s = strchr (b, ' ');
	s++;
	char *e = strchr (s, '\n');
	*e = 0;
	
	e = s;
	while (*s != 0x0 && *s != '\n' && *s != ',' && *s != ' ') s++;
	uint8_t temp = *s;
	*s = 0;
	uint16_t addr = strtol (e, NULL, 16);
	*s = temp;
	emu->brk[cnt].addr = addr;
	while (1) {
		e = strchr (s, ',');
		if (!e) break;
		s = e;
		s++;
		while (*s == ' ') s++;
		uint8_t indx_arr = 0;
		uint8_t *r = NULL;
		switch (*s) {
			case 'A':
				r = &emu->cpu.A;
				indx_arr = 3;
				break;
			case 'X':
				r = &emu->cpu.X;
				indx_arr = 2;
				break;
			case 'Y':
				r = &emu->cpu.Y;
				indx_arr = 1;
				break;
			case 'P':
				r = &emu->cpu.P;
				indx_arr = 0;
				break;
		}
		s++;
		while (*s == ' ') s++;
		uint8_t cond = 0;
		if (!strncmp (s, "==", 2)) {
			cond = 1;
		} else if (!strncmp (s, "!=", 2)) {
			cond = 2;
		} else {
			printf ("s is %s\n", s);
			printf ("error\n");
			return;
		}

		emu->brk[cnt].condition &= ~(0xff << (indx_arr * 8));
		emu->brk[cnt].condition |=  (cond << (indx_arr * 8));

		s += 2;
		while (*s == ' ') s++;
		uint8_t val = strtol (s, NULL, 16);
		emu->brk[cnt].regs[indx_arr] = val;
		while (*s != 0x0 && *s != '\n' && *s != ',' && *s != ' ') s++;

	}

	emu->brk[cnt].is_enabled = 1;
	emu->debug_brk_cnt = ++cnt;
}

static void debug_registers (struct NESEmu *emu)
{
	printf ("PC: %04x\n", emu->cpu.PC);
	printf (" A:   %02x\n", emu->cpu.A);
	printf (" X:   %02x\n", emu->cpu.X);
	printf (" Y:   %02x\n", emu->cpu.Y);
	printf (" P:   %02x\n", emu->cpu.P);
}

static uint32_t check_eq (struct NESEmu *emu, struct breakpoint *brk, uint32_t indx_arr, uint8_t is_print)
{
	switch (indx_arr) {
		case 0: if (is_print) printf ("P == %02x, ", brk->regs[0]); if (emu->cpu.P == brk->regs[0]) return 1;
		case 1: if (is_print) printf ("Y == %02x, ", brk->regs[1]); if (emu->cpu.Y == brk->regs[1]) return 1;
		case 2: if (is_print) printf ("X == %02x, ", brk->regs[2]); if (emu->cpu.X == brk->regs[2]) return 1;
		case 3: if (is_print) printf ("A == %02x, ", brk->regs[3]); if (emu->cpu.A == brk->regs[3]) return 1;
		default: return 0;
	}
}

static uint32_t check_neq (struct NESEmu *emu, struct breakpoint *brk, uint32_t indx_arr, uint8_t is_print)
{
	switch (indx_arr) {
		case 0: if (is_print) printf ("P != %02x, ", brk->regs[0]); if (emu->cpu.P != brk->regs[0]) return 1;
		case 1: if (is_print) printf ("Y != %02x, ", brk->regs[1]); if (emu->cpu.Y != brk->regs[1]) return 1;
		case 2: if (is_print) printf ("X != %02x, ", brk->regs[2]); if (emu->cpu.X != brk->regs[2]) return 1;
		case 3: if (is_print) printf ("A != %02x, ", brk->regs[3]); if (emu->cpu.A != brk->regs[3]) return 1;
		default: return 0;
	}
}

static uint32_t print_check_condition (struct NESEmu *emu, uint8_t cond,
		struct breakpoint *brk,
		uint32_t indx_arr)
{
	if (cond == 1) {
		return check_eq (emu, brk, indx_arr, 1);
	} else if (cond == 2) {
		return check_neq (emu, brk, indx_arr, 1);
	}
}

static uint32_t check_condition (struct NESEmu *emu, uint8_t cond,
		struct breakpoint *brk,
		uint32_t indx_arr)
{
	if (cond == 1) {
		return check_eq (emu, brk, indx_arr, 0);
	} else if (cond == 2) {
		return check_neq (emu, brk, indx_arr, 0);
	}
}

static uint32_t debug_is_condition_true (struct NESEmu *emu, int i)
{
	uint32_t condition = emu->brk[i].condition;
	uint8_t cond = condition & 0xff;
	struct breakpoint *brk = &emu->brk[i];

	uint8_t c0, c1, c2, c3;
	c0 = c1 = c2 = c3 = 0;
	if ((cond > 0) && (c0 = 0xff) && check_condition (emu, cond, brk, 0)) {
		c0 = 1;
	}
	condition >>= 8;
	cond = condition & 0xff;
	if ((cond > 0) && (c1 = 0xff) && check_condition (emu, cond, brk, 1)) {
		c1 = 1;
	}
	condition >>= 8;
	cond = condition & 0xff;
	if ((cond > 0) && (c2 = 0xff) && check_condition (emu, cond, brk, 2)) {
		c2 = 1;
	}
	condition >>= 8;
	cond = condition & 0xff;
	if ((cond > 0) && (c3 = 0xff) && check_condition (emu, cond, brk, 3)) {
		c3 = 1;
	}
	if (c0 == 0xff || c1 == 0xff || c2 == 0xff || c3 == 0xff) {
		return 0;
	}

	printf ("#%d interrupted on %04x: ", i, emu->cpu.PC);
	if (c0 == 1) print_check_condition (emu, condition & 0xff, brk, 0);
	if (c1 == 1) print_check_condition (emu, condition & 0xff00, brk, 0);
	if (c2 == 1) print_check_condition (emu, condition & 0xff0000, brk, 0);
	if (c3 == 1) print_check_condition (emu, condition & 0xff000000, brk, 0);
	printf ("\n");

	return 1;
}

static void print_condition (struct NESEmu *emu, struct breakpoint *brk)
{
	uint32_t condition = brk->condition;
	uint8_t cond = condition & 0xff;

	uint8_t c0, c1, c2, c3;
	c0 = c1 = c2 = c3 = 0;
	if ((cond > 0) && (c0 = 0xff) && print_check_condition (emu, cond, brk, 0)) {
		c0 = 1;
	}
	condition >>= 8;
	cond = condition & 0xff;
	if ((cond > 0) && (c1 = 0xff) && print_check_condition (emu, cond, brk, 1)) {
		c1 = 1;
	}
	condition >>= 8;
	cond = condition & 0xff;
	if ((cond > 0) && (c2 = 0xff) && print_check_condition (emu, cond, brk, 2)) {
		c2 = 1;
	}
	condition >>= 8;
	cond = condition & 0xff;
	if ((cond > 0) && (c3 = 0xff) && print_check_condition (emu, cond, brk, 3)) {
		c3 = 1;
	}
}

static void debug_list_brk (struct NESEmu *emu)
{
	printf ("# breakpoint list\n");
	uint32_t cnt = emu->debug_brk_cnt;
	for (int i = 0; i < cnt; i++) {
		printf ("#%d: %04x ", i, emu->brk[i].addr);
		struct breakpoint *brk = &emu->brk[i];
		print_condition (emu, brk);
		printf (" [%s]\n", brk->is_enabled? "enabled": "disabled");
	}
}

static void turn_break_point (struct NESEmu *emu, char *buf, uint32_t turn)
{
	char *s = strchr (buf, ' ');
	while (*s == ' ') s++;
	char *e = s;
	while (*e >= '0' && *e <= '9') e++;
	*e = 0;
	int num = atoi (s);
	emu->brk[num].is_enabled = turn;
}

char *debugger_print_regs (struct NESEmu *emu)
{
	uint16_t stack = 0x100 + emu->cpu.S;

	snprintf (emu->buf_regs, 256, "A: %02X X: %02X Y: %02X P: %02X S: %04x",
			emu->cpu.A,
			emu->cpu.X,
			emu->cpu.Y,
			emu->cpu.P,
			stack);
	return emu->buf_regs;
}

static void trace_stack (struct NESEmu *emu)
{
	uint8_t top = 0xff;

	while (top >= emu->cpu.S) {
		uint16_t off = 0x100 + top;
		printf ("%04x: #$%02x\n", off, emu->ram[off]);
		top--;
		if (top == emu->cpu.S) break;
	}
}

void debug (struct NESEmu *emu)
{
	if (emu->is_started == 1) {
		uint32_t cnt = emu->debug_brk_cnt;
		for (int i = 0; i < cnt; i++) {
			if (emu->brk[i].is_enabled && (emu->cpu.PC == emu->brk[i].addr)) {
				if (debug_is_condition_true (emu, i)) {
					emu->is_debug = 1;
					break;
				}
			}
		}
		if (emu->is_debug == 0)
			return;
	}
	if (emu->is_started == 0) {
		emu->is_debug = 1;
		printf ("# Debugger mode\n");
	} else if (emu->is_debug) {
		printf ("# Debugger mode\n");
	}

	emu->only_show = 0;

	emu->is_started = 1;
	char buf[255];
	while (emu->is_debug) {
		fgets (buf, 255, stdin); 
		if (!strncmp (buf, "help", 4)) {
			emu->latest_step = LATEST_NO;
			print_help ();
		}
		if (!strncmp (buf, "cnt", 3)) {
			emu->latest_step = LATEST_CNT;
			emu->is_debug = 0;
			return;
		}
		if (!strncmp (buf, "brk", 3)) {
			emu->latest_step = LATEST_NO;
			debug_breakpoint (emu, buf);
		}
		if (!strncmp (buf, "dr", 2)) {
			emu->latest_step = LATEST_NO;
			debug_registers (emu);
		}
		if (!strncmp (buf, "map", 3)) {
			emu->latest_step = LATEST_NO;
			debug_map (emu, buf);
		}
		if (!strncmp (buf, "list", 4)) {
			emu->latest_step = LATEST_NO;
			debug_list_brk (emu);
		}
		if (!strncmp (buf, "exit", 4)) {
			emu->latest_step = LATEST_NO;
			exit (EXIT_SUCCESS);
		}
		if (!strncmp (buf, "enable", 6)) {
			emu->latest_step = LATEST_NO;
			turn_break_point (emu, buf, 1);
		}
		if (!strncmp (buf, "disable", 7)) {
			emu->latest_step = LATEST_NO;
			turn_break_point (emu, buf, 0);
		}
		if (!strncmp (buf, "step", 4)) {
			emu->latest_step = LATEST_STEP;
			emu->debug_step = 1;
			return;
		}
		if (!strncmp (buf, "pc", 2)) {
			emu->latest_step = LATEST_NO;
			printf ("PC: %04x\n", emu->cpu.PC);
		}
		if (!strncmp (buf, "stack", 5)) {
			emu->latest_step = LATEST_NO;
			trace_stack (emu);
		}
		if (!strncmp (buf, "cur", 3)) {
			emu->latest_step = LATEST_NO;
			emu->debug_step = 1;
			emu->only_show = 1;
			return;
		}

		uint32_t len = strlen (buf);
		if (len == 1 && buf[0] == 0xa) {
			switch (emu->latest_step) {
				case LATEST_CNT: emu->is_debug = 0; return;
				case LATEST_STEP: emu->debug_step = 1; return;
				default: emu->debug_step = 0; break;
			}
		}

		memset (buf, 0, 255);
	}
}
