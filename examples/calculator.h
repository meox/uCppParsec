//
// Created by meox on 28/12/17.
//

#ifndef PARSER_CALCULATOR_H
#define PARSER_CALCULATOR_H

#include "../Include/lparser.hpp"
#include "../Include/lparser_bricks.hpp"


lparser::parser_t<long> expr(std::string inp);
lparser::parser_t<long> factor(std::string inp);
lparser::parser_t<long> term(std::string inp);
lparser::parser_t<long> expr(std::string inp);


#endif //PARSER_CALCULATOR_H
