/// @file inspection.hpp
/// @brief Compile-time inspection and validation of regex syntax trees.
///
/// Provides utilities for traversing a type-based abstract syntax tree (AST) and evaluating
/// predicates independently of the underlying structure of its syntax nodes.

#pragma once
#include "syntax.hpp"

namespace tinyregex::detail::inspect
{

/// @brief Represents an ordered list of types.
/// @tparam Types The types contained in the list.
template <typename... Types>
class TypeList {};


/// @brief Type trait for retrieving the immediate children of a syntax node.
/// @tparam Node The syntax node to inspect.
template <typename Node>
class ChildrenOf;


/// @brief Retrieves the children of a literal node.
/// @tparam C The literal character.
template <char C>
class ChildrenOf<syntax::Literal<C>>
{
public:
    using type = TypeList<>; ///< Empty child list.
};


/// @brief Retrieves the children of a backreference node.
/// @tparam Index The referenced capture-group index.
template <std::uint32_t Index>
class ChildrenOf<syntax::Backreference<Index>>
{
public:
    using type = TypeList<>; ///< Empty child list.
};


/// @brief Retrieves the children of a wildcard node.
template <>
class ChildrenOf<syntax::Wildcard>
{
public:
    using type = TypeList<>; ///< Empty child list.
};


/// @brief Retrieves the children of a start anchor node.
template <>
class ChildrenOf<syntax::StartAnchor>
{
public:
    using type = TypeList<>; ///< Empty child list.
};


/// @brief Retrieves the children of an end anchor node.
template <>
class ChildrenOf<syntax::EndAnchor>
{
public:
    using type = TypeList<>; ///< Empty child list.
};


/// @brief Retrieves the children of a sequence node.
/// @tparam Children The expressions contained in the sequence.
template <typename... Children>
class ChildrenOf<syntax::Sequence<Children...>>
{
public:
    using type = TypeList<Children...>; ///< Child nodes in sequence order.
};


/// @brief Retrieves the children of a union node.
/// @tparam Left The left-hand side expression.
/// @tparam Right The right-hand side expression.
template <typename Left, typename Right>
class ChildrenOf<syntax::Union<Left, Right>>
{
public:
    using type = TypeList<Left, Right>; ///< Child nodes in alternation order.
};


/// @brief Retrieves the child of a Kleene star node.
/// @tparam Child The expression being quantified.
template <typename Child>
class ChildrenOf<syntax::RepeatZeroOrMore<Child>>
{
public:
    using type = TypeList<Child>; ///< Child list containing the quantified expression.
};


/// @brief Retrieves the child of a Kleene plus node.
/// @tparam Child The expression being quantified.
template <typename Child>
class ChildrenOf<syntax::RepeatOneOrMore<Child>>
{
public:
    using type = TypeList<Child>; ///< Child list containing the quantified expression.
};


/// @brief Retrieves the child of an optional node.
/// @tparam Child The expression being quantified.
template <typename Child>
class ChildrenOf<syntax::Optional<Child>>
{
public:
    using type = TypeList<Child>; ///< Child list containing the quantified expression.
};


/// @brief Retrieves the child of a capturing group node.
/// @tparam Index The capture-group index.
/// @tparam Child The expression enclosed by the group.
template <std::uint32_t Index, typename Child>
class ChildrenOf<syntax::CaptureGroup<Index, Child>>
{
public:
    using type = TypeList<Child>; ///< Child list containing the grouped expression.
};


/// @brief Retrieves the child of a non-capturing group node.
/// @tparam Child The expression enclosed by the group.
template <typename Child>
class ChildrenOf<syntax::NonCaptureGroup<Child>>
{
public:
    using type = TypeList<Child>; ///< Child list containing the grouped expression.
};


template <typename Node, template <typename> typename Predicate>
class Check;


/// @brief Recursively checks whether every subtree in a type list satisfies a predicate.
/// @tparam List The list of syntax nodes to inspect.
/// @tparam Predicate The predicate to apply to each node.
template <typename List, template <typename> typename Predicate>
class CheckChildren;


/// @brief Expands a list of child nodes and recursively checks each subtree.
/// @tparam Children The syntax nodes to inspect.
/// @tparam Predicate The predicate to apply to each node.
template <typename... Children, template <typename> typename Predicate>
class CheckChildren<TypeList<Children...>, Predicate>
{
public:
    /// @brief Whether all child subtrees satisfy the predicate.
    /// An empty child list evaluates to true, terminating traversal at leaf nodes.
    static constexpr bool value = (Check<Children, Predicate>::value && ...);
};


/// @brief Recursively checks whether a syntax node and all of its descendants satisfy a predicate.
/// @tparam Node The root syntax node of the subtree to inspect.
/// @tparam Predicate The predicate to apply to each node.
template <typename Node, template <typename> typename Predicate>
class Check
{
public:
    /// @brief Whether the entire subtree satisfies the predicate.
    static constexpr bool value = Predicate<Node>::value && CheckChildren<typename ChildrenOf<Node>::type, Predicate>::value;
};

}