#include <tinyregex/detail/parser.hpp>
#include <type_traits>

using namespace tinyregex::detail::parser;
using namespace tinyregex::detail::syntax;

namespace nested_repetition
{
    using Expected = Sequence<
        StartAnchor,
        NonCaptureGroup<
            Union<
                Sequence<Literal<'a'>, Literal<'b'>>,
                RepeatOneOrMore<Literal<'c'>>
            >
        >,
        RepeatZeroOrMore<
            CaptureGroup<
                1,
                Sequence<
                    Literal<'d'>,
                    Optional<
                        CaptureGroup<
                            2,
                            Union<Literal<'e'>, Literal<'f'>>
                        >
                    >
                >
            >
        >,
        Backreference<1>,
        EndAnchor
    >;

    static_assert(std::is_same_v<
        Parse<R"(^(?:ab|c+)(d(e|f)?)*\1$)">::type,
        Expected
    >);
}

namespace nested_capture_numbering
{
    using Expected = Sequence<
        CaptureGroup<
            1,
            Union<
                Literal<'a'>,
                CaptureGroup<
                    2,
                    Sequence<
                        Literal<'b'>,
                        CaptureGroup<3, Literal<'c'>>
                    >
                >
            >
        >,
        Backreference<3>,
        Backreference<2>,
        Backreference<1>
    >;

    static_assert(std::is_same_v<
        Parse<R"((a|(b(c)))\3\2\1)">::type,
        Expected
    >);
}

namespace captures_across_alternatives
{
    using Expected = Sequence<
        NonCaptureGroup<
            Union<
                Sequence<
                    Literal<'a'>,
                    CaptureGroup<1, Literal<'b'>>
                >,
                Sequence<
                    Literal<'c'>,
                    CaptureGroup<2, Literal<'d'>>
                >
            >
        >,
        CaptureGroup<3, Literal<'e'>>,
        Backreference<1>,
        Backreference<2>,
        Backreference<3>
    >;

    static_assert(std::is_same_v<
        Parse<R"((?:a(b)|c(d))(e)\1\2\3)">::type,
        Expected
    >);
}

namespace escaped_metacharacters
{
    using Expected = Sequence<
        StartAnchor,
        Literal<'('>,
        CaptureGroup<
            1,
            Sequence<Literal<'a'>, Literal<'|'>, Literal<'b'>>
        >,
        Literal<')'>,
        Backreference<1>,
        Literal<'.'>,
        Literal<'*'>,
        Literal<'+'>,
        Literal<'?'>,
        Literal<'\\'>,
        EndAnchor
    >;

    static_assert(std::is_same_v<
        Parse<R"(^\((a\|b)\)\1\.\*\+\?\\$)">::type,
        Expected
    >);
}

namespace repeated_backreference_alternatives
{
    using Expected = Sequence<
        StartAnchor,
        NonCaptureGroup<
            Union<
                CaptureGroup<1, Literal<'a'>>,
                Union<
                    CaptureGroup<2, Literal<'b'>>,
                    CaptureGroup<3, Literal<'c'>>
                >
            >
        >,
        RepeatOneOrMore<
            NonCaptureGroup<
                Union<
                    Backreference<1>,
                    Union<Backreference<2>, Backreference<3>>
                >
            >
        >,
        EndAnchor
    >;

    static_assert(std::is_same_v<
        Parse<R"(^(?:(a)|(b)|(c))(?:\1|\2|\3)+$)">::type,
        Expected
    >);
}