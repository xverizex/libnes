#ifndef OP_NAME_H
#define OP_NAME_H

enum {
    BRK_IMPLIED = 0x00,
    ASL_ZEROPAGE = 0x06,
    ASL_ACCUMULATOR = 0x0a,
    ASL_ABSOLUTE = 0x0e,
    BPL_RELATIVE = 0x10,
    ASL_ZEROPAGE_X = 0x16,
    CLC_IMPLIED = 0x18,
    ASL_ABSOLUTE_X = 0x1e,

    AND_INDIRECT_X = 0x21,
    BIT_ZEROPAGE = 0x24,
    AND_ZEROPAGE = 0x25,
    AND_IMMEDIATE = 0x29,
    BIT_ABSOLUTE = 0x2c,
    AND_ABSOLUTE = 0x2d,
    BMI_RELATIVE = 0x30,
    AND_INDIRECT_Y = 0x31,
    AND_ZEROPAGE_X = 0x35,
    AND_ABSOLUTE_Y = 0x39,
    AND_ABSOLUTE_X = 0x3d,

    BVC_RELATIVE = 0x50,
    CLI_IMPLIED = 0x58,
    ADC_INDIRECT_X = 0x61,
    ADC_ZEROPAGE = 0x65,
    ADC_IMMEDIATE = 0x69,
    ADC_ABSOLUTE = 0x6d,
    BVS_RELATIVE = 0x70,
    ADC_INDIRECT_Y = 0x71,
    ADC_ZEROPAGE_X = 0x75,
    ADC_ABSOLUTE_Y = 0x79,
    ADC_ABSOLUTE_X = 0x7d,

    BCC_RELATIVE = 0x90,
    BCS_RELATIVE = 0xb0,
    CLV_IMPLIED = 0xb8,
    BNE_RELATIVE = 0xd0,
    CLD_IMPLIED = 0xd8,
    BEQ_RELATIVE = 0xf0,
};

#endif // OP_NAME_H