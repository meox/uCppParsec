//
// Created by meox on 06/08/17.
//

#ifndef PARSER_LPARSER_BRICKS_HPP_H
#define PARSER_LPARSER_BRICKS_HPP_H

#include "lparser.hpp"


namespace lparser
{
    template <typename T>
    decltype(auto) lists(T parser)
    {
        using RT = typename decltype(parse(parser, ""))::value_t;

        return [=](std::string inp){
            std::vector<RT> vs;
            auto extract = [&vs](RT x) {
                vs.push_back(x);
                return pure(x);
            };

            const auto r = parse(
                    seq(
                            symbol("["),
                            parser_bind(parser, extract),
                            many(seq(symbol(","), parser_bind(parser, extract))),
                            symbol("]")
                    ),
                    inp
            );

            if (r.is_empty())
                return empty<std::vector<RT>>();
            else
                return parser_t<std::vector<RT>>{vs, r.remain};
        };
    }

    decltype(auto) nats(std::string inp)
    {
        return parse(lists(natural), inp);
    }


    decltype(auto) strings(std::string inp)
    {
        return parse(lists(alphanum), inp);
    }


    decltype(auto) assign(std::string inp)
    {
        std::string name, value;

        const auto r = parse(seq(
                parser_bind(ident, [&](std::string x){name = x; return pure(x);}),
                space,
                char_eq('='),
                space,
                parser_bind(ident, [&](std::string x){value = x; return pure(x);})
        ), inp);

        if (r.is_empty())
            return empty<std::pair<std::string, std::string>>();
        else
        {
            return parser_t<std::pair<std::string, std::string>>{
                    {name, value}, r.remain
            };
        }
    }
}

#endif //PARSER_LPARSER_BRICKS_HPP_H
