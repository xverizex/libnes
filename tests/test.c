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

static void test_adc_immediate_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if (emu->cpu.A == 0x80 && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x80 nf and vf flags, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_adc_immediate_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x40;
	emu->cpu.A = 0x8f;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0xcf) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0xcf nf flag, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_adc_immediate_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0xf4;
	emu->cpu.A = 0x10;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x04) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x04 cf flag, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_adc_immediate_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x80;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x04 cf flag, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_adc_immediate_0x04 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x80;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0xff) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0xff nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc_immediate_0x05 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0xff) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0xff nf cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc_immediate_0x06 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 zf cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc_immediate_0x07 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x41;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x81) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x81 nf vf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc_immediate_0x08 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x81;
	emu->cpu.A = 0x81;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x02) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x02 cf vf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc_immediate_0x09 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x01;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 cf zf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc_immediate_0x0a (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	adc_immediate (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 cf zf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_adc ()
{
	BEGIN_TEST;

	test_adc_immediate_0x00 (emu, "ADC immediate 0x00: ");
	test_adc_immediate_0x01 (emu, "ADC immediate 0x01: ");
	test_adc_immediate_0x02 (emu, "ADC immediate 0x02: ");
	test_adc_immediate_0x03 (emu, "ADC immediate 0x03: ");
	test_adc_immediate_0x04 (emu, "ADC immediate 0x04: ");
	test_adc_immediate_0x05 (emu, "ADC immediate 0x05: ");
	test_adc_immediate_0x06 (emu, "ADC immediate 0x06: ");
	test_adc_immediate_0x07 (emu, "ADC immediate 0x07: ");
	test_adc_immediate_0x08 (emu, "ADC immediate 0x08: ");
	test_adc_immediate_0x09 (emu, "ADC immediate 0x09: ");
	test_adc_immediate_0x0a (emu, "ADC immediate 0x0a: ");

	END_TEST;
}

static void test_bit_zeropage_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x02;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x81;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be negative flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x40;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be overflow flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x00;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x04 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x80;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero flag and negative flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x05 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x80;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero flag and negative flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x06 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x01;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x07 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x80;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero negative flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x08 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0xc0;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero negative overflow flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x09 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x00;
	emu->cpu.A = 0xc0;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zero flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x0a (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x40;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be overflow flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x0b (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x40;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be overflow flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x0c (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x00;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be overflow flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x0d (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x0f;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be none flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x0e (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0xcf;
	emu->cpu.A = 0xcf;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be none flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x0f (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x00;
	emu->cpu.A = 0xc0;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be none flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x10 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0xff;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be none flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x11 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x80;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zf, nf, vf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x12 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x40;
	emu->cpu.A = 0x42;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zf, nf, vf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit_zeropage_0x13 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x24;
	emu->mem[1] = 0x02;
	emu->ram[2] = 0x00;
	emu->cpu.A = 0xc0;
	emu->cpu.PC = 0x8000;

	bit_zeropage (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zf, nf, vf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_bit ()
{
	BEGIN_TEST;

	test_bit_zeropage_0x00 (emu, "BIT zeropage 0x00:");
	test_bit_zeropage_0x01 (emu, "BIT zeropage 0x01:");
	test_bit_zeropage_0x02 (emu, "BIT zeropage 0x02:");
	test_bit_zeropage_0x03 (emu, "BIT zeropage 0x03:");
	test_bit_zeropage_0x04 (emu, "BIT zeropage 0x04:");
	test_bit_zeropage_0x05 (emu, "BIT zeropage 0x05:");
	test_bit_zeropage_0x06 (emu, "BIT zeropage 0x06:");
	test_bit_zeropage_0x07 (emu, "BIT zeropage 0x07:");
	test_bit_zeropage_0x08 (emu, "BIT zeropage 0x08:");
	test_bit_zeropage_0x09 (emu, "BIT zeropage 0x09:");
	test_bit_zeropage_0x0a (emu, "BIT zeropage 0x0a:");
	test_bit_zeropage_0x0b (emu, "BIT zeropage 0x0b:");
	test_bit_zeropage_0x0c (emu, "BIT zeropage 0x0c:");
	test_bit_zeropage_0x0d (emu, "BIT zeropage 0x0d:");
	test_bit_zeropage_0x0e (emu, "BIT zeropage 0x0e:");
	test_bit_zeropage_0x0f (emu, "BIT zeropage 0x0f:");
	test_bit_zeropage_0x10 (emu, "BIT zeropage 0x10:");
	test_bit_zeropage_0x11 (emu, "BIT zeropage 0x11:");
	test_bit_zeropage_0x12 (emu, "BIT zeropage 0x12:");
	test_bit_zeropage_0x13 (emu, "BIT zeropage 0x13:");

	END_TEST;
}

static void test_cmp_immediate_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be none flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zf, cf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0x0f;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be zf, cf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x03;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x04 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x05 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x02;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x06 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x92;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x07 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x55;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x08 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0x10;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x09 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x7f;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x0a (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xee;
	emu->cpu.A = 0xee;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x0b (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x0c (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x0d (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0x81;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x0e (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x3f;
	emu->cpu.A = 0x20;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x0f (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x1f;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x10 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x11 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x12 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x20;
	emu->cpu.A = 0x02;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x13 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x80;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x14 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x10;
	emu->cpu.A = 0x20;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x15 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x81;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x16 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x80;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x17 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x7f;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be no flags, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp_immediate_0x18 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xc9;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;

	cmp_immediate (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf and cf, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_cmp ()
{
	BEGIN_TEST;

	test_cmp_immediate_0x00 (emu, "CMP immediate 0x00:");
	test_cmp_immediate_0x01 (emu, "CMP immediate 0x01:");
	test_cmp_immediate_0x02 (emu, "CMP immediate 0x02:");
	test_cmp_immediate_0x03 (emu, "CMP immediate 0x03:");
	test_cmp_immediate_0x04 (emu, "CMP immediate 0x04:");
	test_cmp_immediate_0x05 (emu, "CMP immediate 0x05:");
	test_cmp_immediate_0x06 (emu, "CMP immediate 0x06:");
	test_cmp_immediate_0x07 (emu, "CMP immediate 0x07:");
	test_cmp_immediate_0x08 (emu, "CMP immediate 0x08:");
	test_cmp_immediate_0x09 (emu, "CMP immediate 0x09:");
	test_cmp_immediate_0x0a (emu, "CMP immediate 0x0a:");
	test_cmp_immediate_0x0b (emu, "CMP immediate 0x0b:");
	test_cmp_immediate_0x0c (emu, "CMP immediate 0x0c:");
	test_cmp_immediate_0x0d (emu, "CMP immediate 0x0d:");
	test_cmp_immediate_0x0e (emu, "CMP immediate 0x0e:");
	test_cmp_immediate_0x0f (emu, "CMP immediate 0x0f:");
	test_cmp_immediate_0x10 (emu, "CMP immediate 0x10:");
	test_cmp_immediate_0x11 (emu, "CMP immediate 0x11:");
	test_cmp_immediate_0x12 (emu, "CMP immediate 0x12:");
	test_cmp_immediate_0x13 (emu, "CMP immediate 0x13:");
	test_cmp_immediate_0x14 (emu, "CMP immediate 0x14:");
	test_cmp_immediate_0x15 (emu, "CMP immediate 0x15:");
	test_cmp_immediate_0x16 (emu, "CMP immediate 0x16:");
	test_cmp_immediate_0x17 (emu, "CMP immediate 0x17:");
	test_cmp_immediate_0x18 (emu, "CMP immediate 0x18:");

	END_TEST;
}

static void test_lsr_immediate_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x4a;
	emu->cpu.A = 0x41;
	emu->cpu.PC = 0x8000;

	lsr_accumulator (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_lsr_immediate_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x4a;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;

	lsr_accumulator (emu);

	if (!(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_lsr_immediate_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x4a;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;

	lsr_accumulator (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_lsr_immediate_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x4a;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;

	lsr_accumulator (emu);

	if ((emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be nf flag, but %02x\n", emu->cpu.P);
		exit (0);
	}
}

static void test_lsr ()
{
	BEGIN_TEST;

	test_lsr_immediate_0x00 (emu, "LSR immediate 0x00:");
	test_lsr_immediate_0x01 (emu, "LSR immediate 0x01:");
	test_lsr_immediate_0x02 (emu, "LSR immediate 0x02:");
	test_lsr_immediate_0x03 (emu, "LSR immediate 0x03:");

	END_TEST;
}

static void test_sbc_immediate_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0xe9;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if (emu->cpu.A == 0xff && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0xff nf and cf flags, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_sbc_immediate_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x00;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if (emu->cpu.A == 0x7f && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x80 cf flags, but %02x; P: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_sbc_immediate_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x40;
	emu->cpu.A = 0x8f;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if ((emu->cpu.A == 0x4f) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x4f nf flag, but %02x; P: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_sbc_immediate_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0xf4;
	emu->cpu.A = 0x10;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if ((emu->cpu.A == 0x1c) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x1c, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_sbc_immediate_0x04 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x80;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if ((emu->cpu.A == 0xff) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0xff nf and vf flags, but %02x\n", emu->cpu.A);
		exit (0);
	}
}

static void test_sbc_immediate_0x05 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0x80;
	emu->cpu.A = 0x7f;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if ((emu->cpu.A == 0xfe) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_VF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0xfe nf and vf flags, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_sbc_immediate_0x06 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x69;
	emu->mem[1] = 0xff;
	emu->cpu.A = 0xff;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	sbc_immediate (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_VF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 zf cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_sbc ()
{
	BEGIN_TEST;

	test_sbc_immediate_0x00 (emu, "SBC immediate 0x00:");
	test_sbc_immediate_0x01 (emu, "SBC immediate 0x01:");
	test_sbc_immediate_0x02 (emu, "SBC immediate 0x02:");
	test_sbc_immediate_0x03 (emu, "SBC immediate 0x03:");
	test_sbc_immediate_0x04 (emu, "SBC immediate 0x04:");
	test_sbc_immediate_0x05 (emu, "SBC immediate 0x05:");
	test_sbc_immediate_0x06 (emu, "SBC immediate 0x06:");

	END_TEST;
}

static void test_rol_acc_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x2a;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	rol_accumulator (emu);

	if ((emu->cpu.A == 0x01) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x01 cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_rol_acc_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x2a;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	rol_accumulator (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_rol_acc_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x2a;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	rol_accumulator (emu);

	if ((emu->cpu.A == 0x80) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x80 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_rol_acc_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x2a;
	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	rol_accumulator (emu);

	if ((emu->cpu.A == 0x81) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x81 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_rol ()
{
	BEGIN_TEST;

	test_rol_acc_0x00 (emu, "ROL accumulator 0x00:");
	test_rol_acc_0x01 (emu, "ROL accumulator 0x01:");
	test_rol_acc_0x02 (emu, "ROL accumulator 0x02:");
	test_rol_acc_0x03 (emu, "ROL accumulator 0x03:");

	END_TEST;
}

static void test_ror_acc_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x6a;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	ror_accumulator (emu);

	if ((emu->cpu.A == 0x80) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x01 cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_ror_acc_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x6a;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	ror_accumulator (emu);

	if ((emu->cpu.A == 0x00) && (emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 cf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_ror_acc_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x6a;
	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;
	//emu->cpu.P |= STATUS_FLAG_CF;

	ror_accumulator (emu);

	if ((emu->cpu.A == 0x40) && !(emu->cpu.P & STATUS_FLAG_ZF) && !(emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x80 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_ror_acc_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x6a;
	emu->cpu.A = 0x00;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	ror_accumulator (emu);

	if ((emu->cpu.A == 0x80) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x80 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_ror_acc_0x04 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->mem[0] = 0x6a;
	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;
	emu->cpu.P |= STATUS_FLAG_CF;

	ror_accumulator (emu);

	if ((emu->cpu.A == 0x80) && !(emu->cpu.P & STATUS_FLAG_ZF) && (emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & STATUS_FLAG_CF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x80 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_ror ()
{
	BEGIN_TEST;

	test_ror_acc_0x00 (emu, "ROR accumulator 0x00:");
	test_ror_acc_0x01 (emu, "ROR accumulator 0x01:");
	test_ror_acc_0x02 (emu, "ROR accumulator 0x02:");
	test_ror_acc_0x03 (emu, "ROR accumulator 0x03:");
	test_ror_acc_0x04 (emu, "ROR accumulator 0x04:");

	END_TEST;
}

static void test_asl_acc_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->cpu.A = 0x01;
	emu->cpu.PC = 0x8000;

	asl_accumulator (emu);

	if ((emu->cpu.A == 0x02) && !(emu->cpu.P & (STATUS_FLAG_ZF|STATUS_FLAG_NF|STATUS_FLAG_CF))) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x02 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_asl_acc_0x01 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->cpu.A = 0x80;
	emu->cpu.PC = 0x8000;

	asl_accumulator (emu);

	if ((emu->cpu.A == 0x00) && !(emu->cpu.P & STATUS_FLAG_NF) && (emu->cpu.P & (STATUS_FLAG_CF|STATUS_FLAG_ZF))) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_asl_acc_0x02 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->cpu.A = 0x40;
	emu->cpu.PC = 0x8000;

	asl_accumulator (emu);

	if ((emu->cpu.A == 0x80) && (emu->cpu.P & STATUS_FLAG_NF) && !(emu->cpu.P & (STATUS_FLAG_CF|STATUS_FLAG_ZF))) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_asl_acc_0x03 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->cpu.A = 0xc0;
	emu->cpu.PC = 0x8000;

	asl_accumulator (emu);

	if ((emu->cpu.A == 0x80) && (emu->cpu.P & (STATUS_FLAG_NF|STATUS_FLAG_CF)) && !(emu->cpu.P & STATUS_FLAG_ZF)) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}
static void test_asl ()
{
	BEGIN_TEST;

	test_asl_acc_0x00 (emu, "ASL accumulator 0x00:");
	test_asl_acc_0x01 (emu, "ASL accumulator 0x01:");
	test_asl_acc_0x02 (emu, "ASL accumulator 0x02:");
	test_asl_acc_0x03 (emu, "ASL accumulator 0x03:");

	END_TEST;
}

static void test_eor_acc_0x00 (struct NESEmu *emu, const char *str)
{
	BEGIN_SUBTEST;

	emu->cpu.A = 0xc0;
	emu->cpu.PC = 0x8000;
	emu->mem[1] = 0xa0;

	eor_immediate (emu);

	if ((emu->cpu.A == 0x60) && !(emu->cpu.P & (STATUS_FLAG_NF|STATUS_FLAG_CF|STATUS_FLAG_ZF))) {
		printf ("OK\n");
	} else {
		printf ("FAIL; should be 0x00 nf flag, but %02x and flag: %02x\n", emu->cpu.A, emu->cpu.P);
		exit (0);
	}
}

static void test_eor ()
{
	BEGIN_TEST;

	test_eor_acc_0x00 (emu, "EOR accumulator 0x00:");

	END_TEST;
}

int main (int argc, char **argv)
{
	test_adc ();
	test_bit ();
	test_cmp ();
	test_lsr ();
	test_sbc ();
	test_rol ();
	test_ror ();
	test_asl ();
	test_eor ();
}
