#ifndef FLAGS_CHECKING_H
#define FLAGS_CHECKING_H
#include <cpunes.h>

void flags_adc_imm (struct NESEmu *emu);
void flags_adc_absolute (struct NESEmu *emu);
void flags_adc_zeropage (struct NESEmu *emu);
void flags_adc_zeropage_x (struct NESEmu *emu);
void flags_adc_absolute_x (struct NESEmu *emu);
void flags_adc_absolute_y (struct NESEmu *emu);
void flags_adc_indirect_x (struct NESEmu *emu);
void flags_adc_indirect_y (struct NESEmu *emu);

#endif // FLAGS_CHECKING_H
