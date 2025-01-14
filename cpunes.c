#include "cpunes.h"
#include <flags.h>
#include <exec.h>
#include <op_name.h>

void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions)
{

    for (uint32_t count = 0; count < count_instructions; count++) {

        switch (emu->buf[emu->cur_pc]) {
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
            calc_addr (emu, NULL, NULL, vbs, 2, 2, 2);
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
            calc_addr (emu, immediate, flags_and_imm, _cmp, 2, 2, 0);
            break;
        case CMP_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _cmp, 2, 3, 0);
            break;
        case CMP_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _cmp, 2, 4, 0);
            break;
        case CMP_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _cmp, 3, 4, 0);
            break;
        case CMP_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _cmp, 3, 4, 1);
            break;
        case CMP_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _cmp, 3, 4, 1);
            break;
        case CMP_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _cmp, 2, 6, 0);
            break;
        case CMP_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _cmp, 2, 5, 1);
            break;
	case CPX_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _cpx, 2, 2, 0);
            break;
        case CPX_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _cpx, 2, 3, 0);
            break;
        case CPX_ABSOLUTE:
            calc_addr (emu, absolute flags_and_absolute, _cpx, 3, 4, 0);
            break;
	case CPY_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _cpy, 2, 2, 0);
            break;
        case CPY_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _cpy, 2, 3, 0);
            break;
        case CPY_ABSOLUTE:
            calc_addr (emu, absolute flags_and_absolute, _cpy, 3, 4, 0);
            break;
        case DEC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _dec, 2, 5, 0);
            break;
        case DEC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _dec, 2, 6, 0);
            break;
        case DEC_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _dec, 3, 6, 0);
            break;
        case DEC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _dec, 3, 7, 0);
            break;
        case DEX_IMPLIED:
            calc_addr (emu, NULL, NULL, dex, 1, 2, 0);
            break;
        case DEY_IMPLIED:
            calc_addr (emu, NULL, NULL, dey, 1, 2, 0);
            break;
        case EOR_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _eor, 2, 2, 0);
            break;
        case EOR_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _eor, 2, 3, 0);
            break;
        case EOR_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _eor, 2, 4, 0);
            break;
        case EOR_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _eor, 3, 4, 0);
            break;
        case EOR_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _eor, 3, 4, 1);
            break;
        case EOR_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _eor, 3, 4, 1);
            break;
        case EOR_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _eor, 2, 6, 0);
            break;
        case EOR_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _eor, 2, 5, 1);
            break;
        case INC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _inc, 2, 5, 0);
            break;
        case INC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _inc, 2, 6, 0);
            break;
        case INC_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _inc, 3, 6, 0);
            break;
        case INC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _inc, 3, 7, 0);
            break;
        case INX_IMPLIED:
            calc_addr (emu, NULL, NULL, inx, 1, 2, 0);
            break;
        case INY_IMPLIED:
            calc_addr (emu, NULL, NULL, iny, 1, 2, 0);
            break;
	case JMP_ABSOLUTE:
            calc_addr (emu, absolute, NULL, _jmp, 3, 3, 0);
            break;
	case JMP_INDIRECT:
            calc_addr (emu, indirect, NULL, _jmp, 3, 3, 0);
            break;
	case JSR_ABSOLUTE:
	    calc_addr (emu, absolute, NULL, _jsr, 3, 6, 0);
	    break;
        case LDA_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _lda, 2, 2, 0);
            break;
        case LDA_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _lda, 2, 3, 0);
            break;
        case LDA_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _lda, 2, 4, 0);
            break;
        case LDA_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _lda, 3, 4, 0);
            break;
        case LDA_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _lda, 3, 4, 1);
            break;
        case LDA_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _lda, 3, 4, 1);
            break;
        case LDA_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _lda, 2, 6, 0);
            break;
        case LDA_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _lda, 2, 5, 1);
            break;
        case LDX_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _ldx, 2, 2, 0);
            break;
        case LDX_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _ldx, 2, 3, 0);
            break;
        case LDX_ZEROPAGE_Y:
            calc_addr (emu, zeropage_y, flags_and_zeropage_y, _ldx, 2, 4, 0);
            break;
        case LDX_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _ldx, 3, 4, 0);
            break;
        case LDX_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _ldx, 3, 4, 1);
            break;
        case LDY_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _ldx, 2, 2, 0);
            break;
        case LDY_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _ldy, 2, 3, 0);
            break;
        case LDY_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _ldy, 2, 4, 0);
            break;
        case LDY_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _ldy, 3, 4, 0);
            break;
	case LDY_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _ldy, 3, 4, 1);
            break;
	case LSR_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_and_accumulator, _lsr, 1, 2, 0);
	    break;
        case LSR_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _lsr, 2, 3, 0);
            break;
        case LSR_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _lsr, 2, 4, 0);
            break;
        case LSR_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _lsr, 3, 4, 0);
            break;
	case LSR_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _lsr, 3, 4, 1);
            break;
	case NOP_IMPLIED:
            calc_addr (emu, NULL, NULL, _nop, 1, 2, 0);
	    break;
        case ORA_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _ora, 2, 2, 0);
            break;
        case ORA_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _ora, 2, 3, 0);
            break;
        case ORA_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _ora, 2, 4, 0);
            break;
        case ORA_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _ora, 3, 4, 0);
            break;
        case ORA_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _ora, 3, 4, 1);
            break;
        case ORA_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _ora, 3, 4, 1);
            break;
        case ORA_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _ora, 2, 6, 0);
            break;
        case ORA_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _ora, 2, 5, 1);
            break;
	case PHA_IMPLIED:
            calc_addr (emu, NULL, NULL, _pha, 1, 3, 0);
	    break;
	case PHP_IMPLIED:
            calc_addr (emu, NULL, NULL, _php, 1, 3, 0);
	    break;
	case PLA_IMPLIED:
            calc_addr (emu, NULL, NULL, _pla, 1, 4, 0);
	    break;
	case PLP_IMPLIED:
            calc_addr (emu, NULL, NULL, _plp, 1, 4, 0);
	    break;
	case ROL_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_and_accumulator, _rol, 1, 2, 0);
	    break;
        case ROL_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _rol, 2, 5, 0);
            break;
        case ROL_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _rol, 2, 6, 0);
            break;
        case ROL_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _rol, 3, 6, 0);
            break;
	case ROL_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _rol, 3, 7, 0);
            break;
	case ROR_ACCUMULATOR:
            calc_addr (emu, accumulator, flags_and_accumulator, _ror, 1, 2, 0);
	    break;
        case ROR_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _ror, 2, 5, 0);
            break;
        case ROR_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _ror, 2, 6, 0);
            break;
        case ROR_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _ror, 3, 6, 0);
            break;
	case ROR_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _ror, 3, 7, 0);
            break;
	case RTI_IMPLIED:
            calc_addr (emu, NULL, NULL, _rti, 1, 6, 0);
            break;
	case RTS_IMPLIED:
            calc_addr (emu, NULL, NULL, _rts, 1, 6, 0);
            break;
        case SBC_IMMEDIATE:
            calc_addr (emu, immediate, flags_and_imm, _sbc, 2, 2, 0);
            break;
        case SBC_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _sbc, 2, 3, 0);
            break;
        case SBC_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _sbc, 2, 4, 0);
            break;
        case SBC_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _sbc, 3, 4, 0);
            break;
        case SBC_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _sbc, 3, 4, 1);
            break;
        case SBC_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _sbc, 3, 4, 1);
            break;
        case SBC_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _sbc, 2, 6, 0);
            break;
        case SBC_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _sbc, 2, 5, 1);
            break;
	case SEC_IMPLIED:
            calc_addr (emu, NULL, NULL, _sec, 1, 2, 0);
            break;
	case SED_IMPLIED:
            calc_addr (emu, NULL, NULL, _sed, 1, 2, 0);
            break;
	case SEI_IMPLIED:
            calc_addr (emu, NULL, NULL, _sei, 1, 2, 0);
            break;
        case STA_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _sta, 2, 3, 0);
            break;
        case STA_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _sta, 2, 4, 0);
            break;
        case STA_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _sta, 3, 4, 0);
            break;
        case STA_ABSOLUTE_X:
            calc_addr (emu, absolute_x, flags_and_absolute_x, _sta, 3, 5, 0);
            break;
        case STA_ABSOLUTE_Y:
            calc_addr (emu, absolute_y, flags_and_absolute_y, _sta, 3, 5, 0);
            break;
        case STA_INDIRECT_X:
            calc_addr (emu, indirect_x, flags_and_indirect_x, _sta, 2, 6, 0);
            break;
        case STA_INDIRECT_Y:
            calc_addr (emu, indirect_y, flags_and_indirect_y, _sta, 2, 6, 0);
            break;
        case STX_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _stx, 2, 3, 0);
            break;
        case STX_ZEROPAGE_Y:
            calc_addr (emu, zeropage_y, flags_and_zeropage_y, _stx, 2, 4, 0);
            break;
        case STX_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _stx, 3, 4, 0);
            break;
        case STY_ZEROPAGE:
            calc_addr (emu, zeropage, flags_and_zeropage, _sty, 2, 3, 0);
            break;
        case STY_ZEROPAGE_X:
            calc_addr (emu, zeropage_x, flags_and_zeropage_x, _sty, 2, 4, 0);
            break;
        case STY_ABSOLUTE:
            calc_addr (emu, absolute, flags_and_absolute, _sty, 3, 4, 0);
            break;
	case TAX_IMPLIED:
            calc_addr (emu, NULL, NULL, _tax, 1, 2, 0);
            break;
	case TAY_IMPLIED:
            calc_addr (emu, NULL, NULL, _tay, 1, 2, 0);
            break;
	case TSX_IMPLIED:
            calc_addr (emu, NULL, NULL, _tsx, 1, 2, 0);
            break;
	case TXA_IMPLIED:
            calc_addr (emu, NULL, NULL, _txa, 1, 2, 0);
            break;
	case TXS_IMPLIED:
            calc_addr (emu, NULL, NULL, _txs, 1, 2, 0);
            break;
	case TYA_IMPLIED:
            calc_addr (emu, NULL, NULL, _tya, 1, 2, 0);
            break;
        }
    }
}
