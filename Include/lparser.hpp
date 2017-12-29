//
// Created by meox on 15/01/17.
// Inspired to: http://www.cs.nott.ac.uk/~pszgmh/monparsing.pdf
//

#ifndef PARSER_LPARSER_H_H
#define PARSER_LPARSER_H_H

#include "omega.hpp"
#include <string>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <memory>
#include <vector>



namespace lparser
{

    template<typename T>
    struct parser_t
    {
        using value_t = T;

        parser_t()
        {
            first = nullptr;
            empty = true;
        }

        explicit parser_t(T a, std::string r = "")
        : remain(r)
        {
            first = std::make_unique<T>(std::move(a));
            empty = false;
        }

        parser_t(const parser_t &rhs)
        {
            empty = rhs.empty;
            if (!empty)
            {
                remain = rhs.remain;
                first = std::make_unique<T>(rhs.get());
            }
            else
            {
                remain = "";
            }
        }

        bool is_empty() const { return empty; }
        T get() const { return *first; }

        std::unique_ptr<T> first;
        std::string remain;

    private:
        bool empty;
    };

    template<typename T>
    parser_t<T> empty() { return parser_t<T>{}; }

    template<typename T>
    parser_t<T> empty(const std::string& remain)
    {
        auto p = parser_t<T>{};
        p.remain = remain;
        return p;
    }

    template<typename T>
    parser_t<T> empty_fn(std::string) { return parser_t<T>{}; }

    template<typename Parser>
    decltype(auto) parse(Parser && p, std::string inp)
    {
        return p(inp);
    }

    template <typename T>
    using parser_fun_t = std::function<parser_t<T>(std::string)>;


    template<typename T>
    inline decltype(auto) pure(T v)
    {
        return [=](std::string inp) {
            return parser_t<T>(v, inp);
        };
    }


    template<typename P, typename F>
    inline decltype(auto) parser_bind(P p, F f)
    {
        return [=](std::string inp) {
            auto a = parse(p, inp);
            using R_T = typename decltype(parse(f(a.get()), a.remain))::value_t;

            if (a.is_empty())
                return empty<R_T>(a.remain);
            return parse(f(a.get()), a.remain);
        };
    }


    template<typename P, typename Q>
    inline decltype(auto) seq(P p, Q q)
    {
        return parser_bind(p, [q](auto x) {
            return parser_bind(q, [=](auto y) {
                return pure(
                        std::tuple_cat(
                                omega::as_tuple(x),
                                omega::as_tuple(y)
                        )
                );
            });
        });
    }


    template<typename P, typename Q, typename ...Args>
    inline decltype(auto) seq(P p, Q q, Args ...args)
    {
        return seq(p, seq(q, args...));
    }


    template<typename P, typename Q>
    inline decltype(auto) pipe(P p, Q q)
    {
        return [=](std::string inp) {
            const auto a = parse(p, inp);
            if (a.is_empty())
                return parse(q, inp);
            else
                return a;
        };
    };

    template<typename P, typename Q, typename ...Args>
    inline decltype(auto) pipe(P p, Q q, Args ...args)
    {
        return pipe(p, pipe(q, args...));
    }

    /* PARSERs */

    inline parser_t<char> item(std::string inp)
    {
        if (inp.empty())
            return empty<char>();
        else
        {
            return {
                    parser_t<char>{
                            inp[0],
                            inp.substr(1)
                    }
            };
        }
    }


    template<typename F>
    inline decltype(auto) sat(F f)
    {
        return [=](std::string inp) {
            const auto x = item(inp);
            if (!x.is_empty() && f(x.get()))
                return x;
            else
                return empty<decltype(x.get())>(x.remain);
        };
    }

    inline decltype(auto) char_eq(char x)
    {
        return sat([x](char ch) { return x == ch; });
    }

    inline decltype(auto) digit(std::string inp)
    {
        return parse(sat([](char c){ return isdigit(c); }), inp);
    }

    inline decltype(auto) lower(std::string inp)
    {
        return parse(sat([](char c){ return islower(c); }), inp);
    }

    inline decltype(auto) upper(std::string inp)
    {
        return parse(sat([](char c){ return isupper(c); }), inp);
    }

    inline decltype(auto) letter(std::string inp)
    {
        return parse(sat([](char c){ return isalpha(c); }), inp);
    }

    inline decltype(auto) alphanum(std::string inp)
    {
        return parse(sat([](char c){ return isalnum(c); }), inp);
    }

    inline decltype(auto) string_eq(std::string x)
    {
        return [=](std::string inp) {
            const auto pos = inp.find(x);
            if (pos == std::string::npos)
                return empty<std::string>();

            const auto l = x.size();
            return parser_t<std::string>{
                    inp.substr(0, pos + l),
                    inp.substr(pos + l)
            };
        };
    }


    template<typename P>
    inline decltype(auto) many(P p)
    {
        return [=](std::string inp) {
            using val_t = typename decltype(parse(p, ""))::value_t;
            std::vector<val_t> acc;

            while (!inp.empty())
            {
                auto a = parse(p, inp);
                if (a.is_empty())
                    break;

                acc.push_back(a.get());
                if (inp == a.remain)
                    break;
                else
                    inp = a.remain;
            }

            return parser_t<std::vector<val_t>>{acc, inp};
        };
    }


