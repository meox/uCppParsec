//
// Created by meox on 28/12/17.
//

#include "lparser.hpp"
#include <boost/variant.hpp>


namespace kpml
{
    using namespace lparser;

    struct symbol_t { std::string name; };

    struct statement_t
    {
        std::string op;
        std::vector<statement_t> operands;
        boost::variant<uint64_t, std::string> raw_data;
        bool is_leaf{false};
    };

    parser_t<statement_t> expr(std::string inp);

    parser_t<statement_t> factor(std::string inp)
    {
        statement_t a{};
        const auto r = parse(
                pipe(
                        parser_bind(
                                seq(
                                        space,
                                        char_eq('('),
                                        space,
                                        parser_bind(expr, [&](statement_t s) {
                                            a = s;
                                            return pure(1);
                                        }),
                                        space,
                                        char_eq(')'),
                                        space
                                ), [](auto) { return pure(1); }
                        ),
                        parser_bind(nat, [&](long s) {
                            a = s;
                            return pure(1);
                        })
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>();
        else
            return parser_t<statement_t>(a, r.remain);
    }


    parser_t<statement_t> term(std::string inp)
    {
        char op;
        long a{}, b{};

        const auto r = parse(
                pipe(
                        parser_bind(
                                seq(
                                        space,
                                        parser_bind(factor, [&](long s) {
                                            a = s;
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(pipe(char_eq('*'), char_eq('/')), [&](char c) {
                                            op = c;
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(term, [&](long s) {
                                            b = s;
                                            return pure(1);
                                        }),
                                        space
                                ), [](auto x) { return pure(1); }),
                        parser_bind(factor, [&](long s) {
                            a = s;
                            return pure(1);
                        })
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>();
        else
            return parser_t<statement_t>((op == '*' ? a * b : a / b), r.remain);
    }


    parser_t<statement_t> expr(std::string inp)
    {
        std::string op;
        statement_t a{}, b{};

        const auto r = parse(
                pipe(
                        parser_bind(
                                seq(
                                        space,
                                        parser_bind(term, [&](long s) {
                                            a = s;
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(pipe(char_eq('+'), char_eq('-')), [&](char c) {
                                            op = c;
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(expr, [&](long s) {
                                            b = s;
                                            return pure(1);
                                        }),
                                        space
                                ), [](auto) { return pure(1); }),
                        parser_bind(term, [&](long s_a) {
                            a = s_a;
                            return pure(1);
                        })
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>();
        else
            return parser_t<statement_t>((op == '+' ? a + b : a - b), r.remain);
    }
}