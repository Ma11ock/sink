#include "sink.hpp"

#include <limits>
#include <algorithm>

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
    std::uint32_t tValue = static_cast<std::uint32_t>(value);

    std::uint32_t shifts = 0;
    for(; (tValue & LEADING_ONE) == 0 && shifts < MANTISSA_BITS;
        shifts++, tValue <<= 1);

    std::uint32_t exponent = BIAS + MANTISSA_BITS - shifts;

    *this = CreateLiteral((mValue & SIGN_BIT_LOC) |
                          (exponent << MANTISSA_BITS) |
                          (tValue & MANTISSA_BITS_LOC));
}

Sink32::Sink32(float value)
{
    if constexpr(std::numeric_limits<float>::is_iec559)
        mValue = reinterpret_cast<std::uint32_t&>(value);
    else
        mValue = NAN_MIN;
}

Sink32 Sink32::operator+(Sink32 other)
{
    std::uint32_t addend = other.mValue;
    // Construct a new sink a sign bit (in its proper place (left-most bit)),
    // an unbiased mathematical exponent, and a mathematical mantissa.
    auto constructSink = [=](std::uint32_t sign, std::int32_t exp,
                             std::uint32_t mantissa) -> std::uint32_t
    {
        return sign |
            ((static_cast<std::uint32_t>(exp + (exp == -SUB_BIAS ? SUB_BIAS : BIAS))
              << MANTISSA_BITS) & EXPONENT_BITS_LOC)
            | (mantissa & MANTISSA_BITS_LOC);
    };
    // The 24th bit, leading 1 for the mantissa.
    // Step 1: Extract pieces from the floats.
    // Organize the parts of each Sink by which exponent is greater.
    // Step 2: For the mantissa, prepend leading 1 (if normal).
    std::uint32_t sSign = mValue & SIGN_BIT_LOC;
    // Mathematical exponent.
    std::int32_t sExponent = frexp();
    std::uint32_t sMantissa = mValue & MANTISSA_BITS_LOC;
    if(isNormal())
        sMantissa |= LEADING_ONE;

    std::uint32_t bSign = addend & SIGN_BIT_LOC;
    // Mathematical exponent.
    std::int32_t bExponent = other.frexp();
    std::uint32_t bMantissa = addend & MANTISSA_BITS_LOC;
    if(other.isNormal())
        bMantissa |= LEADING_ONE;

    if(sExponent > bExponent)
    {
        std::swap(sExponent, bExponent);
        std::swap(sSign, bSign);
        std::swap(sMantissa, bMantissa);
    }

    // If the difference in exponent is > MANTISSA_BITS + 1 then the
    // addition operation is not possible because the right shift
    // will 0 out the mantissa.
    if((bExponent - static_cast<std::int32_t>(MANTISSA_BITS + 1)) > sExponent)
        return CreateLiteral(constructSink(bSign, bExponent, bMantissa));

    // Step 3: Compare Exponents, right shift the smaller exp's
    // mantissa by the difference.
    sMantissa >>= bExponent - sExponent;
    sExponent = bExponent;
    // Step 4: If an operand is negative, 2's complement it.
    if(sSign)
        sMantissa = twosComp(sMantissa);
    if(bSign)
        bMantissa = twosComp(bMantissa);
    // Step 5: Add mantissas (24bit addition):
    std::uint32_t finalMantissa = sMantissa + bMantissa;
    // The final exponent is the bigger exponent.
    std::uint32_t finalExponent = bExponent;
    // The final sign is the bigger sign.
    std::uint32_t finalSign = bSign;
    // Step 6: If the total mantissa is negative, 2's complement it.
    if(finalSign)
        finalMantissa = twosComp(sMantissa);
    // Step 7: Normalize the mantissa.
    // If the 25th or 24th bits are 1 we must right shift until they are 0.
    constexpr std::uint32_t MANTISSA_OVERFLOW_BITS = UINT32_C(0x1000000);
    if(finalMantissa & MANTISSA_OVERFLOW_BITS)
    {
        finalMantissa >>= 1;
        finalExponent++;
    }
    // The leftmost 1 bit is after the 23rd bit, requiring a left shit.
    else if(finalMantissa & UINT32_C(0x3FFFFF))
        for(; !(finalMantissa & LEADING_ONE); finalMantissa <<= 1,
                finalExponent--);
    // If the 23rd bit is 1, do nothing

    // Step 8: Compose result.
    return CreateLiteral(constructSink(finalSign, finalExponent, finalMantissa));
}

Sink32 Sink32::operator-(Sink32 minuend)
{
    return *this + -minuend;
}

Sink32::operator float() const
{
    std::uint32_t valToCopy = mValue;
    if constexpr(!std::numeric_limits<float>::is_iec559)
        valToCopy = NAN_MIN;
    return reinterpret_cast<float&>(valToCopy);
}


// Get the mathematical exponent. If the Sink is NaN or
// infinity return the smallest 32 bit integer (INT_MIN).
std::int32_t Sink32::frexp() const
{
    auto exp = static_cast<std::int32_t>((mValue & EXPONENT_BITS_LOC)
                                         >> MANTISSA_BITS);
    if(isNormal())
        return exp - BIAS;
    else if(isSubNormal())
        return exp - SUB_BIAS;
    return std::numeric_limits<std::int32_t>::min();
}
