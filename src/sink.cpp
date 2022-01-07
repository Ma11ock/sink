#include "sink.hpp"

#include <limits>
#include <algorithm>
#include <cstdlib>
#include <string>

using namespace std::string_literals;

namespace
{
    inline std::int32_t twosComp(std::int32_t i)
    { return -i; } 

    inline std::uint32_t twosComp(std::uint32_t i)
    { return ~i + 1; }
}


// Construct a new sink with a sign bit (in its proper place (left-most bit)),
// an unbiased mathematical exponent, and a mathematical mantissa.
inline std::uint32_t constructSink(std::uint32_t sign, std::int32_t exp,
                                   std::uint32_t mantissa) 
{
    return (sign & Sink32::SIGN_BIT_LOC) |
        ((static_cast<std::uint32_t>(exp + (exp == -Sink32::SUB_BIAS ?
                                            Sink32::SUB_BIAS : Sink32::BIAS))
          << Sink32::MANTISSA_BITS) & Sink32::EXPONENT_BITS_LOC)
        | (mantissa & Sink32::MANTISSA_BITS_LOC);
}

// Normalize a float. 
// Exponent should not be normalized.
inline void normalizeResult(std::uint32_t &exponent, std::uint32_t &mantissa)
{
    constexpr std::uint32_t MANTISSA_OVERFLOW_BITS = UINT32_C(0x1000000);
    if(mantissa & MANTISSA_OVERFLOW_BITS)
    {
        mantissa >>= 1;
        exponent++;
    }
    // The leftmost 1 bit is left the 24th bit, requiring a left shift.
    if((mantissa & Sink32::MANTISSA_BITS_LOC) &&
       !(mantissa & Sink32::MANTISSA_LEADING_ONE))
        do
        {
            mantissa <<= 1;
            exponent--;
        } while(!(mantissa & Sink32::MANTISSA_LEADING_ONE));
    else if(mantissa == 0)
        exponent = static_cast<std::uint32_t>(-Sink32::SUB_BIAS);
    // If the 23rd bit is 1, do nothing
}

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
    for(; (value & MANTISSA_LEADING_ONE) == 0; shifts++, value <<= 1);

    std::uint32_t exponent = BIAS + MANTISSA_BITS - shifts;

    *this = CreateLiteral((exponent << MANTISSA_BITS) |
                          (value & MANTISSA_BITS_LOC));
}

Sink32::Sink32(std::int32_t value) : mValue(static_cast<std::uint32_t>(value))
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
    std::uint32_t tValue = static_cast<std::uint32_t>(std::abs(value));

    std::uint32_t shifts = 0;
    for(; (tValue & MANTISSA_LEADING_ONE) == 0 && shifts < MANTISSA_BITS;
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

Sink32 Sink32::operator+(Sink32 other) const
{
    constexpr std::uint32_t FULL_MANTISSA_BITS = UINT32_C(0x1FFFFFF);
    std::uint32_t addend = other.mValue;
    // The 24th bit, leading 1 for the mantissa.
    // Step 1: Extract pieces from the floats.
    // Organize the parts of each Sink by which exponent is greater.
    // Step 2: For the mantissa, prepend leading 1 (if normal).
    std::uint32_t sSign = mValue & SIGN_BIT_LOC;
    // Mathematical exponent.
    std::int32_t sExponent = frexp();
    auto sMantissa = getMantissa();

    std::uint32_t bSign = addend & SIGN_BIT_LOC;
    // Mathematical exponent.
    std::int32_t bExponent = other.frexp();
    auto bMantissa = other.getMantissa();

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
    std::int32_t finalExponent = bExponent;
    // The final sign is the bigger sign.
    std::uint32_t finalSign = bSign;
    // Step 6: If the total mantissa is negative, 2's complement it.
    if(finalSign)
        finalMantissa = twosComp(finalMantissa);
    // Step 7: Normalize the mantissa.
    // If the 25th or 24th bits are 1 we must right shift until they are 0.
    normalizeResult(reinterpret_cast<std::uint32_t&>(finalExponent),
                    finalMantissa);
    // Step 8: Compose result.
    return CreateLiteral(constructSink(finalSign, finalExponent,
                                       finalMantissa));
}

Sink32 Sink32::operator-(Sink32 minuend) const
{
    return *this + -minuend;
}

Sink32 Sink32::operator*(Sink32 multiplier) const
{
    // Multiplier values.
    std::uint32_t plierSign = multiplier.mValue & SIGN_BIT_LOC;
    std::int32_t plierExponent = multiplier.frexp();
    std::uint32_t plierMantissa = multiplier.mValue & MANTISSA_BITS_LOC;
    if(multiplier.isNormal())
        plierMantissa |= MANTISSA_LEADING_ONE;
    // Multiplicand values.
    std::uint32_t candSign = mValue & SIGN_BIT_LOC;
    std::int32_t candExponent = frexp();
    std::uint32_t candMantissa = mValue & MANTISSA_BITS_LOC;
    if(isNormal())
        candMantissa |= MANTISSA_LEADING_ONE;

    std::uint32_t finalSign = (plierSign ^ candSign) ? SIGN_BIT_LOC : 0;
    std::int32_t finalExponent = plierExponent + candExponent;
    std::uint32_t finalMantissa = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(plierMantissa) *
         static_cast<std::uint64_t>(candMantissa))
        >> MANTISSA_BITS);

    normalizeResult(reinterpret_cast<std::uint32_t&>(finalExponent),
                    finalMantissa);

    return CreateLiteral(constructSink(finalSign, finalExponent,
                                       finalMantissa));
}

