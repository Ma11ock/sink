/// Sink emulator unit tests.
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
#[path = "lib.rs"]
mod repr_test {
    use super::super::*;

    #[test]
    fn zero_literal() {
        assert_eq!(0, Sink::build_literal(0).value);
    }

    #[test]
    fn zero_u32() {
        assert_eq!(0f32.to_bits(), Sink::from_uint(0).value);
    }

    #[test]
    fn one_u32() {
        assert_eq!(1f32.to_bits(), Sink::from_uint(1).value);
    }

    #[test]
    fn two_u32() {
        assert_eq!(2f32.to_bits(), Sink::from_uint(2).value);
    }

    #[test]
    fn zero_s32() {
        assert_eq!(0f32.to_bits(), Sink::from_sint(0).value);
    }

    #[test]
    fn one_s32() {
        assert_eq!(1f32.to_bits(), Sink::from_sint(1).value);
    }

    #[test]
    fn two_s32() {
        assert_eq!(2f32.to_bits(), Sink::from_sint(2).value);
    }

    #[test]
    fn max_u32() {
        // TODO investigate as to why 16777216f32 does not work.
        assert_eq!(16777215f32.to_bits(), Sink::from_uint(0xFFFFFF).value);
    }

    #[test]
    fn neg_one_s32() {
        assert_eq!((-1f32).to_bits(), Sink::from_sint(-1).value);
    }

    #[test]
    fn neg_two_s32() {
        assert_eq!((-2f32).to_bits(), Sink::from_uint(2).value & SIGN_BIT_LOC);
    }

    #[test]
    fn min_s32() {
        // TODO investigate as to why 16777216f32 does not work.
        assert_eq!((-16777215f32).to_bits(), Sink::from_sint(-0xFFFFFF).value);
    }
}
