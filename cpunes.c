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
            adc_immediate(emu);
            break;
        case ADC_ZEROPAGE:
            // ADC ZEROPAGE 2 bytes. CYCLES 3

            break;
        case ADC_ZEROPAGE_X:
            // ADC ZEROPAGE,X 2 bytes. CYCLES 4
            break;
        case ADC_ABSOLUTE:
            // ADC ABSOLUTE 3 bytes. CYCLES 4
            break;
        case ADC_ABSOLUTE_X:
            // ADC ABSOLUTE,X 3 bytes. CYCLES 4(+1 if page crossed)
            break;
        case ADC_ABSOLUTE_Y:
            // ADC ABSOLUTE,Y 3 bytes. CYCLES 4(+1 if page crossed)
            break;
        case ADC_INDIRECT_X:
            // ADC (INDIRECT,X) 2 bytes. CYCLES 6
            break;
        case ADC_INDIRECT_Y:
            // ADC (INDIRECT),Y 2 bytes. CYCLES 5(+1 if page crossed)
            break;

        }
    }
}
