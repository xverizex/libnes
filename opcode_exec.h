#ifndef OPCODE_EXEC_H
#define OPCODE_EXEC_H
#include <cpunes.h>

uint16_t immediate (struct NESEmu *emu);

uint16_t absolute (struct NESEmu *emu);

uint16_t zeropage (struct NESEmu *emu);

uint16_t zeropage_x (struct NESEmu *emu);

uint16_t absolute_x (struct NESEmu *emu);

uint16_t absolute_y (struct NESEmu *emu);

uint16_t indirect_x (struct NESEmu *emu);

uint16_t indirect_y (struct NESEmu *emu);

void adc (struct NESEmu *emu, uint16_t addr);

void calc_addr (struct NESEmu *emu,
                uint16_t (*get_addr) (struct NESEmu *emu),
                void (*flags) (struct NESEmu *emu),
                void (*opcode_exec) (struct NESEmu *emu, uint16_t addr),
                uint16_t pc_offset,
                uint8_t cycles
                );

#endif // OPCODE_EXEC_H
