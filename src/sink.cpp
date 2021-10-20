#include "sink.hpp"

Sink32 Sink32::operator+(std::uint32_t addend)
{
    auto incExponent = [=](std::uint32_t &exp)
    {
        exp = ((exp >> MANTISSA_BITS) + 1) << MANTISSA_BITS;
    };
    // The 24th bit, leading 1 for the mantissa.
    // Step 1: Extract pieces from the floats.
    std::uint32_t sSign = mValue & EXPONENT_BITS_LOC;
    std::uint32_t sExponent = mValue & EXPONENT_BITS_LOC;
    std::uint32_t sMantissa = mValue & MANTISSA_BITS_LOC;

    std::uint32_t bSign = addend & EXPONENT_BITS_LOC;
    std::uint32_t bExponent = addend & EXPONENT_BITS_LOC;
    std::uint32_t bMantissa = addend & MANTISSA_BITS_LOC;

    if(sExponent > bExponent)
    {
        std::swap(sExponent, bExponent);
        std::swap(sSign, bSign);
        std::swap(sMantissa, bMantissa);
    }
    // Step 2: Prepend leading 1.
    sMantissa |= LEADING_ONE;
    bMantissa |= LEADING_ONE;

    // Step 3: Compare Exponents, right shift the smaller exp's
    // mantissa by the difference.
    sMantissa >>= bExponent - sExponent;
    sExponent = bExponent;
    // Step 4: If an operand is negative, 2's complement it.
    if(sSign)
        sMantissa = ~sMantissa + 1;
    if(bSign)
        bMantissa = ~bMantissa + 1;
    // Step 5: Add mantissas:
    std::uint32_t totalMantissa = sMantissa + bMantissa;
    // Step 6: If the total mantissa is negative, 2's complement again.
    if(totalMantissa & SIGN_BIT_LOC)
        totalMantissa = ~sMantissa + 1;
    // Step 7: Normalize the mantissa.
    // If the 24th bit is 1, right sift by 1.
    if(totalMantissa & LEADING_ONE)
    {
        totalMantissa >>= 1;
        incExponent(sExponent);
    }
    // The leftmost 1 bit is after the 23rd bit, requiring a left shit.
    else if(totalMantissa & UINT32_C(0x3FFFFF))
        for(; totalMantissa & LEADING_ONE; totalMantissa <<= 1,
                incExponent(sExponent));
    // If the 23rd bit is 1, do nothing
    // TODO to determine final sign: whatever the sign of the bigger number is.

    // Step 8: Compose result.
    return CreateLiteral(sSign | (sSign & EXPONENT_BITS) |
                         (totalMantissa & EXPONENT_BITS_LOC));
}

