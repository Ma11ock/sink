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
}

impl PartialEq for Sink {
    fn eq(&self, other: &Rhs) -> bool {
        !(self.is_nan() || other.is_nan()) && (self.value == other.value)
    }

    fn ne(&self, other: &Rhs) -> bool {
        !(self == other)
    }
}
