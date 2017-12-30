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

        void operator()(const uint64_t& v) const
        {
            _out << v;
        }

        void operator()(const std::string& v) const
        {
            _out << "\"" << v << "\"";
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

        statement_t(const std::string& e)
        {
            set_raw(e);
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
    parser_t<statement_t> function_call(std::string inp);


    inline parser_t<statement_t> factor(std::string inp)
    {
        statement_t a{};
        const auto r = parse(
                seq(
                    space,
                    pipe(
                        parser_bind(
                                seq(
                                    char_eq('('),
                                    space,
                                    parser_bind(expr, [&](statement_t s) {
                                        a = s;
                                        return pure(1);
                                    }),
                                    space,
                                    char_eq(')')
                                ), [](auto) { return pure(1); }
                        ),
                        parser_bind(function_call, [&](statement_t s) {
                            a = s;
                            return pure(1);
                        }),
                        parser_bind(nat, [&](long s) {
                            a.set_raw(s);
                            return pure(1);
                        }),
                        parser_bind(ident, [&](std::string s) {
                            a.set_raw(s);
                            return pure(1);
                        })
                    ),
                    space
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(a, r.remain);
    }


    inline parser_t<statement_t> term(std::string inp)
    {
        statement_t v;

        const auto r = parse(
                seq(
                    space,
                    pipe(
                        parser_bind(
                            seq(
                                parser_bind(factor, [&](statement_t s) {
                                    v.operands.push_back(s);
                                    return pure(1);
                                }),
                                space,
                                parser_bind(pipe(
                                        string_eq("*"), string_eq("/"),
                                        string_eq(">"), string_eq("<"),
                                        string_eq(">="), string_eq("<="),
                                        string_eq("==")
                                ), [&](const std::string& op) {
                                    v.op = op;
                                    return pure(1);
                                }),
                                space,
                                parser_bind(term, [&](statement_t s) {
                                    v.operands.push_back(s);
                                    return pure(1);
                                })
                            ), [](auto x) { return pure(1); }),
                        parser_bind(factor, [&](statement_t s) {
                            v = s;
                            return pure(1);
                        })
                    ),
                    space
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(v, r.remain);
    }


    inline parser_t<statement_t> expr(std::string inp)
    {
        statement_t v;

        const auto r = parse(
                seq(
                    space,
                    pipe(
                        parser_bind(
                            seq(
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
                                })
                            ), [](auto) { return pure(1); }),
                        parser_bind(term, [&](statement_t s) {
                            v = s;
                            return pure(1);
                        })
                    ),
                    space
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(v, r.remain);
    }


    inline parser_t<statement_t> function_call(std::string inp)
    {
        statement_t stm;

        auto r = parse(
                seq(
                    space,
                    parser_bind(ident, [&stm](std::string function_name){
                        stm.op = "apply";
                        stm.operands.push_back(function_name);
                        return pure(1);
                    }),
                    space,
                    char_eq('('),
                    space,
                    parser_bind(params(expr), [&](const std::vector<statement_t> xs) {
                        for (const auto& x : xs)
                            stm.operands.push_back(x);
                        return pure(1);
                    }),
                    space,
                    char_eq(')'),
                    space
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(stm, r.remain);
    }

    inline parser_t<statement_t> statement(std::string inp);

    inline parser_t<statement_t> if_else(std::string inp)
    {
        statement_t stm;
        stm.op = "if";

        auto r = parse(
                seq(
                    symbol("if"),
                    symbol("("),
                    parser_bind(expr, [&](const statement_t& s){
                        stm.operands.push_back(s);
                        return pure(1);
                    }),
                    symbol(")"),
                    symbol("{"),
                    parser_bind(statement, [&](const statement_t& s){
                        stm.operands.push_back(s);
                        return pure(1);
                    }),
                    symbol("}"),
                    symbol("else"),
                    symbol("{"),
                    parser_bind(statement, [&](const statement_t& s) {
                        stm.operands.push_back(s);
                        return pure(1);
                    }),
                    symbol("}")
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(stm, r.remain);
    }


    inline parser_t<statement_t> statement(std::string inp)
    {
        auto r = parse(
                seq(space, pipe(if_else, expr), space),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(std::get<1>(r.get()), r.remain);
    }


    inline parser_t<statement_t> function_body(std::string inp)
    {
        statement_t stm;

        auto r = parse(
                seq(
                    parser_bind(statement, [&](statement_t s){
                        stm.op = "begin";
                        stm.operands.push_back(s);
                        return pure(1);
                    }),
                    parser_bind(many(seq(symbol(";"), statement)), [&](const auto& xs) {
                        for(const auto& x: xs)
                            stm.operands.push_back(std::get<1>(x));
                        return pure(1);
                    })
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(stm, r.remain);
    }


    inline parser_t<statement_t> function_def(std::string inp)
    {
        statement_t stm;

        auto r = parse(
                seq(
                    symbol("def"),
                    parser_bind(ident, [&stm](std::string function_name){
                        stm.op = "def";
                        stm.operands.push_back(function_name);
                        return pure(1);
                    }),
                    symbol("("),
                    parser_bind(params(ident), [&](const std::vector<std::string>& xs) {
                        statement_t parameters;
                        parameters.op = "parameters";
                        for (const auto& x : xs)
                            parameters.operands.push_back(x);
                        stm.operands.push_back(parameters);
                        return pure(1);
                    }),
                    symbol(")"),
                    symbol("{"),
                    parser_bind(function_body, [&](const statement_t& s){
                        stm.operands.push_back(s);
                        return pure(1);
                    }),
                    symbol("}")
                ),
                inp
        );

        if (r.is_empty())
            return empty<statement_t>(r.remain);
        else
            return parser_t<statement_t>(stm, r.remain);
    }
}
