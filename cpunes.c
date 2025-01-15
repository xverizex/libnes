#include "cpunes.h"
#include <flags.h>
#include <exec.h>
#include <op_name.h>
#include <stddef.h>

void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions)
{

    for (uint32_t count = 0; count < count_instructions; count++) {

        switch (emu->buf[emu->cpu.PC]) {
        case ASL_ZEROPAGE:
            calc_addr (emu, zeropage, flags_asl_zeropage, asl, 2, 5, 0);
            break;
        case ASL_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_asl_accumulator, asl, 1, 2, 0);
            break;
        case ASL_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_asl_zeropage_x, asl, 2, 6, 0);
            break;
        case ASL_ABSOLUTE:
            calc_addr (emu, absolute, flags_asl_absolute, asl, 3, 6, 0);
            break;
        case ASL_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_asl_absolute_x, asl, 3, 7, 0);
            break;
        case ADC_IMMEDIATE:
            calc_addr (emu, immediate, flags_adc_imm, adc, 2, 2, 0);
            break;
        case ADC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_adc_zeropage, adc, 2, 3, 0);
            break;
        case ADC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_adc_zeropage_x, adc, 2, 4, 0);
            break;
        case ADC_ABSOLUTE:
            calc_addr (emu, absolute, flags_adc_absolute, adc, 3, 4, 0);
            break;
        case ADC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_adc_absolute_x, adc, 3, 4, 1);
            break;
        case ADC_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_adc_absolute_y, adc, 3, 4, 1);
            break;
        case ADC_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_adc_indirect_x, adc, 2, 6, 0);
            break;
        case ADC_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_adc_indirect_y, adc, 2, 5, 1);
            break;
        case AND_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _and, 2, 2, 0);
            break;
        case AND_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _and, 2, 3, 0);
            break;
        case AND_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _and, 2, 4, 0);
            break;
        case AND_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _and, 3, 4, 0);
            break;
        case AND_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _and, 3, 4, 1);
            break;
        case AND_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _and, 3, 4, 1);
            break;
        case AND_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _and, 2, 6, 0);
            break;
        case AND_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _and, 2, 5, 1);
            break;
        case BCC_RELATIVE:
            calc_addr (emu, NULL, NULL, bcc, 2, 2, 2); /* +1 if branch succeeds, +2 if to a new page */
            break;
        case BCS_RELATIVE:
            calc_addr (emu, NULL, NULL, bcs, 2, 2, 2); /* +1 if branch succeeds, +2 if to a new page */
            break;
        case BIT_ZEROPAGE:
            calc_addr (emu, zeropage, flags_bit_zeropage, bit, 2, 3, 0);
            break;
        case BIT_ABSOLUTE:
            calc_addr (emu, absolute, flags_bit_absolute, bit, 3, 4, 0);
            break;
        case BMI_RELATIVE:
            calc_addr (emu, NULL, NULL, bmi, 2, 2, 2);
            break;
        case BNE_RELATIVE:
            calc_addr (emu, NULL, NULL, bne, 2, 2, 2);
            break;
        case BRK_IMPLIED:
            calc_addr (emu, NULL, NULL, brk, 1, 7, 0);
            break;
        case BVC_RELATIVE:
            calc_addr (emu, NULL, NULL, bvc, 2, 2, 2);
            break;
        case BVS_RELATIVE:
            calc_addr (emu, NULL, NULL, bvs, 2, 2, 2);
            break;
        case CLC_IMPLIED:
            calc_addr (emu, NULL, NULL, clc, 1, 2, 0);
            break;
        case CLD_IMPLIED:
            calc_addr (emu, NULL, NULL, cld, 1, 2, 0);
            break;
        case CLI_IMPLIED:
            calc_addr (emu, NULL, NULL, cli, 1, 2, 0);
            break;
        case CLV_IMPLIED:
            calc_addr (emu, NULL, NULL, clv, 1, 2, 0);
            break;
        case CMP_IMMEDIATE:
            calc_addr (emu, immediate, flags_cmp_imm, cmp, 2, 2, 0);
            break;
        case CMP_ZEROPAGE:
            calc_addr (emu, zeropage, flags_cmp_zeropage, cmp, 2, 3, 0);
            break;
        case CMP_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_cmp_zeropage_x, cmp, 2, 4, 0);
            break;
        case CMP_ABSOLUTE:
            calc_addr (emu, absolute, flags_cmp_absolute, cmp, 3, 4, 0);
            break;
        case CMP_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_cmp_absolute_x, cmp, 3, 4, 1);
            break;
        case CMP_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_cmp_absolute_y, cmp, 3, 4, 1);
            break;
        case CMP_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_cmp_indirect_x, cmp, 2, 6, 0);
            break;
        case CMP_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_cmp_indirect_y, cmp, 2, 5, 1);
            break;
	case CPX_IMMEDIATE:
            calc_addr (emu, immediate, flags_cpx_imm, cpx, 2, 2, 0);
            break;
        case CPX_ZEROPAGE:
            calc_addr (emu, zeropage, flags_cpx_zeropage, cpx, 2, 3, 0);
            break;
        case CPX_ABSOLUTE:
            calc_addr (emu, absolute, flags_cpx_absolute, cpx, 3, 4, 0);
            break;
	case CPY_IMMEDIATE:
            calc_addr (emu, immediate, flags_cpy_imm, cpy, 2, 2, 0);
            break;
        case CPY_ZEROPAGE:
            calc_addr (emu, zeropage, flags_cpy_zeropage, cpy, 2, 3, 0);
            break;
        case CPY_ABSOLUTE:
            calc_addr (emu, absolute, flags_cpy_absolute, cpy, 3, 4, 0);
            break;
        case DEC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_dec_zeropage, dec, 2, 5, 0);
            break;
        case DEC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_dec_zeropage_x, dec, 2, 6, 0);
            break;
        case DEC_ABSOLUTE:
            calc_addr (emu, absolute, flags_dec_absolute, dec, 3, 6, 0);
            break;
        case DEC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_dec_absolute_x, dec, 3, 7, 0);
            break;
        case DEX_IMPLIED:
            calc_addr (emu, NULL, NULL, dex, 1, 2, 0);
            break;
        case DEY_IMPLIED:
            calc_addr (emu, NULL, NULL, dey, 1, 2, 0);
            break;
        case EOR_IMMEDIATE:
            calc_addr (emu, immediate, flags_eor_imm, eor, 2, 2, 0);
            break;
        case EOR_ZEROPAGE:
            calc_addr (emu, zeropage, flags_eor_zeropage, eor, 2, 3, 0);
            break;
        case EOR_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_eor_zeropage_x, eor, 2, 4, 0);
            break;
        case EOR_ABSOLUTE:
            calc_addr (emu, absolute, flags_eor_absolute, eor, 3, 4, 0);
            break;
        case EOR_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_eor_absolute_x, eor, 3, 4, 1);
            break;
        case EOR_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_eor_absolute_y, eor, 3, 4, 1);
            break;
        case EOR_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_eor_indirect_x, eor, 2, 6, 0);
            break;
        case EOR_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_eor_indirect_y, eor, 2, 5, 1);
            break;
        case INC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_inc_zeropage, inc, 2, 5, 0);
            break;
        case INC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_inc_zeropage_x, inc, 2, 6, 0);
            break;
        case INC_ABSOLUTE:
            calc_addr (emu, absolute, flags_inc_absolute, inc, 3, 6, 0);
            break;
        case INC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_inc_absolute_x, inc, 3, 7, 0);
            break;
        case INX_IMPLIED:
            calc_addr (emu, NULL, NULL, inx, 1, 2, 0);
            break;
        case INY_IMPLIED:
            calc_addr (emu, NULL, NULL, iny, 1, 2, 0);
            break;
	case JMP_ABSOLUTE:
            calc_addr (emu, absolute, NULL, jmp, 3, 3, 0);
            break;
	case JMP_INDIRECT:
            calc_addr (emu, indirect, NULL, jmp, 3, 3, 0);
            break;
	case JSR_ABSOLUTE:
	    calc_addr (emu, absolute, NULL, jsr, 3, 6, 0);
	    break;
        case LDA_IMMEDIATE:
            calc_addr (emu, immediate, flags_lda_imm, lda, 2, 2, 0);
            break;
        case LDA_ZEROPAGE:
            calc_addr (emu, zeropage, flags_lda_zeropage, lda, 2, 3, 0);
            break;
        case LDA_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_lda_zeropage_x, lda, 2, 4, 0);
            break;
        case LDA_ABSOLUTE:
            calc_addr (emu, absolute, flags_lda_absolute, lda, 3, 4, 0);
            break;
        case LDA_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_lda_absolute_x, lda, 3, 4, 1);
            break;
        case LDA_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_lda_absolute_y, lda, 3, 4, 1);
            break;
        case LDA_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_lda_indirect_x, lda, 2, 6, 0);
            break;
        case LDA_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_lda_indirect_y, lda, 2, 5, 1);
            break;
        case LDX_IMMEDIATE:
            calc_addr (emu, immediate, flags_ldx_imm, ldx, 2, 2, 0);
            break;
        case LDX_ZEROPAGE:
            calc_addr (emu, zeropage, flags_ldx_zeropage, ldx, 2, 3, 0);
            break;
        case LDX_ZEROPAGE_Y:
            calc_addr (emu, zeropage_y, flags_ldx_zeropage_y, ldx, 2, 4, 0);
            break;
        case LDX_ABSOLUTE:
            calc_addr (emu, absolute, flags_ldx_absolute, ldx, 3, 4, 0);
            break;
        case LDX_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_ldx_absolute_y, ldx, 3, 4, 1);
            break;
        case LDY_IMMEDIATE:
            calc_addr (emu, immediate, flags_ldy_imm, ldx, 2, 2, 0);
            break;
        case LDY_ZEROPAGE:
            calc_addr (emu, zeropage, flags_ldy_zeropage, ldy, 2, 3, 0);
            break;
        case LDY_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_ldy_zeropage_x, ldy, 2, 4, 0);
            break;
        case LDY_ABSOLUTE:
            calc_addr (emu, absolute, flags_ldy_absolute, ldy, 3, 4, 0);
            break;
	case LDY_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_ldy_absolute_x, ldy, 3, 4, 1);
            break;
	case LSR_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_lsr_accumulator, lsr, 1, 2, 0);
	    break;
        case LSR_ZEROPAGE:
            calc_addr (emu, zeropage, flags_lsr_zeropage, lsr, 2, 3, 0);
            break;
        case LSR_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_lsr_zeropage_x, lsr, 2, 4, 0);
            break;
        case LSR_ABSOLUTE:
            calc_addr (emu, absolute, flags_lsr_absolute, lsr, 3, 4, 0);
            break;
	case LSR_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_lsr_absolute_x, lsr, 3, 4, 1);
            break;
	case NOP_IMPLIED:
            calc_addr (emu, NULL, NULL, nop, 1, 2, 0);
	    break;
        case ORA_IMMEDIATE:
            calc_addr (emu, immediate, flags_ora_imm, ora, 2, 2, 0);
            break;
        case ORA_ZEROPAGE:
            calc_addr (emu, zeropage, flags_ora_zeropage, ora, 2, 3, 0);
            break;
        case ORA_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_ora_zeropage_x, ora, 2, 4, 0);
            break;
        case ORA_ABSOLUTE:
            calc_addr (emu, absolute, flags_ora_absolute, ora, 3, 4, 0);
            break;
        case ORA_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_ora_absolute_x, ora, 3, 4, 1);
            break;
        case ORA_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_ora_absolute_y, ora, 3, 4, 1);
            break;
        case ORA_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_ora_indirect_x, ora, 2, 6, 0);
            break;
        case ORA_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_ora_indirect_y, ora, 2, 5, 1);
            break;
	case PHA_IMPLIED:
            calc_addr (emu, NULL, NULL, pha, 1, 3, 0);
	    break;
	case PHP_IMPLIED:
            calc_addr (emu, NULL, NULL, php, 1, 3, 0);
	    break;
	case PLA_IMPLIED:
            calc_addr (emu, NULL, NULL, pla, 1, 4, 0);
	    break;
	case PLP_IMPLIED:
            calc_addr (emu, NULL, NULL, plp, 1, 4, 0);
	    break;
	case ROL_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_rol_accumulator, rol, 1, 2, 0);
	    break;
        case ROL_ZEROPAGE:
            calc_addr (emu, zeropage, flags_rol_zeropage, rol, 2, 5, 0);
            break;
        case ROL_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_rol_zeropage_x, rol, 2, 6, 0);
            break;
        case ROL_ABSOLUTE:
            calc_addr (emu, absolute, flags_rol_absolute, rol, 3, 6, 0);
            break;
	case ROL_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_rol_absolute_x, rol, 3, 7, 0);
            break;
	case ROR_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_ror_accumulator, ror, 1, 2, 0);
	    break;
        case ROR_ZEROPAGE:
            calc_addr (emu, zeropage, flags_ror_zeropage, ror, 2, 5, 0);
            break;
        case ROR_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_ror_zeropage_x, ror, 2, 6, 0);
            break;
        case ROR_ABSOLUTE:
            calc_addr (emu, absolute, flags_ror_absolute, ror, 3, 6, 0);
            break;
	case ROR_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_ror_absolute_x, ror, 3, 7, 0);
            break;
	case RTI_IMPLIED:
            calc_addr (emu, NULL, NULL, rti, 1, 6, 0);
            break;
	case RTS_IMPLIED:
            calc_addr (emu, NULL, NULL, rts, 1, 6, 0);
            break;
        case SBC_IMMEDIATE:
            calc_addr (emu, immediate, flags_sbc_imm, sbc, 2, 2, 0);
            break;
        case SBC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_sbc_zeropage, sbc, 2, 3, 0);
            break;
        case SBC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_sbc_zeropage_x, sbc, 2, 4, 0);
            break;
        case SBC_ABSOLUTE:
            calc_addr (emu, absolute, flags_sbc_absolute, sbc, 3, 4, 0);
            break;
        case SBC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_sbc_absolute_x, sbc, 3, 4, 1);
            break;
        case SBC_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_sbc_absolute_y, sbc, 3, 4, 1);
            break;
        case SBC_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_sbc_indirect_x, sbc, 2, 6, 0);
            break;
        case SBC_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_sbc_indirect_y, sbc, 2, 5, 1);
            break;
	case SEC_IMPLIED:
            calc_addr (emu, NULL, NULL, sec, 1, 2, 0);
            break;
	case SED_IMPLIED:
            calc_addr (emu, NULL, NULL, sed, 1, 2, 0);
            break;
	case SEI_IMPLIED:
            calc_addr (emu, NULL, NULL, sei, 1, 2, 0);
            break;
        case STA_ZEROPAGE:
            calc_addr (emu, zeropage, NULL, sta, 2, 3, 0);
            break;
        case STA_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, NULL, sta, 2, 4, 0);
            break;
        case STA_ABSOLUTE:
            calc_addr (emu, absolute, NULL, sta, 3, 4, 0);
            break;
        case STA_ABSOLUTE_X:
            calc_addr (emu, absolute_x, NULL, sta, 3, 5, 0);
            break;
        case STA_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, NULL, sta, 3, 5, 0);
            break;
        case STA_INDIRECT_X:
            calc_addr (emu, indirect_x, NULL, sta, 2, 6, 0);
            break;
        case STA_INDIRECT_Y:
            calc_addr (emu, indirect_y, NULL, sta, 2, 6, 0);
            break;
        case STX_ZEROPAGE:
            calc_addr (emu, zeropage, NULL, stx, 2, 3, 0);
            break;
        case STX_ZEROPAGE_Y:
            calc_addr (emu, zeropage_y, NULL, stx, 2, 4, 0);
            break;
        case STX_ABSOLUTE:
            calc_addr (emu, absolute, NULL, stx, 3, 4, 0);
            break;
        case STY_ZEROPAGE:
            calc_addr (emu, zeropage, NULL, sty, 2, 3, 0);
            break;
        case STY_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, NULL, sty, 2, 4, 0);
            break;
        case STY_ABSOLUTE:
            calc_addr (emu, absolute, NULL, sty, 3, 4, 0);
            break;
	case TAX_IMPLIED:
            calc_addr (emu, NULL, flags_tax_implied, tax, 1, 2, 0);
            break;
	case TAY_IMPLIED:
            calc_addr (emu, NULL, flags_tay_implied, tay, 1, 2, 0);
            break;
	case TSX_IMPLIED:
            calc_addr (emu, NULL, flags_tsx_implied, tsx, 1, 2, 0);
            break;
	case TXA_IMPLIED:
            calc_addr (emu, NULL, flags_txa_implied, txa, 1, 2, 0);
            break;
	case TXS_IMPLIED:
            calc_addr (emu, NULL, NULL, txs, 1, 2, 0);
            break;
	case TYA_IMPLIED:
            calc_addr (emu, NULL, flags_tya_implied, tya, 1, 2, 0);
            break;
        }
    }
}
