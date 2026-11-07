#pragma once
#include <cstdint>

namespace tinyregex::detail::syntax
{

template <char C>
class Literal {};

template <std::uint32_t Index>
class Backreference {};

class Wildcard {};

class StartAnchor {};

class EndAnchor {};

template <typename... Children>
class Sequence {};

template <typename Left, typename Right>
class Union {};

template <typename Child>
class RepeatZeroOrMore {};

template <typename Child>
class RepeatOneOrMore {};

template <typename Child>
class Optional {};

template <std::uint32_t Index, typename Child> 
class CaptureGroup {};

template <typename Child>
class NonCaptureGroup {};

}