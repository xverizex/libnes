#include "cpunes.h"
#include <flags_checking.h>
#include <opcode_exec.h>
#include <operator_names.h>

void nes_emu_execute (struct NESEmu *emu, uint32_t count_instructions)
{

    for (uint32_t count = 0; count < count_instructions; count++) {

        switch (emu->buf[emu->cur_pc]) {
        case ADC_IMMEDIATE:
            // ADC IMMEDIATE 2 bytes. CYCLES 2
            calc_addr (emu, immediate, flags_adc_imm, adc, 2, 2, 0);
            break;
        case ADC_ZEROPAGE:
            // ADC ZEROPAGE 2 bytes. CYCLES 3
            calc_addr (emu, zeropage, flags_adc_zeropage, adc, 2, 3, 0);
            break;
        case ADC_ZEROPAGE_X:
            // ADC ZEROPAGE,X 2 bytes. CYCLES 4
            calc_addr (emu, zeropage_x, flags_adc_zeropage_x, adc, 2, 4, 0);
            break;
        case ADC_ABSOLUTE:
            // ADC ABSOLUTE 3 bytes. CYCLES 4
            calc_addr (emu, absolute, flags_adc_absolute, adc, 3, 4, 0);
            break;
        case ADC_ABSOLUTE_X:
            // ADC ABSOLUTE,X 3 bytes. CYCLES 4(+1 if page crossed)
            calc_addr (emu, absolute_x, flags_adc_absolute_x, adc, 3, 4, 1);
            break;
        case ADC_ABSOLUTE_Y:
            // ADC ABSOLUTE,Y 3 bytes. CYCLES 4(+1 if page crossed)
            calc_addr (emu, absolute_y, flags_adc_absolute_y, adc, 3, 4, 1);
            break;
        case ADC_INDIRECT_X:
            // ADC (INDIRECT,X) 2 bytes. CYCLES 6
            calc_addr (emu, indirect_x, flags_adc_indirect_x, adc, 2, 6, 0);
            break;
        case ADC_INDIRECT_Y:
            // ADC (INDIRECT),Y 2 bytes. CYCLES 5(+1 if page crossed)
            calc_addr (emu, indirect_y, flags_adc_indirect_y, adc, 2, 5, 1);
            break;

        }
    }
}
