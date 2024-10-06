#ifndef OPERATOR_NAMES_H
#define OPERATOR_NAMES_H

enum {
    AND_INDIRECT_X = 0x21,
    AND_ZEROPAGE = 0x25,
    AND_IMMEDIATE = 0x29,
    AND_ABSOLUTE = 0x2d,
    AND_INDIRECT_Y = 0x31,
    AND_ZEROPAGE_X = 0x35,
    AND_ABSOLUTE_Y = 0x39,
    AND_ABSOLUTE_X = 0x3d,

    ADC_INDIRECT_X = 0x61,
    ADC_ZEROPAGE = 0x65,
    ADC_IMMEDIATE = 0x69,
    ADC_ABSOLUTE = 0x6d,
    ADC_INDIRECT_Y = 0x71,
    ADC_ZEROPAGE_X = 0x75,
    ADC_ABSOLUTE_Y = 0x79,
    ADC_ABSOLUTE_X = 0x7d,


};

#endif // OPERATOR_NAMES_H
