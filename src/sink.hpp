#ifndef SINK_HPP
#define SINK_HPP

#include <cstdint>
#include <algorithm>

class Sink32
{
public:
    static constexpr std::uint32_t SIGN_BITS = 1;
    static constexpr std::uint32_t EXPONENT_BITS = 8;
    static constexpr std::uint32_t MANTISSA_BITS = 23;
    static constexpr std::uint32_t SIGN_BIT_LOC = UINT32_C(0x80000000);
    static constexpr std::uint32_t EXPONENT_BITS_LOC = UINT32_C(0x7F800000);
    static constexpr std::uint32_t MANTISSA_BITS_LOC = UINT32_C(0x007FFFFF);
    static constexpr std::uint32_t NEG_INFINITY = EXPONENT_BITS_LOC | SIGN_BIT_LOC;
    static constexpr std::uint32_t INFINITY = EXPONENT_BITS_LOC;
    static constexpr std::uint32_t NAN_MIN = EXPONENT_BITS_LOC + 1;
    static constexpr std::uint32_t NAN_MAX = EXPONENT_BITS_LOC | MANTISSA_BITS_LOC;
    static constexpr std::uint32_t BIAS = 127;
    

    Sink32() : mValue(0)
    {
    }

    Sink32(std::uint32_t value) : mValue(value)
    {
    }

    ~Sink32() = default;

    inline Sink32 operator-() const
    {
        return Sink32(mValue ^ SIGN_BIT_LOC);
    }

    Sink32 operator+(std::uint32_t addend);

    inline Sink32 operator-(std::uint32_t minuend)
    {
        // TODO i think this wrong :/
        return *this + (minuend ^ SIGN_BIT_LOC);
    }

    inline bool isNaN() const
    {
        return (mValue & EXPONENT_BITS_LOC == EXPONENT_BITS_LOC) &&
            (mValue & MANTISSA_BITS_LOC > 0);
    }

    inline bool isInfinity() const
    {
        return mValue ^ INFINITY == 0 || mValue ^ NEG_INFINITY == 0;
    }

    inline bool isNegative() const
    {
        return mValue & SIGN_BIT_LOC;
    }
    
private:
    std::uint32_t mValue;
};

#endif /* SINK_HPP */
