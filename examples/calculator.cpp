//
// Created by meox on 28/12/17.
//

#include "calculator.h"

using namespace lparser;

parser_t<long> factor(std::string inp)
{
    long a{};

    const auto r = parse(
            pipe(
                    parser_bind(
                            seq(
                                    space,
                                    char_eq('('),
                                    space,
                                    parser_bind(expr, [&](long s){a = s; return pure(1);}),
                                    space,
                                    char_eq(')'),
                                    space
                            ), [](auto) { return pure(1);}
                    ),
                    parser_bind(nat, [&](long s){a = s; return pure(1);})
            ),
            inp
    );

    if (r.is_empty())
        return empty<long>();
    else
        return parser_t<long>(a, r.remain);
}

parser_t<long> term(std::string inp)
{
    char op;
    long a{}, b{1};

    const auto r = parse(
            pipe(
                    parser_bind(
                            seq(
                                    space,
                                    parser_bind(factor, [&](long s){a = s; return pure(1);}),
                                    space,
                                    parser_bind(pipe(char_eq('*'), char_eq('/')), [&](char c){op = c; return pure(1);}),
                                    space,
                                    parser_bind(term, [&](long s){b = s; return pure(1);}),
                                    space
                            ),[](auto x) { return pure(1);}),
                    parser_bind(factor, [&](long s){a = s; return pure(1);})
            ),
            inp
    );

    if (r.is_empty())
        return empty<long>();
    else
        return parser_t<long>((op == '*' ? a * b : a / b), r.remain);
}

parser_t<long> expr(std::string inp)
{
    char op;
    long a{}, b{};

    const auto r = parse(
            pipe(
                    parser_bind(
                            seq(
                                    space,
                                    parser_bind(term, [&](long s){a = s; return pure(1);}),
                                    space,
                                    parser_bind(pipe(char_eq('+'), char_eq('-')), [&](char c){op = c; return pure(1);}),
                                    space,
                                    parser_bind(expr, [&](long s){b = s; return pure(1);}),
                                    space
                            ), [](auto) { return pure(1); }),
                    parser_bind(term, [&](long s_a){a = s_a; return pure(1);})
            ),
            inp
    );

    if (r.is_empty())
        return empty<long>();
    else
        return parser_t<long>((op == '+' ? a + b : a - b), r.remain);
}
