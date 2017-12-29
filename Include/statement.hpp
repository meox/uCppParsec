//
// Created by meox on 28/12/17.
//

#include "lparser.hpp"
#include <boost/variant.hpp>


namespace kpml
{
    using namespace lparser;

    struct symbol_t { std::string name; };

    struct Render : boost::static_visitor<void>
    {
        Render(std::ostream& out) : _out(out) {}

        template <typename T>
        void operator()(T && v) const
        {
            _out << std::forward<T>(v);
        }

        std::ostream& _out;
    };

    struct statement_t
    {
        statement_t() = default;
        statement_t(const statement_t& rhs)
        {
            op = rhs.op;
            operands = rhs.operands;
            raw_data = rhs.raw_data;
            leaf = rhs.leaf;
        }

        std::string op{"?"};
        std::vector<statement_t> operands;

        template <typename T>
        void set_raw(T d)
        {
            leaf = true;
            raw_data = d;
        }

        void show_leaf(std::ostream& out) const
        {
            boost::apply_visitor(Render(out), raw_data);
        }

        bool is_leaf() const { return leaf; }
    private:
        boost::variant<uint64_t, std::string> raw_data;
        bool leaf{false};
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
                            a.set_raw(s);
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
        statement_t v;

        const auto r = parse(
                pipe(
                        parser_bind(
                                seq(
                                        space,
                                        parser_bind(factor, [&](statement_t s) {
                                            v.operands.push_back(s);
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(pipe(char_eq('*'), char_eq('/')), [&](char c) {
                                            v.op = c;
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(term, [&](statement_t s) {
                                            v.operands.push_back(s);
                                            return pure(1);
                                        }),
                                        space
                                ), [](auto x) { return pure(1); }),
                        parser_bind(factor, [&](statement_t s) {
                            v = s;
                            return pure(1);
                        })
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>();
        else
            return parser_t<statement_t>(v, r.remain);
    }


    parser_t<statement_t> expr(std::string inp)
    {
        statement_t v;

        const auto r = parse(
                pipe(
                        parser_bind(
                                seq(
                                        space,
                                        parser_bind(term, [&](statement_t s) {
                                            v.operands.push_back(s);
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(pipe(char_eq('+'), char_eq('-')), [&](char c) {
                                            v.op = c;
                                            return pure(1);
                                        }),
                                        space,
                                        parser_bind(expr, [&](statement_t s) {
                                            v.operands.push_back(s);
                                            return pure(1);
                                        }),
                                        space
                                ), [](auto) { return pure(1); }),
                        parser_bind(term, [&](statement_t s) {
                            v = s;
                            return pure(1);
                        })
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>();
        else
            return parser_t<statement_t>(v, r.remain);
    }
}