    template<typename P>
    inline decltype(auto) some(P p)
    {
        return [=](std::string inp) {
            auto a = parse(seq(p, many(p)), inp);
            using VT = std::remove_reference_t<decltype(std::get<1>(a.get()))>;

            if (a.is_empty())
                return empty<VT>(a.remain);
            else
            {
                VT v;
                v.push_back(std::get<0>(a.get()));
                auto rest = std::get<1>(a.get());
                for (auto e : rest)
                    v.push_back(e);
                return parser_t<VT>(v, a.remain);
            }
        };
    }


    inline parser_t<std::string> ident(std::string inp)
    {
        return parse(
                parser_bind(
                        seq(lower, many(pipe(alphanum, sat([](char c) { return c == '_'; })))),
                        [](auto x) {
                            const auto& v_rest = std::get<1>(x);
                            std::string rest = std::accumulate(std::begin(v_rest), std::end(v_rest), std::string{}, [](auto c, auto acc) { return c + acc; });
                            return pure(std::string{std::get<0>(x)} + rest);
                        }
                ),
                inp
        );
    }


    inline decltype(auto) space(std::string inp)
    {
        return parse(
                parser_bind(
                        many(sat([](char c) { return isspace(c); })),
                        [](auto x) {
                            return pure(std::string(x.size(), ' '));
                        }
                ),
                inp
        );
    }

    inline decltype(auto) space1(std::string inp)
    {
        return parse(
                seq(
                    sat([](char c) { return isspace(c); }),
                    space
                ),
                inp
        );
    }


    template<typename P>
    inline decltype(auto) token(P p)
    {
        return [=](std::string inp) {
            using p_t = decltype(parse(p, inp));
            using value_t = typename p_t::value_t;

            value_t value;
            const auto r = parse(
                    seq(
                            space,
                            parser_bind(p, [&value](auto v) {
                                value = v;
                                return pure(v);
                            }),
                            space
                    ), inp
            );

            if (r.is_empty())
                return empty<value_t>(r.remain);
            else
                return parser_t<value_t>{value, r.remain};
        };
    }


    inline parser_t<long> nat(std::string inp)
    {
        const auto r = parse(some(digit), inp);
        if (r.is_empty())
            return empty<long>(r.remain);

        auto v = r.get();
        long n{}, s = v.size() - 1;
        for(unsigned char x : v)
        {
            const auto d = (unsigned int)(x - '0');
            n += std::pow(10, s--) * d;
        }

        return parser_t<long>{n, r.remain};
    }


    inline decltype(auto) intg(std::string inp)
    {
        return parse(pipe(
                nat,
                parser_bind(
                        seq(char_eq('-'), nat),
                        [](auto x) { return pure(-std::get<1>(x)); }
                )
        ), inp);
    }

    inline decltype(auto) integer(std::string inp)
    {
        return parse(token(intg), inp);
    }

    inline decltype(auto) natural(std::string inp)
    {
        return parse(token(nat), inp);
    }

    inline decltype(auto) string(std::string inp)
    {
        return parse(token(seq(letter, many(alphanum))), inp);
    }

    inline decltype(auto) symbol(std::string x)
    {
        return [=](std::string inp) {
            return parse(seq(space, string_eq(x), space), inp);
        };
    }
}


/* UTILS */

template <typename T>
std::ostream& operator<<(std::ostream& out, const lparser::parser_t<T>& p)
{
    if (p.is_empty())
        out << "()";
    else
        out << "(" << p.get() << " ; " << p.remain << ")";
    return out;
}


template <typename T>
std::ostream& operator<<(std::ostream& out, const lparser::parser_t<std::vector<T>>& v)
{
    if (v.is_empty())
    {
        out << "(< null > ; " << v.remain << ")";
    }
    else
    {
        out << "([";

        bool f{false};
        for (const auto& x : v.get())
        {
            if (f)
                out << ",";
            else
                f = true;
            out << x;
        }

        out << "] ; " << v.remain << ")";
    }
    return out;
}


template <typename T, typename R>
std::ostream& operator<<(std::ostream& out, const lparser::parser_t<std::pair<T, R>>& e)
{
    if (e.is_empty())
    {
        out << "(< null > ; " << e.remain << ")";
        return out;
    }
    else
    {
        out << "(<";
        out << e.get().first;
        out << ", ";
        out << e.get().second;
        out << "> ; " << e.remain << ")";
    }
    return out;
}


template <typename ...Args>
std::ostream& operator<<(std::ostream& out, const lparser::parser_t<std::tuple<Args...>>& e)
{
    if (e.is_empty())
    {
        out << "(< null > ; " << e.remain << ")";
        return out;
    }
    else
    {
        out << "(";
        omega::show(std::cout, e.get());
        out << " ; " << e.remain << ")";
    }
    return out;
}


template <typename T, typename R>
std::ostream& operator<<(std::ostream& out, const std::pair<T, R>& p)
{
    out << "(" << p.first << ", " << p.second << ")";
    return out;
}


#endif //PARSER_LPARSER_H_H
