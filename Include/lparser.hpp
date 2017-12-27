//
// Created by meox on 15/01/17.
//

#ifndef PARSER_LPARSER_H_H
#define PARSER_LPARSER_H_H

#include <string>


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

        parser_t(T a, std::string r = "")
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
    parser_t<T> empty_fn(std::string) { return parser_t<T>{}; }

    template<typename Parser>
    decltype(auto) parse(Parser && p, std::string inp)
    {
        return p(inp);
    }

    template <typename T>
    using parser_fun_t = std::function<parser_t<T>(std::string)>;


    template<typename G, typename P>
    decltype(auto) fmap(G g, P p)
    {
        return [=](std::string inp) {
            const auto a = parse(p, inp);
            using B_t = decltype(g(a.get()));
            if (a.is_empty())
                return empty<B_t>();
            else
                return parser_t<B_t>(
                        g(a.get()),
                        a.remain
                );
        };
    }


    template<typename T>
    decltype(auto) pure(T v)
    {
        return [=](std::string inp) {
            return parser_t<T>(
                    std::move(v),
                    inp
            );
        };
    }


    template<typename Pg, typename Px>
    decltype(auto) combine(Pg pg, Px px)
    {
        return [=](std::string inp) {
            const auto r = parse(pg, inp);
            using B_t = decltype(parse(fmap(r.get(), px), r.remain).get());

            if (r.is_empty())
                return empty<B_t>();
            else
                return parse(fmap(r.get(), px), r.remain);
        };
    }


    template<typename Pg, typename Px, typename A, typename ...Args>
    decltype(auto) combine(Pg pg, Px px, A a, Args ...args)
    {
        return combine(combine(pg, px), a, std::forward<Args>(args)...);
    }


    template<typename P, typename Q>
    decltype(auto) pipe(P p, Q q)
    {
        return [=](std::string inp) {
            const auto a = parse(p, inp);
            if (a.is_empty())
                return parse(q, inp);
            else
                return a;
        };
    };


    /* PARSERs */

    parser_t<char> item(std::string inp)
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
    decltype(auto) sat(F f)
    {
        return [=](std::string inp) {
            const auto x = item(inp);
            if (!x.is_empty() && f(x.get()))
                return x;
            else
                return empty<decltype(x.get())>();
        };
    }

    decltype(auto) char_eq(char x)
    {
        return sat([x](char ch) { return x == ch; });
    }

    decltype(auto) digit(std::string inp)
    {
        return parse(sat([](char c){ return isdigit(c); }), inp);
    }

    decltype(auto) lower(std::string inp)
    {
        return parse(sat([](char c){ return islower(c); }), inp);
    }

    decltype(auto) upper(std::string inp)
    {
        return parse(sat([](char c){ return isupper(c); }), inp);
    }

    decltype(auto) letter(std::string inp)
    {
        return parse(sat([](char c){ return isalpha(c); }), inp);
    }

    decltype(auto) alphanum(std::string inp)
    {
        return parse(sat([](char c){ return isalnum(c); }), inp);
    }

/*
    // utils for seq
    template<typename T, typename Q>
    std::string plus(T a, Q b)
    {
        std::stringstream s;
        s << a << b;
        return s.str();
    }
*/

    template<typename P, typename Q>
    decltype(auto) seq(P p, Q q)
    {
        return [=](std::string inp) {
            const auto a = parse(p, inp);

            if (a.is_empty())
                return empty<std::string>();
            else
            {
                const auto b = parse(q, a.remain);
                if (b.is_empty())
                    return empty<std::string>();
                else
                    return parser_t<std::string>{plus(a.get(), b.get()), b.remain};
            }
        };
    }


    template<typename P, typename Q, typename ...Args>
    decltype(auto) seq(P p, Q q, Args ...args)
    {
        return seq(p, seq(q, std::forward<Args>(args)...));
    }


    decltype(auto) string_eq(std::string x)
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
    decltype(auto) many(P p)
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
                inp = a.remain;
            }

            return parser_t<std::vector<val_t>>{acc, inp};
        };
    }


    template<typename P>
    decltype(auto) some(P p)
    {
        return seq(p, many(p));
    }


    parser_t<std::string> ident(std::string inp)
    {
        return parse(seq(lower, many(alphanum)), inp);
    }


    decltype(auto) space(std::string inp)
    {
        return parse(many(sat([](char c) { return isspace(c); })), inp);
    }


    template<typename P>
    decltype(auto) token(P p)
    {
        return [=](std::string inp) {
            using p_t = decltype(parse(p, inp));
            using value_t = typename p_t::value_t;

            value_t value;
            const auto r = parse(
                    seq(
                            space,
                            fmap([&value](auto v) {
                                value = v;
                                return v;
                            }, p),
                            space
                    ), inp
            );

            if (r.is_empty())
                return empty<value_t>();
            else
                return parser_t<value_t>{value, r.remain};
        };
    }

    decltype(auto) nat(std::string inp)
    {
        const auto r = parse(some(digit), inp);
        if (r.is_empty())
            return empty<long>();

        return parser_t<long>{std::stol(r.get()), r.remain};
    }

    decltype(auto) intg(std::string inp)
    {
        return parse(pipe(
                nat,
                fmap(
                        [](std::string x) { return std::stol(x); },
                        seq(char_eq('-'), nat)
                )
        ), inp);
    }

    decltype(auto) integer(std::string inp)
    {
        return parse(token(intg), inp);
    }

    decltype(auto) natural(std::string inp)
    {
        return parse(token(nat), inp);
    }

    decltype(auto) string(std::string inp)
    {
        return parse(token(seq(letter, many(alphanum))), inp);
    }

    decltype(auto) symbol(std::string x)
    {
        return [=](std::string inp) {
            return parse(token(string_eq(x)), inp);
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
    return out;
}

template <typename T, typename R>
std::ostream& operator<<(std::ostream& out, const std::pair<T, R>& p)
{
    out << "(" << p.first << ", " << p.second << ")";
    return out;
}


#endif //PARSER_LPARSER_H_H
