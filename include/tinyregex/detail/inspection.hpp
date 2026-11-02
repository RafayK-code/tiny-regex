#pragma once
#include "syntax.hpp"

namespace tinyregex::detail::inspect
{

template <typename... Types>
class TypeList {};

template <typename Node>
class ChildrenOf;

template <char C>
class ChildrenOf<Literal<C>>
{
public:
    using type = TypeList<>;
};

template <std::uint32_t Index>
class ChildrenOf<Backreference<Index>>
{
public:
    using type = TypeList<>;
};

template <>
class ChildrenOf<Wildcard>
{
public:
    using type = TypeList<>;
};

template <>
class ChildrenOf<StartAnchor>
{
public:
    using type = TypeList<>;
};

template <>
class ChildrenOf<EndAnchor>
{
public:
    using type = TypeList<>;
};

template <RegexExpression... Children>
class ChildrenOf<Sequence<Children...>>
{
public:
    using type = TypeList<Children...>;
};

template <RegexExpression Left, RegexExpression Right>
class ChildrenOf<Union<Left, Right>>
{
public:
    using type = TypeList<Left, Right>;
};

template <RegexExpression Child>
class ChildrenOf<RepeatZeroOrMore<Child>>
{
public:
    using type = TypeList<Child>;
};

template <RegexExpression Child>
class ChildrenOf<RepeatOneOrMore<Child>>
{
public:
    using type = TypeList<Child>;
};

template <RegexExpression Child>
class ChildrenOf<Optional<Child>>
{
public:
    using type = TypeList<Child>;
};

template <std::uint32_t Index, RegexExpression Child>
class ChildrenOf<CaptureGroup<Index, Child>>
{
public:
    using type = TypeList<Child>;
};

template <RegexExpression Child>
class ChildrenOf<NonCaptureGroup<Child>>
{
public:
    using type = TypeList<Child>;
};

template <typename Node, template <typename> typename Predicate>
class Check;

template <typename List, template <typename> typename Predicate>
class CheckChildren;

template <typename... Children, template <typename> typename Predicate>
class CheckChildren<TypeList<Children...>, Predicate>
{
public:
    static constexpr bool value = (Check<Children, Predicate>::value && ...);
};

template <typename Node, template <typename> typename Predicate>
class Check
{
public:
    static constexpr bool value = Predicate<Node>::value && CheckChildren<typename ChildrenOf<Node>::type, Predicate>::value;
};

}