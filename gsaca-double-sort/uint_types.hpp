/*******************************************************************************
 * thrill/common/uint_types.hpp
 *
 * Class representing a 40-bit or 48-bit unsigned integer encoded in five or
 * six bytes.
 *
 * Part of Project Thrill - http://project-thrill.org
 *
 * Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the BSD-2 license printed below
 * 
 * 
 * MODIFIED FOR gsaca-double-sort
 * Copyright (C) 2020 Jonas Ellert
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#pragma once

#define uint64_binary_operator(x) \
template<typename T> \
auto operator x(const T &b) const { \
  static_assert(std::is_convertible_v<T, UIntPair>); \
  static_assert(!std::is_same_v<T, UIntPair>); \
  return *this x (UIntPair) b; \
}

#include <cassert>
#include <limits>
#include <cstdint>
#include "macros.hpp"

namespace gsaca_lyndon {

using int128_t = __int128;
using uint128_t = unsigned __int128;

namespace uint_internal {

/*!
 * Construct an 40-bit or 48-bit unsigned integer stored in five or six bytes.
 *
 * The purpose of this class is to provide integers with smaller data storage
 * footprints when more than 32-bit, but less than 64-bit indexes are
 * needed. This is commonly the case for storing file offsets and indexes. Here
 * smaller types currently suffice for files < 1 TiB or < 16 TiB.
 *
 * The class combines a 32-bit integer with a HighType (either 8-bit or 16-bit)
 * to get a larger type. Only unsigned values are supported, which fits the
 * general application of file offsets.
 *
 * Calculation in UIntPair are generally done by transforming everything to
 * 64-bit data type, so that 64-bit register arithmetic can be used. The
 * exception here is \b increment and \b decrement, which is done directly on
 * the lower/higher part. Not all arithmetic operations are supported, patches
 * welcome if you really need the operations.
 */

template<typename High_>
class UIntPair {
public:
  static_assert(std::is_unsigned<High_>::value);

  template<typename H>
  friend UIntPair<H> add_flag(UIntPair<H>);

  //! lower part type, always 32-bit
  using Low = uint32_t;
  //! higher part type, currently either 8-bit or 16-bit
  using High = High_;

  //TODO: Sinnvoll?
  //! member containing lower significant integer value
  Low low_;
  //! member containing higher significant integer value
  High high_;

private:
  //! return highest value storable in lower part, also used as a mask.
  static unsigned low_max() {
    return std::numeric_limits<Low>::max();
  }

  //! number of bits in the lower integer part, used a bit shift value.
  static constexpr size_t low_bits = 8 * sizeof(Low);

  //! return highest value storable in higher part, also used as a mask.
  static unsigned high_max() {
    return std::numeric_limits<High>::max();
  }

  //! number of bits in the higher integer part, used a bit shift value.
  static constexpr size_t high_bits = 8 * sizeof(High);

public:
  //! number of binary digits (bits) in UIntPair
  static constexpr size_t digits = low_bits + high_bits;

  //! number of bytes in UIntPair
  static constexpr size_t bytes = sizeof(Low) + sizeof(High);

  // compile-time assertions about size of Low
  static_assert(8 * sizeof(Low) == 32, "sizeof Low is 32-bit");
  static_assert(digits / 8 == bytes, "digit and bytes ratio is wrong");

  //! empty constructor, does not even initialize to zero!
  UIntPair() = default;

  //! construct unit pair from lower and higher parts.
  UIntPair(const Low &l, const High &h)
      : low_(l), high_(h) {}

  //! copy constructor
  UIntPair(const UIntPair &) = default;

  //! move constructor
  UIntPair(UIntPair &&) = default;

  //! const from a simple 32-bit unsigned integer
  UIntPair(const uint32_t &a) // NOLINT
      : low_(a), high_(0) {}

  //! const from a simple 32-bit signed integer
  UIntPair(const int32_t &a)  // NOLINT
      : low_(a), high_(0) {
    if (a >= 0)
      low_ = a;
    else
      low_ = a, high_ = (High) high_max();
  }

  //! construct from an 64-bit unsigned integer
  UIntPair(const unsigned long long &a) // NOLINT
      : low_((Low) (a & low_max())),
        high_((High) ((a >> low_bits) & high_max())) {
    // check for overflow
    assert((a >> (low_bits + high_bits)) == 0);
  }

  //! construct from an 64-bit signed integer
  UIntPair(const uint64_t &a) // NOLINT
      : UIntPair(static_cast<unsigned long long>(a)) {}

  //! construct from an 64-bit signed integer
  UIntPair(const int64_t &a)       // NOLINT
      : UIntPair(static_cast<unsigned long long>(a)) {}

  //! copy assignment operator
  UIntPair &operator=(const UIntPair &) = default;

  //! move assignment operator
  UIntPair &operator=(UIntPair &&) = default;

  //! return the number as an uint64 (unsigned long long)
  uint64_t ull() const {
    return ((uint64_t) high_) << low_bits | (uint64_t) low_;
  }

