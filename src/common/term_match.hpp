#pragma once

#ifndef _common_term_match_hpp
#define _common_term_match_hpp

#include "term.hpp"
#include "term_env.hpp"

namespace prologcoin { namespace common {

template<size_t N, size_t K, typename Arg, typename...Args> struct pattern_args {
    pattern_args(Arg arg, Args... args)
	: arg_(arg), args_(args...) { }

    bool operator () (term_env &env, const term t) const
    {
	return arg_(env, env.arg(t, K)) && args_(env,t);
    }

    Arg arg_;
    pattern_args<N-1, K+1, Args...> args_;
};

template<size_t K, typename Arg> struct pattern_args<1,K,Arg> {
    pattern_args(Arg arg) : arg_(arg)
    { }

    bool operator () (term_env &env, const term t) const
    {
	return arg_(env, env.arg(t, K));
    }
    Arg arg_;
};

struct pattern_con {
    pattern_con(con_cell c) : c_(c) { }

    bool operator () (term_env &, const term t) const {
	return t.tag() == tag_t::CON
	       && reinterpret_cast<const con_cell &>(t) == c_;
    }
    con_cell c_;
};

template<typename...Args> struct pattern_str {
    pattern_str(con_cell f, Args... args)
	: f_(f), args_(args...) { }

    bool operator () (term_env &env, const term t) const
    { return t.tag() == tag_t::STR && env.functor(t) == f_ && args_(env,t); }

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
	return pattern_str<Args...>(f, args...);
    }

    term_env &env_;
};


}}

#endif
