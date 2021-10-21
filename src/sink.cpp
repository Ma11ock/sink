#include "sink.hpp"

static inline std::int32_t twosComp(std::int32_t i)
{ return -i; } 

static inline std::uint32_t twosComp(std::uint32_t i)
{ return ~i + 1; }

Sink32::Sink32(std::uint32_t value) : mValue(0)
{
    // Value must be in 24 bit range.
    if(value == 0)
        return;
    if(value > UINT32_C(0xFFFFFF) * 2)
    {
        // The value is out of range of a float. Return NaN.
        *this = CreateLiteral(NAN_MIN);
        return;
    }

    std::uint32_t shifts = 0;
    for(; (value & LEADING_ONE) == 0; shifts++, value <<= 1);

    std::uint32_t exponent = BIAS + MANTISSA_BITS - shifts;

    *this = CreateLiteral((exponent << MANTISSA_BITS) |
                          (value & MANTISSA_BITS_LOC));
}

Sink32::Sink32(std::int32_t value) : mValue(0)
{
    // Extract the sign value, exponent (and unbias it), and mantissa.
    if(value == 0)
        return;
    if(value < -INT32_C(0xFFFFFF) * 2 || value > INT32_C(0xFFFFFF) * 2)
    {
        // The value is out of range of a float. Return NAN.
        *this = CreateLiteral(NAN_MIN);
        return;
    }
    mValue |= (value & SIGN_BIT_LOC);

    std::uint32_t tValue = static_cast<std::uint32_t>(value);

    std::uint32_t shifts = 0;
    for(; (tValue & LEADING_ONE) == 0; shifts++, tValue <<= 1);

    std::uint32_t exponent = BIAS + MANTISSA_BITS - shifts;

    *this = CreateLiteral((exponent << MANTISSA_BITS) |
                          (tValue & MANTISSA_BITS_LOC));
}

Sink32::Sink32(float value)
{
    mValue = *reinterpret_cast<std::uint32_t*>(&value);
}

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
        sMantissa = twosComp(sMantissa);
    if(bSign)
        bMantissa = twosComp(bMantissa);
    // Step 5: Add mantissas:
    std::uint32_t totalMantissa = sMantissa + bMantissa;
    // Step 6: If the total mantissa is negative, 2's complement again.
    if(totalMantissa & SIGN_BIT_LOC)
        totalMantissa = twosComp(sMantissa);
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

