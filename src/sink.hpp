#ifndef SINK_HPP
#define SINK_HPP

#include <cstdint>
#include <string>

class Sink32
{
public:
    // The number of sign bits.
    static constexpr std::uint32_t SIGN_BITS = 1;
    // The number of exponent bits.
    static constexpr std::uint32_t EXPONENT_BITS = 8;
    // The number of mantissa bits.
    static constexpr std::uint32_t MANTISSA_BITS = 23;
    // Sign bit set to 1, all other bits 0; location of sign bit.
    // Basically -0.
    static constexpr std::uint32_t SIGN_BIT_LOC = UINT32_C(0x80000000);
    // All exponent bits set to 1, all others set to 0.
    static constexpr std::uint32_t EXPONENT_BITS_LOC = UINT32_C(0x7F800000);
    // All mantissa bits set to 1, all others to 0.
    static constexpr std::uint32_t MANTISSA_BITS_LOC = UINT32_C(0x007FFFFF);
    // Negative infinity.
    static constexpr std::uint32_t NEG_INFINITY = EXPONENT_BITS_LOC | SIGN_BIT_LOC;
    // Infinity.
    static constexpr std::uint32_t INFINITY = EXPONENT_BITS_LOC;
    // Minimum (if converted to a uint) NAN value.
    static constexpr std::uint32_t NAN_MIN = EXPONENT_BITS_LOC + 1;
    // Maximum (if converted to a uint) NAN value.
    static constexpr std::uint32_t NAN_MAX = EXPONENT_BITS_LOC | MANTISSA_BITS_LOC;
    // The exponent bias.
    static constexpr std::int32_t BIAS = 127;
    // The exponent bias for subnormal numbers.
    static constexpr std::int32_t SUB_BIAS = 126;
    // The location of the mantissa's implicit leading one.
    static constexpr std::uint32_t MANTISSA_LEADING_ONE = UINT32_C(0x800000);

    // Default constructor. Set to 0.
    Sink32() : mValue(0)
    {
    }

    // Set the Sink to an integer value.
    Sink32(std::uint32_t value);

    // Set the Sink to an integer value. 
    Sink32(std::int32_t value);

    // Set the Sink to a IEEE 754 float value. If the system is not
    // IEEE compliant, set to NAN.
    Sink32(float value);

    // Set the Sink to an integer value and calculate the error.
    Sink32(std::uint32_t value, std::int32_t error)
        : Sink32(value)
    {
        error = static_cast<std::int32_t>(*this) - value;
    }

    // Set the sink to an integer value and calculate the error.
    Sink32(std::int32_t value, std::int32_t &error)
        : Sink32(value)
    {
        error = static_cast<std::int32_t>(*this) - value; 
    }

    // Create a Sink from a uint32_t and copy its bits literally.
    static inline Sink32 CreateLiteral(std::uint32_t value)
    {
        Sink32 n;
        n.mValue = value;
        return n;
    }

    // Destructor.
    ~Sink32() = default;

    // Convert to a signed integer.
    explicit operator std::int32_t() const;

    // Convert to a signed integer.
    inline std::int32_t toInt() const
    {
        return static_cast<std::int32_t>(*this);
    }

    // Convert to an IEEE 754 float. If the system is not IEEE 754
    // return IEEE NAN.
    explicit operator float() const;

    // Convert to an IEEE 754 float. If the system is not IEEE 754
    // return IEEE NAN.
    inline float getIEEEFloat() const
    {
        return static_cast<float>(*this);
    }

    // Negate sink.
    inline Sink32 operator-() const
    {
        return CreateLiteral(mValue ^ SIGN_BIT_LOC);
    }

    // Add two sinks.
    Sink32 operator+(Sink32 addend) const;

    // Subtract two sinks.
    Sink32 operator-(Sink32 minuend) const;

    // Multiply two sinks.
    Sink32 operator*(Sink32 multiplier) const;

    // Divide two sinks.
    Sink32 operator/(Sink32 divisor) const;

    // Check equality of two floats.
    inline bool operator==(Sink32 other) const
    {
        // NaNs always evaluate to false when compared.
        if(other.isNaN() || isNaN())
            return false;
        return other.mValue == mValue;
    }

    // Check if sink is zero (positive or negative).
    inline bool isZero() const
    {
        return (~SIGN_BIT_LOC & mValue) == 0;
    }

    // Check if sink is NaN.
    inline bool isNaN() const
    {
        return (mValue & EXPONENT_BITS_LOC == EXPONENT_BITS_LOC) &&
            (mValue & MANTISSA_BITS_LOC > 0);
    }
    
    // Check if sink is infinity.
    inline bool isInfinity() const
    {
        return mValue == INFINITY || mValue == NEG_INFINITY;
    }

    // Check if the sink is negative.
    inline bool isNegative() const
    {
        return mValue & SIGN_BIT_LOC;
    }

    // Check if the sink is subnormnal.
    inline bool isSubNormal() const
    {
        return (mValue & EXPONENT_BITS_LOC) == 0;
    }

    // Check if the sink is normal.
    inline bool isNormal() const
    {
        std::uint32_t exp = (mValue & EXPONENT_BITS_LOC) >> MANTISSA_BITS;
        return
            (exp != 0) &&
            (exp != 0xFF);
    }

    // Get the bits/memory of this Sink.
    inline std::uint32_t getValue() const
    {
        return mValue;
    }

    // Get the mathematical mantissa of the float.
    inline std::uint32_t getMantissa() const
    {
        return mValue & MANTISSA_BITS_LOC |
            (static_cast<std::uint32_t>(isNormal()) << MANTISSA_BITS);
    }

    // TODO square root, string conversion,

    std::string toString(std::string::size_type precision = 8) const;

    // Get the mathematical exponent. If the Sink is NaN or
    // infinity return the smallest 32 bit integer (INT_MIN).
    std::int32_t frexp() const;
    
private:
    // 4 byte value that makes up a sink.
    std::uint32_t mValue;
};

#endif /* SINK_HPP */
