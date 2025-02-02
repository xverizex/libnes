#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <handlers.h>
#include <cpunes.h>

static void *alloc_emu ()
{
	struct NESEmu *emu = malloc (sizeof (struct NESEmu));
	memset (emu, 0, sizeof (struct NESEmu));
	emu->ppu = malloc (0x4000);
	emu->chr = malloc (0x2000);
	emu->mem = malloc (0x8000);
	return emu;
}

static void dealloc_emu (struct NESEmu *emu)
{
	if (emu->mem) free (emu->mem);
	if (emu->chr) free (emu->chr);
	if (emu->ppu) free (emu->ppu);
	free (emu);
}

#define BEGIN_TEST struct NESEmu *emu = alloc_emu ()
#define END_TEST dealloc_emu (emu)

#define BEGIN_SUBTEST { \
	if (emu->ppu) \
		memset (emu->ppu, 0, 0x4000); \
	if (emu->chr) \
		memset (emu->chr, 0, 0x2000); \
	if (emu->mem) \
		memset (emu->mem, 0, 0x8000); \
	memset (&emu->cpu, 0, sizeof (struct CPUNes)); \
	printf ("%s", str); \
}

static void test_adc_zeropage_with_carry_flag (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x65;
	emu->mem[1] = 0x01;
	emu->cpu.P |= STATUS_FLAG_CF;
	emu->ram[1] = 0x14;
	emu->cpu.A = 0x10;
	emu->cpu.PC = 0x8000;

	adc_zeropage (emu);

	if (emu->cpu.A == 0x25) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x25, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_adc_zeropage_without_carry_flag (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x65;
	emu->mem[1] = 0x01;
	emu->ram[1] = 0x14;
	emu->cpu.A = 0x10;
	emu->cpu.PC = 0x8000;

	adc_zeropage (emu);

	if (emu->cpu.A == 0x24) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x25, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_adc ()
{
	BEGIN_TEST;

	test_adc_zeropage_with_carry_flag (emu, "ADC zeropage with carry FLAG: ");
	test_adc_zeropage_without_carry_flag (emu, "ADC zeropage without carry FLAG: ");

	END_TEST;
}

int main (int argc, char **argv)
{
	test_adc ();
}
