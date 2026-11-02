#include <tinyregex/detail/parser.hpp>
#include <type_traits>

using namespace tinyregex::detail;

namespace literal
{
    using Expected = Sequence<Literal<'a'>>;

    static_assert(std::is_same_v<Parse<"a">::type, Expected>);
}

namespace sequence
{
    using Expected = Sequence<Literal<'a'>, Literal<'b'>, Literal<'c'>>;

    static_assert(std::is_same_v<Parse<"abc">::type, Expected>);
}

namespace alternation
{
    using Expected = Union<
        Sequence<Literal<'a'>>,
        Sequence<Literal<'b'>>
    >;

    static_assert(std::is_same_v<Parse<"a|b">::type, Expected>);
}

namespace alternation_order
{
    using Expected = Union<
        Sequence<Literal<'a'>>,
        Union<Sequence<Literal<'b'>>, Sequence<Literal<'c'>>>
    >;

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
    using Expected = Sequence<RepeatZeroOrMore<Literal<'a'>>>;

    static_assert(std::is_same_v<Parse<"a*">::type, Expected>);
}

namespace one_or_more
{
    using Expected = Sequence<RepeatOneOrMore<Literal<'a'>>>;

    static_assert(std::is_same_v<Parse<"a+">::type, Expected>);
}

namespace optional
{
    using Expected = Sequence<Optional<Literal<'a'>>>;

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
    using Expected = Sequence<
        CaptureGroup<1, Sequence<Literal<'a'>, Literal<'b'>>>
    >;

    static_assert(std::is_same_v<Parse<"(ab)">::type, Expected>);
}

namespace noncapture_group
{
    using Expected = Sequence<
        NonCaptureGroup<Sequence<Literal<'a'>, Literal<'b'>>>
    >;

    static_assert(std::is_same_v<Parse<"(?:ab)">::type, Expected>);
}

namespace quantified_group
{
    using Expected = Sequence<
        RepeatOneOrMore<
            CaptureGroup<
                1,
                Union<Sequence<Literal<'a'>>, Sequence<Literal<'b'>>>
            >
        >
    >;

    static_assert(std::is_same_v<Parse<"(a|b)+">::type, Expected>);
}

namespace capture_numbering
{
    using Expected = Sequence<
        CaptureGroup<1, Sequence<Literal<'a'>>>,
        NonCaptureGroup<Sequence<Literal<'b'>>>,
        CaptureGroup<2, Sequence<Literal<'c'>>>
    >;

    static_assert(std::is_same_v<Parse<"(a)(?:b)(c)">::type, Expected>);
}

namespace wildcard
{
    using Expected = Sequence<Wildcard>;

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
    using Expected = Sequence<RepeatZeroOrMore<Literal<'^'>>>;

    static_assert(std::is_same_v<Parse<R"(\^*)">::type, Expected>);
}

namespace backreference
{
    using Expected = Sequence<
        CaptureGroup<1, Sequence<Literal<'a'>>>,
        Backreference<1>
    >;

    static_assert(std::is_same_v<Parse<R"((a)\1)">::type, Expected>);
}