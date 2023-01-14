#pragma once

union ui16
{
    uint16_t vUI16;
    struct {
        uint8_t b1;
        uint8_t b2;
    };
};

union ui32
{
    uint32_t vUI32;
    struct {
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
        uint8_t b4;
    };
};
