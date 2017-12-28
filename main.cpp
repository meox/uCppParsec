/*
 * C++14 Parser Combinator
 * Gian Lorenzo Meocci (glmeocci@gmail.com)
 *
 * inspired by Haskell Parsec library
 * */


#include <iostream>
#include <vector>
#include <utility>
#include <memory>
#include <sstream>
#include "Include/lparser.hpp"
#include "Include/lparser_bricks.hpp"
#include "Include/statement.hpp"


using namespace lparser;


int main()
{
    std::cout << parse(many(space), "xyz") << std::endl;

    std::cout << parse(item, "") << std::endl;
    std::cout << parse(item, "3+5") << std::endl;
    std::cout << parse(seq(item, item, item), "ABCDE") << std::endl;
    std::cout << parse(item, "abc") << std::endl;

    std::cout << parse(pure(1), "abc") << std::endl;

    std::cout << parse(pipe(empty_fn<char>, item), "abc") << std::endl;

    std::cout << "isDigit " << parse(digit, "123") << std::endl;
    std::cout << "isLower " << parse(lower, "home") << std::endl;
    std::cout << "isUpper " << parse(upper, "Home") << std::endl;
    std::cout << "chareq " << parse(char_eq('I'), "Inghe") << std::endl;

    std::cout << "seq " << parse(seq(char_eq('I'), char_eq('n'), char_eq('g')), "Inghe") << std::endl;
    std::cout << "string_eq " << parse(string_eq("Ing"), "Inghe") << std::endl;

    std::cout << "many(space) " << parse(space, " 123abc") << std::endl;
    std::cout << "many(digit) " << parse(many(digit), "123abc") << std::endl;
    std::cout << "some(digit) " << parse(some(digit), "123abc") << std::endl;
    std::cout << "some(digit) " << parse(some(digit), "x23abc") << std::endl;
    std::cout << "some(letter) " << parse(some(letter), "abc123abc") << std::endl;

    std::cout << "ident " << parse(ident, "abc123 abc") << std::endl;
    std::cout << "nat " << parse(nat, "1789abc") << std::endl;
    std::cout << "space " << parse(space, "    abc") << std::endl;

    std::cout << "intg " << parse(intg, "1235") << std::endl;
    std::cout << "intg " << parse(intg, "-1235   xxx") << std::endl;
    std::cout << "integer " << parse(integer, "  -107     95") << std::endl;

    std::cout << "symbol " << parse(symbol("A"), " [ x ] ") << std::endl;
    std::cout << "symbol " << parse(symbol("x"), " x = 123 ") << std::endl;

    std::cout << "assign " << parse(assign, "abc = x123") << std::endl;

    std::cout << "nats " << parse(nats, "[5, 8, 1982, 3 ]") << std::endl;

    const std::string expr_s = "((5  * 7) + (31 - 9) + 21);finish!";
    const auto b_expr = parse(expr, expr_s);
    if (b_expr.is_empty())
    {
        std::cout << "invalid expr: " << expr_s << std::endl;
    }
    else
    {
        std::cout << "expr: <" << expr_s << "> = "
                  << *b_expr.first
                  << ", not parsed: " << b_expr.remain
                  << ", correct value = " << ((5  * 7) + (31 - 9) + 21)
                  << std::endl;
    }

    return 0;
}
