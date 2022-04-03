/// Sink, an IEEE 754 software 32bit floating point emulator.
/// (C) Ryan Jeffrey <ryan@ryanmj.xyz>, 2022
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
#[cfg(test)]
mod lib_test;

#[allow(arithmetic_overflow)]

/// The number of sign bits.
const SIGN_BITS: u32 = 1;
/// The number of exponent bits.
const EXPONENT_BITS: u32 = 8;
/// The number of mantissa bits.
const MANTISSA_BITS: u32 = 23;
/// Sign bit set to 1, all other bits 0; location of sign bit.
/// Basically -0.
const SIGN_BIT_LOC: u32 = 0x80000000;
/// All exponent bits set to 1, all others set to 0.
const EXPONENT_BITS_LOC: u32 = 0x7F800000;
/// All mantissa bits set to 1, all others to 0.
const MANTISSA_BITS_LOC: u32 = 0x007FFFFF;
/// Negative infinity.
const NEG_INFINITY: u32 = EXPONENT_BITS_LOC | SIGN_BIT_LOC;
/// Infinity.
const INFINITY: u32 = EXPONENT_BITS_LOC;
/// Minimum (if converted to a uint) NAN value.
const NAN_MIN: u32 = EXPONENT_BITS_LOC + 1;
/// Maximum (if converted to a uint) NAN value.
const NAN_MAX: u32 = EXPONENT_BITS_LOC | MANTISSA_BITS_LOC;
/// The exponent bias.
const BIAS: i32 = 127;
/// The exponent bias for subnormal numbers.
const SUB_BIAS: i32 = 126;
/// The location of the mantissa's implicit leading one.
const MANTISSA_LEADING_ONE: u32 = 0x800000;

fn twos_comp(i: u32) -> u32 {
    !i + 1
}

struct Sink {
    value: u32,
}

impl Sink {
    fn is_nan(&self) -> bool {
        (self.value & EXPONENT_BITS_LOC == EXPONENT_BITS_LOC)
            && (self.value & MANTISSA_BITS_LOC > 0)
    }

    fn is_zero(&self) -> bool {
        (!SIGN_BIT_LOC & self.value) == 0
    }

    pub fn build_literal(m: u32) -> Self {
        Self { value: m }
    }

    pub fn build(sign: u32, exp: i32, mantissa: u32) -> Self {
        Self {
            value: (sign & SIGN_BIT_LOC)
                | ((((exp + (if exp == -SUB_BIAS { SUB_BIAS } else { BIAS })) as u32)
                    << MANTISSA_BITS)
                    & EXPONENT_BITS_LOC)
                | (mantissa & MANTISSA_BITS_LOC),
        }
    }

    pub fn create_nan() -> Self {
        Self { value: NAN_MIN }
    }

    pub fn new() -> Self {
        Self { value: 0 }
    }

    pub fn build_float(f: f32) -> Self {
        Self::build_literal(f.to_bits())
    }

    pub fn from_uint(v: u32) -> Self {
        if v == 0 {
            Self::new()
        } else if v > 0xFFFFFF * 2 {
            // Out of range of a 32 bit float. Return NaN.
            Self::create_nan()
        } else {
            let mut value = v;
            let mut shifts: u32 = 0;
            while (value & MANTISSA_LEADING_ONE) == 0 && shifts < MANTISSA_BITS {
                shifts += 1;
                value <<= 1;
            }

            let exponent: u32 = BIAS as u32 + MANTISSA_BITS - shifts;
            Self::build_literal((exponent << MANTISSA_BITS) | (value & MANTISSA_BITS_LOC))
        }
    }

    pub fn from_sint(v: i32) -> Self {
        if v == 0 {
            Self::new()
        } else if (v > 0xFFFFFF * 2) || (v < -(0xFFFFFF * 2)) {
            // Out of range of a 32 bit float. Return NaN.
            Self::create_nan()
        } else {
            let mut value = v as u32;
            let mut shifts: u32 = 0;
            while (value & MANTISSA_LEADING_ONE) == 0 && shifts < MANTISSA_BITS {
                shifts += 1;
                value <<= 1;
            }

            let exponent: u32 = BIAS as u32 + MANTISSA_BITS - shifts;
            Self::build_literal(
                (if v < 0 { SIGN_BIT_LOC } else { 0 })
                    | (exponent << MANTISSA_BITS)
                    | (value & MANTISSA_BITS_LOC),
            )
        }
    }
}

impl PartialEq for Sink {
    fn eq(&self, other: &Self) -> bool {
        !(self.is_nan() || other.is_nan()) && (self.value == other.value)
    }

    fn ne(&self, other: &Self) -> bool {
        !(self == other)
    }
}