  //! implicit cast to an unsigned long long
  operator uint64_t() const {
    return ull();
  }

  //! return the number as a uint64_t
  uint64_t u64() const {
    return ((uint64_t) high_) << low_bits | (uint64_t) low_;
  }

  //! prefix increment operator (directly manipulates the integer parts)
  UIntPair &operator++() {
    if (gsaca_unlikely(low_ == low_max()))
      ++high_, low_ = 0;
    else
      ++low_;
    return *this;
  }

  //! prefix decrement operator (directly manipulates the integer parts)
  UIntPair &operator--() {
    if (gsaca_unlikely(low_ == 0))
      --high_, low_ = (Low) low_max();
    else
      --low_;
    return *this;
  }

  //! postfix increment operator (directly manipulates the integer parts)
  UIntPair operator++(int) {
    UIntPair result = *this;
    if (gsaca_unlikely(low_ == low_max()))
      ++high_, low_ = 0;
    else
      ++low_;
    return result;
  }

  //! postfix decrement operator (directly manipulates the integer parts)
  UIntPair operator--(int) {
    UIntPair result = *this;
    if (gsaca_unlikely(low_ == 0))
      --high_, low_ = (Low) low_max();
    else
      --low_;
    return result;
  }

  //! addition operator (uses 64-bit arithmetic)
  UIntPair &operator+=(const UIntPair &b) {
    uint64_t add = low_ + uint64_t(b.low_);
    low_ = (Low) (add & low_max());
    high_ = (High) (high_ + b.high_ + ((add >> low_bits) & high_max()));
    return *this;
  }

  //! addition operator (uses 64-bit arithmetic)
  UIntPair operator+(const UIntPair &b) const {
    uint64_t add = low_ + uint64_t(b.low_);
    return UIntPair(
        (Low) (add & low_max()),
        (High) (high_ + b.high_ + ((add >> low_bits) & high_max())));
  }

  //! subtraction operator (uses 64-bit arithmetic)
  UIntPair &operator-=(const UIntPair &b) {
    uint64_t sub = low_ - uint64_t(b.low_);
    low_ = (Low) (sub & low_max());
    high_ = (High) (high_ - b.high_ + ((sub >> low_bits) & high_max()));
    return *this;
  }

  //! subtraction operator (uses 64-bit arithmetic)
  UIntPair operator-(const UIntPair &b) const {
    uint64_t sub = low_ - uint64_t(b.low_);
    return UIntPair(
        (Low) (sub & low_max()),
        (High) (high_ - b.high_ + ((sub >> low_bits) & high_max())));
  }

  //! equality checking operator
  bool operator==(const UIntPair &b) const {
    return (low_ == b.low_) && (high_ == b.high_);
  }

  //! inequality checking operator
  bool operator!=(const UIntPair &b) const {
    return (low_ != b.low_) || (high_ != b.high_);
  }

  //! less-than comparison operator
  bool operator<(const UIntPair &b) const {
    return (high_ < b.high_) || (high_ == b.high_ && low_ < b.low_);
  }

  //! less-or-equal comparison operator
  bool operator<=(const UIntPair &b) const {
    return (high_ < b.high_) || (high_ == b.high_ && low_ <= b.low_);
  }

  //! greater comparison operator
  bool operator>(const UIntPair &b) const {
    return (high_ > b.high_) || (high_ == b.high_ && low_ > b.low_);
  }

  //! greater-or-equal comparison operator
  bool operator>=(const UIntPair &b) const {
    return (high_ > b.high_) || (high_ == b.high_ && low_ >= b.low_);
  }

  UIntPair &operator>>=(uint8_t const &shift) {
    *this = *this >> shift;
    return *this;
  }

  uint64_binary_operator(==)

  uint64_binary_operator(!=)

  uint64_binary_operator(<)

  uint64_binary_operator(<=)

  uint64_binary_operator(>)

  uint64_binary_operator(>=)

  uint64_binary_operator(-)

  uint64_binary_operator(-=)

  uint64_binary_operator(+)

  uint64_binary_operator(+=)

  //! make a UIntPair outputtable via iostreams, using unsigned long long.
  friend std::ostream &operator<<(std::ostream &os, const UIntPair &a) {
    return os << a.ull();
  }

  //! return an UIntPair instance containing the smallest value possible
  static UIntPair min() {
    return UIntPair(std::numeric_limits<Low>::min(),
                    std::numeric_limits<High>::min());
  }

  //! return an UIntPair instance containing the largest value possible
  static UIntPair max() {
    return UIntPair(std::numeric_limits<Low>::max(),
                    std::numeric_limits<High>::max());
  }
} __attribute((packed));

} // namespace internal


