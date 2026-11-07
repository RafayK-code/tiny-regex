#include <tinyregex/detail/parser.hpp>
#include <type_traits>

using namespace tinyregex::detail::parser;
using namespace tinyregex::detail::syntax;

namespace literal
{
    using Expected = Literal<'a'>;

    static_assert(std::is_same_v<Parse<"a">::type, Expected>);
}

namespace sequence
{
    using Expected = Sequence<Literal<'a'>, Literal<'b'>, Literal<'c'>>;

    static_assert(std::is_same_v<Parse<"abc">::type, Expected>);
}

namespace alternation
{
    using Expected = Union<Literal<'a'>, Literal<'b'>>;

    static_assert(std::is_same_v<Parse<"a|b">::type, Expected>);
}

namespace alternation_order
{
    using Expected = Union<Literal<'a'>, Union<Literal<'b'>, Literal<'c'>>>;

    static_assert(std::is_same_v<Parse<"a|b|c">::type, Expected>);
}

namespace concatenation_precedence
{
    using Expected = Union<
        Sequence<Literal<'a'>, Literal<'b'>>,
        Sequence<Literal<'c'>, Literal<'d'>>
    >;

    static_assert(std::is_same_v<Parse<"ab|cd">::type, Expected>);
}

namespace zero_or_more
{
    using Expected = RepeatZeroOrMore<Literal<'a'>>;

    static_assert(std::is_same_v<Parse<"a*">::type, Expected>);
}

namespace one_or_more
{
    using Expected = RepeatOneOrMore<Literal<'a'>>;

    static_assert(std::is_same_v<Parse<"a+">::type, Expected>);
}

namespace optional
{
    using Expected = Optional<Literal<'a'>>;

    static_assert(std::is_same_v<Parse<"a?">::type, Expected>);
}

namespace quantifier_precedence
{
    using Expected = Sequence<
        Literal<'a'>,
        RepeatZeroOrMore<Literal<'b'>>
    >;

    static_assert(std::is_same_v<Parse<"ab*">::type, Expected>);
}

namespace capture_group
{
    using Expected = CaptureGroup<1, Sequence<Literal<'a'>, Literal<'b'>>>;

    static_assert(std::is_same_v<Parse<"(ab)">::type, Expected>);
}

namespace noncapture_group
{
    using Expected = NonCaptureGroup<Sequence<Literal<'a'>, Literal<'b'>>>;

    static_assert(std::is_same_v<Parse<"(?:ab)">::type, Expected>);
}

namespace quantified_group
{
    using Expected = RepeatOneOrMore<
        CaptureGroup<1, Union<Literal<'a'>, Literal<'b'>>>
    >;

    static_assert(std::is_same_v<Parse<"(a|b)+">::type, Expected>);
}

namespace capture_numbering
{
    using Expected = Sequence<
        CaptureGroup<1, Literal<'a'>>,
        NonCaptureGroup<Literal<'b'>>,
        CaptureGroup<2, Literal<'c'>>
    >;

    static_assert(std::is_same_v<Parse<"(a)(?:b)(c)">::type, Expected>);
}

namespace wildcard
{
    using Expected = Wildcard;

    static_assert(std::is_same_v<Parse<".">::type, Expected>);
}

namespace anchors
{
    using Expected = Sequence<StartAnchor, Literal<'a'>, EndAnchor>;

    static_assert(std::is_same_v<Parse<"^a$">::type, Expected>);
}

namespace escaped_metacharacters
{
    using Expected = Sequence<
        Literal<'*'>,
        Literal<'+'>,
        Literal<'?'>,
        Literal<'|'>,
        Literal<'('>,
        Literal<')'>,
        Literal<'.'>,
        Literal<'^'>,
        Literal<'$'>,
        Literal<'\\'>
    >;

    static_assert(std::is_same_v<
        Parse<R"(\*\+\?\|\(\)\.\^\$\\)">::type,
        Expected
    >);
}

namespace quantified_escaped_anchor
{
    using Expected = RepeatZeroOrMore<Literal<'^'>>;

    static_assert(std::is_same_v<Parse<R"(\^*)">::type, Expected>);
}

namespace backreference
{
    using Expected = Sequence<
        CaptureGroup<1, Literal<'a'>>,
        Backreference<1>
    >;

    static_assert(std::is_same_v<Parse<R"((a)\1)">::type, Expected>);
}