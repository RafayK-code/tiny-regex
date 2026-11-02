#pragma once
#include <concepts>
#include <cstdint>

namespace tinyregex::detail
{

class Expression {};

template <typename T>
concept RegexExpression = std::derived_from<T, Expression>;

template <char C>
class Literal : public Expression
{
};

template <RegexExpression... Children>
class Sequence : public Expression
{
};

template <RegexExpression Left, RegexExpression Right>
class Union : public Expression
{
};

template <RegexExpression Child>
class RepeatZeroOrMore : public Expression
{
};

template <RegexExpression Child>
class RepeatOneOrMore : public Expression
{
};

template <RegexExpression Child>
class Optional : public Expression
{
};

template <std::uint32_t Index, RegexExpression Child> 
class CaptureGroup : public Expression
{
};

template <RegexExpression Child>
class NonCaptureGroup : public Expression
{
};

template <std::uint32_t Index>
class Backreference : public Expression
{
};

class Wildcard : public Expression
{
};

class StartAnchor : public Expression
{
};

class EndAnchor : public Expression
{
};

}