struct flag_type_bitset {
  template<typename T>
  inline static T add_flag(T t) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
    constexpr T mask = ((T) 1) << (sizeof(T) * 8 - 1);
    return t | mask;
  }

  template<typename H>
  inline static uint_internal::UIntPair<H>
  add_flag(uint_internal::UIntPair<H> t) {
    constexpr H mask = ((H) 1) << (sizeof(H) * 8 - 1);
    t.high_ |= mask;
    return t;
  }

  template<typename T>
  inline static T conditional_add_flag(bool b, T t) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
    constexpr size_t shift = sizeof(T) * 8 - 1;
    return t | (((T) b) << shift);
  }

  template<typename H>
  inline static uint_internal::UIntPair<H>
  conditional_add_flag(bool b, uint_internal::UIntPair<H> t) {
    constexpr size_t shift = sizeof(H) * 8 - 1;
    t.high_ |= (((H) b) << shift);
    return t;
  }

  template<typename T>
  inline static T remove_flag(T t) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
    return (t << 1) >> 1;
  }

  template<typename H>
  inline static uint_internal::UIntPair<H>
  remove_flag(uint_internal::UIntPair<H> t) {
    (t.high_ <<= 1) >>= 1;
    return t;
  }

  template<typename T>
  inline static bool is_flagged(T t) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
    using signed_T = typename std::make_signed<T>::type;
    return ((signed_T) t) < 0;
  }

  template<typename H>
  inline static bool is_flagged(uint_internal::UIntPair<H> t) {
    using signed_H = typename std::make_signed<H>::type;
    return ((signed_H) t.high_) < 0;
  }
};

struct flag_type_none {
  template<typename T>
  inline static T const &add_flag(T const &t) {
    return t;
  }

  template<typename T>
  inline static T const &conditional_add_flag(bool, T const &t) {
    return t;
  }

  template<typename T>
  inline static T const &remove_flag(T const &t) {
    return t;
  }

  template<typename T>
  inline static bool is_flagged(T const &) {
    return false;
  }
};

template<bool use_flags>
using flag_type = typename std::conditional<
    use_flags, flag_type_bitset, flag_type_none>::type;


using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

//! Construct a 40-bit unsigned integer stored in five bytes.
using uint40_t = uint_internal::UIntPair<uint8_t>;

//! Construct a 48-bit unsigned integer stored in six bytes.
using uint48_t = uint_internal::UIntPair<uint16_t>;

// compile-time assertions about size of our data structure, this tests packing
// of structures by the compiler
static_assert(sizeof(uint40_t) == 5, "sizeof uint40 is wrong");
static_assert(sizeof(uint48_t) == 6, "sizeof uint48 is wrong");

using auto_buffer_type = nullptr_t;
template<typename buffer_type, typename index_type>
using get_buffer_type =
typename std::conditional<
    std::is_same_v<buffer_type, auto_buffer_type>,
    typename std::conditional<
        (sizeof(index_type) > 5),
        uint40_t,
        index_type>::type,
    buffer_type>::type;

template<typename buffer_type, typename index_type, typename used_buffer_type>
constexpr bool check_buffer_type =
    std::is_same_v<used_buffer_type, get_buffer_type<buffer_type, index_type>>;

template<typename uint_type, typename... uint_types>
constexpr size_t sizeof_min() {
  if constexpr (sizeof...(uint_types))
    return std::min((size_t) sizeof(uint_type), sizeof_min<uint_types...>());
  else return sizeof(uint_type);
}

template<typename uint_type, typename... uint_types>
using get_count_type = typename std::conditional<
    ((sizeof_min<uint_type, uint_types...>()) > 4), uint64_t, uint32_t>::type;


} // namespace gsaca_lyndon

namespace std {

template<typename HighType>
struct is_unsigned<gsaca_lyndon::uint_internal::UIntPair<HighType> >
    : std::true_type {
};

//! template class providing some numeric_limits fields for UIntPair types.
template<typename HighType>
class numeric_limits<gsaca_lyndon::uint_internal::UIntPair<HighType> > {
public:
  using UIntPair = gsaca_lyndon::uint_internal::UIntPair<HighType>;

  //! yes we have information about UIntPair
  static const bool is_specialized = true;

  //! return an UIntPair instance containing the smallest value possible
  static UIntPair min() { return UIntPair::min(); }

  //! return an UIntPair instance containing the largest value possible
  static UIntPair max() { return UIntPair::max(); }

  //! return an UIntPair instance containing the smallest value possible
  static UIntPair lowest() { return min(); }

  //! unit_pair types are unsigned
  static const bool is_signed = false;

  //! UIntPair types are integers
  static const bool is_integer = true;

  //! unit_pair types contain exact integers
  static const bool is_exact = true;

  //! unit_pair radix is binary
  static const int radix = 2;

  //! number of binary digits (bits) in UIntPair
  static const int digits = UIntPair::digits;

  //! epsilon is zero
  static const UIntPair epsilon() { return UIntPair(0, 0); }

  //! rounding error is zero
  static const UIntPair round_error() { return UIntPair(0, 0); }

  //! no exponent
  static const int min_exponent = 0;

  //! no exponent
  static const int min_exponent10 = 0;

  //! no exponent
  static const int max_exponent = 0;

  //! no exponent
  static const int max_exponent10 = 0;

  //! no infinity
  static const bool has_infinity = false;
};

} // namespace std

#undef uint64_binary_operator

/******************************************************************************/
