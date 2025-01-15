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


void flags_cmp_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_absolute_y (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_indirect_x (struct NESEmu *emu, uint16_t addr) {}
void flags_cmp_indirect_y (struct NESEmu *emu, uint16_t addr) {}
void flags_cpx_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_cpx_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_cpx_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_cpy_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_cpy_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_cpy_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_dec_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_dec_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_dec_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_dec_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_absolute_y (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_indirect_x (struct NESEmu *emu, uint16_t addr) {}
void flags_eor_indirect_y (struct NESEmu *emu, uint16_t addr) {}
void flags_inc_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_inc_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_inc_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_inc_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_absolute_y (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_indirect_x (struct NESEmu *emu, uint16_t addr) {}
void flags_lda_indirect_y (struct NESEmu *emu, uint16_t addr) {}
void flags_ldx_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_ldx_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_ldx_zeropage_y (struct NESEmu *emu, uint16_t addr) {}
void flags_ldx_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_ldx_absolute_y (struct NESEmu *emu, uint16_t addr) {}
void flags_ldy_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_ldy_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_ldy_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ldy_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_ldy_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_lsr_accumulator (struct NESEmu *emu, uint16_t addr) {}
void flags_lsr_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_lsr_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_lsr_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_lsr_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_absolute_y (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_indirect_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ora_indirect_y (struct NESEmu *emu, uint16_t addr) {}
void flags_rol_accumulator (struct NESEmu *emu, uint16_t addr) {}
void flags_rol_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_rol_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_rol_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_rol_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ror_accumulator (struct NESEmu *emu, uint16_t addr) {}
void flags_ror_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_ror_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_ror_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_ror_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_imm (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_absolute (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_zeropage (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_zeropage_x (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_absolute_x (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_absolute_y (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_indirect_x (struct NESEmu *emu, uint16_t addr) {}
void flags_sbc_indirect_y (struct NESEmu *emu, uint16_t addr) {}
void flags_tax_implied (struct NESEmu *emu, uint16_t addr) {}
void flags_tay_implied (struct NESEmu *emu, uint16_t addr) {}
void flags_tsx_implied (struct NESEmu *emu, uint16_t addr) {}
void flags_txa_implied (struct NESEmu *emu, uint16_t addr) {}
void flags_tya_implied (struct NESEmu *emu, uint16_t addr) {}
