#ifndef SINK_HP2
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
    static constexpr std::uint32_t LEADING_ONE = UINT32_C(0x800000);

    Sink32() : mValue(0)
    {
    }

    Sink32(std::uint32_t value);

    Sink32(std::int32_t value);

    Sink32(float value);

    static inline Sink32 CreateLiteral(std::uint32_t value)
    {
        Sink32 n;
        n.mValue = value;
        return n;
    }

    ~Sink32() = default;

    inline explicit operator std::int32_t() const
    {
        std::uint32_t sign = mValue & SIGN_BIT_LOC;
        std::uint32_t exponent = ((mValue & EXPONENT_BITS_LOC)
                                  >> MANTISSA_BITS) - BIAS;
        std::uint32_t mantissa = (mValue & MANTISSA_BITS_LOC)
            | LEADING_ONE;
        return mantissa >> (MANTISSA_BITS - exponent);
    }

    inline std::int32_t toInt() const
    {
        return static_cast<std::int32_t>(*this);
    }

    inline explicit operator float() const
    {
        return *reinterpret_cast<const float*>(&mValue);
    }

    inline Sink32 operator-() const
    {
        return CreateLiteral(mValue ^ SIGN_BIT_LOC);
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

    inline std::uint32_t getValue() const
    {
        return mValue;
    }
    
private:
    std::uint32_t mValue;
};

#endif /* SINK_HPP */