Sink32 Sink32::operator/(Sink32 divisor) const
{
    // Divide by 0 returns NAN.
    if(divisor == Sink32(0))
        return CreateLiteral(NAN_MIN);
    
    // Dividend values.
    std::uint32_t dendSign = mValue & SIGN_BIT_LOC;
    std::int32_t dendExponent = frexp();
    std::uint64_t dendMantissa = mValue & MANTISSA_BITS_LOC;
    if(isNormal())
        dendMantissa |= MANTISSA_LEADING_ONE;
    // Extend dividend mantissa to 48 bits.
    dendMantissa <<= MANTISSA_BITS;

    // Divisor values.
    std::uint32_t visorSign = divisor.mValue & SIGN_BIT_LOC;
    std::int32_t visorExponent = frexp();
    std::uint64_t visorMantissa = divisor.mValue & MANTISSA_BITS_LOC;
    if(divisor.isNormal())
        visorMantissa |= MANTISSA_LEADING_ONE;

    std::uint32_t finalSign = (dendSign ^ visorSign) ? SIGN_BIT_LOC : 0;
    std::int32_t finalExponent = visorExponent - dendExponent; 
    std::uint32_t finalMantissa = dendMantissa / visorMantissa;
    normalizeResult(reinterpret_cast<std::uint32_t&>(finalExponent),
                    finalMantissa);

    return CreateLiteral(constructSink(finalSign, finalExponent,
                                      finalMantissa));
}

std::string Sink32::toString(std::string::size_type precision) const
{
    if(isNaN())
        return "NaN";
    else if(isInfinity())
        return (isNegative() ? "-"s : ""s) + "Inf"s;
    // 32bit floats cannot go beyond 8 decimal points of precision.
    // Precision plus dot plus whole components. 
    auto exponent = frexp();
    Sink32 remainder = Sink32(toInt()) - *this;
    return std::to_string(toInt()) + "."s +
        std::to_string(remainder.getMantissa() << 9).substr(0, precision);
}

Sink32::operator float() const
{
    std::uint32_t valToCopy = mValue;
    if constexpr(!std::numeric_limits<float>::is_iec559)
        valToCopy = NAN_MIN;
    return reinterpret_cast<float&>(valToCopy);
}

Sink32::operator std::int32_t() const
{
    // Non-standard IEEE: Intel returns 0x80000000 when an int cannot
    // represent the float.
    if(isNaN() || isInfinity())
        return static_cast<std::int32_t>(0x80000000);
    // 0 is special because it is the only subnormal integer.
    else if(isZero())
        return 0;
    std::uint32_t sign = mValue & SIGN_BIT_LOC;
    auto exp = static_cast<std::int32_t>((mValue & EXPONENT_BITS_LOC)
                                         >> MANTISSA_BITS) -
        static_cast<std::int32_t>(BIAS); 
    std::uint32_t mantissa = (mValue & MANTISSA_BITS_LOC)
        | MANTISSA_LEADING_ONE;
    return (mantissa >> (static_cast<std::int32_t>(MANTISSA_BITS) - exp))
        | sign;
}

// Get the mathematical exponent. If the Sink is NaN or
// infinity return the smallest 32 bit integer (INT_MIN).
std::int32_t Sink32::frexp() const
{
    auto exp = static_cast<std::int32_t>((mValue & EXPONENT_BITS_LOC)
                                         >> MANTISSA_BITS);
    if(isNormal())
        return exp - static_cast<std::int32_t>(BIAS);
    else if(isSubNormal())
        return exp - static_cast<std::int32_t>(SUB_BIAS);
    return std::numeric_limits<std::int32_t>::min();
}
