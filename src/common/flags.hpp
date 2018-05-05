#pragma once

#ifndef _common_flags_hpp
#define _common_flags_hpp

#include <type_traits>

namespace prologcoin { namespace common {

template<typename Enum, bool IsEnum = std::is_enum<Enum>::value>
class flags;

template<typename Enum> class flags<Enum, true> {
public:
    constexpr const static int NUM_BITS = std::numeric_limits<typename std::underlying_type<Enum>::type>::digits;

    constexpr flags() = default;
    inline constexpr flags(Enum value) : bits(1 << static_cast<std::size_t>(value)) {}
    inline constexpr flags(const flags& other) : bits(other.bits) {}

    inline void operator |= (const flags<Enum, true> &other) { bits |= other.bits; }
    inline void operator &= (const flags<Enum, true> &other) { bits &= other.bits; }

    inline flags operator|(Enum value) const { flags result = *this; result.bits.set(static_cast<std::size_t>(value)); return result; }
    inline flags operator~() const { flags result = *this; result.bits.flip(); return result; }

    inline flags& operator|=(Enum value) { bits.set(static_cast<std::size_t>(value)); return *this; }

    inline constexpr bool any() const { return bits.any(); }
    inline constexpr bool all() const { return bits.all(); }
    inline constexpr bool none() const { return bits.none(); }
    inline constexpr operator bool() const { return any(); }

    inline constexpr bool test(Enum value) const { return bits.test(static_cast<std::size_t>(value)); }
    inline void set(Enum value) { bits.set(static_cast<std::size_t>(value)); }
    inline void clear(Enum value) { bits.reset(static_cast<std::size_t>(value)); }

public:
    std::bitset<NUM_BITS> bits;
};

template<typename Enum>
constexpr typename std::enable_if<std::is_enum<Enum>::value, flags<Enum>>::type operator|(Enum left, Enum right)
{
    return flags<Enum>(left) | right;
}
template<typename Enum>
constexpr typename std::enable_if<std::is_enum<Enum>::value, flags<Enum>>::type operator&(Enum left, Enum right)
{
    return flags<Enum>(left) & right;
}
template<typename Enum>

constexpr typename std::enable_if<std::is_enum<Enum>::value, flags<Enum>>::type operator^(Enum left, Enum right)
{
    return flags<Enum>(left) ^ right;
}

}}

#endif
