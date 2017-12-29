//
// Created by meox on 26/12/17.
//

#ifndef PARSER_KPML_H
#define PARSER_KPML_H

#include "lparser.hpp"
#include "lparser_bricks.hpp"


// (fn(x) { x + 1 })(1)

namespace kpml
{
    using namespace lparser;

    struct lambda_t
    {
        std::vector<std::string> params;
        std::vector<statement_t> statements;
    };


    inline decltype(auto) statement(std::string inp)
    {
        auto a = parse(pipe(
                basic_expr,
                if_expr,
                assign
        ), inp);
    }

    inline decltype(auto) general_statement(std::string inp)
    {
        return parse(
                parser_bind(
                        seq(space, statement, space, symbol(";"), space),
                        [](auto e) { return std::get<1>(e); }
                ),
                inp
        );
    }

    inline decltype(auto) seq_statements(std::string inp)
    {
        return parse(many(general_statement), inp);
    }

    inline decltype(auto) lambda(std::string inp)
    {
        return parse(
                seq(
                        string_eq("fn"),
                        space,
                        char_eq('('),
                        space,
                        params(ident),
                        space,
                        char_eq(')'),
                        space,
                        seq_statements
                ),
                inp);
    }

}



#endif //PARSER_KPML_H
