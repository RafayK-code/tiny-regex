#pragma once
#include "syntax.hpp"

namespace tinyregex::detail::inspect
{

template <typename... Types>
class TypeList {};

template <typename Node>
class ChildrenOf;

template <char C>
class ChildrenOf<syntax::Literal<C>>
{
public:
    using type = TypeList<>;
};

template <std::uint32_t Index>
class ChildrenOf<syntax::Backreference<Index>>
{
public:
    using type = TypeList<>;
};

template <>
class ChildrenOf<syntax::Wildcard>
{
public:
    using type = TypeList<>;
};

template <>
class ChildrenOf<syntax::StartAnchor>
{
public:
    using type = TypeList<>;
};

template <>
class ChildrenOf<syntax::EndAnchor>
{
public:
    using type = TypeList<>;
};

template <typename... Children>
class ChildrenOf<syntax::Sequence<Children...>>
{
public:
    using type = TypeList<Children...>;
};

template <typename Left, typename Right>
class ChildrenOf<syntax::Union<Left, Right>>
{
public:
    using type = TypeList<Left, Right>;
};

template <typename Child>
class ChildrenOf<syntax::RepeatZeroOrMore<Child>>
{
public:
    using type = TypeList<Child>;
};

template <typename Child>
class ChildrenOf<syntax::RepeatOneOrMore<Child>>
{
public:
    using type = TypeList<Child>;
};

template <typename Child>
class ChildrenOf<syntax::Optional<Child>>
{
public:
    using type = TypeList<Child>;
};

template <std::uint32_t Index, typename Child>
class ChildrenOf<syntax::CaptureGroup<Index, Child>>
{
public:
    using type = TypeList<Child>;
};

template <typename Child>
class ChildrenOf<syntax::NonCaptureGroup<Child>>
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