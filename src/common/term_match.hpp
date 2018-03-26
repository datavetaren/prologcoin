#pragma once

#ifndef _common_term_match_hpp
#define _common_term_match_hpp

#include "term.hpp"
#include "term_env.hpp"

namespace prologcoin { namespace common {

template<size_t N, size_t K, typename Arg, typename...Args> struct pattern_args {
    pattern_args(term_env &env, Arg arg, Args... args)
	: env_(env), arg_(arg), args_(env, args...) { }

    bool operator () (const term t) const
    {
	return arg_(env_.arg(t, K)) && args_(t);
    }

    term_env &env_;
    Arg arg_;
    pattern_args<N-1, K+1, Args...> args_;
};

template<size_t K, typename Arg> struct pattern_args<1,K,Arg> {
    pattern_args(term_env &env, Arg arg) : env_(env), arg_(arg)
    { }

    bool operator () (const term t) const
    {
	return arg_(env_.arg(t, K));
    }

    term_env &env_;
    Arg arg_;
};

struct pattern_con {
    pattern_con(con_cell c) : c_(c) { }

    bool operator () (const term t) const {
	return t.tag() == tag_t::CON
	       && reinterpret_cast<const con_cell &>(t) == c_;
    }

    con_cell c_;
};

template<typename...Args> struct pattern_str {
    pattern_str(term_env &env, con_cell f, Args... args)
	: env_(env), f_(f), args_(env, args...) { }

    bool operator () (const term t) const
    { return t.tag() == tag_t::STR && env_.functor(t) == f_ && args_(t); }

    term_env &env_;
    con_cell f_;
    pattern_args<sizeof...(Args), 0, Args...> args_;
};

struct pattern {
    pattern(term_env &env) : env_(env) { }

    pattern_con con(con_cell c)
    {
	return pattern_con(c);
    }

    pattern_con con(const std::string &name, size_t arity)
    {
	return pattern_con(env_.functor(name,arity));
    }

    template<typename...Args> pattern_str<Args...> str(con_cell f,
						       Args... args)
    {
	return pattern_str<Args...>(env_, f, args...);
    }

    term_env &env_;
};


}}

#endif
