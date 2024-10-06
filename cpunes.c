#include "cpunes.h"
#include <flags_checking.h>
#include <opcode_exec.h>
#include <operator_names.h>

void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions)
{

    for (uint32_t count = 0; count < count_instructions; count++) {

        switch (emu->buf[emu->cur_pc]) {
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
        }
    }
}
